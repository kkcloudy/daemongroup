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
* wp_expconf.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for export file
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


#define CONF_XML_P "/mnt/conf_xml.conf"

int ShowExportPage();     /*m stands of string after encry*/

int load_file(char *filepath); 

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
	int ret,sum_file=0;
	char number[10];
	memset(number,0,10);
	char cmd[128] = {0};

	ST_VERSION_PARAM new_param;
	memset(&new_param,0,sizeof(ST_VERSION_PARAM));  

	cgiHeaderContentType("application/octect-stream ");	
	//version       
	ret=load_file(CONF_XML_P);
	strcpy(new_param.routeip,CONF_XML_P); //address
	strcpy(new_param.sendtype,DOWN_LOC); //operate type
	strcpy(new_param.filetype,T_CONF); //version file
	set_version_param(new_param,ISS_XML);
	return 0;
}          


//single function
int load_file(char *filepath)
{
FILE *fp;
char c;
if((fp=fopen(filepath,"r"))==NULL)	 //readonly mode for one file
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

