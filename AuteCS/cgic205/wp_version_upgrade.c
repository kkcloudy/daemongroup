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
* wp_version_upgrade.c
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
#include <fcntl.h>
#include <sys/stat.h>  
#include <sys/wait.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_dcli_wlans.h"
#include "ws_version_param.h"
#include "bsd/bsdpub.h"
#include "dbus/bsd/BsdDbusPath.h"

#define UPLOAD_FILE_PATH	"/var/run/upload.txt"

int ShowVersionUpgradePage(char *m,struct list *lpublic,struct list *lsystem);
void Local_Upgrade(char *local_url,struct list *lpublic,struct list *lsystem);
void Web_Upgrade(char *web_url,char *usrname,char *passwd,struct list *lpublic,struct list *lsystem);


int cgiMain()
{  
  char encry[BUF_LEN] = { 0 }; 
  char *str = NULL;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsystem = NULL;     /*解析system.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsystem=get_chain_head("../htdocs/text/system.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
  	;
  }  
  else
  {  
    cgiFormStringNoNewlines("encry_verupgrade",encry,BUF_LEN);
  }
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
  	ShowVersionUpgradePage(encry,lpublic,lsystem);
  release(lpublic);  
  release(lsystem);
  destroy_ccgi_dbus();
  return 0;
}

int ShowVersionUpgradePage(char *m,struct list *lpublic,struct list *lsystem)
{  
  int i = 0;
  char IsSubmit[5] = { 0 };
  char local_url[1024] = { 0 };
  char web_url[128] = { 0 };
  char usrname[30] = { 0 };
  char passwd[30] = { 0 };
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>VersionUpgrade</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>");
  
  fprintf(cgiOut,"<script type=\"text/javascript\" src=/jquery-1.8.3.min.js></script>");
  fprintf(cgiOut,"<script src=/probar.js>"\
  "</script>"\
  "<script type=text/javascript>"\
     "function check_sysinfo_div_pos()"\
     "{"\
     "	    var   oRect   =   document.all.only.getBoundingClientRect();"\
     "  var obj_div=document.getElementById('only');"\
     "  obj_div.style.top = oRect.top+3;"\
     "  obj_div.style.left = oRect.left+204;"\
     "}"\
 "</script>"\
  "<script type=\"text/javascript\">"\
     "function HandleFileButtonClick(obj)"\
     "{" \
     "   with(obj){"\
     "	    var   oRect   =   document.all.fakeButton.getBoundingClientRect();"\
     "     style.posTop=oRect.top+3;"\
     "     style.posLeft=oRect.left+2;"\
     "	  }"\
     "}\n"\
  "</script>"\
  "<body onResize=check_sysinfo_div_pos()>");

  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);

  if((cgiFormSubmitClicked("local_upload") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
	  fprintf(stderr,"local_url:%s\n",local_url);	
	  char cmdcc[100]={0};
	  FILE *fp = NULL;
	  int upload_flag=0;
	  if(access(UPLOAD_FILE_PATH,0) != 0)
	  {
		  fprintf(stderr,"access(UPLOAD_FILE_PATH,0)\n"); 
		  memset(cmdcc,0,sizeof(cmdcc));
		  snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo touch %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		  system(cmdcc);
		  memset(cmdcc,0,sizeof(cmdcc));
		  snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo chmod 666 %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		  system(cmdcc);
	  }
	  fp = fopen(UPLOAD_FILE_PATH ,"w+");
	  if(NULL == fp)
	  {
		  fprintf(stderr,"(NULL == fp)\n"); 
		  ShowAlert(search(lpublic,"lupload_fail"));
		  upload_flag = 1;
	  }
	  else
	  {
		  fprintf(stderr,"f---puts(1, fp)\n"); 
		  fputs("1", fp);
		  fclose(fp);
		  memset(cmdcc,0,sizeof(cmdcc));
		  snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo chmod 666 %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		  system(cmdcc);
	  }

	  if(upload_flag == 0)
	  {
		  memset(local_url,0,sizeof(local_url));
		   if(cgiFormFileName("myFile", local_url, sizeof(local_url)) == cgiFormSuccess) 
		   {
			   
			   Local_Upgrade(local_url,lpublic,lsystem);
		   }
		   else
		   {
			   memset(cmdcc,0,sizeof(cmdcc));
			   snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo rm %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
			   system(cmdcc);
			   ShowAlert(search(lpublic,"url_not_empty"));
		   }
	  }
  }

  memset(web_url,0,sizeof(web_url));
  memset(usrname,0,sizeof(usrname));
  memset(passwd,0,sizeof(passwd));
  if((cgiFormSubmitClicked("web_upload") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
	  char cmdcc[100]={0};
	  FILE *fp = NULL;
	  int upload_flag=0;
	  if(access(UPLOAD_FILE_PATH,0) != 0)
	  {
		  fprintf(stderr,"access(UPLOAD_FILE_PATH,0)\n"); 
		  memset(cmdcc,0,sizeof(cmdcc));
		  snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo touch %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		  system(cmdcc);
		  memset(cmdcc,0,sizeof(cmdcc));
		  snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo chmod 666 %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		  system(cmdcc);
	  }
	  fp = fopen(UPLOAD_FILE_PATH ,"w+");
	  if(NULL == fp)
	  {
		  fprintf(stderr,"(NULL == fp)\n"); 
		  ShowAlert(search(lpublic,"lupload_fail"));
		  upload_flag = 1;
	  }
	  else
	  {
		  fprintf(stderr,"f---puts(1, fp)\n"); 
		  fputs("1", fp);
		  fclose(fp);
		  memset(cmdcc,0,sizeof(cmdcc));
		  snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo chmod 666 %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		  system(cmdcc);
	  }

	  if(upload_flag == 0)
	  {

		  cgiFormStringNoNewlines("url",web_url,128);
		  cgiFormStringNoNewlines("usrname",usrname,30);
		  cgiFormStringNoNewlines("passwd",passwd,30);
		  if((strcmp(web_url,""))&&(strcmp(usrname,""))&&(strcmp(passwd,"")))
		  {
			  Web_Upgrade(web_url,usrname,passwd,lpublic,lsystem);
		  }
		  else
		  {
			  memset(cmdcc,0,sizeof(cmdcc));
			  snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo rm %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
			  system(cmdcc);
			  
			  if(strcmp(web_url,"")==0)
				  ShowAlert(search(lpublic,"url_not_empty"));
			  else if(strcmp(usrname,"")==0)
				  ShowAlert(search(lsystem,"userna_err"));
			  else if(strcmp(passwd,"")==0)
				  ShowAlert(search(lsystem,"pass_err"));
		  }
	  }
  }
  
  fprintf(cgiOut,"<form name=frmUpload method=\"POST\" enctype=\"multipart/form-data\">"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=300 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>"\
    "<td width=590 align=right valign=bottom background=/images/di22.jpg>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
	  
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
		  "<td width=62 align=center><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
     	  fprintf(cgiOut,"<td width=62 align=center><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"import_config"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"export_config"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"version_up"));  /*突出显示*/
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"log_info"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"l_user"));
					fprintf(cgiOut,"</tr>");
					//新增时间条目
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsystem,"systime"));
					fprintf(cgiOut,"</tr>");

					//新增NTP条目
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
					fprintf(cgiOut,"</tr>");

					//新增pppoe条目
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),"PPPOE");
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_pppoe_snp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),"PPPOE SNP");
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
				    "<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"web_service"));
				    fprintf(cgiOut,"</tr>");
                  for(i=0;i<0;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:12px\">"\
          "<table width=500 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr height=30>"\
		    "<td align=left width=70>%s:</td>",search(lpublic,"local_upload"));
			fprintf(cgiOut,"<td width=330>"\
				"<input type=\"file\" size=\"30\" name=\"myFile\" id=only style=\"position:absolute; filter:alpha(opacity=0); width:30px;\" onchange=\"document.frmUpload.txtFakeText.value = this.value;\" value=\"\"/>");
				  fprintf(cgiOut,"<input type=text size=\"32\" name=txtFakeText value=\"\" disabled/>");
				fprintf(cgiOut,"<input type=button style=\"width:70px;height:22px;\" name=fakeButton onmouseover=\"HandleFileButtonClick(document.frmUpload.myFile);\" value=\"%s\">",search(lpublic,"browse"));
			fprintf(cgiOut,"</td>"\
		    "<td width=100><input type=submit name=local_upload value=\"%s\" onclick=\"test('%s','%s','%s','%s','%s')\"></td>",search(lpublic,"local_upload"),search(lpublic,"locup_warn"),search(lpublic,"locup_stag_first"),search(lpublic,"locup_stag_sec"),search(lpublic,"locup_stag_third"),search(lpublic,"locup_stag_fourth"));
		  fprintf(cgiOut,"</tr>"\

		  "<tr height=30>"\
		  	"<td align=left>%s:</td>",search(lpublic,"web_source"));
	  		  fprintf(cgiOut,"<td><input type=text size=\"40\" name=url value=\"%s\"></td>",web_url);	
		    fprintf(cgiOut,"<td><input type=submit name=web_upload value=\"%s\" onclick=\"test('%s','%s','%s','%s','%s')\"></td>",search(lpublic,"web_up"),search(lpublic,"webup_warn"),search(lpublic,"webup_stag_first"),search(lpublic,"webup_stag_sec"),search(lpublic,"webup_stag_third"),search(lpublic,"webup_stag_fourth")); 
		  fprintf(cgiOut,"</tr>"\
		  "<tr height=30>"\
			"<td align=left>%s:</td>",search(lsystem,"user_na"));
			fprintf(cgiOut,"<td colspan=2><input type=text name=usrname size=22 style=\"width:200px;\" value=\"%s\"></td>\n",usrname);
		  fprintf(cgiOut,"</tr>"\
		  "<tr height=30>"\
	        "<td align=left>%s:</td>",search(lsystem,"password"));
	        fprintf(cgiOut,"<td colspan=2><input type=password name=passwd size=22 style=\"width:200px;\" value=\"%s\"></td>\n",passwd);
	      fprintf(cgiOut,"</tr>"\

		  "<tr><td colspan=3>&nbsp;</td></tr>"\
		  "<tr><td colspan=3><a href=wp_version_del.cgi?UN=%s><font color=blue size=2>&nbsp;&nbsp;&nbsp;%s</font></a></td></tr>",m,search(lpublic,"view_version"));
		  
		  fprintf(cgiOut,"<tr>"\
		    "<td colspan=3 id=sec1 style=\"padding-left:23px;width=600; border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
		  fprintf(cgiOut,"</tr>"\
		  "<tr style=\"padding-left:23px; padding-top:2px\">"\
			"<td colspan=3 style=font-size:14px;color:#FF0000>%s</td>",search(lpublic,"loc_des"));
		  fprintf(cgiOut,"</tr>"\
		  "<tr style=\"padding-left:23px; padding-top:2px\">"\
			"<td colspan=3 style=font-size:14px;color:#FF0000>%s</td>",search(lpublic,"web_des"));
		  fprintf(cgiOut,"</tr>"\
		  "<tr style=\"padding-left:23px; padding-top:2px\">"\
			"<td colspan=3 style=font-size:14px;color:#FF0000>%s</td>",search(lpublic,"view_des"));
		  fprintf(cgiOut,"</tr>"
			   
		  "<tr>"\
		    "<td><input type=hidden name=encry_verupgrade value=%s></td>",m);
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
"<script type=text/javascript>"\
"check_sysinfo_div_pos()"\
"</script>"\
"</html>");
return 0;
}

