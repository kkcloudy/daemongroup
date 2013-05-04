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
* wp_logsys_export.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for syslog info export
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

int ShowExportPage();     
int load_file(char *filepath); 

#define EXPORT_FILES "/var/log/system.log"

int cgiMain()
{
  ShowExportPage();
  return 0;
}

int ShowExportPage()
{ 

	char encry[128] = {0};
	char *str;
	cgiFormStringNoNewlines("UN", encry, sizeof(encry));
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage("error user");		   /*用户非法*/
		return 0;
	}

  cgiHeaderContentType("application/octect-stream "); 
  char cmd[128];
  memset(cmd,0,128);
  if(access(EXPORT_FILES,0)==0)
  {
	  sprintf(cmd,"sudo chmod 666 %s",EXPORT_FILES);
	  system(cmd);
	  load_file(EXPORT_FILES);
  }  
  return 0;
}          


int load_file(char *filepath)
{
	FILE *fp;
	char c;
	if((fp=fopen(filepath,"r"))==NULL)	
		ShowAlert(ser_var("error_open"));
	else
	{
		while((c=fgetc(fp))!=EOF)
		{ 
			if((c=='\n'))
			fprintf(cgiOut,"\r\n");
			else
			fprintf(cgiOut,"%c",c); 
		}
		fclose(fp);
	}
	return 0;
}
