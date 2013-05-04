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
* wp_snmp_config.c
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


void ShowSnmpConfigPage(char *m,struct list *lpublic,struct list *lsnmpd);  
void ConfigSnmp(int cllection_mode,DBusConnection *select_connection,instance_parameter *paraHead,struct list *lpublic,struct list *lsnmpd);


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
    cgiFormStringNoNewlines("encry_snmpconfig",encry,BUF_LEN);
  }  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowSnmpConfigPage(encry,lpublic,lsnmpd);
  release(lpublic);  
  release(lsnmpd);
  destroy_ccgi_dbus();
  return 0;
}

void ShowSnmpConfigPage(char *m,struct list *lpublic,struct list *lsnmpd)
{    
  char IsSubmit[5] = { 0 };
  int i = 0,limit = 0;     
  int cllection_mode = 0;/*0表示关闭分板采集，1表示开启分板采集*/  
  char select_slotid[10] = { 0 };
  char temp[10] = { 0 };
  unsigned int slot_id = 0;
  DBusConnection *select_connection = NULL;
  instance_parameter *paraHead = NULL,*paraNode = NULL;
  char *endptr = NULL;	
  int ret = AC_MANAGE_DBUS_ERROR;
  unsigned int snmp_state = 0;
  int status = 1;
  FILE *fp = NULL;
  char BindInter[50] = { 0 };
  char *retu = NULL;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>SNMP</title>");
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

  if(snmp_cllection_mode(ccgi_dbus_connection))
  {
	cllection_mode = 1;
  }
  else
  {
	cllection_mode = 0;
  }	
  if(1 == cllection_mode)/*开启分板采集*/
  {
  	  slot_id=strtoul(select_slotid,&endptr,10);
	  get_slot_dbus_connection(slot_id,&select_connection,SNMPD_INSTANCE_MASTER_V3);
  }

  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("snmpconfig_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	ConfigSnmp(cllection_mode,select_connection,paraHead,lpublic,lsnmpd);
  }

  /*update snmp sysinfo*/
  if((cgiFormSubmitClicked("update_sysinfo") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	ret = AC_MANAGE_DBUS_ERROR;
  	if(1 == cllection_mode) /*开启分板采集*/
	{
		ret = ac_manage_config_snmp_update_sysinfo(select_connection);
	}
	else
	{
		ret = ac_manage_config_snmp_update_sysinfo(ccgi_dbus_connection);
	}
	if(AC_MANAGE_SUCCESS == ret) 
	{
		ShowAlert(search(lsnmpd,"update_sysinfo_succ"));
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) 
	{
		ShowAlert(search(lsnmpd,"disable_snmp_service"));
    }
    else
	{
		ShowAlert(search(lsnmpd,"update_sysinfo_fail"));
    }
  }
  if(snmp_cllection_mode(ccgi_dbus_connection))
  {
	cllection_mode = 1;
  }
  else
  {
	cllection_mode = 0;
  }	  
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>SNMP</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");        
	 
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=snmpconfig_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
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
					"<td align=left id=tdleft><a href=wp_snmp_summary.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>SNMP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"info")); 					  
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san> SNMP</font></td>",search(lpublic,"menu_san"),search(lpublic,"config"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");	 
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_community_list.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"community_list"));                       
                  fprintf(cgiOut,"</tr>");
                  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_community_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"add_community"));                       
                  fprintf(cgiOut,"</tr>");

				  ret = AC_MANAGE_DBUS_ERROR;
				  if(1 == cllection_mode)
			      {
			      	ret = ac_manage_show_snmp_state(select_connection, &snmp_state);
					limit = 7;
				  }
				  else
				  {
				  	ret = ac_manage_show_snmp_state(ccgi_dbus_connection, &snmp_state);
				  	limit = 5;
				  }

				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
			    
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
	"<table width=500 border=0 cellspacing=0 cellpadding=0>"\
		"<tr height=30>"\
			"<td width=110>SNMP %s:</td>",search(lpublic,"decentralized_collect"));
			if(1 == cllection_mode)
			{
				fprintf(cgiOut,"<td width=100 align=left><input name=decentralized_collect type=radio value=open checked=checked >%s</td>",search(lsnmpd,"open"));
				fprintf(cgiOut,"<td width=190><input type=radio name=decentralized_collect value=close >%s</td>",search(lsnmpd,"close"));
			}
			else
			{
				fprintf(cgiOut,"<td width=100 align=left><input name=decentralized_collect type=radio value=open >%s</td>",search(lsnmpd,"open"));
				fprintf(cgiOut,"<td width=190><input type=radio name=decentralized_collect value=close checked=checked>%s</td>",search(lsnmpd,"close"));
			}
			fprintf(cgiOut,"<td width=100>&nbsp;</td>"\
		"</tr>");
		if(1 == cllection_mode)/*开启分板采集*/
	    {
		  fprintf(cgiOut,"<tr height=30>"\
			"<td>SLOT ID:</td>"\
			  "<td colspan=3>"\
				  "<select name=slot_id id=slot_id style=width:80px onchange=slotid_change(this,\"wp_snmp_config.cgi\",\"%s\")>",m);
				  for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
				  {
					 memset(temp,0,sizeof(temp));
					 snprintf(temp,sizeof(temp)-1,"%d",paraNode->parameter.slot_id);
		  
					 if(strcmp(select_slotid,temp) == 0)
					   fprintf(cgiOut,"<option value='%s' selected=selected>%s",temp,temp);
					 else
					   fprintf(cgiOut,"<option value='%s'>%s",temp,temp);
				  } 		  
				  fprintf(cgiOut,"</select>"\
			  "</td>"\
		  "</tr>");
	    }
		fprintf(cgiOut,"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"snmp_state"));
			fprintf(cgiOut,"<td colspan=2><select name=snmp_service id=snmp_service style=width:80px>"\
				"<option value=>"\
				"<option value=enable>enable"\
				"<option value=disable>disable"\
			"</select></td>"\			
			"<td align=left><input type=submit style=\"width:100px; margin-left:20px\" border=0 name=update_sysinfo style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lsnmpd,"update_sysinfo"));
		fprintf(cgiOut,"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"cache_time"));
			if((AC_MANAGE_SUCCESS == ret)&&(snmp_state))
				fprintf(cgiOut,"<td><input name=cache_time size=11 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" style=\"background-color:#cccccc\" disabled></td>");
			else
				fprintf(cgiOut,"<td><input name=cache_time size=11 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
			fprintf(cgiOut,"<td colspan=2><font color=red>(1--1800)%s</font></td>",search(lpublic,"second"));
		fprintf(cgiOut,"</tr>"\
		"<tr height=30>"\
			"<td>SNMP %s:</td>",search(lpublic,"mode"));
			fprintf(cgiOut,"<td>");
				if((AC_MANAGE_SUCCESS == ret)&&(snmp_state))
					fprintf(cgiOut,"<select name=snmp_mode id=snmp_mode style=width:80px disabled>");
				else
					fprintf(cgiOut,"<select name=snmp_mode id=snmp_mode style=width:80px>");
				fprintf(cgiOut,"<option value=>"\
				"<option value=v1>v1"\
				"<option value=v2>v2"\
				"<option value=v3>v3"\
				"</select>"\
			"</td>"\
			"<td colspan=2>");
				if((AC_MANAGE_SUCCESS == ret)&&(snmp_state))
					fprintf(cgiOut,"<select name=snmp_mode_state id=snmp_mode_state style=width:80px disabled>");
				else
					fprintf(cgiOut,"<select name=snmp_mode_state id=snmp_mode_state style=width:80px>");
				fprintf(cgiOut,"<option value=>"\
				"<option value=enable>enable"\
				"<option value=disable>disable"\
				"</select>"\
			"</td>"\
		"</tr>"\
		"<tr height=30 valign=top style=\"padding-top:10px\">"\
			"<td colspan=4 width=282>"\
				"<fieldset align=left>"\
				"<legend><font color=Navy>%s</font></legend>",search(lsnmpd,"config_snmp_port"));
					fprintf(cgiOut,"<table width=282 border=0 cellspacing=0 cellpadding=0>"\
						"<tr height=30>"\
							"<td>%s:</td>",search(lpublic,"version_opt"));
							fprintf(cgiOut,"<td colspan=2>");
								if((AC_MANAGE_SUCCESS == ret)&&(snmp_state))
									fprintf(cgiOut,"<select name=snmp_p_type id=snmp_p_type style=width:80px disabled>");
								else
									fprintf(cgiOut,"<select name=snmp_p_type id=snmp_p_type style=width:80px>");
								fprintf(cgiOut,"<option value=>"\
								"<option value=apply>apply"\
								"<option value=delete>delete"\
							"</select></td>"\
						"</tr>"\
						"<tr height=30>"\
							"<td>%s:</td>",search(lsnmpd,"interface_name"));
							fprintf(cgiOut,"<td colspan=2>");
								status = system("snmp_bind_inter.sh"); 
								if(status==0)
								{
									if((AC_MANAGE_SUCCESS == ret)&&(snmp_state))
										fprintf(cgiOut,"<select name=snmp_p_ifname id=snmp_p_ifname style=width:80px disabled>");
									else
										fprintf(cgiOut,"<select name=snmp_p_ifname id=snmp_p_ifname style=width:80px>");
									if((fp=fopen("/var/run/apache2/snmp_bind_inter.tmp","r"))==NULL) 	 /*以只读方式打开资源文件*/
									{
									   ShowAlert(search(lpublic,"error_open"));
									}
									else
									{
										fprintf(cgiOut,"<option value=>");
										memset(BindInter,0,sizeof(BindInter));
										retu=fgets(BindInter,50,fp);
										while(retu!=NULL)
										{
											fprintf(cgiOut,"<option value=%s>%s",retu,retu);
											memset(BindInter,0,sizeof(BindInter));
											retu=fgets(BindInter,50,fp);
										}				   
										fclose(fp);
									}
									fprintf(cgiOut,"</select>");
								}
								else
								{
				  				    fprintf(cgiOut,"%s",search(lpublic,"exec_shell_fail"));
								}
							fprintf(cgiOut,"</td>"\
						"</tr>"\
						"<tr height=30>"\
							"<td width=102>%s:</td>",search(lpublic,"port"));
							if((AC_MANAGE_SUCCESS == ret)&&(snmp_state))
								fprintf(cgiOut,"<td width=100><input name=snmp_p_port size=11 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" style=\"background-color:#cccccc\" disabled></td>");
							else
								fprintf(cgiOut,"<td width=100><input name=snmp_p_port size=11 maxLength=5 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
							fprintf(cgiOut,"<td align=left width=80><font color=red>(1--65535)</font></td>"
						"</tr>"\
					"</table>"\
				"</fieldset>"\
			"</td>"\
		"</tr>"\
		"<tr>"\
	 	"<td><input type=hidden name=encry_snmpconfig value=%s></td>",m);
		fprintf(cgiOut,"<td><input type=hidden name=SLOT_ID value=%s></td>",select_slotid);
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


void ConfigSnmp(int cllection_mode,DBusConnection *select_connection,instance_parameter *paraHead,struct list *lpublic,struct list *lsnmpd)
{
	int flag = 1;
	instance_parameter *paraNode = NULL;
	char decentralized_collect[10] = { 0 };
	int decentralized_state = 0;
	char snmp_service[10] = { 0 };
	unsigned int state = 0;
	int ret = AC_MANAGE_DBUS_ERROR;
	char snmp_p_type[10] = { 0 };
	char snmp_p_ifname[50] = { 0 };
	char snmp_p_port[10] = { 0 };
	unsigned int snmp_port = 0;
	int ifindex = -1;
	char cache_time[10] = { 0 };
	unsigned int cachetime = 0;
	char snmp_mode[10] = { 0 };
	char snmp_mode_state[10] = { 0 };
	unsigned int snmp_mode_int = 2; //v2c
		

	/*config snmp decentralized collect*/
	memset(decentralized_collect,0,sizeof(decentralized_state));
	cgiFormStringNoNewlines("decentralized_collect",decentralized_collect,10);  
	if(strcmp(decentralized_collect,""))
	{
		if (0 == strcmp(decentralized_collect, "open"))
		{
			decentralized_state = 1;
		}
		else if(0 == strcmp(decentralized_collect, "close"))
		{
			decentralized_state = 0;
		}
		else
		{
			decentralized_state = -1;
			flag = 0;
			ShowAlert(search(lpublic,"input_para_error"));
		}	
		if(-1 != decentralized_state)
		{
			for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
			{
				ret = ac_manage_config_snmp_collection_mode(paraNode->connection, decentralized_state); 
				if((AC_MANAGE_DBUS_ERROR == ret)&&(1 == flag))
				{
					flag = 0;
					ShowAlert(search(lsnmpd,"config_decentralized_collect_fail"));
				}
			}
		}
	}	

	/*config snmp cache time*/
	memset(cache_time,0,sizeof(cache_time));
	cgiFormStringNoNewlines("cache_time",cache_time,10);
	if(strcmp(cache_time,""))
	{
		cachetime = atoi(cache_time);
		if((cachetime<1)||(cachetime>1800))
		{
			flag = 0;
			ShowAlert(search(lsnmpd,"cache_time_illegal"));
		}
		else
		{
			ret = AC_MANAGE_DBUS_ERROR;
			if(1 == cllection_mode) /*开启分板采集*/
			{
				ret = ac_manage_config_snmp_cachetime(select_connection, cachetime);
			}
			else
			{
				ret = ac_manage_config_snmp_cachetime(ccgi_dbus_connection, cachetime);
			}
			if(AC_MANAGE_SUCCESS == ret) 
			{
				;
			}
			else if(AC_MANAGE_DBUS_ERROR == ret) 
			{
				flag = 0;
				ShowAlert(search(lsnmpd,"cache_time_fail"));
			}
			else if(AC_MANAGE_SERVICE_ENABLE == ret) 
			{
				flag = 0;
				ShowAlert(search(lsnmpd,"disable_snmp_service"));
			}
			else
			{
				flag = 0;
				ShowAlert(search(lsnmpd,"cache_time_fail"));
			}
		}
	}	


	/*config snmp mode*/
	memset(snmp_mode,0,sizeof(snmp_mode));
	cgiFormStringNoNewlines("snmp_mode",snmp_mode,10);
	memset(snmp_mode_state,0,sizeof(snmp_mode_state));
	cgiFormStringNoNewlines("snmp_mode_state",snmp_mode_state,10);
	if((strcmp(snmp_mode,""))&&(strcmp(snmp_mode_state,"")))
	{
		if(0 == strcmp(snmp_mode, "v1")) {
			snmp_mode_int = 1;
		}
		else if(0 == strcmp(snmp_mode, "v2")) {
			snmp_mode_int = 2;
		}
		else if(0 == strcmp(snmp_mode, "v3")) {
			snmp_mode_int = 3;
		}
		else 
		{
			snmp_mode_int = -1;
			flag = 0;
			ShowAlert(search(lpublic,"input_para_error"));
		}
	
		state = RULE_ENABLE;
		if(0 == strcmp(snmp_mode_state, "enable")) {
			state = RULE_ENABLE;
		}
		else if(0 ==  strcmp(snmp_mode_state, "disable")) {
			state = RULE_DISABLE;
		}

		if(-1 != snmp_mode_int)
		{
			ret = AC_MANAGE_DBUS_ERROR;
			if(1 == cllection_mode)	/*开启分板采集*/
			{
				ret = ac_manage_config_snmp_version_mode(select_connection, snmp_mode_int, state);
			}
			else
			{
				ret = ac_manage_config_snmp_version_mode(ccgi_dbus_connection, snmp_mode_int, state);
			}
			if(AC_MANAGE_SUCCESS == ret) 
			{
		        ;
		    }
		    else if(AC_MANAGE_DBUS_ERROR == ret) 
			{
				flag = 0;
				ShowAlert(search(lsnmpd,"config_snmp_mode_fail"));
		    }
		    else if(AC_MANAGE_SERVICE_ENABLE == ret) 
			{
				flag = 0;
				ShowAlert(search(lsnmpd,"disable_snmp_service"));
		    }
		    else 
			{
				flag = 0;
				ShowAlert(search(lsnmpd,"config_snmp_mode_fail"));
		    }
		}
	}
	else if(!((0 == (strcmp(snmp_mode,"")))&&(0 == (strcmp(snmp_mode_state,"")))))
	{
		flag = 0;
		ShowAlert(search(lpublic,"para_incom"));
	}


	/*config snmp port*/
	memset(snmp_p_type,0,sizeof(snmp_p_type));
	cgiFormStringNoNewlines("snmp_p_type",snmp_p_type,10); 
	memset(snmp_p_ifname,0,sizeof(snmp_p_ifname));
	cgiFormStringNoNewlines("snmp_p_ifname",snmp_p_ifname,50); 
	memset(snmp_p_port,0,sizeof(snmp_p_port));
	cgiFormStringNoNewlines("snmp_p_port",snmp_p_port,10);
	if((strcmp(snmp_p_type,""))&&(strcmp(snmp_p_ifname,""))&&(strcmp(snmp_p_port,"")))
	{
		snmp_port = atoi(snmp_p_port);
		if(0 == strcmp(snmp_p_type, "apply")) 
		{
        	state = 1;
	    }
	    else if(0 == strcmp(snmp_p_type, "delete")) 
		{
	        state = 0;
	    }
	    else 
		{
	        state = -1;
			flag = 0;
			ShowAlert(search(lpublic,"input_para_error"));
	    }

		if(1 == state)
		{
			ifindex = ifname2ifindex_by_ioctl(snmp_p_ifname);
			if(0 == ifindex)
			{
				flag = 0;
				ShowAlert(search(lpublic,"Vrrp_Interface_Not_Exist"));
			}
		}
		if((-1 != state)&&(0 != ifindex))
		{
			ret = AC_MANAGE_DBUS_ERROR;
			if(1 == cllection_mode)	/*开启分板采集*/
			{
				ret = ac_manage_config_snmp_pfm_requestpkts(select_connection, snmp_p_ifname, snmp_port, state);
			}
			else
			{
				ret = ac_manage_config_snmp_pfm_requestpkts(ccgi_dbus_connection, snmp_p_ifname, snmp_port, state);
			}
			if(AC_MANAGE_SUCCESS == ret) 
			{
		        ;
		    }
		    else if(AC_MANAGE_DBUS_ERROR == ret)
			{
				flag = 0;
				ShowAlert(search(lsnmpd,"config_snmp_port_fail"));
		    }
		    else if(AC_MANAGE_SERVICE_ENABLE == ret) 
			{
				flag = 0;
				ShowAlert(search(lsnmpd,"disable_snmp_service"));
		    }
		    else if(AC_MANAGE_CONFIG_EXIST == ret) 
			{
				flag = 0;
				ShowAlert(search(lpublic,"Vrrp_Interface_Has_Exist"));
		    }
		    else if(AC_MANAGE_CONFIG_NONEXIST == ret) 
			{
				flag = 0;
				ShowAlert(search(lpublic,"Vrrp_Interface_Not_Exist"));
		    }
			else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) 
			{
				flag = 0;
				ShowAlert(search(lpublic,"input_para_error"));
		    }
		    else if(AC_MANAGE_CONFIG_FAIL == ret)
			{
				flag = 0;
				ShowAlert(search(lsnmpd,"delete_udp_port_config"));
		    }
		    else 
			{
				flag = 0;
		        ShowAlert(search(lsnmpd,"config_snmp_port_fail"));
		    }
		}
	}
	else if(!((0 == (strcmp(snmp_p_type,"")))&&(0 == (strcmp(snmp_p_ifname,"")))&&(0 == (strcmp(snmp_p_port,"")))))
	{
		flag = 0;
		ShowAlert(search(lpublic,"para_incom"));
	}


	/*config snmp service*/
	memset(snmp_service,0,sizeof(snmp_service));
	cgiFormStringNoNewlines("snmp_service",snmp_service,10); 
	if(strcmp(snmp_service,""))
	{
		if (!strcmp(snmp_service, "enable"))
		{
			state = 1;
		}
		else if (!strcmp(snmp_service, "disable")) 
		{
			state = 0;
		}
		else
		{
			state = -1;
			flag = 0;
			ShowAlert(search(lpublic,"input_para_error"));
		}	
		if(-1 != state)
		{
			ret = AC_MANAGE_DBUS_ERROR;
			if(1 == cllection_mode)	/*开启分板采集*/
			{
				ret = ac_manage_config_snmp_service(select_connection, state);
			}
			else
			{
				ret = ac_manage_config_snmp_service(ccgi_dbus_connection, state);
			}
			if(AC_MANAGE_SUCCESS == ret) 
			{
		        ;
		    }
		    else if(AC_MANAGE_DBUS_ERROR == ret) 
			{
				flag = 0;
				ShowAlert(search(lsnmpd,"config_snmp_service_fail"));
		    }
		    else 
			{
				flag = 0;
				ShowAlert(search(lpublic,"error"));
		    }
		}
	}
	
	
	if(1 == flag)
	{
		ShowAlert(search(lpublic,"oper_succ"));
	}
}