static int ccgi_dcli_bsd_get_slot_ids(DBusConnection *connection, int *ID, const int op)
{
    int ret = 0;
    DBusMessageIter  iter;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int i = 0;

    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME, BSD_DBUS_OBJPATH, \
        BSD_DBUS_INTERFACE, BSD_GET_ALIVE_SLOT_IDS);

    dbus_message_append_args(query,
    					DBUS_TYPE_UINT32,&op,
    					DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret != 0){
		for(i = 0; i < ret; i++)
		{
		    dbus_message_iter_next(&iter);
	        dbus_message_iter_get_basic(&iter,&(ID[i]));
		}
	}

    return ret;
}

static int ccgi_dcli_bsd_copy_file_to_board(DBusConnection *connection, const int slot_id, const char *src_path, const char *des_path, const int flag, const int op)
{   
    int ret = 0;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	char *resultMd5 = NULL;
    char *tmp_src_path = src_path;
    char *tmp_des_path = des_path;
    
    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
						BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
    
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_STRING,&tmp_src_path,
							 DBUS_TYPE_STRING,&tmp_des_path,
							 DBUS_TYPE_UINT32,&flag,
							 DBUS_TYPE_UINT32,&op,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (connection,query,120000, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_STRING,&resultMd5,
					DBUS_TYPE_INVALID)) {
		
	}
	dbus_message_unref(reply);
    
    return ret;
}

