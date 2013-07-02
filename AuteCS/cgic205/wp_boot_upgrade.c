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
* wp_boot_upgrade.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for boot file upgrade
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

#include "ws_sysinfo.h"

#define LENG 512

int ShowVersionUpgradePage(struct list *lpublic, struct list *lsystem);     /*m代表加密后的字符串*/
int getFileNameboot(char *fpath);     /*获得上传图片的名字fname及全名fpath*/
int upfileboot(char *fpath,struct list *lpublic, struct list *lsystem);           /*上传文件全名为fpath的配置文件*/
int locupboot(struct list *lpublic, struct list *lsystem,char *encry) ;

//**********
int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt");

	ShowVersionUpgradePage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem);

	return 0;
}

int ShowVersionUpgradePage(struct list *lpublic, struct list *lsystem)
{ 
 
  char *encry=(char *)malloc(BUF_LEN);      		
  char *str;

  
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
  }else{
  
  cgiFormStringNoNewlines("encry_import",imp_encry,BUF_LEN);
  str=dcryption(imp_encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user"));          /*用户非法*/
      return 0;
    }
    memset(imp_encry,0,BUF_LEN);  

  	}

  cgiFormStringNoNewlines("encry_import",imp_encry,BUF_LEN);
  cgiFormStringNoNewlines("UN",encry,BUF_LEN);
  /***********************2008.5.26*********************/
  cgiHeaderContentType("text/html");

  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lpublic,"version_up"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  	"<style type=text/css>\n"\
  	".a3{width:30;border:0; text-align:center}\n"\
  	"</style>\n"\
   "</head>\n"\
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
   "</script>\n");  
  fprintf(cgiOut,"<body onResize=check_sysinfo_div_pos()>\n");
  char * procductId=readproductID();

	if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
	{   
	    if(cgiFormSubmitClicked("local_upload") == cgiFormSuccess)
		{
			locupboot(lpublic,lsystem,encry);
    	}
	}
	else
	{  
			locupboot(lpublic,lsystem,encry);
	}
   
   	  
     //* * * * * * * * * * * * * * * *  * * * * * * * * *  
       
  fprintf(cgiOut,"<form method=post encType=multipart/form-data name=frmUpload>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
"<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
  fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
 
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>");
	fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=upload_file style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	//fprintf(cgiOut,"<td width=62 align=left><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame><img src=/images/ok-ch.jpg border=0 width=62 height=20/></a></td>",encry);

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
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));
					fprintf(cgiOut,"</tr>");

					//version upgrade
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
					fprintf(cgiOut,"</tr>");

					//boot upgrade
					fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"boot_item"));  /*突出显示*/
					fprintf(cgiOut,"</tr>");


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
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_sysconfig.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lsystem,"sys_config"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_impconf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lsystem,"import_config"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_export.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",imp_encry,search(lpublic,"menu_san"),search(lsystem,"export_config"));
					fprintf(cgiOut,"</tr>");

					//version upgrade
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_version_upgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"version_up"));
					fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsystem,"boot_item"));  /*突出显示*/
					fprintf(cgiOut,"</tr>");


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
				for(i=0;i<3;i++)
				{
					fprintf(cgiOut,"<tr height=25>"\
					"<td id=tdleft>&nbsp;</td>"\
					"</tr>");
				}
				fprintf(cgiOut,"</table>"\
				"</td>");
			  
				fprintf(cgiOut,"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
				"<table width=480 border=0 cellspacing=0 cellpadding=0>");
				fprintf(cgiOut,"<tr height=25><td></td></tr>");
				fprintf(cgiOut,"<tr>"\
				"<td align=left>"\
				"<p>%s:&nbsp;<input type=\"file\" size=\"30\" name=\"myFile\" id=only style=\"position:absolute;filter:alpha(opacity=0);width:30px;\" onchange='document.frmUpload.txtFakeText.value = this.value;' value=\"\" />",search(lpublic,"local_upload"));

				fprintf(cgiOut,"<input type=text size=\"32\" name=txtFakeText value=\"\" disabled/>");

				fprintf(cgiOut,"<input type=button style=\"width:70px;height:22px;\"   name=fakeButton onmouseover=\"HandleFileButtonClick(document.frmUpload.myFile);\" value=\"%s\"></p>",search(lpublic,"browse"));

				fprintf(cgiOut,"<td><input type=submit name=local_upload value=%s ></td>",search(lpublic,"local_upload"));	

				if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
				{
					fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=encry_import value=%s></td></tr>",encry);
				}
				else if(cgiFormSubmitClicked("upload_file") == cgiFormSuccess || cgiFormSubmitClicked("")==cgiFormSuccess || cgiFormSubmitClicked("")==cgiFormSuccess)
				{
					fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=encry_import value=%s></td></tr>",imp_encry);

				}	 
				fprintf(cgiOut,"<tr height=6><td></td></tr>");

				fprintf(cgiOut,"</td>");
				fprintf(cgiOut,"</tr>");	

				fprintf(cgiOut,"<tr height=20><td></td></tr>");	

				fprintf(cgiOut,"<tr>"\
				"<td  id=sec1 style=\"padding-left:23px;width=600; border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
				fprintf(cgiOut,"</tr>");

				fprintf(cgiOut,"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
				"<td style=font-size:14px;color:#FF0000></td>"\
				"</tr>");
				fprintf(cgiOut,"<tr height=25 style=\"padding-left:23px; padding-top:2px\">"\
				"<td style=font-size:14px;color:#FF0000>%s</td>"\
				"</tr>",search(lsystem,"boot_des"));
               
			   
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
free(procductId);
free(encry); 
return 0;
}          



