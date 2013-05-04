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
* wp_pftp_deal.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system infos 
*
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include <fcntl.h>
#include <sys/wait.h>
#include "ws_log_conf.h"
#include <dbus/dbus.h>
#include "ws_init_dbus.h"

#define TEMPFILEZ "/opt/eag/www"
#define PATH_SLOTID "/dbm/local_board/slot_id"
#define LOG(format, args...) fprintf(stderr,"%s:%d:%s -> " format "\n", __FILE__, __LINE__, __func__, ##args)

int ShowFtpPage(struct list *lcontrol,struct list *lpublic);
int pftp_del(const char *);
int pftp_getint(const char *path);

int cgiMain()
{
	
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	
	ShowFtpPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	return 0;
}



int ShowFtpPage(struct list *lcontrol,struct list *lpublic)
{ 
	char *encry=(char *)malloc(BUF_LEN);
	char *str;

	char file_name[128];  
	memset(file_name,0,128);

	char *cmd = (char *)malloc(128);   
	memset(cmd,0,128); 

	int i = 0;   
	char typez[10];
	memset(typez,0,10);

	cgiFormStringNoNewlines("FILE", file_name, 128);  
	cgiFormStringNoNewlines("TYPE", typez, 10);  

	memset(encry,0,BUF_LEN);
	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}

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

    if(strcmp(typez,"1")==0)
    {
		fprintf(cgiOut,"<form method=post>"\
		"<div align=center>"\
		"<table width=976 border=0 cellpadding=0 cellspacing=0>");    
		fprintf(cgiOut,"<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
		"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg></td>",search(lpublic,"title_style"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

		fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
		fprintf(cgiOut,"<input type=hidden name=FILE value=%s />",file_name);
		fprintf(cgiOut,"<input type=hidden name=TYPE value=%s />",typez);

		
		
		fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");  
		fprintf(cgiOut,"<tr>");	

		//fprintf(cgiOut,"<td width=62 align=left><a href=wp_pftp_deal.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=log_view style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_portal_ftp.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));

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

		for(i=0;i<18;i++)
		{
			fprintf(cgiOut,"<tr height=25>"\
			"<td id=tdleft>&nbsp;</td>"\
			"</tr>");
		}

		fprintf(cgiOut,"</table>"); 
		fprintf(cgiOut,"</td>");
		fprintf(cgiOut,"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");

		fprintf(cgiOut,"<div class=usrlis><table width=600 border=0 cellspacing=0 cellpadding=0>");	  
		//////
	    FILE *pp;
		char buff[128];
		memset(buff,0,128);
		sprintf(cmd,"ls %s/%s",TEMPFILEZ,file_name);
		int cl =1;
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
				fprintf(cgiOut,"</td>");
				fprintf(cgiOut,"</tr>");
				fgets( buff, sizeof(buff), pp ); 		
				cl = !cl;
			}while( !feof(pp) ); 					   
			pclose(pp);
		}

		//////

		fprintf(cgiOut,"</table></div>");		

		fprintf(cgiOut,"</td></tr>");
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
		"</form>");
    }
	else if(strcmp(typez,"2")==0)
	{
		int status,iRet;
		
		char cmd[128]={0};
		memset(cmd,0,128);
		sprintf(cmd,"sudo ftpdealsd.sh %s >/dev/null",file_name);	
		status = system(cmd);
	    iRet = WEXITSTATUS(status);	
        ////deal with wenjian

		if(iRet==0)
		{
			ShowAlert(search(lpublic,"oper_succ"));
			pftp_del(file_name);
		}
		else
		{
			ShowAlert(search(lpublic,"oper_fail"));
		}
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_portal_ftp.cgi?UN=%s';\n", encry);
		fprintf( cgiOut, "</script>\n" );	
	}
	fprintf(cgiOut,"</body>");
	fprintf(cgiOut,"</html>");  

	free(encry);
	free(cmd);
	return 0;
}

int pftp_getint(const char *path)
{
	int fd;
	
	fd = open(path,O_RDONLY);

	if(fd == -1)
	{
		return -1;
	}
	
	char buf[16] = {0};

	read(fd,buf,16);

	return  strtoul(buf,NULL,10);
}

int pftp_del(const char *file)
{

	ccgi_dbus_init();
	
	int slot,l_slot;
	
	DBusConnection* conn;
	char command[128] = {0};
	void *p = (void *)0;
	sprintf(command, "sudo ftpdealsd.sh %s >/dev/null", file);

	l_slot = pftp_getint(PATH_SLOTID);

	for(slot = 1 ; slot < 16 ; slot++)
	{
	
		if(slot == l_slot)
		{
			continue;
		}
		
		if((conn = init_slot_dbus_connection(slot)) != NULL)
		{

			LOG("%d",slot);
			
			if(ac_manage_exec_extend_command(conn,1,command,&p) == 0)
			{
				LOG("slot %d pftp_del successful%d\n",slot);
			}
		
		}
	
	}
	
	return 0;
}

