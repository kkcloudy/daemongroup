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
* wp_trap_config.c
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
#include <dbus/dbus.h>
#include "ws_snmpd_engine.h"
#include "ac_manage_interface.h"
#include "ws_dcli_wlans.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "ac_manage_def.h"
#include "ws_public.h"


void ShowTrapConfigPage(char *m,struct list *lpublic,struct list *lsnmpd);  
void ConfigTrap(instance_parameter *paraHead,struct list *lpublic,struct list *lsnmpd);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;      
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsnmpd = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsnmpd=get_chain_head("../htdocs/text/snmpd.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
  	;
  }
  else
  {
    cgiFormStringNoNewlines("encry_trapconfig",encry,BUF_LEN);
  }  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowTrapConfigPage(encry,lpublic,lsnmpd);
  release(lpublic);  
  release(lsnmpd);
  destroy_ccgi_dbus();
  return 0;
}

void ShowTrapConfigPage(char *m,struct list *lpublic,struct list *lsnmpd)
{    
  char IsSubmit[5] = { 0 };
  int i = 0,limit = 0;     
  char select_slotid[10] = { 0 };
  unsigned int slot_id = 0;
  DBusConnection *select_connection = NULL;
  instance_parameter *paraHead = NULL;
  char *endptr = NULL;	
  int ret = AC_MANAGE_DBUS_ERROR;
  unsigned int trap_state = 0;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>TRAP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<script src=/slotid_onchange.js>"\
  "</script>"\
  "<body>");
  memset(select_slotid,0,sizeof(select_slotid));
  cgiFormStringNoNewlines( "SLOT_ID", select_slotid, 10 );
  list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
  if(strcmp(select_slotid,"")==0)
  { 
	if(paraHead)
	{
		snprintf(select_slotid,sizeof(select_slotid)-1,"%d",paraHead->parameter.slot_id);
	} 
  }  

  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("trapconfig_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	ConfigTrap(paraHead,lpublic,lsnmpd);
  }
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>TRAP</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");        
	 
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=trapconfig_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_snmp.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_trap_summary.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>TRAP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"info"));
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> TRAP</font></td>",search(lpublic,"menu_san"),search(lpublic,"config"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");	 
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_trap_list.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>TRAP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));
                  fprintf(cgiOut,"</tr>");
                  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_trap_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s </font><font id=yingwen_san>TRAP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"ntp_add"));
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_trap_detail.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>TRAP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"switch"));                       
                  fprintf(cgiOut,"</tr>");

				  slot_id=strtoul(select_slotid,&endptr,10);
				  get_slot_dbus_connection(slot_id,&select_connection,SNMPD_INSTANCE_MASTER_V3);
		      	  ret = ac_manage_show_trap_state(select_connection, &trap_state);

				  limit = 2;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
			    
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
	"<table width=700 border=0 cellspacing=0 cellpadding=0>"\
		"<tr height=30>"\
			"<td width=110>%s:</td>",search(lsnmpd,"trap_state"));
			fprintf(cgiOut,"<td width=100><select name=trap_service id=trap_service style=width:80px>"\
				"<option value=>"\
				"<option value=enable>enable"\
				"<option value=disable>disable"\
			"</select></td>"\
			"<td align=left width=490>&nbsp;</td>"\
		"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"heart_time"));
			if((AC_MANAGE_SUCCESS == ret)&&(trap_state))
				fprintf(cgiOut,"<td><input name=heart_time size=11 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" style=\"background-color:#cccccc\" disabled></td>");
			else
				fprintf(cgiOut,"<td><input name=heart_time size=11 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
			fprintf(cgiOut,"<td><font color=red>(0--3600)%s</font></td>",search(lpublic,"second"));
		fprintf(cgiOut,"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"heart_mode"));
			fprintf(cgiOut,"<td>");
				if((AC_MANAGE_SUCCESS == ret)&&(trap_state))
					fprintf(cgiOut,"<select name=heart_mode id=heart_mode style=width:80px disabled>");
				else
					fprintf(cgiOut,"<select name=heart_mode id=heart_mode style=width:80px>");
				fprintf(cgiOut,"<option value=>"\
				"<option value=0>0"\
				"<option value=1>1"\
				"</select>"\
			"</td>"\
			"<td><font color=red>(%s)</font></td>",search(lsnmpd,"heart_mode_description"));
		fprintf(cgiOut,"</tr>"\		
		"<tr height=30 valign=top style=\"padding-top:10px\">"\
			"<td colspan=3 width=282>"\
				"<fieldset align=left>"\
				"<legend><font color=Navy>%s</font></legend>",search(lsnmpd,"trap_resend"));
					fprintf(cgiOut,"<table width=282 border=0 cellspacing=0 cellpadding=0>"\
						"<tr height=30>"\
							"<td width=102>%s:</td>",search(lsnmpd,"resend_interval"));
							if((AC_MANAGE_SUCCESS == ret)&&(trap_state))
								fprintf(cgiOut,"<td width=100><input name=resend_interval size=11 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" style=\"background-color:#cccccc\" disabled></td>");
							else
								fprintf(cgiOut,"<td width=100><input name=resend_interval size=11 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
							fprintf(cgiOut,"<td width=80><font color=red>(0--65535)%s</font></td>",search(lpublic,"second"));
						fprintf(cgiOut,"</tr>"\
						"<tr height=30>"\
							"<td>%s:</td>",search(lsnmpd,"resend_times"));
							if((AC_MANAGE_SUCCESS == ret)&&(trap_state))
								fprintf(cgiOut,"<td><input name=resend_times size=11 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" style=\"background-color:#cccccc\" disabled></td>");
							else
								fprintf(cgiOut,"<td><input name=resend_times size=11 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
							fprintf(cgiOut,"<td><font color=red>(0--50)</font></td>"\
						"</tr>"\
					"</table>"\
				"</fieldset>"\
			"</td>"\
		"</tr>"\
		"<tr>"\
		 	"<td><input type=hidden name=encry_trapconfig value=%s></td>",m);
			fprintf(cgiOut,"<td colspan=2><input type=hidden name=SubmitFlag value=%d></td>",1);
	 fprintf(cgiOut,"</tr>"\
	"</table>"\
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
free_instance_parameter_list(&paraHead);
}


void ConfigTrap(instance_parameter *paraHead,struct list *lpublic,struct list *lsnmpd)
{
	int flag = 1;
	instance_parameter *paraNode = NULL;
	int ret = AC_MANAGE_DBUS_ERROR;
	char heart_time[10] = { 0 };
	unsigned int data = 0;
	char heart_mode[10] = { 0 };
	int temp_ret = AC_MANAGE_DBUS_ERROR;
	char resend_interval[10] = { 0 };
	char resend_times[10] = { 0 };
	unsigned int inter_data = 0;
    unsigned int times_data = 0;
	char trap_service[10] = { 0 };
	unsigned int state = 0;	


	/*config heartbeat interval*/
	memset(heart_time,0,sizeof(heart_time));
	cgiFormStringNoNewlines("heart_time",heart_time,10);  
	if(strcmp(heart_time,""))
	{
		data = atoi(heart_time);		
		if((data >= 0)&&(data < 3601))
		{
			for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
			{
				ret = ac_manage_config_trap_parameter(paraNode->connection, HEARTBEAT_INTERVAL, data); 
				if(AC_MANAGE_SUCCESS == ret)
				{
					;
				}
				else if((AC_MANAGE_DBUS_ERROR == ret)&&(1 == flag))
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"config_heart_time_fail"));
				}
				else if((AC_MANAGE_SERVICE_ENABLE == ret)&&(1 == flag))
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"disable_trap_service"));
				}
				else if((AC_MANAGE_INPUT_TYPE_ERROR == ret)&&(1 == flag))
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"input_type_error"));
				}
				else if(1 == flag)
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"config_heart_time_fail"));
				}
			}	
		}
		else
		{
			flag = 0;
			ShowAlert(search(lpublic,"input_para_illegal"));
		}
	}


	/*config heartbeat mode*/
	memset(heart_mode,0,sizeof(heart_mode));
	cgiFormStringNoNewlines("heart_mode",heart_mode,10);  
	if(strcmp(heart_mode,""))
	{
		data = atoi(heart_mode);		
		if((data >= 0)&&(data < 2))
		{
			for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
			{
				ret = AC_MANAGE_DBUS_ERROR;
				ret = ac_manage_config_trap_parameter(paraNode->connection, HEARTBEAT_MODE, data); 
				if(AC_MANAGE_SUCCESS == ret)
				{
					;
				}
				else if((AC_MANAGE_DBUS_ERROR == ret)&&(1 == flag))
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"config_heart_mode_fail"));
				}
				else if((AC_MANAGE_SERVICE_ENABLE == ret)&&(1 == flag))
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"disable_trap_service"));
				}
				else if((AC_MANAGE_INPUT_TYPE_ERROR == ret)&&(1 == flag))
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"input_type_error"));
				}
				else if(1 == flag)
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"config_heart_mode_fail"));
				}
			}	
		}
		else
		{
			flag = 0;
			ShowAlert(search(lpublic,"input_para_illegal"));
		}
	}


	/*config trap resend*/
	memset(resend_interval,0,sizeof(resend_interval));
	cgiFormStringNoNewlines("resend_interval",resend_interval,10);  
	memset(resend_times,0,sizeof(resend_times));
	cgiFormStringNoNewlines("resend_times",resend_times,10);  
	if((strcmp(resend_interval,""))&&(strcmp(resend_times,"")))
	{
		inter_data = atoi(resend_interval);
   		times_data = atoi(resend_times);
		if(((inter_data >= 0)&&(inter_data < 65536))&&((times_data >= 0)&&(times_data < 51)))
		{
			for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
			{
				ret = AC_MANAGE_DBUS_ERROR;
				ret = ac_manage_config_trap_parameter(paraNode->connection, RESEND_INTERVAL, inter_data); 
				if(AC_MANAGE_SUCCESS == ret)
				{
					temp_ret = ac_manage_config_trap_parameter(paraNode->connection, RESEND_TIMES, times_data); 
					if(AC_MANAGE_SUCCESS == temp_ret)
					{
						;
					}
					else if((AC_MANAGE_SERVICE_ENABLE == temp_ret)&&(1 == flag))
					{
						flag = 0;
						ShowAlert(search(lsnmpd,"disable_trap_service"));
					}
					else if((AC_MANAGE_DBUS_ERROR == temp_ret)&&(1 == flag))
					{
						flag = 0;
						ShowAlert(search(lsnmpd,"config_resend_times_fail"));
					}
					else if((AC_MANAGE_INPUT_TYPE_ERROR == temp_ret)&&(1 == flag))
					{
						flag = 0;
						ShowAlert(search(lsnmpd,"input_type_error"));
					}
					else if(1 == flag)
					{
						flag = 0;
						ShowAlert(search(lsnmpd,"config_resend_times_fail"));
					}
				}
				else if((AC_MANAGE_SERVICE_ENABLE == ret)&&(1 == flag))
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"disable_trap_service"));
				}
				else if(AC_MANAGE_DBUS_ERROR == ret)
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"config_resend_interval_fail"));
				}
				else if((AC_MANAGE_INPUT_TYPE_ERROR == ret)&&(1 == flag))
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"input_type_error"));
				}
				else if(1 == flag)
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"config_resend_interval_fail"));
				}
			}	
		}
		else
		{
			flag = 0;
			ShowAlert(search(lpublic,"input_para_illegal"));
		}
	}
	else if(!((0 == strcmp(resend_interval,""))&&(0 == strcmp(resend_times,""))))
	{
		flag = 0;
		ShowAlert(search(lpublic,"para_incom"));
	}
	

	/*config trap service*/
	memset(trap_service,0,sizeof(trap_service));
	cgiFormStringNoNewlines("trap_service",trap_service,10);  
	if(strcmp(trap_service,""))
	{
		if(0 == strcmp(trap_service, "enable")) 
		{
	        state = 1;
	    }
	    else if(0 == strcmp(trap_service, "disable")) {
	        state = 0;
	    }
	    else 
		{
	        ShowAlert(search(lpublic,"input_para_illegal"));
	        return;
	    }
		
		for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
		{
			ret = AC_MANAGE_DBUS_ERROR;
			ret = ac_manage_config_trap_service(paraNode->connection, state); 
			if((AC_MANAGE_SUCCESS != ret)&&(1 == flag))
			{
				flag = 0;
				ShowAlert(search(lsnmpd,"config_trap_service_fail"));
			}
		}
	}		
	
	if(1 == flag)
	{
		ShowAlert(search(lpublic,"oper_succ"));
	}
}


