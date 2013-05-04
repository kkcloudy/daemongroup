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
* wp_wtpmcon.c
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
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include <fcntl.h>
#include <sys/wait.h>
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_ac.h"
#include "ws_dbus_list_interface.h"


#define MSG_LEN 256


int ShowmodeladdPage(struct list *lwlan,struct list *lpublic);
void Showmodeladd(char *pn,char *ins_id,instance_parameter *ins_para,char *model,struct list *lwlan,struct list *lpublic);


int cgiMain()
{
	struct list *lwlan = NULL;
	struct list *lpublic = NULL;
	
	lwlan = get_chain_head("../htdocs/text/wlan.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	
	DcliWInit();
	ccgi_dbus_init();	
	ShowmodeladdPage(lwlan,lpublic);
	release(lwlan);
	release(lpublic); 
	destroy_ccgi_dbus();
	return 0;
}

int ShowmodeladdPage(struct list *lwlan,struct list *lpublic)
{ 
  char encry[BUF_LEN] = { 0 };  
  char old_pn[5] = { 0 }; 
  char new_pn[5] = { 0 }; 
  char old_instance_id[10] = { 0 }; 
  char new_instance_id[10] = { 0 }; 
  char *str = NULL;
  char model[64] = { 0 };
  char ver[64] = { 0 };
  char path[64] = { 0 };
  char rad[5] = { 0 };
  char bss[5] = { 0 };
  char old_name[64] = { 0 };
  char dhcp_encry[BUF_LEN] = { 0 }; 
  int i = 0;   
  int ap_reboot_result = 0;
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  
  memset(model,0,sizeof(model));
  memset(ver,0,sizeof(ver));
  memset(path,0,sizeof(path));
  memset(rad,0,sizeof(rad));
  memset(bss,0,sizeof(bss));
  
  if((cgiFormSubmitClicked("model_add") != cgiFormSuccess)&&(cgiFormSubmitClicked("ap_reboot") != cgiFormSuccess))
  {
	memset(encry,0,sizeof(encry));	
	memset(old_pn,0,sizeof(old_pn));
	memset(old_instance_id,0,sizeof(old_instance_id));
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 	
	cgiFormStringNoNewlines("PN", old_pn, 5); 	
	cgiFormStringNoNewlines("INSTANCE_ID", old_instance_id, 10);
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	memset(dhcp_encry,0,sizeof(dhcp_encry));                   /*清空临时变量*/
	memset(new_pn,0,sizeof(new_pn));
	memset(new_instance_id,0,sizeof(new_instance_id));
	memset(old_name,0,sizeof(old_name));
  }
  else
  {
  	cgiFormStringNoNewlines("encry_dhcp", dhcp_encry, BUF_LEN); 	
	cgiFormStringNoNewlines("pno", new_pn, 5); 	
	cgiFormStringNoNewlines("instance_id",new_instance_id,10);  
	cgiFormStringNoNewlines("old_model", old_name, 64); 
    str=dcryption(dhcp_encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	memset(dhcp_encry,0,sizeof(dhcp_encry));                   /*清空临时变量*/
	memset(old_name,0,sizeof(old_name));
  }
  cgiFormStringNoNewlines("WtpID", model, 64);  
  cgiFormStringNoNewlines("ver", ver, 64);
  cgiFormStringNoNewlines("path", path, 64);
  cgiFormStringNoNewlines("rad", rad, 20);
  cgiFormStringNoNewlines("bss", bss, 20);   
  cgiFormStringNoNewlines("encry_dhcp",dhcp_encry,BUF_LEN);
  cgiFormStringNoNewlines("old_model", old_name, 64); 
  
  
  get_slotID_localID_instanceID(new_instance_id,&ins_para);	
  get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>"); 
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "<script language=javascript src=/ip.js>"\
  "</script>"\
   "<script language=javascript>");
  fprintf(cgiOut,"function mysubmit()"\
  					"{"\
  						"var mod = document.all.mode.value;"\
						"var ver = document.all.version.value;"\
						"var pat = document.all.path.value;"\
						"var rad = document.all.radio.value;"\
						"var bss = document.all.bss.value;"\
  						"if(mod==\"\")"\
  						"{"\
  							"alert(\"%s\");"\
  							"return false;"\
  						"}"\
  						"else if(ver == \"\")"\
  						"{"\
  							"alert(\"%s\");"\
  							"return false;"\
  						"}"\
  						"else if(pat == \"\")"\
  						"{"\
  							"alert(\"%s\");"\
							"return false;"\
  						"}"\
  						"else if(rad < 1||rad >4 )"\
  						"{"\
  							"alert(\"%s\");"\
  							"return false;"\
  						"}"\
  						"else if(bss <1||bss > %d)"\
  						"{"\
  							"alert(\"%s\");"\
  							"return false;"\
  						"}"\
  					"}",search(lwlan,"mode_empty"),search(lwlan,"ver_empty"),search(lwlan,"pat_empty"),search(lwlan,"radio_id"),L_BSS_NUM,search(lwlan,"bss_id_illegal"));
 fprintf(cgiOut,"</script></head>"\
  "<body>");
  if(cgiFormSubmitClicked("model_add") == cgiFormSuccess)
  {	 
  	 if(paraHead1)
	 {
		 Showmodeladd(new_pn,new_instance_id,paraHead1,old_name,lwlan,lpublic);
	 } 
  }
  if(cgiFormSubmitClicked("ap_reboot") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		ap_reboot_result=set_ap_reboot_by_model_func(paraHead1->parameter,paraHead1->connection,old_name);/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																										  /*返回-2表示model is not exist，返回-3表示error*/
	} 
	switch(ap_reboot_result)
	{
		case SNMPD_CONNECTION_ERROR:
    	case 0:ShowAlert(search(lpublic,"ap_reboot_fail"));
			   break;
		case 1:ShowAlert(search(lpublic,"ap_reboot_succ"));
			   break;
		case -1:ShowAlert(search(lpublic,"unknown_id_format"));
			    break;
		case -2:ShowAlert(search(lwlan,"model_not_exist"));
				break;
		case -3:ShowAlert(search(lpublic,"error"));
				break;
    }
	fprintf(cgiOut, "<html>"\
				 	"<body>"\
				 	"<script language=javascript>"\
				 	"var str_url = document.location.href;"\
				 	"var index = str_url.indexOf(\"&\");"\
	    			 "var str_clean = str_url.substr(0,index);"\
	    			 "str_clean = str_clean.replace(/\\w+\\.\\w+\\?/,\"%s?\");"\
					 "str_clean += \"&PN=\"+\"%s\"+\"&INSTANCE_ID=\"+\"%s\";"\
					"document.location.replace(str_clean);"\
					"</script>"\
					"</body>"\
					"</html>","wp_wtpver.cgi",new_pn,new_instance_id);
  }
  fprintf(cgiOut,"<form method=post onsubmit=\"return mysubmit()\" >"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
		fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=model_add style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		
		  if((cgiFormSubmitClicked("model_add") != cgiFormSuccess)&&(cgiFormSubmitClicked("ap_reboot") != cgiFormSuccess))
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_wtpver.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,old_pn,old_instance_id,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_wtpver.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,new_pn,new_instance_id,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
					
	fprintf(cgiOut,"</td>"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
      "<tr>"\
        "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"\
            "<tr height=4 valign=bottom>"\
              "<td width=120>&nbsp;</td>"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
            "</tr>"\

	"<tr>"\
              "<td><table width=120 border=0 cellspacing=0 cellpadding=0>"\
                   "<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
						fprintf(cgiOut,"<tr height=26>"\
         				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></a></td>",search(lpublic,"menu_san"),search(lwlan,"con_model"));
         				fprintf(cgiOut,"</tr>");  
				for(i=0;i<4;i++)
					{
						fprintf(cgiOut,"<tr height=10>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}
 fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");

fprintf(cgiOut,"<table width=450 border=0 cellspacing=0 cellpadding=0>"\
"<tr height=35>");
  if((cgiFormSubmitClicked("model_add") != cgiFormSuccess)&&(cgiFormSubmitClicked("ap_reboot") != cgiFormSuccess))
    fprintf(cgiOut,"<td colspan=3 id=ins_style>%s:%s</td>",search(lpublic,"instance"),old_instance_id);
  else
  	fprintf(cgiOut,"<td colspan=3 id=ins_style>%s:%s</td>",search(lpublic,"instance"),new_instance_id);
fprintf(cgiOut,"</tr>");

fprintf(cgiOut,"<tr height=35>"\
 					"<td width=100 >%s:</td>",search(lpublic,"mode"));
fprintf(cgiOut,"<td  width=140 align=left><input type=text name=mode maxLength=63 value=\"%s\"  ><td>",model);
fprintf(cgiOut,"<td  width=210 align=left style=\"padding-left:30px\"><input type=submit style=\"width:100px; margin-left:20px\" border=0 name=ap_reboot style=background-image:url(/images/SubBackGif.gif) value=\"%s\"><td>",search(lpublic,"ap_reboot"));
fprintf(cgiOut,"</tr>");
#if 0
  				"<tr height=35>"\
  					"<td>%s:</td>",search(lwlan,"ap_version"));
fprintf(cgiOut,"<td colspan=2 align=left><input type=text name=version maxLength=63 value=\"%s\" style=\"background-color:#cccccc\" disabled><td>",ver);
fprintf(cgiOut,"</tr>"\
 				"<tr height=35>"\
 					"<td>%s:</td>",search(lwlan,"path"));
fprintf(cgiOut,"<td colspan=2 align=left><input type=text name=path maxLength=63 value=\"%s\"  /><td>",path);
fprintf(cgiOut,"</tr>"\
 				"<tr height=35>"\
			    	"<td>Radio %s:</td>",search(lpublic,"count"));
    fprintf(cgiOut,"<td colspan=2 align=left><input type=text name=radio value=\"%s\" size=5 maxLength=1 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"/><font color=red>(1-4)</font><td>",rad);
fprintf(cgiOut,"</tr>"\
				"<tr height=35>"\
				"<td>Bss %s:</td>",search(lpublic,"count"));
fprintf(cgiOut,"<td colspan=2 align=left><input type=text name=bss value=\"%s\" size=5 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"/><font color=red>(1-%d)</font><td>",bss,L_BSS_NUM);
fprintf(cgiOut,"</tr>");
#endif
				if((cgiFormSubmitClicked("model_add") != cgiFormSuccess)&&(cgiFormSubmitClicked("ap_reboot") != cgiFormSuccess))
				{
					fprintf(cgiOut,"<tr>"\
					  "<td><input type=hidden name=encry_dhcp value=\"%s\"></td>",encry);
					  fprintf(cgiOut,"<td><input type=hidden name=old_model value=\"%s\"></td>",model);					  
					  fprintf(cgiOut,"<td><input type=hidden name=instance_id value=%s></td>",old_instance_id);
				    fprintf(cgiOut,"</tr>"\
					"<tr>"\
					  "<td colspan=3><input type=hidden name=pno value=%s></td>",old_pn);
					fprintf(cgiOut,"</tr>");
				}
				else if((cgiFormSubmitClicked("model_add") == cgiFormSuccess)||(cgiFormSubmitClicked("ap_reboot") == cgiFormSuccess))
				{
				 	fprintf(cgiOut,"<tr>"\
					  "<td><input type=hidden name=encry_dhcp value=\"%s\"></td>",dhcp_encry);
					  fprintf(cgiOut,"<td><input type=hidden name=old_model value=\"%s\"></td>",old_name);						   
					  fprintf(cgiOut,"<td><input type=hidden name=instance_id value=%s></td>",new_instance_id);
					fprintf(cgiOut,"</tr>"\
					"<tr>"\
					  "<td colspan=3><input type=hidden name=pno value=%s></td>",new_pn);
					fprintf(cgiOut,"</tr>");
				}
		   fprintf(cgiOut,"</table>"\
			 "</td>"\
            "</tr>"\
            "<tr height=4 valign=top>"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
            "</tr>"\
          "</table>"\
        "</td>"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
      "</tr>"\
    "</table></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
  "</tr>"\
"</table>"\
"</div>"\
"</form>"\
"</body>"\
"</html>");
free_instance_parameter_list(&paraHead1);
return 0;
}
						 
void  Showmodeladd(char *pn,char *ins_id,instance_parameter *ins_para,char *model,struct list *lwlan,struct list *lpublic)
{
	int ret1 = 0;
	//int flag=1,ret2;	
	char new_model[64] = { 0 };
	#if 0
	char ver[64] = { 0 };
	char path[64] = { 0 };
	char radio[5] = { 0 };
	char bss[5] = { 0 };
	#endif
	
	memset(new_model,0,sizeof(new_model));
	#if 0
	memset(ver,0,sizeof(ver));
	memset(path,0,sizeof(path));
	memset(radio,0,sizeof(radio));
	memset(bss,0,sizeof(bss));
	#endif
	
	cgiFormStringNoNewlines("mode",new_model,64);  
	#if 0
	cgiFormStringNoNewlines("version",ver,64); 
	cgiFormStringNoNewlines("path",path,64); 
	cgiFormStringNoNewlines("radio",radio,5);
	cgiFormStringNoNewlines("bss",bss,5);
	#endif
	
	if(strcmp(model,new_model))
	{
		ret1=set_model_cmd(ins_para->parameter,ins_para->connection,model,new_model);
		switch(ret1)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0://flag=0;
				   ShowAlert(search(lwlan,"con_m_fail"));
				   break;
			case 1:ShowAlert(search(lwlan,"con_m_succ"));
				   break;
			case -1://flag=0;
				    ShowAlert(search(lwlan,"new_model_exist"));
				    break;
			case -2://flag=0;
				    ShowAlert(search(lwlan,"model_not_exist"));	
				    break;
		}
	}

	#if 0
	if(flag)
	{				
		ret2 = set_ap_model(ins_para->parameter,ins_para->connection,new_model,ver,path,radio,bss);
		switch(ret2)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:flag=0;
				   ShowAlert(search(lwlan,"con_m_fail"));
				   break;
			case 1:break;
			case -1:flag=0;
				    ShowAlert(search(lwlan,"add_m_fail1"));
				    break;
			case -2:flag=0;
				   ShowAlert(search(lpublic,"error"));	
				   break;
		}
	}
	#endif

	/*if(flag)
		ShowAlert(search(lwlan,"con_m_succ"));*/  
	
	fprintf(cgiOut, "<html>"\
				 	"<body>"\
				 	"<script language=javascript>"\
				 	"var str_url = document.location.href;"\
				 	"var index = str_url.indexOf(\"&\");"\
	    			 "var str_clean = str_url.substr(0,index);"\
	    			 "str_clean = str_clean.replace(/\\w+\\.\\w+\\?/,\"%s?\");"\
					 "str_clean += \"&PN=\"+\"%s\"+\"&INSTANCE_ID=\"+\"%s\";"\
					"document.location.replace(str_clean);"\
					"</script>"\
					"</body>"\
					"</html>","wp_wtpver.cgi",pn,ins_id);
	
}


