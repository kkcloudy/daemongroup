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
* wp_eagcustomer.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
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
#include <sys/wait.h>
#include "ws_eag_conf.h"

#define NASIP_LEN	64
#define NASPORT_LEN	10
#define LINE_LEN	1024

  
int cgiMain()
{
	char nasip[NASIP_LEN]="";
	char line[LINE_LEN]="";
	char idstr[20]="";
	char nasport[NASPORT_LEN]="";
	char logomsg[1024] = "";               /* get logo message, add by chensheng, 2009-12-16 */
	char *getlogomsg = "cat /devinfo/enterprise_name | sed '2,200d'";
	FILE *fp=NULL;
	int i;
	struct st_eagz eq;
	
	cgiHeaderContentType("text/javascript");
	
	if(cgiFormStringNoNewlines("nasip", nasip, sizeof(nasip)) ==cgiFormNotFound )
  	{
 		/*get default nasip*/
 		fprintf( cgiOut, "alert('get nasip failed! url param error!');\n" );
 		return 0;
	}
	
	/*load config*/
	for(i=0;i<5;i++)
	{
		
	    memset(&eq,0,sizeof(eq));
		memset(idstr,0,20);
		sprintf(idstr,"%d",i+1);
	    get_eag_struct(MULTI_EAG_F, MTC_N, ATT_Z, idstr, &eq);
	    if( strcmp(eq.db_listen, nasip ) == 0 )
	    {
	    	strcpy( nasport, eq.listen_port );
	    	break;
	    }
	}
	
	if( i == 5 )/*not get this config*/
	{
		strcpy(	nasport, "3990" );
	}
		
	/* get logo message, add by chensheng, 2009-12-16*/
	if (NULL != (fp = fopen("/devinfo/enterprise_name", "r")))
	{
		fgets(logomsg, 1024, fp);
		fclose(fp);
	}
	for( i=0; i<sizeof(logomsg); i++ )
	{
		if( 0x0a==logomsg[i] || 0x0d==logomsg[i] || 0==logomsg[i])
		{
			logomsg[i] = 0;
			break;
		}
	}
	
	if (strcmp(logomsg, "") == 0 || strcmp(logomsg, "\n") == 0)
		strcpy(logomsg, "EAG");
	if (logomsg[strlen(logomsg)-1] == '\n')
		logomsg[strlen(logomsg)-1] = '\0';
		
	/*output file*/
	/**/
	
	fprintf( cgiOut, "eagController.host = '%s';\n", nasip );
	fprintf( cgiOut, "eagController.port = '%s';\n", nasport );
	fprintf( cgiOut, "eagController.refresh();\n" );
	fprintf( cgiOut, "var logom=document.getElementById('logomessage');\n" );
	fprintf( cgiOut, "if( null != logom ){\n" );
	fprintf( cgiOut, "	logom.innerHTML = '%s';\n", logomsg);    /* get logo message */
	fprintf( cgiOut, "}\n" );

	fprintf( stderr, "eagController.host = '%s';\n", nasip );
	fprintf( stderr, "eagController.port = '%s';\n", nasport );
	fprintf( stderr, "document.getElementById('logomessage').innerHTML = '%s';\n", logomsg);

	return 0; 
}

