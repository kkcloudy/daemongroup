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
* wp_bk_import.c
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
#include <sys/wait.h>

int ShowImportPage();     /*m代表加密后的字符串*/
int getFileName(char *fpath);     /*获得上传图片的名字fname及全名fpath*/
int upfile(char *fpath);           /*上传文件全名为fpath的配置文件*/


int cgiMain()
{	
	ShowImportPage();  	
	return 0;
}

int ShowImportPage()
{ 
  
  char *encry=(char *)malloc(BUF_LEN);      		

  char imp_encry[BUF_LEN];  
  if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
  {
    memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    
    if(strcmp(encry,"123456")!=0)
    {
      ShowErrorPage("用户非法");          /*用户非法*/
      return 0;
    }
    memset(imp_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
  cgiFormStringNoNewlines("encry_import",imp_encry,BUF_LEN);
  

  /***********************2008.5.26*********************/
  cgiHeaderContentType("text/html");

  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>","导入配置文件");  
  fprintf(cgiOut,"<body>");
  
  if(cgiFormSubmitClicked("upload_file") == cgiFormSuccess)
  {
	 
    char *fpath = (char *)malloc(PATH_LENG);
    int ret_fpath;
    memset(fpath,0,PATH_LENG);
    ret_fpath=getFileName(fpath);
	
    if(ret_fpath==1)
    {
  	int ret_upfile;
  	ret_upfile=upfile(fpath);

  	if(ret_upfile==1)
  	  ShowAlert("上传成功");
  	else if(ret_upfile==0)
  	  ShowAlert("上传失败");
    }
    else
    {
  	  ShowAlert("无文件上传");
    }
    
  } 
   fprintf(cgiOut,"<form method=post encType=multipart/form-data>");
   fprintf(cgiOut,"<table>"\
    "<tr>");
	
	fprintf(cgiOut,"<td>"\
    "<table width=480 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>"\
	"<td align=left>"\
	"<p>%s<input type=\"file\" size=\"30\" name=\"file\" value=\"\"></p>","导入配置");
	fprintf(cgiOut,"<td><input type=submit name=upload_file value=\"%s\"></td>","上传");
    fprintf(cgiOut,"</tr>");

				
              if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
			   {
				 fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=encry_import value=%s></td></tr>",encry);
			   }
			   else if(cgiFormSubmitClicked("upload_file") == cgiFormSuccess)
			   {
				 fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=encry_import value=%s></td></tr>",imp_encry);
			   }	
			   
fprintf(cgiOut,"</table>");
fprintf(cgiOut,"</form>"\
        "</body>"\
        "</html>");
free(encry);
return 0;
}          



int getFileName(char *fpath)        /*返回0表示失败，返回1表示成功*/
{
  
  cgiFilePtr file;
  char name[1024];    /*存放本地路径名*/ 
  char *tmpStr=NULL;   
  int t;
  
  if (cgiFormFileName("file", name, sizeof(name)) != cgiFormSuccess) 
    return 0;


  if (cgiFormFileOpen("file", &file) != cgiFormSuccess) 
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


int upfile(char *fpath)        /*成功 返回1，否则返回0，返回-1表示没有文件上传*/
{
    
	char *path_conf=(char *)malloc(1024);
	memset(path_conf,0,1024);

	int status=0,ret=-1;
	
	strcpy(path_conf,"/mnt/conf_xml.conf");
	
  cgiFilePtr file;
  char name[1024];    /*存放本地路径名*/ 
  int targetFile; 
  mode_t mode;

  char *tempf=(char *)malloc(128);

  char buffer[1024]; 
  int got; 
  if (cgiFormFileName("file", name, sizeof(name)) != cgiFormSuccess) 
  {
    free(path_conf);
	free(tempf);
    return -1; 
  }
  
  if (cgiFormFileOpen("file", &file) != cgiFormSuccess) 
  {
    free(path_conf);
	free(tempf);

    return 0;
  }  

  mode=S_IRWXU|S_IRGRP|S_IROTH; 
  //在当前目录下建立新的文件，第一个参数实际上是路径名，此处的含义是在cgi程序所在的目录（当前目录））建立新文件 
  targetFile=open(path_conf,O_RDWR|O_CREAT|O_TRUNC|O_APPEND,mode); 
  if(targetFile == -1)
  { 
    free(path_conf);
	free(tempf);

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

  memset(tempf,0,128);
  sprintf(tempf,"sudo sor.sh cp conf_xml.conf %d > /dev/null",SHORT_SORT);
  status = system(tempf);
  ret = WEXITSTATUS(status);

  free(path_conf);
  free(tempf);

  if(ret==0)
  	return 1;
  else
  	return -1;
  
}


