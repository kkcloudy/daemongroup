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
* wp_bk_upload.c
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
#include "cgic.h"
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <stdlib.h>
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include <sys/wait.h>

int ShowImportPage();    
int webup(char *address,char *cmd,char *usrname,char *passwd) ;


int cgiMain()
{	
	ShowImportPage();  	
	return 0;
}

int ShowImportPage()
{ 
                  
  char *encry=(char *)malloc(BUF_LEN);     
  char *address = (char *)malloc(128);
  char usrname[30];
  char passwd[30];
  int ret=-1;

  char imp_encry[BUF_LEN];  
  if(cgiFormSubmitClicked("upload_file") != cgiFormSuccess)
  {
    memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    
    if(strcmp(encry,"123456")!=0)
    {
      ShowErrorPage("用户非法");        
      return 0;
    }
    memset(imp_encry,0,BUF_LEN);                  
  }
  cgiFormStringNoNewlines("encry_import",imp_encry,BUF_LEN);

  memset(address,0,128);
  memset(usrname,0,30);
  memset(passwd,0,30);
  cgiFormStringNoNewlines("address",address,128);
  cgiFormStringNoNewlines("usrname",usrname,30);
  cgiFormStringNoNewlines("passwd",passwd,30);

  char cmd[128];
  memset(cmd,0,sizeof(cmd));
  //sprintf(cmd,"wget -N -P /mnt %s > /dev/null",address); 
  sprintf(cmd," wget -N -P /mnt --user=%s --password=%s %s > /dev/null",usrname,passwd,address);


  cgiHeaderContentType("text/html");

  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>","版本升级");  
  fprintf(cgiOut,"<body>");
  
  if(cgiFormSubmitClicked("upload_file") == cgiFormSuccess)
  {	 
    ret=webup(address,cmd,usrname,passwd);		
	if(ret==0)
		ShowAlert("上传成功");
	else
		ShowAlert("上传失败");
  } 
   fprintf(cgiOut,"<form method=post encType=multipart/form-data>");
   fprintf(cgiOut,"<table>"\
    "<tr>");
	
	fprintf(cgiOut,"<td>"\
    "<table width=480 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>"\
	"<td align=left>"\
	"%s</td><td><input type=text name=address value=\"\">","版本文件");
	fprintf(cgiOut,"<td><input type=submit name=upload_file value=\"%s\"></td>","上传");
    fprintf(cgiOut,"</tr>");
	fprintf(cgiOut,"<tr>\n");
	fprintf(cgiOut,"<td>用户名:\n");
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"<td>\n");
	fprintf(cgiOut,"<input type=text name=usrname value=\"\">");
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"</tr>\n");

	fprintf(cgiOut,"<tr>\n");
	fprintf(cgiOut,"<td>密码:\n");
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"<td>\n");
	fprintf(cgiOut,"<input type=text name=passwd value=\"\">");
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"</tr>\n");


				
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
free(address);

return 0;
}          

//网络上传函数
int webup(char *address,char *cmd,char *usrname,char *passwd) 
{
    char *tempf =(char *)malloc(128);
	char* filename=strrchr(address,'/');
    int flag=-1;
	int ret,op_ret,status=0;
	
    if((strcmp(address,"")!=0) && (strcmp(usrname,"")!=0) && (strcmp(passwd,"")!=0))
	{
			ret=system(cmd);
			if(ret==0)	 
			{

				memset(tempf,0,128);
				if(filename)
				{
					sprintf(tempf,"sudo sor.sh cp %s 600 > /dev/null",filename+1);
					status = system(tempf);
                 	op_ret = WEXITSTATUS(status);

					if(op_ret==0)
					{
						//memset(tempf,0,128);
						//sprintf(tempf,"boot.sh  %s >/dev/null",filename+1);		
						//system(tempf);  
						flag=0;
					}
				}  
			}
			
	}

  free(tempf);
  
  
  return flag;

}


