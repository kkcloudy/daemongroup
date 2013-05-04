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
* wp_stakick.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
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
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_sta.h"
#include "ws_dcli_vrrp.h"
#include "ws_dbus_list_interface.h"

int cgiMain()
{
	struct list *lpublic = NULL;
	struct list *lwlan = NULL;
	int instRun = DCLI_VRRP_INSTANCE_NO_CREATED;
	char encry[BUF_LEN] = { 0 };
	char mac[30] = { 0 };
	char *str = NULL;
    char select_insid[10] = { 0 };
	char type[10] = { 0 };
	char pno[10] = { 0 };
	char ID[10] = { 0 };
	instance_parameter *paraHead1 = NULL;
	dbus_parameter ins_para;	
	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lwlan=get_chain_head("../htdocs/text/wlan.txt");	
  
 	memset(encry,0,sizeof(encry));
	memset(mac,0,sizeof(mac));
	memset(type,0,sizeof(type));
	
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN);
	cgiFormStringNoNewlines("Nm", mac, 30);
	cgiFormStringNoNewlines("Type", type, 10);
	cgiFormStringNoNewlines("PN", pno, 10);
	cgiFormStringNoNewlines("ID", ID, 10);
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
	
	DcliWInit();
	ccgi_dbus_init();
  	memset(select_insid,0,sizeof(select_insid));
	cgiFormStringNoNewlines( "INSTANCE_ID", select_insid, 10 );
	if(strcmp(select_insid,"")==0)
	{ 
	  list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB); 
	  if(paraHead1)
	  {
		  snprintf(select_insid,sizeof(select_insid)-1,"%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.local_id,paraHead1->parameter.instance_id); 
	  }
	}
	else
	{
		get_slotID_localID_instanceID(select_insid,&ins_para);	
		get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
	}
	
	
	int ret = 0;
	if(paraHead1)
	{
		ret =  kick_sta_MAC(paraHead1->parameter,paraHead1->connection,mac);
	}
	if(ret == -1)
	{
		ShowAlert(search(lwlan,"no_sta"));
	}
	else if(ret == -2)
	{
		ShowAlert(search(lpublic,"unknown_mac_format"));
	}
	else if(ret == 1)
	{
		ShowAlert(search(lwlan,"kick_suc"));
	}
	else 
	{
		ShowAlert(search(lwlan,"kick_fail"));
	}
	fprintf( cgiOut, "<script type='text/javascript'>\n" );
	if(strcmp(type,"stalis") == 0)
			fprintf( cgiOut, "window.location.href='wp_stalis.cgi?UN=%s&INSTANCE_ID=%s';\n", encry,select_insid);
	else
		fprintf( cgiOut, "window.location.href='wp_stakind.cgi?UN=%s&INSTANCE_ID=%s&ST=%s&PN=%s&ID=%s';\n", encry,select_insid,type,pno,ID);
	fprintf( cgiOut, "</script>\n" );		
	fprintf( cgiOut, "</body>\n" );
	fprintf( cgiOut, "</html>\n" );

	release(lpublic); 
	release(lwlan); 
	free_instance_parameter_list(&paraHead1);
	destroy_ccgi_dbus();
	return 0;	
}


