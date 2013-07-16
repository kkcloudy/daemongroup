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
* wp_wtpdown.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
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
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include <fcntl.h> 
#include <sys/stat.h>  
#include <sys/wait.h>



int ShowdownPage(char *m,struct list *lpublic,struct list *lwlan,struct list *lsystem);    
void down_wtp_ver(struct list * lpublic,struct list *lwlan,struct list *lsystem);
//int getFileNamewtp(char *fpath);    
int upfilewtp();          
int locupwtp(struct list *lpublic, struct list *lsystem) ;


int cgiMain()
{  
  char encry[BUF_LEN] = { 0 };
  char *str = NULL; 			   
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  struct list *lsystem = NULL;     /*解析wlan.txt文件的链表头*/
  memset(encry,0,sizeof(encry));
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  lsystem=get_chain_head("../htdocs/text/system.txt");
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	str=dcryption(encry);
	if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
    else
      ShowdownPage(encry,lpublic,lwlan,lsystem);
  }
  else
  {    
	cgiFormStringNoNewlines("encry_newwtp",encry,BUF_LEN);
	str=dcryption(encry);
	if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user"));			/*用户非法*/
    else
      ShowdownPage(encry,lpublic,lwlan,lsystem);
  } 
  release(lpublic);  
  release(lwlan);
  release(lsystem);
  return 0;
}

