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
* wp_dhcpdel.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system cotrl for dhcp delete
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


int cgiMain()
{
//	fwRule *rule;
	struct list *lpublic;
	struct list *lcontrol;
//	int i=0;

		//得到rule的类型， fileter dnat  snat
	char *encry=(char *)malloc(BUF_LEN);
	char *subnet=(char *)malloc(30);
	char *mask=(char*)malloc(30);
	char *cmd=(char*)malloc(128);
	char *str;
	char lan[3]; 
	FILE *fp1;
		
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");
	
	memset(subnet,0,30);
	memset(mask,0,30);
 	memset(encry,0,BUF_LEN);
	memset(cmd,0,128);
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN);
	cgiFormStringNoNewlines("Nb", subnet, BUF_LEN);
	cgiFormStringNoNewlines("Nm", mask, BUF_LEN);
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
		  	

	


//	else if(  )
	fprintf( cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( cgiOut, "<head> \n" );
	fprintf( cgiOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );
	//下面三句话用于禁止页面缓存
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");
  	
	fprintf( cgiOut, "<title>%s</title> \n", search( lcontrol, "del" ) );
	fprintf( cgiOut, "<link rel=stylesheet href=/style.css type=text/css> \n" );
	fprintf( cgiOut, "<style type=text/css> \n" );
	fprintf( cgiOut, ".usrlis {overflow-x:hidden; overflow:auto; width: 416px; height: 270px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} \n" );
	fprintf( cgiOut, "</style> \n" );
	fprintf( cgiOut, "<style type=text/css> \n" );
	fprintf( cgiOut, "tr.even td { \n" );
	fprintf( cgiOut, "background-color: #eee; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "tr.odd td { \n" );
	fprintf( cgiOut, "background-color: #fff; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "tr.changed td { \n" );
	fprintf( cgiOut, "background-color: #ffd; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, " \n" ); 
	fprintf( cgiOut, "tr.new td { \n" );  
	fprintf( cgiOut, "background-color: #dfd; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "</style> \n" );
	fprintf( cgiOut, "</head> \n" );
	fprintf( cgiOut, "<script src=\"/fw.js\"></script> \n" );
		
	fprintf( cgiOut, "<body> \n" );
	
{
	
	{
		//ShowAlert(subnet);
		//ShowAlert(mask);
		strcpy(cmd,"dhcp_sub_write.sh 2 ");
		strcat(cmd,subnet);
		strcat(cmd," ");
		strcat(cmd,mask);
		strcat(cmd," ");
		strcat(cmd,"4");
		//ShowAlert(cmd);
		int ret=system(cmd);
		if(ret!=0)
			{
				ShowAlert(search(lcontrol,"del_err"));
			}
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
   		fprintf( cgiOut, "window.location.href='wp_dhcpcon.cgi?UN=%s';\n", encry);
		fprintf( cgiOut, "</script>\n" );		
		fprintf( cgiOut, "</body>\n" );
		fprintf( cgiOut, "</html>\n" );
	}

}

	
	free(encry);
	release(lpublic); 
	
	return 0;	
}

