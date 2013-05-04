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
* wp_logsys_conf.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
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
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include <fcntl.h>
#include <sys/wait.h>
#include "ws_log_conf.h"
#include "ws_dhcp_conf.h"

int ShowLogconfPage(struct list *lcontrol,struct list *lpublic);
int modify_systemip(struct list *lpublic,struct list *lcontrol);

int cgiMain()
{
	
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	if((access(XML_FPATH,0)==0 && access(CONF_FPATH,0)==0))
	{}
	else
	{
		restart_syslog();
	}
	ShowLogconfPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	return 0;
}



int ShowLogconfPage(struct list *lcontrol,struct list *lpublic)
{ 
	char *encry=(char *)malloc(BUF_LEN); 
	char *str; 
    char addn[N];
	struct filter_st f_head,*fq;
	memset(&f_head,0,sizeof(f_head));
	int fnum=0,lnum=0,cl=1,i = 0,ret=-1,limit=0;
	struct log_st logst,*lq; 
	memset(&logst,0,sizeof(logst)); 
	memset(encry,0,BUF_LEN);
	char *timeflag=(char *)malloc(128);
	memset(timeflag,0,128);
	char selectz[10];
	memset(selectz,0,10);
	cgiFormStringNoNewlines("ID", timeflag, 128);
	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	str=dcryption(encry);
	if(str==NULL)
	{
	ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	strcpy(addn,str);
  
 
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
    "<script language=javascript>"\
	"function mysubmit()"\
    "{"\
	    "var indexz=\"\";"\
	    "indexz=document.all.filterz.options.selectedIndex;"\
	    "document.all.ip_s1.value = indexz;"
    "}"\
    "</script>"\
  	"<body>");
 
    if(cgiFormSubmitClicked("version") == cgiFormSuccess)
    {
       modify_systemip(lpublic,lcontrol);
    } 
	if(strcmp(timeflag,"")!=0)
	{
		if_syslog_exist();
		int flagz=0;	
		find_second_xmlnode(XML_FPATH, NODE_LOG, NODE_MARKZ, timeflag, &flagz);
		del_syslog_server(XML_FPATH,flagz);
		save_syslog_file();		
		ret=restart_syslog();
		if(ret==0)
			ShowAlert(search(lpublic,"oper_succ"));
		else
			ShowAlert(search(lpublic,"oper_fail"));			
	}
	fprintf(cgiOut,"<form method=post onsubmit=\"return mysubmit()\">"\
		"<div align=center>"\
		"<table width=976 border=0 cellpadding=0 cellspacing=0>"); 
	fprintf(cgiOut,"<tr>"\
	    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lpublic,"log_info"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

	fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");
	fprintf(cgiOut,"<tr>");
	if(checkuser_group(addn)==0)  
	{
		fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=version style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	}

	fprintf(cgiOut,"<td width=62 align=left><a href=wp_log_info.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	fprintf(cgiOut,"</tr>"\
	"</table>");  

		
	fprintf(cgiOut,"</td>"\
	    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
	    "</tr>"\
	    "<tr>");
    fprintf(cgiOut,"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>");
                            
	fprintf(cgiOut,"<tr>");
	fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
		"<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>");
	fprintf(cgiOut,"<tr height=4 valign=bottom>"\
		"<td width=120>&nbsp;</td>"\
		"<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
		"</tr>");
	fprintf(cgiOut,"<tr>");  
	fprintf(cgiOut,"<td><table width=120 border=0 cellspacing=0 cellpadding=0>"); 
	fprintf(cgiOut,"<tr height=25>"\
		"<td id=tdleft>&nbsp;</td>"\
		"</tr>");

	fprintf(cgiOut,"<tr height=26>"\
		"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"log_addip"));	
	fprintf(cgiOut,"</tr>");
    read_log_xml(&logst, &lnum);
	limit=lnum+4;
	for(i=0;i<limit;i++)
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
	fprintf(cgiOut,"<tr style=padding-top:10px>");
	fprintf(cgiOut,"<td bgcolor=#FFFFFF width=140>ID:</td>");
	fprintf(cgiOut,"<td colspan=4><input type=text name=sysid value=\"\" maxLength=5></td></tr>\n");

	/////////////
	fprintf(cgiOut,"<tr style=padding-top:10px>\n");
	fprintf(cgiOut,"<td width=140>%s</td>\n",search(lpublic,"log_deservice"));
    fprintf(cgiOut,"<td colspan=4>"\
    "<select name=filterz style=\"width:90px\">\n");
	read_filter_xml(&f_head, &fnum);
	fprintf(cgiOut,"<option value=''>--%s--</option>\n",search(lcontrol,"select"));
    fq=f_head.next;
	while(fq!=NULL)
	{
	    if((strcmp(fq->viewz,"0")!=0)&&(strcmp(fq->infos,"")!=0))
	    {
			fprintf(cgiOut,"<option value='%s'>%s</option>\n",fq->valuez,fq->infos);
	    }
		fq=fq->next;
	}
	fprintf(cgiOut,"</select>\n"\
		"</td>\n");
	fprintf(cgiOut,"</tr>\n");
	if(fnum>0)
		Free_read_filter_xml(&f_head);
	////////////

	fprintf(cgiOut,"<tr style=padding-top:10px><td bgcolor=#FFFFFF width=140>");
	fprintf(cgiOut, "<label class=\"col1\" for=\"Package.DestinationAddress.Single\">%s</label></td><td colspan=4> \n", search(lpublic,"log_port") );
	fprintf(cgiOut, "<select style=width:140px  name=tcp>\n" );  
	fprintf(cgiOut,"<option value=udp>UDP</option>");
	fprintf(cgiOut,"<option value=tcp>TCP</option>");                     
	fprintf(cgiOut, "</select>\n" );					
	fprintf(cgiOut,"</td></tr>");   

	fprintf(cgiOut,"<tr style=padding-top:10px>");
	fprintf(cgiOut,"<td width=140>%s</td>",search(lpublic,"log_ip"));
	fprintf(cgiOut,"<td colspan=4 width=140>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	fprintf(cgiOut,"<input type=text	name=gate_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text	name=gate_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text	name=gate_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text	name=gate_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	fprintf(cgiOut,"</div></td>");					 
	fprintf(cgiOut,"</tr>");

	fprintf(cgiOut,"<tr style=padding-top:10px>");
	fprintf(cgiOut,"<td width=140>%s:</td>",search(lpublic,"log_desport"));
	fprintf(cgiOut,"<td colspan=4><input type=text name=portid style=width:140px value=514><font color=red>(1--65535 || 514)</font></td>");					 
	fprintf(cgiOut,"</tr>");

	/*分割线*/
	fprintf(cgiOut, "<tr><td colspan=54><hr width=100%% size=1 color=#fff align=center noshade />"\
		"</td></tr>" );
    fprintf(cgiOut,"<tr><td colspan=5>\n");
	fprintf(cgiOut,"<table border=0>\n");
	fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>\n");
	fprintf(cgiOut,"<th width=120 style=font-size:12px>%s</th>","Index");
	fprintf(cgiOut,"<th width=120 style=font-size:12px>%s</th>","Level");
	fprintf(cgiOut,"<th width=120 style=font-size:12px>%s</th>","IP");
	fprintf(cgiOut,"<th width=120 style=font-size:12px>%s</th>","Port");
	fprintf(cgiOut,"<th width=120 style=font-size:12px></th>");
	fprintf(cgiOut,"</tr>\n");
	
	char *gets=(char *)malloc(128);
	int flagz=0;
	lq=logst.next;
	i=0;
	while(lq!=NULL)
	{
		if(strcmp(lq->timeflag,"")!=0)
		{
			i++;
			fprintf(cgiOut,"<tr bgcolor=%s>\n",setclour(cl));
			fprintf(cgiOut,"<td>%s</td>",lq->indexz);
			find_second_xmlnode(XML_FPATH, NODE_FILTER, NODE_VALUE, lq->filterz, &flagz);
			memset(gets,0,128);
			get_second_xmlnode(XML_FPATH, NODE_FILTER, NODE_INFOS,  gets, flagz);
			fprintf(cgiOut,"<td>%s</td>",gets);
			fprintf(cgiOut,"<td>%s</td>",lq->sysipz);
			fprintf(cgiOut,"<td>%s</td>",lq->sysport);
			fprintf(cgiOut,"<td><a href=wp_logsys_conf.cgi?UN=%s&ID=%s target=mainFrame><font color=black>%s</font></a></td>",encry,lq->timeflag,search(lcontrol,"del"));
			fprintf(cgiOut,"</tr>\n");
		}
		cl=!cl;								
		lq=lq->next;
	}
	if(lnum>0)
		Free_read_log_xml(&logst);

	free(gets);
	fprintf(cgiOut,"</table>\n");
	fprintf(cgiOut,"</td></tr>\n");
	
	fprintf(cgiOut,"<tr><td colspan=5><input type=hidden name=ip_s1 value=%s></td></tr>",selectz);	
	
	fprintf(cgiOut,"</table>"); 
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
	free(timeflag);
	return 0;
}
//deal 
int modify_systemip(struct list *lpublic,struct list *lcontrol)
{
    if_syslog_exist();
	int ret=-1,port=0,ftime=-1,fid=0;
	char ip1[4];
	char ip2[4];
	char ip3[4];
	char ip4[4];
	char *sysip=(char *)malloc(50);
	char *sysport=(char *)malloc(20);
	char *sysudp=(char *)malloc(20);
	char *desz=(char *)malloc(128);
	char *filterz=(char *)malloc(64);	
	char *alertz=(char *)malloc(128);
	char *timeflag=(char *)malloc(50);
	char selectz[10],sysid[10];
	memset(selectz,0,10);
	memset(sysport,0,20);
	memset(sysudp,0,20);
	memset(filterz,0,64);
	memset(sysid,0,10);
	
	cgiFormStringNoNewlines("gate_ip1", ip1, 4); 
	cgiFormStringNoNewlines("gate_ip2", ip2, 4); 
	cgiFormStringNoNewlines("gate_ip3", ip3, 4); 
	cgiFormStringNoNewlines("gate_ip4", ip4, 4); 
	cgiFormStringNoNewlines("filterz", filterz, 64); 
	cgiFormStringNoNewlines("tcp", sysudp, 20); 
	cgiFormStringNoNewlines("portid", sysport, 20); 
	cgiFormStringNoNewlines("ip_s1", selectz, 10); 
	cgiFormStringNoNewlines("sysid", sysid, 10); 
	
	if((strcmp(ip1,"")==0)||(strcmp(ip2,"")==0)||(strcmp(ip3,"")==0)||(strcmp(ip4,"")==0)||strcmp(filterz,"")==0||(strcmp(sysport,"")==0)||(strcmp(sysid,"")==0))
	{
        if((strcmp(filterz,"")==0)||(strcmp(sysport,"")==0))
		{
			memset(alertz,0,128);
			sprintf(alertz,"filter level or port %s",search(lpublic,"param_null"));
			ShowAlert(alertz);
		}
		else if(strcmp(sysid,"")==0)
		{
			memset(alertz,0,128);
			sprintf(alertz,"ID %s",search(lpublic,"param_null"));
			ShowAlert(alertz);
		}
		else
		{
			ShowAlert(search(lcontrol,"ip_null"));
		}
	}
	else
	{
		memset(sysip,0,50);
		sprintf(sysip,"%s.%s.%s.%s",ip1,ip2,ip3,ip4);
		port=strtoul(sysport,0,10);
		if((port<1)||(port>65535))
		{
			ShowAlert(search(lpublic,"input_overflow"));
		}
		else
		{
			memset(timeflag,0,50);
			ftime=if_dup_info(sysudp, sysip,sysport, filterz, timeflag);
			find_second_xmlnode(XML_FPATH, NODE_LOG, NODE_INDEXZ, sysid, &fid);			
			if((ftime==0)&&(fid==0))
			{
		    	add_syslog_serve_web(XML_FPATH, "1", sysip, sysport, filterz, sysudp,selectz,sysid);
				save_syslog_file();
				ret=restart_syslog();
				ret=0;
				if(ret==0)
					ShowAlert(search(lpublic,"oper_succ"));
				else
					ShowAlert(search(lpublic,"oper_fail"));			
			}			
			else
			{
				ShowAlert(search(lpublic,"log_design"));
			}
		}
	}
	free(sysip);
	free(sysport);
	free(sysudp);
	free(desz);
	free(filterz);
	free(alertz);
	return 0;
}
