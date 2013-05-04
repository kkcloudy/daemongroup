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
* wp_logsys_view.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for syslog info view
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
#include "ws_returncode.h"
#include "ws_usrinfo.h"


#define SYSLOGFILEZ "/var/log/syslogservice.log"
#define SYSLOGDATEFILE "/var/log/systemlog"
#define DATEFILETMP  "/var/run/filter_date"

static char *syslevellist[] = {
	"debug",
	"inform",
	"notice",
	"warning",
	"err",
	"crit"
};

static char *sysoptlist[] = {
	"none",
	"read",
	"delete",
};


#define SYSLEVELNUM sizeof(syslevellist)/sizeof(syslevellist[0])
#define SYSOPTNUM sizeof(sysoptlist)/sizeof(sysoptlist[0])

int ShowVersionDelPage(struct list *lcontrol,struct list *lpublic);

void show_content_sys(char *cmd);
void show_date_list(char *datestring,int daynum);
void show_syslevel_list(char *syslevel);
void show_sysopt_list(struct list *lpublic,char *sysopt);



int cgiMain()
{
	
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	
	ShowVersionDelPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	return 0;
}



int ShowVersionDelPage(struct list *lcontrol,struct list *lpublic)
{ 
	char encry[128] = {0}; 
	char *str; 
	char *ptr;
	char showtype[10] = {0};
	char cmd[128] = {0};   
	int i = 0;   
	char key_word[128] = {0};
	char savecycle[30] = {0};
	char datestring[128] = {0};
	char filepath[128] = {0};
	char filename[30]  = {0};
	char useropt[30] = {0};	
	int daynum = 0;
	int dayscount = 0;
	char log_info[20] = {0};
	char oper_info[20] = {0};
	int rmflag = 0;

	if(cgiFormSubmitClicked("log_view") != cgiFormSuccess)
	{
		memset(encry,0,128);
		cgiFormStringNoNewlines("UN", encry, 128); 
		str=dcryption(encry);
		if(str==NULL)
		{
			ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
		}


	}
	cgiFormStringNoNewlines("UN", encry, 128); 
	cgiFormStringNoNewlines("DATES", datestring, 128); 

	ptr=dcryption(encry);
	get_user_syslog_by_name(ptr,&log_info,&oper_info);

	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");

	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	"<style type=text/css>"\
	"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	"#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	"#link{ text-decoration:none; font-size: 12px}"\
	".usrlis {overflow-x:hidden;	overflow:auto; width: 750px; height: 420px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	"</style>"\
	"</head>"\
	"<body>");  


	fprintf(cgiOut,"<form method=post>"\
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

	fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=log_view style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
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
	fprintf(cgiOut,"<tr height=25>"\
	"<td id=tdleft>&nbsp;</td>"\
	"</tr>");
	fprintf(cgiOut,"<tr height=25>"\
	"<td id=tdleft>&nbsp;</td>"\
	"</tr>");       		

	for(i=0;i<20;i++)
	{
		fprintf(cgiOut,"<tr height=25>"\
		"<td id=tdleft>&nbsp;</td>"\
		"</tr>");
	}

	fprintf(cgiOut,"</table>");
	fprintf(cgiOut,"</td>");
	fprintf(cgiOut,"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
	fprintf(cgiOut,"<table width=600 border=0 cellspacing=0 cellpadding=0>");	 

	fprintf(cgiOut,"<tr><th width=150 align=left style=font-size:14px>%s</th></tr>",search(lpublic,"log_detail"));
	fprintf(cgiOut,"<tr>");
	fprintf(cgiOut,"<td colspan=4 id=sec style=\"border-bottom:2px solid #53868b\">&nbsp;</td>"\
	"</tr>");

	fprintf(cgiOut,"<tr height=5><td colspan=4></td></tr>");


	memset(showtype,0,10);
	cgiFormStringNoNewlines("showtype", showtype, 10);
	memset(savecycle,0,30);
	get_first_xmlnode(XML_FPATH, NODE_SAVECYCLE,&savecycle);
	daynum = 0;
	daynum = strtoul(savecycle,NULL,10);
	memset(key_word,0,128);
	cgiFormStringNoNewlines("keys", key_word, 128);
	//////////////////////
	fprintf(cgiOut,"<tr>");
	fprintf(cgiOut,"<td colspan=2>%s:</td>",search(lpublic,"log_opt"));
	fprintf(cgiOut,"<td colspan=2>\n");
	fprintf(cgiOut,"<select name=useropt style='width:140;height:auto'>\n");
	show_sysopt_list(lpublic,oper_info);
	fprintf(cgiOut,"</select>\n");
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"</tr>");

	//////////////////////		
	fprintf(cgiOut,"<tr height=5><td colspan=4></td></tr>");
	if ((strcmp(showtype,"2") == 0)||(strcmp(showtype,"") == 0))
	{
		fprintf(cgiOut,"<tr>");
		fprintf(cgiOut,"<td width=20><input type=\"radio\" name=\"showtype\" value=\"2\"  checked></td>");
		fprintf(cgiOut,"<td>%s</td>",search(lpublic,"log_date"));
		fprintf(cgiOut,"<td><select name=keys style='width:140;height:auto'>\n");	
		show_date_list(key_word,daynum);	
		fprintf(cgiOut,"</select></td>");
		fprintf(cgiOut,"<td>\n");
		fprintf(cgiOut,"<select name=syslevel style='width:140;height:auto'>\n");
		show_syslevel_list(log_info);
		fprintf(cgiOut,"</select>");
		fprintf(cgiOut,"</td>\n");
		fprintf(cgiOut,"</tr>");    
	}
	else
	{
		fprintf(cgiOut,"<tr>");
		fprintf(cgiOut,"<td width=20><input type=\"radio\" name=\"showtype\" value=\"2\"  ></td>");
		fprintf(cgiOut,"<td>%s</td>",search(lpublic,"log_date"));
		fprintf(cgiOut,"<td><select name=keys style='width:140;height:auto'>\n");
		show_date_list(key_word,daynum);		
		fprintf(cgiOut,"</select></td>");
		fprintf(cgiOut,"<td>\n");
		fprintf(cgiOut,"<select name=syslevel style='width:140;height:auto'>\n");
		show_syslevel_list(log_info);
		fprintf(cgiOut,"</select>");
		fprintf(cgiOut,"</td>\n");
		fprintf(cgiOut,"</tr>");
	}
	fprintf(cgiOut,"<tr height=5><td colspan=4></td></tr>");
	if (strcmp(showtype,"3") == 0)
	{
		fprintf(cgiOut,"<tr>");
		fprintf(cgiOut,"<td width=20><input type=\"radio\" name=\"showtype\" value=\"3\" checked></td>");
		fprintf(cgiOut,"<td>%s</td>",search(lpublic,"log_savecycly"));
		fprintf(cgiOut,"<td colspan=2><input type=text name=savecycle value=\"%s\" Maxlength=3><font color=red>(7-14)</font></td>",savecycle);
		fprintf(cgiOut,"</tr>");
	}
	else
	{
		fprintf(cgiOut,"<tr>");
		fprintf(cgiOut,"<td width=20><input type=\"radio\" name=\"showtype\" value=\"3\"  ></td>");
		fprintf(cgiOut,"<td>%s</td>",search(lpublic,"log_savecycly"));
		fprintf(cgiOut,"<td colspan=2><input type=text name=savecycle value=\"%s\" Maxlength=3><font color=red>(7-14)</font></td>",savecycle);
		fprintf(cgiOut,"</tr>");
	}
#ifdef _D_CC_
	fprintf(cgiOut,"<tr height=5><td colspan=4></td></tr>");
	if (strcmp(showtype,"4") == 0)
	{
		fprintf(cgiOut,"<tr>");
		fprintf(cgiOut,"<td width=20><input type=\"radio\" name=\"showtype\" value=\"4\" checked></td>");
		fprintf(cgiOut,"<td colspan=3>%s</td>",search(lpublic,"critical_syslog"));
		fprintf(cgiOut,"</tr>");
	}
	else
	{
		fprintf(cgiOut,"<tr>");
		fprintf(cgiOut,"<td width=20><input type=\"radio\" name=\"showtype\" value=\"4\"  ></td>");
		fprintf(cgiOut,"<td colspan=3>%s</td>",search(lpublic,"critical_syslog"));
		fprintf(cgiOut,"</tr>");
	}
#endif
 
	fprintf(cgiOut,"<tr>");
	fprintf(cgiOut,"<td colspan=4 id=sec style=\"border-bottom:2px solid #53868b\">&nbsp;</td>"\
	"</tr>");

	if(cgiFormSubmitClicked("log_view") == cgiFormSuccess)
	{
		memset(showtype,0,10);
		cgiFormStringNoNewlines("showtype", showtype, 10);
		memset(savecycle,0,30);
		cgiFormStringNoNewlines("savecycle", savecycle, 30);
		memset(key_word,0,128);
		cgiFormStringNoNewlines("keys",key_word,128);
		memset(filename,0,30);
		cgiFormStringNoNewlines("syslevel",filename,30);
		memset(useropt,0,30);
		cgiFormStringNoNewlines("useropt",useropt,30);
		rmflag = 0;
		
		if(strcmp(showtype,"2")==0)
		{
			if(strcmp(key_word,"")==0)
			{
				ShowAlert(search(lpublic,"log_key_null"));
			}
			else
			{
			    memset(cmd,0,128);
				memset(filepath,0,128);
				sprintf(filepath,"%s/%s/%s",SYSLOGDATEFILE,key_word,filename);
				delete_line_blank(useropt);
				if (strcmp(useropt,sysoptlist[0]) == 0)
				{}
				else if (strcmp(useropt,sysoptlist[1]) == 0)
				{
					sprintf(cmd,"sudo cat %s |tail -n 50 |sort -r ",filepath); 
				}
				else if (strcmp(useropt,sysoptlist[2]) == 0)
				{
					sprintf(cmd,"sudo rm %s > /dev/null",filepath); 
					if (access(filepath,0) == 0)
					{
						rmflag = 1;
					}
				}
				show_content_sys(cmd);  
				if (rmflag == 1)
				{
					restart_syslog();
				}
			}
		}
		if(strcmp(showtype,"3")==0)
		{
			if(strcmp(savecycle,"")==0)
			{
				ShowAlert(search(lpublic,"log_key_null"));
			}
			else
			{
				dayscount = strtoul(savecycle,NULL,10);
				if ((dayscount >= 7)&&(dayscount <= 14))
				{
					mod_first_xmlnode(XML_FPATH, NODE_SAVECYCLE, savecycle);
					memset(cmd,0,128);
					sprintf(cmd,"sudo syslog_date.sh %d",(dayscount-1));
					system(cmd);
					restart_syslog();
				}
				else
				{
					ShowAlert(search(lpublic,"input_overflow"));
				}
			}
		}
		
#ifdef _D_CC_
		if(strcmp(showtype,"4")==0)
		{
			char file_path[] = "/mnt/critlog/syscrit.log";	
			char buff[1024] = "";
			FILE *p_file = NULL;				
			memset(buff, 0, sizeof(buff));
			fprintf(cgiOut,"<tr>");
			fprintf(cgiOut,"<td align=left colspan=4>");
			fprintf(cgiOut,"<div class=usrlis><table frame=below rules=rows width=750 border=0 cellspacing=0 cellpadding=0>"\
			"<tr height=30 bgcolor=#eaeff9 style=font-size:16px>"\
			"</tr>"); 
			fprintf(cgiOut,"<tr height=10><td></td></tr>");	                       			  

			p_file = fopen(file_path,"r");		
			int cl=1;
			if(p_file)
			{
				fgets(buff, sizeof(buff), p_file);
				while(buff[0] != '\0')
				{
					fprintf(cgiOut,"<tr bgcolor=%s><td>",setclour(cl));
					fprintf(cgiOut,"<br>");   					     
					fprintf(cgiOut,"%s",buff); 
					fprintf(cgiOut,"</td></tr>");
					cl = !cl;
					memset(buff, 0, sizeof(buff));
					fgets(buff, sizeof(buff), p_file);		
				}				
				fclose(p_file);
			}
			else
			{
				fprintf(cgiOut,"<tr><td>%s</td></tr>",search(lpublic,"no_syslog"));	   
			}
			fprintf(cgiOut,"</table></div></td>");	
			fprintf(cgiOut,"</tr>");			

		}
#endif
	}
	fprintf(cgiOut,"</table>");	
	fprintf(cgiOut,"</td></tr>");
	fprintf(cgiOut,"<input type=hidden name=DATES value=\"%s\" />",key_word);
	fprintf(cgiOut,"<tr height=4 valign=top>"\
	"<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
	"<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
	"</tr>");

	fprintf(cgiOut, "</table>");
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
	return 0;
}

void show_content_sys(char *cmd)
{
	FILE *pp;
	char buff[128]; 
	int cl=1;
	fprintf(cgiOut,"<tr>");
	fprintf(cgiOut,"<td align=left colspan=4>");

	fprintf(cgiOut,"<div class=usrlis><table frame=below rules=rows width=750 border=0 cellspacing=0 cellpadding=0>"\
	"<tr height=30 bgcolor=#eaeff9 style=font-size:16px>"\
	"</tr>"); 

	fprintf(cgiOut,"<tr height=10><td></td></tr>");	                                               			  

	pp=popen(cmd,"r");  
	if(pp==NULL)
		fprintf(cgiOut,"error open the pipe");
	else
	{
		fgets( buff, sizeof(buff), pp );					 
		do
		{										   
			fprintf(cgiOut,"<tr bgcolor=%s><td>",setclour(cl));		

			fprintf(cgiOut,"<br>");   					     
			fprintf(cgiOut,"%s",buff); 
			fprintf(cgiOut,"</td></tr>");
			fgets( buff, sizeof(buff), pp ); 	
			cl = !cl;
		}while( !feof(pp) ); 					   
	}
	pclose(pp);

	fprintf(cgiOut,"</table></div></td>");	
	fprintf(cgiOut,"</tr>");			

}


void show_date_list(char *datestring,int daynum)
{
	FILE *pp = NULL;
	char buff[128] = {0}; 
	char cmd[128] = {0};
	sprintf(cmd,"sudo syslog_date.sh %d",(daynum-1));
	system(cmd);
	int i = 0;
	memset(cmd,0,128);
	sprintf(cmd,"cat %s",DATEFILETMP);
	pp=popen(cmd,"r");  
	if(pp==NULL)
	{
		fprintf(stderr,"error open the pipe");
		return ;
	}
	else
	{
		fgets( buff, sizeof(buff), pp );					 
		do
		{		
			i++;
			delete_line_blank(datestring);
			delete_line_blank(buff);
			if (strcmp(datestring,buff) == 0)
			{
				fprintf(cgiOut,"<option value='%s' selected=selected>%s</option>",buff,buff);  
			}
			else
			{
				fprintf(cgiOut,"<option value='%s'>%s</option>",buff,buff);  
			}
			fgets( buff, sizeof(buff), pp ); 	
			if (i == daynum)
			{
					break;
			}
		}while( !feof(pp) ); 	
		pclose(pp);
	}

}
void show_syslevel_list(char *syslevel)
{
	int i = 0;
	int flag = 0;
	char tmp[128] = {0};
	strcpy(tmp,syslevel);
	delete_line_blank(tmp);
	for (i=0;i<SYSLEVELNUM;i++)
	{
		if (strcmp(tmp,syslevellist[i]) == 0)
		{
			fprintf(cgiOut,"<option value='%s' selected=selected>%s</option>",syslevellist[i],syslevellist[i]);
			flag = 1;
		}
		else
		{
			fprintf(cgiOut,"<option value='%s'>%s</option>",syslevellist[i],syslevellist[i]);
		}
		if (flag == 1)
		{
			break;
		}
	}
}

void show_sysopt_list(struct list *lpublic,char *sysopt)
{
	int i = 0;
	int flag = 0;
	char tmp[128] = {0};
	delete_line_blank(sysopt);

	for (i=0;i<SYSOPTNUM;i++)
	{
		memset(tmp,0,128);
		if (i == 0)
		{
			sprintf(tmp,"%s",search(lpublic,"log_none"));
		}
		else if (i == 1)
		{
			sprintf(tmp,"%s",search(lpublic,"log_read"));
		}
		else if (i == 2)
		{ 
			sprintf(tmp,"%s",search(lpublic,"log_delete"));
		}
		if (strcmp(sysopt,sysoptlist[i]) == 0)
		{
			
			fprintf(cgiOut,"<option value='%s'>%s</option>",sysoptlist[i],tmp);
			flag = 1;
		}
		else
		{
			fprintf(cgiOut,"<option value='%s'>%s</option>",sysoptlist[i],tmp);
		}
		if (flag == 1)
		{
			break;
		}
	}
}

