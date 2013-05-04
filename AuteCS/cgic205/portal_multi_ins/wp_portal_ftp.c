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
* wp_portal_ftp.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system infos for portal ftp
*
*
*******************************************************************************/
#include <sys/stat.h> 
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include <fcntl.h>
#include <sys/wait.h>
#include "ws_portal_container.h"

#include "bsd/bsdpub.h"
#include "ws_init_dbus.h"


#define TEMPFILEZ "/opt/eag/www"
#define _PATH_BOARD_MASK_ "/dbm/product/board_on_mask"
#define LOG(format, args...) fprintf(stderr,"%s:%d:%s -> " format "\n", __FILE__, __LINE__, __func__, ##args)

int ShowPortalFtpPage(char *m,struct list *lpublic,struct list *lLicense,struct list *lsystem);    

void down_portal_file(struct list * lpublic,struct list *lLicense,struct list *lsystem);

int  pftp_getint(const char *);
void pftp_checkbox(const char *);
int  pftp_dircpy(char *, char *, char *);

int cgiMain()
{  
  char *encry=(char *)malloc(BUF_LEN);	  
  char *str; 			   
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lLicense;     /*解析wlan.txt文件的链表头*/  
  struct list *lsystem;     /*解析wlan.txt文件的链表头*/
  
  memset(encry,0,BUF_LEN);
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lLicense=get_chain_head("../htdocs/text/authentication.txt");
  lsystem=get_chain_head("../htdocs/text/system.txt");
  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	str=dcryption(encry);
	if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
    else
      ShowPortalFtpPage(encry,lpublic,lLicense,lsystem);
  }
  else
  {    
	cgiFormStringNoNewlines("encry_newwtp",encry,BUF_LEN);
	str=dcryption(encry);
	if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
    else
      ShowPortalFtpPage(encry,lpublic,lLicense,lsystem);
  } 
  free(encry);
  release(lpublic);  
  release(lLicense);
  release(lsystem);
  return 0;
}

