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
* wp_wlanbla_del.c
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
#include "ws_ec.h"
#include "wcpss/asd/asd.h"
#include "ws_sta.h"
#include "ws_dbus_list_interface.h"

int cgiMain()
{
	struct list *lpublic = NULL;
	struct list *lcontrol = NULL;
	struct list *lwlan = NULL; 
	char encry[BUF_LEN] = { 0 };
	char ID[10] = { 0 };
	char WID[10] = { 0 };
	char mac[50] = { 0 };
	char stat[20] = { 0 };
	char type[10] = { 0 };
	char flag[5] = { 0 }; /*fla=="1",表示上一页为wp_radiolis.cgi,否则上一页为wp_wtpdta.cgi*/ 
	char wtp_id[10] = { 0 };
	int id = 0;
	char *str = NULL;
	char alt[100] = { 0 };
	char max_wlan_num[10] = { 0 };
  	char max_wtp_num[10] = { 0 };
	char max_radio_num[10] = { 0 };
	char instance_id[10] = { 0 };
	instance_parameter *paraHead1 = NULL;
	dbus_parameter ins_para;	
		
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");
	lwlan=get_chain_head("../htdocs/text/wlan.txt");
	
	memset(ID,0,sizeof(ID));
	memset(WID,0,sizeof(WID));
	memset(stat,0,sizeof(stat));
	memset(mac,0,sizeof(mac));
 	memset(encry,0,sizeof(encry));
	memset(type,0,sizeof(type));
	memset(flag,0,sizeof(flag));
	memset(wtp_id,0,sizeof(wtp_id));
	memset(instance_id,0,sizeof(instance_id));
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN);
	cgiFormStringNoNewlines("ID", ID, 10);
	cgiFormStringNoNewlines("WLAN_ID", WID, 10);
	cgiFormStringNoNewlines("stat", stat,20);
	cgiFormStringNoNewlines("mac", mac, 50);
	cgiFormStringNoNewlines("Type", type, 10);
	cgiFormStringNoNewlines("FL", flag, 5);
	cgiFormStringNoNewlines("WID", wtp_id, 10); 
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
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
  	
	fprintf( cgiOut, "<title>%s</title> \n", search( lpublic, "delete" ) );
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

	id = atoi(ID);	
	get_slotID_localID_instanceID(instance_id,&ins_para);  
	get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);

	int ret = 0;
	if(strcmp(type,"wlan")==0)
	{
	  if(paraHead1)
	  {
		  ret= wlan_delete_black_white(paraHead1->parameter,paraHead1->connection,id,stat,mac);/*返回0表示失败，返回1表示成功*/
																							  /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																							  /*返回-2表示input patameter should only be 'black/white' or 'b/w'*/
																							  /*返回-3返回Unknown mac addr format*/
																							  /*返回-4表示wlan isn't existed，返回-5表示mac is not in the list*/
	  } 
	  if((ret==0)||(ret == SNMPD_CONNECTION_ERROR))
	    ShowAlert(search(lcontrol,"del_err"));
	  else if(ret==-1)
	  {
	  	memset(alt,0,sizeof(alt));
	    strncpy(alt,search(lwlan,"wlan_id_illegal1"),sizeof(alt)-1);
	    memset(max_wlan_num,0,sizeof(max_wlan_num));
	    snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
	    strncat(alt,max_wlan_num,sizeof(alt)-strlen(alt)-1);
	    strncat(alt,search(lwlan,"wlan_id_illegal2"),sizeof(alt)-strlen(alt)-1);
	    ShowAlert(alt);
	  }
	  else if(ret==-2)
	    ShowAlert(search(lpublic,"input_para_illegal"));
	  else if(ret==-3)
	    ShowAlert(search(lcontrol,"unkown_mac_format"));
	  else if(ret==-4)
	    ShowAlert(search(lwlan,"wlan_not_exist"));
	  else if(ret==-5)
	    ShowAlert(search(lwlan,"mac_not_in_list"));
	  fprintf( cgiOut, "<script type='text/javascript'>\n" );
	  if(strcmp(stat,"black")==0)
   		fprintf( cgiOut, "window.location.href='wp_wlanblack.cgi?UN=%s&ID=%s&INSTANCE_ID=%s';\n", encry,ID,instance_id);
	  else
		fprintf( cgiOut, "window.location.href='wp_wlanwhite.cgi?UN=%s&ID=%s&INSTANCE_ID=%s';\n", encry,ID,instance_id);
	}
	else if(strcmp(type,"wtp")==0)
	{
	  ret = 0;
	  if(paraHead1)
	  {
		  ret= wtp_delete_black_white(paraHead1->parameter,paraHead1->connection,id,stat,mac);/*返回0表示失败，返回1表示成功*/
																							 /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																							 /*返回-2返回input patameter should only be 'black/white' or 'b/w'*/
																							 /*返回-3返回Unknown mac addr format，返回-4表示wtp is not existed*/
																							 /*返回-5表示mac is not in the list*/
	  } 
	  if((ret==0)||(ret == SNMPD_CONNECTION_ERROR))
	    ShowAlert(search(lcontrol,"del_err"));
	  else if(ret==-1)
	  {
	  	  memset(alt,0,sizeof(alt));
		  strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
		  memset(max_wtp_num,0,sizeof(max_wtp_num));
		  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
		  strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
		  strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
		  ShowAlert(alt);
	  }
	  else if(ret==-2)
	    ShowAlert(search(lpublic,"input_para_illegal"));
	  else if(ret==-3)
	    ShowAlert(search(lcontrol,"unkown_mac_format"));
	  else if(ret==-4)
	    ShowAlert(search(lwlan,"wtp_not_exist"));
	  else if(ret==-5)
	    ShowAlert(search(lwlan,"mac_not_in_list"));
	  fprintf( cgiOut, "<script type='text/javascript'>\n" );
	  if(strcmp(stat,"black")==0)
   		fprintf( cgiOut, "window.location.href='wp_wtpblack.cgi?UN=%s&ID=%s&INSTANCE_ID=%s';\n", encry,ID,instance_id);
	  else
		fprintf( cgiOut, "window.location.href='wp_wtpwhite.cgi?UN=%s&ID=%s&INSTANCE_ID=%s';\n", encry,ID,instance_id);
	}
	else
    {
      ret = 0;
	  if(paraHead1)
	  {
		  ret= radio_bss_delete_black_white(paraHead1->parameter,paraHead1->connection,id,WID,stat,mac);
	  } 
	  if((ret==0)||(ret == SNMPD_CONNECTION_ERROR))
	    ShowAlert(search(lcontrol,"del_err"));
	  else if(ret==-1)
	  {
	  	memset(alt,0,sizeof(alt));
		strncpy(alt,search(lwlan,"radio_id_illegal1"),sizeof(alt)-1);
		memset(max_radio_num,0,sizeof(max_radio_num));
		snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
		strncat(alt,max_radio_num,sizeof(alt)-strlen(alt)-1);
		strncat(alt,search(lwlan,"radio_id_illegal2"),sizeof(alt)-strlen(alt)-1);
	    ShowAlert(alt);
	  }
	  else if(ret==-2)
	  {
		memset(alt,0,sizeof(alt));
		strncpy(alt,search(lwlan,"wlan_id_illegal1"),sizeof(alt)-1);
		memset(max_wlan_num,0,sizeof(max_wlan_num));
		snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
		strncat(alt,max_wlan_num,sizeof(alt)-strlen(alt)-1);
		strncat(alt,search(lwlan,"wlan_id_illegal2"),sizeof(alt)-strlen(alt)-1);
		ShowAlert(alt);
	  }
	  else if(ret==-3)
	    ShowAlert(search(lcontrol,"unkown_mac_format"));
	  else if(ret==-4)
	    ShowAlert(search(lwlan,"bss_not_exist"));
	  else if(ret==-5)
	    ShowAlert(search(lwlan,"mac_not_in_list"));
	  else if(ret==-6)
	    ShowAlert(search(lpublic,"unknown_id_format"));
	  else if(ret==-7)
	    ShowAlert(search(lwlan,"wlan_not_exist"));
	  else if(ret==-8)
	    ShowAlert(search(lwlan,"rad_dont_bind_wlan"));
	  else if(ret==-9)
	    ShowAlert(search(lwlan,"radio_id_not_exist"));
	  fprintf( cgiOut, "<script type='text/javascript'>\n" );
	  if(strcmp(stat,"black")==0)
   		fprintf( cgiOut, "window.location.href='wp_radioblack.cgi?UN=%s&ID=%s&WLAN_ID=%s&FL=%s&WID=%s&INSTANCE_ID=%s';\n", encry,ID,WID,flag,wtp_id,instance_id);
	  else
		fprintf( cgiOut, "window.location.href='wp_radiowhite.cgi?UN=%s&ID=%s&WLAN_ID=%s&FL=%s&WID=%s&INSTANCE_ID=%s';\n", encry,ID,WID,flag,wtp_id,instance_id);
	}
	
	fprintf( cgiOut, "</script>\n" );		
	fprintf( cgiOut, "</body>\n" );
	fprintf( cgiOut, "</html>\n" );

	release(lpublic); 
	release(lcontrol);
	release(lwlan);
	free_instance_parameter_list(&paraHead1);
	destroy_ccgi_dbus();
	return 0;	
}