void Local_Upgrade(char *local_url,struct list *lpublic,struct list *lsystem)
{
	cgiFilePtr file;
	FILE *fp = NULL;
	char cmdcc[100]={0};

	char *ptr_s=NULL;
	char *ptr=NULL;
	char c='\\';
	fp = fopen(UPLOAD_FILE_PATH ,"w+");
	if(NULL == fp)
	{ 
		memset(cmdcc,0,sizeof(cmdcc));
		snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo rm %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		system(cmdcc);
		ShowAlert(search(lpublic,"lupload_fail"));
		return; 
	}
	fprintf(stderr,"-----------------------------fp--uts(2, fp)\n"); 
	fputs("2", fp);
	fclose(fp);
	memset(cmdcc,0,sizeof(cmdcc));
	snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo chmod 666 %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
	system(cmdcc);
	sleep(10);
	
	ptr_s = strrchr(local_url, c);
	if(ptr_s==NULL)
	{
		ptr = local_url;
	}
	else if((ptr_s+1)==NULL)
	{
		memset(cmdcc,0,sizeof(cmdcc));
		snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo rm %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		system(cmdcc);
		ShowAlert(search(lpublic,"lupload_fail"));
		return; 
	}
	else
	{
		ptr = ptr_s+1;
	}
	if (cgiFormFileOpen("myFile", &file) != cgiFormSuccess) 
	{
		memset(cmdcc,0,sizeof(cmdcc));
		snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo rm %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		system(cmdcc);
		ShowAlert(search(lpublic,"lupload_fail"));
		return;
	}  
	
	char *version_name[2]={".img",".IMG"};
	char *tmpz = NULL;
	int i=0,flag=-1;
	for(i=0;i<2;i++)
	{ 
	  tmpz = strstr(ptr,version_name[i]);
	  if(tmpz)
	  {
		flag = 0;
		break;
	  }
	}
	if(flag != 0)
	{
		memset(cmdcc,0,sizeof(cmdcc));
		snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo rm %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		system(cmdcc);
		ShowAlert(search(lpublic,"wtp_typez"));
	  	return;
	}	

	char path_conf[128] = { 0 };
	memset(path_conf,0,sizeof(path_conf));	
	snprintf(path_conf,sizeof(path_conf)-1,"/mnt/%s",ptr); 

	int status=0,tt_ret=-1;
	int targetFile = 0; 
	mode_t mode;
	char buffer[1024] = { 0 }; 
	int got = 0; 
	mode=S_IRWXU|S_IRGRP|S_IROTH; 
	//在当前目录下建立新的文件，第一个参数实际上是路径名，此处的含义是在cgi程序所在的目录（当前目录））建立新文件 
	targetFile=open(path_conf,O_RDWR|O_CREAT|O_TRUNC|O_APPEND,mode); 
	if(targetFile == -1)
	{ 
		memset(cmdcc,0,sizeof(cmdcc));
		snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo rm %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		system(cmdcc);
		ShowAlert(search(lpublic,"lupload_fail"));
		return; 
	} 
	//从系统临时文件中读出文件内容，并放到刚创建的目标文件中 
	while (cgiFormFileRead(file, buffer, 1024, &got) ==cgiFormSuccess)
	{ 
		if(got>0) 
		write(targetFile,buffer,got);
	} 

	cgiFormFileClose(file); 
	close(targetFile);


	fp = fopen(UPLOAD_FILE_PATH ,"w+");
	if(NULL == fp)
	{ 
		memset(cmdcc,0,sizeof(cmdcc));
		snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo rm %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		system(cmdcc);
		ShowAlert(search(lpublic,"lupload_fail"));
		return; 
	}
	fprintf(stderr,"-----------------------------fp--uts(3, fp)\n"); 
	fputs("3", fp);
	fclose(fp);
	memset(cmdcc,0,sizeof(cmdcc));
	snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo chmod 666 %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
	system(cmdcc);

	
	char tempf[128] = { 0 };
	memset(tempf,0,sizeof(tempf));
	snprintf(tempf,sizeof(tempf)-1,"sudo sor.sh cp %s %d > /dev/null 2>&1",ptr,300);
	status = system(tempf);
	tt_ret = WEXITSTATUS(status);	
	if(tt_ret==0)
	{
		int ret = -1;
		char sys_cmd[128] = { 0 };
		
		memset(sys_cmd,0,sizeof(sys_cmd));
		sprintf(sys_cmd,	
				"source vtysh_start.sh >/dev/null 2>&1\n"
				"vtysh -c \"show boot_img\" |grep '^%s$' >/dev/null"
				,ptr);

		status = 0;
		status = system(sys_cmd);
		ret = WEXITSTATUS(status);	
		fp = fopen(UPLOAD_FILE_PATH ,"w+");
		if ((ret == 0)||(NULL == fp))
		{	
			fprintf(stderr,"-----------------------------fp--uts(4, fp)\n"); 
			 fputs("4", fp);
			 fclose(fp);
			 memset(cmdcc,0,sizeof(cmdcc));
			 snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo chmod 666 %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
			 system(cmdcc);

			char src_path[PATH_LEN] = {0};
			char des_path[PATH_LEN] = {0};
			sprintf(src_path, "/blk/%s", ptr);
			sprintf(des_path, "/blk/%s", ptr);
   			int ID[MAX_SLOT_NUM] = {0};
			int board_count = -1;
		 	board_count =ccgi_dcli_bsd_get_slot_ids(ccgi_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
			for(i = 0; i < board_count; i++)
			{				
			 	ccgi_dcli_bsd_copy_file_to_board(ccgi_dbus_connection,ID[i],src_path,des_path,0,BSD_TYPE_BLK);
			}

			memset(sys_cmd,0,128);
			sprintf(sys_cmd,"sudo boot.sh %s >/dev/null 2>&1",ptr);
			system(sys_cmd);

			ShowAlert(search(lpublic,"lupload_succ"));
		}
		else
		{
			memset(sys_cmd,0,128);
			sprintf(sys_cmd,"sudo sor.sh rm %s %d > /dev/null 2>&1",ptr,SHORT_SORT);
			system(sys_cmd);
			
			ShowAlert(search(lpublic,"lupload_fail"));			
		}
	}
	memset(tempf,0,sizeof(tempf));
	snprintf(tempf,sizeof(tempf)-1,"sudo rm /mnt/%s > /dev/null 2>&1",ptr);
	system(tempf);
	
	memset(cmdcc,0,sizeof(cmdcc));
	snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo rm %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
	system(cmdcc);
	return;
}


void Web_Upgrade(char *web_url,char *usrname,char *passwd,struct list *lpublic,struct list *lsystem)
{
	FILE *fp = NULL;
	char cmdcc[100]={0};
	sleep(6);

	char *filename=strrchr(web_url,'/');		
	if(filename)
	{
		char *version_name[2]={".img",".IMG"};
		char *tmpz = NULL;
		int i=0,flag=-1;
		for(i=0;i<2;i++)
		{ 
		  tmpz = strstr(filename+1,version_name[i]);
		  if(tmpz)
		  {
			flag = 0;
			break;
		  }
		}
		if(flag != 0)
		{
			ShowAlert(search(lpublic,"wtp_typez"));
			return;
		}	

		fp = fopen(UPLOAD_FILE_PATH ,"w+");
		if(NULL == fp)
		{ 
			memset(cmdcc,0,sizeof(cmdcc));
			snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo rm %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
			system(cmdcc);
			ShowAlert(search(lpublic,"lupload_fail"));
			return; 
		}
		fprintf(stderr,"-----------------------------fp--uts(2, fp)\n"); 
		fputs("2", fp);
		fclose(fp);
		memset(cmdcc,0,sizeof(cmdcc));
		snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo chmod 666 %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
		system(cmdcc);
		
		int ret = -1;	
		char cmd[128] = { 0 };
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"wget -N -P /mnt --user=%s --password=%s %s > /dev/null",usrname,passwd,web_url);	//下载到指定目录，并保持最新版本
		ret=system(cmd);
		fp = fopen(UPLOAD_FILE_PATH ,"w+");
		if((ret==0)||(NULL == fp))
		{			
			fprintf(stderr,"-----------------------------fp--uts(3, fp)\n"); 
			fputs("3", fp);
			fclose(fp);
			memset(cmdcc,0,sizeof(cmdcc));
			snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo chmod 666 %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
			system(cmdcc);

			
			char tempf[128] = { 0 };
			int op_ret = -1,status = -1;
	
			memset(tempf,0,sizeof(tempf));			
			sprintf(tempf,"sudo sor.sh cp %s %d > /dev/null 2>&1",filename+1,300);
			status = system(tempf);
			op_ret = WEXITSTATUS(status);
			if(op_ret==0)
			{
				char sys_cmd[128] = { 0 };
				
				ret = -1;	
				status = -1;
				memset(sys_cmd,0,128);
				sprintf(sys_cmd,	
					"source vtysh_start.sh >/dev/null 2>&1\n"
					"vtysh -c \"show boot_img\" |grep '^%s$' >/dev/null"
					,filename+1);
				status = system(sys_cmd);
				ret = WEXITSTATUS(status);
				fp = fopen(UPLOAD_FILE_PATH ,"w+");
				if((ret==0)||(NULL == fp))
				{
					fprintf(stderr,"-----------------------------fp--uts(4, fp)\n"); 
					 fputs("4", fp);
					 fclose(fp);
					 memset(cmdcc,0,sizeof(cmdcc));
					 snprintf(cmdcc, sizeof(cmdcc) - 1, "sudo chmod 666 %s >/dev/null 2>&1", UPLOAD_FILE_PATH);
					 system(cmdcc);

					char src_path[PATH_LEN] = {0};
					char des_path[PATH_LEN] = {0};
					sprintf(src_path, "/blk/%s", filename+1);
					sprintf(des_path, "/blk/%s", filename+1);
					int ID[MAX_SLOT_NUM] = {0};
					i =0;
					int board_count = -1;
					board_count =ccgi_dcli_bsd_get_slot_ids(ccgi_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
					for(i = 0; i < board_count; i++)
					{					
						ccgi_dcli_bsd_copy_file_to_board(ccgi_dbus_connection,ID[i],src_path,des_path,0,BSD_TYPE_BLK);
					}
					
					memset(tempf,0,sizeof(tempf));
					sprintf(tempf,"sudo boot.sh  %s >/dev/null 2>&1",filename+1);		
					system(tempf); 
					
					ShowAlert(search(lpublic,"wupload_succ"));
				}
				else
				{
					memset(sys_cmd,0,128);
					sprintf(sys_cmd,"sudo sor.sh rm %s %d > /dev/null 2>&1",filename+1,SHORT_SORT);
					system(sys_cmd);
					
					ShowAlert(search(lpublic,"wupload_fail"));			
				}
			}
			else
			{
				ShowAlert(search(lpublic,"wupload_fail"));
			}			
			memset(cmd,0,sizeof(cmd));
			snprintf(cmd,sizeof(cmd)-1,"sudo rm /mnt/%s > /dev/null 2>&1",filename+1);
			system(cmd);	
			return;
		}
		else
		{
			ShowAlert(search(lpublic,"wupload_fail"));
			return;
		}
	}	
	else 
	{
		ShowAlert(search(lpublic,"wupload_fail"));
		return;
	}		
}