int ShowPortalFtpPage(char *m,struct list *lpublic,struct list *lLicense,struct list *lsystem)
{  
 
	int i;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Portal</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	"<style type=text/css>"\
	"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	"#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	"#link{ text-decoration:none; font-size: 12px}"\
	".usrlis {overflow-x:hidden;	overflow:auto; width: 750px; height: 150px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	"</style>"\
   "</head>\n"\
	"<body>");

  
   if(cgiFormSubmitClicked("wtp_down") == cgiFormSuccess)
  { 

	down_portal_file(lpublic,lLicense,lsystem);
   
  }

  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"Portal Ftps");
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=wtp_down style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
        fprintf(cgiOut,"<td width=62 align=center><a href=wp_user_portal.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
	  
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_user_portal.cgi?UN=%s&portal_id=0' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",m,search(lpublic,"menu_san"), search( lLicense, "captive_Portal") );
	  				//白名单
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_white_list.cgi?UN=%s&portal_id=0' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",m,search(lpublic,"menu_san"), search( lLicense, "portal_white_list") );
				//eag
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_eag_conf.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",m,search(lpublic,"menu_san"), search( lLicense, "eag_title") );
				//user manage
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_user_manage.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",m,search(lpublic,"menu_san"), search( lLicense, "user_mng") );								
				//nas
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_nasid_byvlan.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",m,search(lpublic,"menu_san"), search( lLicense, "nasid_management") );	
				//multi portal
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_multi_portal.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",m,search(lpublic,"menu_san"), search( lLicense, "multi_portal_management") );	
				//黑名单
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_black_list.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",m,search(lpublic,"menu_san"), search( lLicense, "portal_black_list") );	

  				
				fprintf(cgiOut,"<tr height=26>"\
  					  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search( lLicense, "portal_ftp"));        /*突出显示*/         
                fprintf(cgiOut,"</tr>");	
				
				//multi raidus
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_multi_radius.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",m, search(lpublic,"menu_san"),search( lLicense, "multi_radius_management") );	
                //wtp wlan vlan
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_wtpwlan_map_vlan.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",m, search(lpublic,"menu_san"),search( lLicense, "vlan_maping") );	
				for(i=0;i<3;i++)
				{
					fprintf(cgiOut,"<tr height=25>"\
					"<td id=tdleft>&nbsp;</td>"\
					"</tr>");
				}
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
			  "<table width=750 border=0 cellspacing=0 cellpadding=0>");
				/////////////////////////
			fprintf(cgiOut,"<tr height=30>"\
				"<td align=left width=70>%s:</td>",search(lpublic,"web_source"));
			fprintf(cgiOut,"<td width=330><input type=text name=url size=50 ></td>");
			fprintf(cgiOut,"<td></td>");
			fprintf(cgiOut,"</tr>");

			fprintf(cgiOut,"<tr height=30>"\
			"<td align=left width=70>%s:</td>",search(lLicense,"file_remote"));
			fprintf(cgiOut,"<td colspan=2><input type=text name=sourpath size=30></td>"\
			"</tr>");

			pftp_checkbox(search(lLicense,"slot_id"));
			
			fprintf(cgiOut,"<tr height=30>");
			fprintf(cgiOut,"<td align=left width=70>%s:</td>",search(lLicense,"file_loc"));
			fprintf(cgiOut,"<td colspan=2><input type=text name=despath size=30></td>");
			fprintf(cgiOut,"</tr>");

			fprintf(cgiOut,"<tr>"\
			"<td align=left width=70>%s:</td>",search(lsystem,"user_na"));
			fprintf(cgiOut,"<td colspan=2><input type=text name=usr size=30></td>");
			fprintf(cgiOut,"</tr>");
			
			fprintf(cgiOut,"<tr height=30>"\
			"<td align=left width=70>%s:</td>",search(lsystem,"password"));
			fprintf(cgiOut,"<td colspan=2><input type=password name=pawd size=33.8></td>"\
			"</tr>");

            fprintf(cgiOut,"<tr>\n");
			fprintf(cgiOut,"<td colspan=3>\n");
			fprintf(cgiOut,"<div class=usrlis><table width=600 border=0 cellspacing=0 cellpadding=0>\n");
            FILE *pp;
			char buff[128];
			memset(buff,0,128);
			char *temf=(char *)malloc(128);
			memset(temf,0,128);
			sprintf(temf,"ls -l %s|grep ^d|grep -v -i cvs|awk '{print $9}'",TEMPFILEZ);
			pp=popen(temf,"r");  
			if(pp==NULL)
			fprintf(cgiOut,"error open the pipe");
			else
			{
				fgets( buff, sizeof(buff), pp );			 
				do
				{										   
					fprintf(cgiOut,"<tr><td>");		

					fprintf(cgiOut,"<br>");   					     
					fprintf(cgiOut,"%s",buff); 
					fprintf(cgiOut,"</td>");
					if(strlen(buff)!=0)
					{
					    fprintf(cgiOut,"<td><a href=wp_pftp_deal.cgi?UN=%s&TYPE=1&FILE=%s>%s</a></td>\n",m,buff,search(lpublic,"log_scan"));
					    fprintf(cgiOut,"<td><a href=wp_pftp_deal.cgi?UN=%s&TYPE=2&FILE=%s>%s</a></td>\n",m,buff,search(lpublic,"delete"));
					}
					fprintf(cgiOut,"</tr>");
					fgets( buff, sizeof(buff), pp ); 		
				}while( !feof(pp) ); 					   
			}
			pclose(pp);
			free(temf);
			fprintf(cgiOut,"</table></div>\n");
			fprintf(cgiOut,"</td></tr>\n");