int getFileNameboot(char *fpath)        /*返回0表示失败，返回1表示成功*/
{
  cgiFilePtr file;
  char name[1024];    /*存放本地路径名*/ 
  char *tmpStr=NULL;   
  int t;
  if (cgiFormFileName("myFile", name, sizeof(name)) != cgiFormSuccess) 
    return 0;
  
  if (cgiFormFileOpen("myFile", &file) != cgiFormSuccess) 
  {
	fprintf(stderr, "Could not open the file.<p>\n");
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
  strcpy(fpath,name+t+1);     /*将文件全名赋给fpath*/ 

  return 1;
}


int upfileboot(char *fpath,struct list *lpublic, struct list *lsystem)        /*成功 返回1，否则返回0，返回-1表示没有文件上传*/
{

	//sprintf(path_conf,"/mnt/%s",fpath);	

	cgiFilePtr file;
	char name[1024];    /*存放本地路径名*/ 
	int targetFile; 
	mode_t mode;

	char buffer[1024]; 
	int got; 
	if (cgiFormFileName("myFile", name, sizeof(name)) != cgiFormSuccess) 
	return -1; 

	if (cgiFormFileOpen("myFile", &file) != cgiFormSuccess) 
	{
		fprintf(stderr, "Could not open the file.<p>\n");
		return 0;
	}  

	mode=S_IRWXU|S_IRGRP|S_IROTH; 
	//在当前目录下建立新的文件，第一个参数实际上是路径名，此处的含义是在cgi程序所在的目录（当前目录））建立新文件 
	targetFile=open(BOOT_TEMP_Z,O_RDWR|O_CREAT|O_TRUNC|O_APPEND,mode); 
	if(targetFile == -1)
	{ 
		fprintf(stderr,"could not create the new file,%s\n",fpath); 
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


	return 1;
}

//本地上传函数
int locupboot(struct list *lpublic, struct list *lsystem,char *encry) 
{
	char cmd[128];
	memset(cmd,0,128);

	char *fpath = (char *)malloc(PATH_LENG);
	int ret_fpath;
	memset(fpath,0,PATH_LENG);
	ret_fpath=getFileNameboot(fpath);
	if(ret_fpath==1)
	{
		int ret_upfileboot,ret=-3;

		ret_upfileboot=upfileboot(fpath,lpublic,lsystem);	
		ret=write_to_boorom(BOOT_TEMP_Z);

		if(ret_upfileboot==1)
		{
			if (ret==0)
				ShowAlert(search(lpublic,"lupload_succ"));
			else if(ret==-2)
				ShowAlert(search(lsystem,"boot_nov"));
			else if(ret==-1)
				ShowAlert(search(lsystem,"boot_filefail"));
				
		}
		else 
		    ShowAlert(search(lpublic,"lupload_fail"));

	}
	else
	{
		ShowAlert(search(lpublic,"no_upload"));
	}

	return 0;
}


