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
* wp_version_rm.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for version file delete
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
#include "ws_init_dbus.h"
#include <sys/wait.h>
#include "bsd/bsdpub.h"
#include "dbus/bsd/BsdDbusPath.h"
#include "ws_dbus_def.h"
#include "ws_dbus_list_interface.h"
#include "ac_manage_acinfo.h"

static int ccgi_dcli_bsd_get_slot_ids(DBusConnection *connection, int *ID, const int op)
{
    int ret = 0;
    DBusMessageIter  iter;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int i = 0;
	int slot_id = 0;
   

    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME, BSD_DBUS_OBJPATH, \
        BSD_DBUS_INTERFACE, BSD_GET_ALIVE_SLOT_IDS);

    dbus_message_append_args(query,
    					DBUS_TYPE_UINT32,&op,
    					DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret != 0){
		for(i = 0; i < ret; i++)
		{
		    dbus_message_iter_next(&iter);
	        dbus_message_iter_get_basic(&iter,&(ID[i]));
		}
	}

    return ret;
}

int cgiMain()
{
	struct list *lpublic;
	struct list *lcontrol;

		//得到rule的类型， fileter dnat  snat
	char *encry=(char *)malloc(BUF_LEN);	
	char *cmd=(char*)malloc(128);
	char *str;

	int status=0,tt_ret=-1;

	char type[N];
	memset(type,0,N);
		
	char *file_name=(char *)malloc(128); //原来的是 char 此处是 char *        
	memset(file_name,0,128);	
	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");	
	
	ccgi_dbus_init();
 	memset(encry,0,BUF_LEN);
	memset(cmd,0,128);
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN);	
	cgiFormStringNoNewlines("Nb", file_name, 128);    //从原来到地方查看 wp_del.cgi 中传递  		
	cgiFormStringNoNewlines("ID", type, N);    //从原来到地方查看 wp_del.cgi 中传递  		

  	str=dcryption(encry);
  	if(str==NULL)
  	{
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
  	}
	cgiHeaderContentType("text/html");	
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
	fprintf( cgiOut, "<body> \n" );

	
	
{
	if(strcmp(type,"1")==0)
	{

		FILE *sys_boot_img_fp=NULL;
		char sys_boot_img[32]="";
		memset(cmd,0,128);
		sprintf(cmd, 
			"source vtysh_start.sh >/dev/null 2>&1\n"
			"vtysh -c \"show system boot_img\" |awk '{print $6}' >/mnt/sysboot");	
		system(cmd);
		sys_boot_img_fp=fopen("/mnt/sysboot", "r");
		if(NULL == sys_boot_img_fp)
			ShowAlert(search(lpublic,"oper_fail"));
		else
		{
			fread(sys_boot_img, sizeof(char), sizeof(sys_boot_img)-1, sys_boot_img_fp);
			fclose(sys_boot_img_fp);
			sys_boot_img[strlen(sys_boot_img)-1]='\0'; // remove the '\n' after boot file name. 
//			fprintf(stderr, "sys_boot_img=%s, hex=%d, file_name=%s\n",sys_boot_img, sys_boot_img[strlen(sys_boot_img)-1], file_name);
			
			if(0 == strcmp(sys_boot_img, file_name))
			{
				ShowAlert(search(lpublic,"rm_sys_boot_img"));
			}
			
			else
			{
				int i = 0;
				int ID[MAX_SLOT_NUM] = {0};
				int board_count = -1;
				void *connection = NULL;
			 	board_count =ccgi_dcli_bsd_get_slot_ids(ccgi_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
				for(i = 0; i < board_count; i++)
				{
					connection = NULL;
					if(SNMPD_DBUS_SUCCESS != get_slot_dbus_connection(ID[i], &connection, SNMPD_INSTANCE_MASTER_V3))
					{
						continue;
					}
					ac_manage_delete_system_version_file(connection, file_name);					
				}

				memset(cmd,0,128);
				sprintf(cmd,"sudo sor.sh rm %s %d > /dev/null",file_name,SHORT_SORT);
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
			}
		}	
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
   		fprintf( cgiOut, "window.location.href='wp_version_del.cgi?UN=%s';\n", encry);
		fprintf( cgiOut, "</script>\n" );	
		
		fprintf( cgiOut, "</body>\n" );
		fprintf( cgiOut, "</html>\n" );
	}

	if(strcmp(type,"2")==0)
		{

		memset(cmd,0,128);
		sprintf(cmd,"rm /mnt/wtp/%s > /dev/null",file_name);	
		system(cmd);

		memset(cmd,0,128);
		sprintf(cmd,"sudo sor.sh rm wtp/%s %d > /dev/null",file_name,SHORT_SORT);
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
   		fprintf( cgiOut, "window.location.href='wp_ap_del.cgi?UN=%s';\n", encry);
		fprintf( cgiOut, "</script>\n" );	
		
		fprintf( cgiOut, "</body>\n" );
		fprintf( cgiOut, "</html>\n" );
	}

	if(strcmp(type,"3")==0)
		{
		memset(cmd,0,128);
		sprintf(cmd,"rm /mnt/wtp/%s > /dev/null",file_name);	
		system(cmd);
		
		memset(cmd,0,128);
		sprintf(cmd,"sudo sor.sh rm wtp/%s %d > /dev/null",file_name,SHORT_SORT);
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
   		fprintf( cgiOut, "window.location.href='wp_cer_del.cgi?UN=%s';\n", encry);
		fprintf( cgiOut, "</script>\n" );	
		
		fprintf( cgiOut, "</body>\n" );
		fprintf( cgiOut, "</html>\n" );
	}

}

	
	free(encry);
	free(file_name);
	release(lpublic); 
	destroy_ccgi_dbus();
	return 0;	
}