/////////////////
	fprintf(cgiOut,"<tr>"\
   				 "<td colspan=2><input type=hidden name=encry_newwtp value=%s></td>",m);
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
return 0;
}
void down_portal_file(struct list * lpublic,struct list *lLicense,struct list *lsystem)
{
	char **responses;
	int i = 0;
	
	int result = cgiFormStringMultiple("boarder", &responses);
	
	char * url = (char*)malloc(100);
	char * usrname = (char*)malloc(50);
	char * passwd = (char*)malloc(50);
	char * despath = (char*)malloc(50);
	char * sourpath = (char*)malloc(50);
	char * temp = (char *)malloc(100);
	char * alertc = (char *)malloc(50);

    memset(alertc,0,50);
	memset(url,0,100);
	memset(usrname,0,50);
	memset(passwd,0,50);
	memset(despath,0,50);
	memset(sourpath,0,50);
	memset(temp,0,100);

	char cmd[256];
	int op_ret=-1;
	int status;


	ccgi_dbus_init();
	cgiFormStringNoNewlines("url",url,100);  
	cgiFormStringNoNewlines("usr",usrname,50);
	cgiFormStringNoNewlines("pawd",passwd,50);
	cgiFormStringNoNewlines("despath",despath,50);
	cgiFormStringNoNewlines("sourpath",sourpath,50);

	if((strcmp(url,"")!=0) && (strcmp(usrname,"")!=0) &&(strcmp(passwd,"")!=0)&&(strcmp(despath,"")!=0)&&(strcmp(sourpath,"")!=0))
	{
		if(replace_url(url,"#","%23") != NULL)
		strcpy(temp,replace_url(url,"#","%23"));
		else
		strcpy(temp,url);	

	    memset(cmd,0,256);

		sprintf(cmd,"sudo ftpbatch.sh  %s %s %s %s %s ",temp,usrname,passwd,sourpath,despath);		
		LOG("%s",cmd);

		strcat(cmd,"> /dev/null");
		
		status=system(cmd);
		
		op_ret=WEXITSTATUS(status);

		
		if(op_ret == 0)
		{

			if (result != cgiFormNotFound) {
				
				while (responses[i]) {

					LOG("%s",responses[i]);
					op_ret = pftp_dircpy((int)responses[i],despath,despath);
					i++;

					if(op_ret != 0)
					{
						ShowAlert(search(lpublic,"oper_fail"));
					}
				}
			}
			
			ShowAlert(search(lpublic,"oper_succ"));
		}
		else
		{
			ShowAlert(search(lpublic,"oper_fail"));
		}
		
		

	}
	else
	{
		if(strcmp(url,"")==0)
			ShowAlert(search(lpublic,"web_serr"));
		else if(strcmp(sourpath,"")==0)
		{
			memset(alertc,0,50);
			sprintf(alertc,"%s%s",search(lLicense,"file_remote"),search(lpublic,"param_null"));
			ShowAlert(alertc);
		}
		else if(strcmp(despath,"")==0)
		{
			memset(alertc,0,50);
			sprintf(alertc,"%s%s",search(lLicense,"file_loc"),search(lpublic,"param_null"));
			ShowAlert(alertc);
		}
		else if(strcmp(usrname,"")==0)
			ShowAlert(search(lsystem,"userna_err"));
		else if(strcmp(passwd,"")==0)
			ShowAlert(search(lsystem,"pass_err"));
	}
	free(url);
	free(usrname);
	free(passwd);
	free(despath);	
	free(sourpath);
	free(temp);

	cgiStringArrayFree(responses);

	return;
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

void pftp_checkbox(const char *slot_id)
{
	int bmask,i = 1;

	bmask = pftp_getint(_PATH_BOARD_MASK_);

	if(bmask == -1 || bmask == 0)
	{
		return;
	}
	
	char path_slot_type[64];
	char path_slot_master[64];

	int function_type;
	int is_master;

	fprintf(cgiOut,"<tr height=30>");
	fprintf(cgiOut,"<td align=left width=70>%s:</td>",slot_id);
	fprintf(cgiOut, "<td colspan=2>\n");
			
	while(bmask)
	{
		if( bmask & 0x1 )	
		{
			memset(path_slot_type,0,64);
			memset(path_slot_master,0,64);
			sprintf(path_slot_type,"/dbm/product/slot/slot%d/function_type",i);
			sprintf(path_slot_master,"/dbm/product/slot/slot%d/is_active_master",i);

			function_type = pftp_getint(path_slot_type);
			is_master = pftp_getint(path_slot_master);

			if((function_type & 0x1 )|| (function_type & 0x2))
			{
				if(is_master)
				{
					fprintf(cgiOut, "<input type=\"checkbox\" name=\"boarder\" value=\"%d\" checked disabled>%d\n",i,i);
				}
				else
				{
					fprintf(cgiOut, "<input type=\"checkbox\" name=\"boarder\" value=\"%d\">%d\n",i,i);
				}
			}

		}
		bmask = bmask >> 1; 
		i++;
	}

	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut,"</tr>");
	
	return;
}

int pftp_dircpy(char *slot ,char *sourpath,char *despath)
 {
	
	
	ccgi_dbus_init();
	
	int slot_id = strtoul(slot,NULL,10);

	DBusConnection* conn = init_slot_dbus_connection(slot_id);

	if(conn == NULL)
	{
		return -1;
	}

	char src_path[128] = {0};
	char des_path[128] = {0};
	char command[128] = {0};

	void *p = (void *)0;
	
	sprintf(src_path, "/opt/eag/www/%s", sourpath);
	sprintf(des_path, "/opt/eag/www/%s", despath);
	sprintf(command, "sudo eagbatch.sh %s", despath);
	
	LOG("%d %s %s ",slot_id,sourpath,despath);

	int ret = dcli_bsd_copy_file_to_board_v2(ccgi_dbus_connection,slot_id,src_path,des_path,1,BSD_TYPE_NORMAL);
	
	if(ret == 0)
	{
		if(ac_manage_exec_extend_command(conn,1,command,&p) == 0);
		{
			LOG("slot %d pftp_dircpy successful%d\n",slot_id);
		}
	}
	else if(ret == -2)
	{
		LOG("over time.\n");
	}
	else if(ret == 1)
	{
		LOG("not enough memery on slot_%d",2);
	}
	else
	{
		LOG("failed.\n");
	}

	return 0;
}