int ShowdownPage(char *m,struct list *lpublic,struct list *lwlan,struct list *lsystem)
{   
  int i = 0;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
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
	"</script>\n"\
	"<body onResize=check_sysinfo_div_pos()>");

  if(cgiFormSubmitClicked("local_upload") == cgiFormSuccess)
  { 
	locupwtp(lpublic, lsystem);
  }
  
   if(cgiFormSubmitClicked("sup") == cgiFormSuccess)
  { 

	down_wtp_ver(lpublic,lwlan,lsystem);
   
  }

  fprintf(cgiOut,"<form method=post encType=multipart/form-data name=frmUpload>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=wtp_down style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
        fprintf(cgiOut,"<td width=62 align=center><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
  					"<td align=left id=tdleft><a href=wp_wtplis.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
                  fprintf(cgiOut,"</tr>"\
  				  "<tr height=25>"\
                    "<td align=left id=tdleft ><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></td>",m,search(lpublic,"menu_san"),search(lpublic,"create")); 
				  fprintf(cgiOut,"</tr>");
				   fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpver.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"mode"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=26>"\
  					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"version_upload"));        /*突出显示*/         
                  fprintf(cgiOut,"</tr>");	
				  fprintf(cgiOut,"<tr height=25>"\
				  	"<td align=left id=tdleft><a href=wp_verbind.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"version_bind"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpupgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"batch_update"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_showAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"show_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_conAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                  fprintf(cgiOut,"</tr>");
                  for(i=0;i<0;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
			  "<table width=750 border=0 cellspacing=0 cellpadding=0>");

			  fprintf(cgiOut,"<tr>"\
				"<td align=left width=70>%s</td>",search(lpublic,"local_upload"));
			  fprintf(cgiOut,"<td width=330>");
			  fprintf(cgiOut,"<input type=\"file\" size=\"30\" name=\"myFile\" id=only style=\"position:absolute;filter:alpha(opacity=0);width:30px;\" onchange='document.frmUpload.txtFakeText.value = this.value;' vale=\"\" />");
			  fprintf(cgiOut,"<input type=text size=\"32\" name=txtFakeText value=\"\" disabled/>");
              fprintf(cgiOut,"<input type=button style=\"width:70px;height:22px;\"   name=fakeButton onmouseover=\"HandleFileButtonClick(document.frmUpload.myFile);\" value=\"%s\">",search(lpublic,"browse"));
			  fprintf(cgiOut,"</td>");
			  fprintf(cgiOut,"<td><input type=submit name=local_upload value=%s ></td>",search(lpublic,"local_upload"));	
              fprintf(cgiOut,"</tr>");				

    fprintf(cgiOut,"<tr height=30>"\
    "<td align=left width=70>%s:</td>",search(lpublic,"web_source"));
fprintf(cgiOut,"<td width=330><input type=text name=url size=50 ></td>");
fprintf(cgiOut,"<td><input type=submit name=sup value=%s></td>",search(lpublic,"web_up"));
fprintf(cgiOut,"</tr>"\
"<tr>"\
	"<td align=left width=70>%s:</td>",search(lsystem,"user_na"));
fprintf(cgiOut,"<td colspan=2><input type=text name=usr size=20.8></td>");
fprintf(cgiOut,"</tr>"\
 "<tr height=30>"\
				"<td align=left width=70>%s:</td>",search(lsystem,"password"));
fprintf(cgiOut,"<td colspan=2><input type=password name=pawd size=22></td>"\
	"</tr>");

fprintf(cgiOut,"<tr height=10><td>"\
      "<a href=wp_ap_del.cgi?UN=%s><font color=blue size=2>%s</font></a></td></tr>",m,search(lpublic,"view_version"));	

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
"<script type=text/javascript>"\
"check_sysinfo_div_pos()"\
"</script>"\
"</html>");		
return 0;
}
void down_wtp_ver(struct list * lpublic,struct list *lwlan,struct list *lsystem)
{
	char url[100] = { 0 };
	char usrname[50] = { 0 };
	char passwd[50] = { 0 };
	char temp[100] = { 0 };
	memset(url,0,sizeof(url));
	memset(usrname,0,sizeof(usrname));
	memset(passwd,0,sizeof(passwd));
	memset(temp,0,sizeof(temp));

	DcliWInit();
	ccgi_dbus_init();
	cgiFormStringNoNewlines("url",url,100);  
	cgiFormStringNoNewlines("usr",usrname,50);
	cgiFormStringNoNewlines("pawd",passwd,50);
	
	char *version_name[9]={".bin",".tar",".img",".BIN",".TAR",".IMG",".gni",".GNI",".tar.bz2"};
	char *tmpz = NULL;
	int i=0,flag=-1;	

	for(i=0;i<9;i++)
	{ 
	  tmpz = strstr(url,version_name[i]);
	  if(tmpz)
	  {
	    flag = 0;
	  	break;
	  }
	}
	if(flag != 0)
	{
      ShowAlert(search(lpublic,"wtp_typez"));
	}	
    else
	{
		if((strcmp(url,"")!=0) && (strcmp(usrname,"")!=0) &&(strcmp(passwd,"")!=0))
		{
			if(replace_url(url,"#","%23") != NULL)
				strncpy(temp,replace_url(url,"#","%23"),sizeof(temp)-1);
			else
				strncpy(temp,url,sizeof(temp)-1);	

			int ret = download_ap_version(temp,usrname,passwd);
			if(ret == 0)
			{
				ShowAlert(search(lpublic,"upload_succ"));
			}
			else
			{
				ShowAlert(search(lpublic,"upload_fail"));
			}
		}
		else
		{
			if(strcmp(url,"")==0)
				ShowAlert(search(lpublic,"web_serr"));
			else if(strcmp(usrname,"")==0)
				ShowAlert(search(lsystem,"userna_err"));
			else if(strcmp(passwd,"")==0)
				ShowAlert(search(lsystem,"pass_err"));
		}
	}
}

#if 0
int getFileNamewtp(char *fpath)        /*返回0表示失败，返回1表示成功*/
{
  cgiFilePtr file;
  char name[1024] = { 0 };    /*存放本地路径名*/ 
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
  while(1)
  { 
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
#endif

int upfilewtp()        /*成功 返回1，否则返回0，返回-1表示没有文件上传，返回-3表示文件类型不符合要求*/
{
	char name[1024] = { 0 };    /*存放本地路径名*/ 	
	cgiFilePtr file;
	
	if (cgiFormFileName("myFile", name, sizeof(name)) != cgiFormSuccess) 
		return -1; 

	if (cgiFormFileOpen("myFile", &file) != cgiFormSuccess) 
	{
		fprintf(stderr, "Could not open the file.<p>\n");
		return 0;
	}  
	
	char *version_name[9]={".bin",".tar",".img",".BIN",".TAR",".IMG",".gni",".GNI",".tar.bz2"};
	char *tmpz = NULL;
	int i=0,flag=-1;

	for(i=0;i<9;i++)
	{ 
	  tmpz = strstr(name,version_name[i]);
	  if(tmpz)
	  {
	    flag = 0;
	  	break;
	  }
	}
	if(flag != 0)
	{
	  return -3;
	}	

	char path_conf[128] = { 0 };
	memset(path_conf,0,sizeof(path_conf));
	char tempf[128] = { 0 };
	
    snprintf(path_conf,sizeof(path_conf)-1,"/mnt/wtp/%s",name);	

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
		fprintf(stderr,"could not create the new file,%s\n",name); 
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

    memset(tempf,0,sizeof(tempf));
	snprintf(tempf,sizeof(tempf)-1,"sudo sor.sh cp wtp/%s %d > /dev/null",name,LONG_SORT);
	status = system(tempf);
    tt_ret = WEXITSTATUS(status);

	if(tt_ret==0)
	{
		return 1;
	}
	else
	{
		memset(tempf,0,sizeof(tempf));
		snprintf(tempf,sizeof(tempf)-1,"sudo rm /mnt/wtp/%s > /dev/null",name);
		system(tempf);
		return -1;
	}
}

//本地上传函数
int locupwtp(struct list *lpublic, struct list *lsystem) 
{
	/*char fpath[PATH_LENG] = { 0 };
	int ret_fpath = 0;
	memset(fpath,0,sizeof(fpath));
	ret_fpath=getFileNamewtp(fpath);
	if(ret_fpath==1)
	{*/
		int ret_upfileboot = 0;

		ret_upfileboot=upfilewtp();	
		if(ret_upfileboot==1)
		{
			ShowAlert(search(lpublic,"lupload_succ"));
		}
		else if(ret_upfileboot==-3)
		{
		    ShowAlert(search(lpublic,"wtp_typez"));
		}
		else
		{
		    ShowAlert(search(lpublic,"lupload_fail"));
		}
	/*}
	else
	{
		ShowAlert(search(lpublic,"no_upload"));
	}*/

	return 0;
}


