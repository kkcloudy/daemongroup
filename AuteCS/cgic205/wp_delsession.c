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
* wp_delsession.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
* system infos 
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
#include <syslog.h>
   
void ShowDelSessionPage(char *m,char *n);   /*m代表用户名，n代表加密后的字符串*/

int Cookies(char *, char *);
#define SCRIPT_NAME "/cgi-bin/wp_login.cgi"

int cgiMain()
{
	char encry[BUF_LEN] = {0};       
	char *str;
	char usrname[128] = {0};
	
	char realm[] = "AuteLAN";
	cgiHeaderCookieSetString(realm,"",-3600,SCRIPT_NAME,cgiServerName);	

	cgiFormStringNoNewlines("UNAME", usrname, sizeof(usrname));
	struct list *lpublic;
	lpublic=get_chain_head("../htdocs/text/public.txt");
	memset(encry,0,BUF_LEN);
	cgiFormStringNoNewlines("UN", encry, BUF_LEN);
	str=dcryption(encry);
	if(str==NULL)
	{
	 	if (0 == strcmp(usrname,""))
 		{
			ShowErrorPage(search(lpublic,"ill_user"));
 		}
		else
		{
			cgiHeaderContentType("text/html" );
			del_user_infor(usrname);
			fprintf( cgiOut, "<script type='text/javascript'>\n" );
			fprintf( cgiOut, "window.location.href='/index.html';\n");
			fprintf( cgiOut, "</script>\n" );		
		}		
	}
	else
	{
		ShowDelSessionPage(str,encry); 
	}
	release(lpublic);  
	return 0;
}

void ShowDelSessionPage(char *m,char *n)
{ 
  cgiHeaderContentType("text/html" );
  openlog("logout", LOG_PID, LOG_DAEMON);
  syslog(LOG_INFO|LOG_LOCAL7, "user %s logout!" ,m);
  del_user_infor(n);
  fprintf( cgiOut, "<script type='text/javascript'>\n" );
  fprintf( cgiOut, "window.location.href='/index.html';\n");
  fprintf( cgiOut, "</script>\n" );		
}

int Cookies(char *name , char *passwd)
{
	char **array, **arrayStep;

	char value[32];

	if (cgiCookies(&array) != cgiFormSuccess) {
		return 0;
	}
	arrayStep = array;

	cgiCookieString(*arrayStep, value, sizeof(value));

	if(NULL != *arrayStep && NULL != value){
		strcpy(name,*arrayStep);
		strcpy(passwd,value);
		return 1;
	}

	else{
		return 0;
	}

}


