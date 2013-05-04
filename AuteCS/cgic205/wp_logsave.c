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
* wp_logsave.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for syslog config save
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_log_conf.h"
#include <sys/wait.h>

int cgiMain()
{

	struct list *lpublic;
	struct list *lcontrol;	
	char *encry=(char *)malloc(BUF_LEN);	
	
	char *str;
	char lan[3]; 
	FILE *fp1;	
	
		
	char *file_name=(char *)malloc(128); //原来的是 char 此处是 char *        
	memset(file_name,0,128);	

	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");	
	
 	memset(encry,0,BUF_LEN);
	
	char *cmd=(char*)malloc(128);
	memset(cmd,0,128);

	int status=0,tt_ret=-1;

	//char *file1=(char*)malloc(128);   //日志文件地址
	//memset(file1,0,128);

	//char *file2=(char*)malloc(128);   //设备上日志文件地址
	//memset(file2,0,128);

	//char *file3=(char*)malloc(128);   //临时日志文件地址
	//memset(file3,0,128);
	
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN);	
	cgiFormStringNoNewlines("Nb", file_name, 128);    //从原来到地方查看 wp_del.cgi 中传递  		
  	str=dcryption(encry);
  	if(str==NULL)
  	{
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
  	}
	cgiHeaderContentType("text/html");
	
 	if((fp1=fopen("../htdocs/text/public.txt","r"))==NULL)		   /*以只读方式打开资源文件*/
 	{
		ShowAlert(search(lpublic,"error_open"));
		return 0;
	}
	fseek(fp1,4,0);						  /*将文件指针移到离文件首4个字节处，即lan=之后*/
	fgets(lan,3,fp1); 	 
	fclose(fp1);
	
	fprintf( cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( cgiOut, "<head> \n" );
	fprintf( cgiOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );
	//下面三句话用于禁止页面缓存
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");
  	
	fprintf( cgiOut, "<title>%s</title> \n", search( lcontrol, "del" ) );
	fprintf( cgiOut, "<link rel=stylesheet href=/style.css type=text/css> \n" );	
	fprintf( cgiOut, "</head> \n" );		
	fprintf( cgiOut, "<body> \n" );
	
{
	
	{
		
		sprintf(cmd,"cat /var/log/%s > /mnt/%s",file_name,file_name);		
		int ret=system(cmd);

        memset(cmd,0,128);
		sprintf(cmd,"sudo sor.sh cp %s %d > /dev/null",file_name,SHORT_SORT);
		status = system(cmd);
        tt_ret = WEXITSTATUS(status);


		
		if(tt_ret==0)
		{
			ShowAlert(search(lpublic,"oper_succ"));
		}
		else
		{
			ShowAlert(search(lpublic,"oper_fail"));
		}


		fprintf( cgiOut, "<script type='text/javascript'>\n" );
   		fprintf( cgiOut, "window.location.href='wp_log_info.cgi?UN=%s';\n", encry);
		fprintf( cgiOut, "</script>\n" );		
		fprintf( cgiOut, "</body>\n" );
		fprintf( cgiOut, "</html>\n" );
	}

}

	
	free(encry);
	release(lpublic); 
	
	return 0;	
}

