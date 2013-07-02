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
* wp_impconf.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for import file
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <stdlib.h>
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_public.h"
#include "ws_version_param.h"
#include <sys/wait.h>

int ShowImportPage(struct list *lpublic, struct list *lsystem);     /*m代表加密后的字符串*/
int getFileName(char *fname,char *fpath);     /*获得上传图片的名字fname及全名fpath*/
int upfile(char *fpath,struct list *lpublic, struct list *lsystem,int type);          
int right_version_conf();
int right_filesize();
int upfile_temp(char *fpath,struct list *lpublic, struct list *lsystem,int type);     
void delete_linefeed(char * string);


int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt");

	ShowImportPage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem);

	return 0;
}

int ShowImportPage(struct list *lpublic, struct list *lsystem)
{ 
  
  char *encry=(char *)malloc(BUF_LEN);      		
  char *str;

  char select[N];//options 
  
  int i;
  char imp_encry[BUF_LEN];  
  if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
  {
      memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user"));          /*用户非法*/
      return 0;
    }
    memset(imp_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
  cgiFormStringNoNewlines("encry_import",imp_encry,BUF_LEN);
  /***********************2008.5.26*********************/
  cgiHeaderContentType("text/html");

  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lsystem,"import_config"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  "<script type=text/javascript>"\
     "function check_sysinfo_div_pos()\n"\
     "{"\
     "	    var   oRect   =   document.all.only.getBoundingClientRect();\n"\
     "  var obj_div=document.getElementById('only');\n"\
     "  obj_div.style.top = oRect.top+3;\n"\
     "  obj_div.style.left = oRect.left+204;\n"\
     "}\n"\
     "</script>"\
   "<script type=\"text/javascript\">\n"\
   "function HandleFileButtonClick(obj)\n"\
   "{\n" \
   "   with(obj){\n"\
   "	    var   oRect   =   document.all.fakeButton.getBoundingClientRect();\n"\
   "     style.posTop=oRect.top+3;\n"\
   "     style.posLeft=oRect.left+2;\n"\
   "	  }\n"\
   "}\n"\
   "</script>\n"\
  "<body onResize=check_sysinfo_div_pos()>");
	if(cgiFormSubmitClicked("upload_file") == cgiFormSuccess)
	{
		memset(select,0,N); 
		cgiFormStringNoNewlines("filetype",select,N);
		char fpath[PATH_LENG] = {0};
		char fname[PATH_LENG] = {0};
		int ret_fpath;
		ret_fpath=getFileName(fname,fpath);
		if(ret_fpath==1)
		{
		int ret_upfile=-1;
		if(strcmp(select,"1")==0)
		{
			upfile(fpath,lpublic,lsystem,1);
		}
		else if(strcmp(select,"2")==0)
		{
			upfile(fpath,lpublic,lsystem,2);	  	
		}
		}
		else
		{
			ShowAlert(search(lpublic,"no_upload"));
		}
	} 
  fprintf(cgiOut,"<form method=post encType=multipart/form-data name=frmUpload>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
"<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
  
  
 
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>"\
	"<td width=62 align=center><input id=but type=submit name=upload_file style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));

	if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
	{
	  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));

	}
	else
	{
		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",imp_encry,search(lpublic,"img_cancel"));
	}
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
				if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
				{					 
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
					fprintf(cgiOut,"</tr>"\
					"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"import_config"));  /*突出显示*/
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));
					fprintf(cgiOut,"</tr>");

					//新增条目
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
					fprintf(cgiOut,"</tr>");

					//boot upgrade 
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
					fprintf(cgiOut,"</tr>");



					/*日志信息*/
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"log_info"));
					fprintf(cgiOut,"</tr>");


					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"l_user"));
					fprintf(cgiOut,"</tr>");


					//新增时间条目
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
					fprintf(cgiOut,"</tr>");

					//新增NTP条目
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
					fprintf(cgiOut,"</tr>");

					//新增pppoe条目
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"PPPOE");
					fprintf(cgiOut,"</tr>");
				}
				else if(cgiFormSubmitClicked("upload_file") == cgiFormSuccess)			
				{
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_sysinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lsystem,"sys_infor"));
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
					fprintf(cgiOut,"</tr>"\
					"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"import_config"));  /*突出显示*/
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));
					fprintf(cgiOut,"</tr>");
					//新增条目
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
					fprintf(cgiOut,"</tr>");

					//boot upgrade 
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_boot_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lsystem,"boot_item"));
					fprintf(cgiOut,"</tr>");


					/*日志信息*/
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_log_info.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lpublic,"log_info"));
					fprintf(cgiOut,"</tr>");


					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_login_limit.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lpublic,"l_user"));
					fprintf(cgiOut,"</tr>");


					//新增时间条目
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_showtime.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lsystem,"systime"));
					fprintf(cgiOut,"</tr>");

					//新增NTP条目
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_ntp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lpublic,"ntp_s"));
					fprintf(cgiOut,"</tr>");

					//新增pppoe条目
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_pppoe_server.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),"PPPOE");
					fprintf(cgiOut,"</tr>");
				}
				for(i=0;i<2;i++)
				{
				  fprintf(cgiOut,"<tr height=25>"\
					"<td id=tdleft>&nbsp;</td>"\
				  "</tr>");
				}
			  fprintf(cgiOut,"</table>"\
			"</td>"\
			"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                "<table width=480 border=0 cellspacing=0 cellpadding=0>"\
			  "<tr>"\
				"<td align=left>");

			  ////select options
              fprintf(cgiOut,"<select name=filetype>\n");
			  fprintf(cgiOut,"<option value=1>%s</option>\n",search(lsystem,"import"));
			  fprintf(cgiOut,"<option value=2>%s</option>\n",search(lsystem,"import_ln"));
			  fprintf(cgiOut,"</select>\n"); 
				fprintf(cgiOut,"<input type=\"file\" size=\"30\" name=\"myFile\" id=only style=\"position:absolute;filter:alpha(opacity=0);width:30px;\" onchange='document.frmUpload.txtFakeText.value = this.value;' />");
			  
			  fprintf(cgiOut,"<input type=text size=\"32\" name=txtFakeText value=\"\" disabled/>");
              fprintf(cgiOut,"<input type=button style=\"width:70px;height:22px;\"   name=fakeButton onmouseover=\"HandleFileButtonClick(document.frmUpload.myFile);\" value=\"%s\">",search(lpublic,"browse"));

			  
			  ////
				fprintf(cgiOut,"<table width=350 border=0 cellspacing=0 cellpadding=0>");
	  
			   if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
			   {
				 fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=encry_import value=%s></td></tr>",encry);
			   }
			   else if(cgiFormSubmitClicked("upload_file") == cgiFormSuccess)
			   {
				 fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=encry_import value=%s></td></tr>",imp_encry);
			   }	 
			   fprintf(cgiOut,"</table>"\
			    "</td>"\
			  "</tr>");
			   fprintf(cgiOut,"<tr height=15><td></td></tr>\n");
			   fprintf(cgiOut,"<tr>"\
			   "<td  id=sec1 style=\"padding-left:23px;width=500; border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
			   fprintf(cgiOut,"</tr>");
			   fprintf(cgiOut,"<tr height=7><td></td></tr>\n");
			  
				   fprintf(cgiOut,"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
					 "<td style=font-size:14px;color:#FF0000>%s</td>"\
				   "</tr>",search(lpublic,"imp_des"));

				   	fprintf(cgiOut,"<tr height=7><td></td></tr>\n");

				    fprintf(cgiOut,"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
					 "<td style=font-size:14px;color:#FF0000>%s</td>"\
				   "</tr>",search(lsystem,"imp_ln_des"));
			   

fprintf(cgiOut,"<tr height=50></tr>"\
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
free(encry); 
return 0;
}          



int getFileName(char *fname,char *fpath)        /*返回0表示失败，返回1表示成功*/
{
  cgiFilePtr file;
  char name[1024];    /*存放本地路径名*/ 
  char *tmpStr=NULL;   
  char *end=NULL;
  int i,t; 
  if (cgiFormFileName("myFile", name, sizeof(name)) != cgiFormSuccess) 
    return 0;
  
  if (cgiFormFileOpen("myFile", &file) != cgiFormSuccess) 
  {
    return 0;
  }
  
  t=-1; 
  //从路径名解析出用户文件名 
  while(1){ 
  tmpStr=strstr(name+t+1,"\\");   /*在name+t+1中寻找"\\"*/
  if(NULL==tmpStr)                /*成功返回位置，否则返回NULL*/
  tmpStr=strstr(name+t+1,"/");    /*  if "\\" is not path separator, try "/"   */
  if(NULL!=tmpStr) 
    t=(int)(tmpStr-name);         /*如果找到，t存储偏移地址*/       
  else 
    break;  
  } 
  strncpy(fpath,name+t+1,PATH_LENG);     /*将文件全名赋给fpath*/ 
  end=strstr(fpath,".");		 /*在fpath中寻找"." */
  if(end != NULL)				 /*成功返回位置，否则返回NULL*/
  {
	t=(int)(end-fpath); 		 /*如果找到，t存储偏移地址*/
	for(i=0;i<t;i++)
	  fname[i]=fpath[i];
	fname[i]='\0';
  }   
  return 1;
}

int upfile(char *fpath,struct list *lpublic, struct list *lsystem,int type)        /*成功 返回1，否则返回0，返回-1表示没有文件上传*/
{

    upfile_temp(fpath,lpublic,lsystem,type);
	char cmd[128];
	memset(cmd,0,128);
	char tempf[128] = {0};
	int status=0,tt_ret=-1;

	sprintf(cmd,"sudo cp %s %s > /dev/null",CONF_TEMP_S,CONF_XML_S);
    if(type==1)
    { 
		system(cmd);
		memset(tempf,0,128);

		sprintf(tempf,"sudo sor.sh cp %s %d > /dev/null","conf_xml.conf",SHORT_SORT);

		status = system(tempf);
		tt_ret = WEXITSTATUS(status);


		if(tt_ret==0)
		{
			ShowAlert(search(lpublic,"upload_succ"));
		}
		else
		{
			ShowAlert(search(lpublic,"upload_fail"));
		}

    }
	else if(type==2)
	{

		sprintf(tempf,"sudo sor.sh cp %s %d > /dev/null","license.txt",SHORT_SORT);
		status = system(tempf);
		tt_ret = WEXITSTATUS(status);


		if(tt_ret==0)
			ShowAlert(search(lpublic,"upload_succ"));
		else
			ShowAlert(search(lpublic,"upload_fail"));
	}
	
  return 1;
}


int upfile_temp(char *fpath,struct list *lpublic, struct list *lsystem,int type)        /*成功 返回1，否则返回0，返回-1表示没有文件上传*/
{
	char path_conf[128] = {0};
	
	if(type==1)
	strncpy(path_conf,CONF_TEMP_S,sizeof(path_conf));
	else if(type==2)
	strncpy(path_conf,"/mnt/license.txt",sizeof(path_conf));

	cgiFilePtr file;
	char name[1024];    /*存放本地路径名*/ 
	int targetFile; 
	mode_t mode;
	char buffer[1024]; 
	int got; 
	if (cgiFormFileName("myFile", name, sizeof(name)) != cgiFormSuccess) 
	{
		return -1; 
	}

	if (cgiFormFileOpen("myFile", &file) != cgiFormSuccess) 
	{
		return 0;
	}  

	mode=S_IRWXU|S_IRGRP|S_IROTH; 
	//在当前目录下建立新的文件，第一个参数实际上是路径名，此处的含义是在cgi程序所在的目录（当前目录））建立新文件 
	targetFile=open(path_conf,O_RDWR|O_CREAT|O_TRUNC|O_APPEND,mode); 
	if(targetFile == -1)
	{ 
		return 0; 
	} 
	//从系统临时文件中读出文件内容，并放到刚创建的目标文件中 
	while (cgiFormFileRead(file, buffer, 1024, &got) ==cgiFormSuccess)
	{ 
		if(got>0) 
		write(targetFile,buffer,got);   /*将本地文件的内容写入服务器端文件*/
	} 
	cgiFormFileClose(file); 
	close(targetFile); 	
	return 0;
}

/*the conf file is or not right conf*/
int right_version_conf()
{
int flag=-1;
char outkey[128];
memset(outkey,0,128);

//version       
	char get_sw_ver_cmd[256]="";
    char version[30];
	memset(version,0,30);

	if(access("/etc/version/version",0)==0)
    {
    	sprintf( get_sw_ver_cmd, "if [ -f /etc/version/version ];then cat /etc/version/version 2>/dev/null;fi" );
    	GET_CMD_STDOUT(version,sizeof(version),get_sw_ver_cmd);
    }

   flag=find_conf_node(CONF_TEMP_S, CONF_VERSION,outkey);

   delete_linefeed(&outkey);
   delete_linefeed(&version);

   if(strcmp(outkey,"")!=0)
   	{
	   if(strcmp(outkey,version)==0)
	   {
	   	   return 0;
	   }
	   else
	   {   
	   	   return -1;
	   }
   	}
   else
   	{
   		return 0;
   	}
}

/*right filesize*/
int right_filesize()
{
int flag=-1;
char outkey[128];
memset(outkey,0,128);
int sum=0,key=0;

flag=find_conf_node(CONF_TEMP_S, CONF_FILESIZE,outkey);
del_conf_node(CONF_TEMP_S,CONF_VERSION);
del_conf_node(CONF_TEMP_S,CONF_FILESIZE);
sum=get_file_bytesum(CONF_TEMP_S);
key=strtoul(outkey,0,10);
   if(key==sum)
   {
   	   return 0;
   }
   else
   {   
   	   return -1;
   }
}

void delete_linefeed(char * string)
{
	int len = 0;
	len = strlen(string);
    int len_l = 0;
	if(string == NULL)
		return;
	char * tmp = string;
	while(*tmp != '\n')
	{
		len_l++;
		if(len_l >= len)
			return;
		tmp++;
	}
	*tmp = '\0';	
}

