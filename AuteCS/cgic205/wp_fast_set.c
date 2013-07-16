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
* wp_fast_set.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* system function for syslog info config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list.h"
#include "ws_dbus_list_interface.h"
#include "ws_fast_forward.h"




void show_error(struct list *lpublic, int ret);
int ShowWebservicePage(struct list *lpublic, struct list *lcontrol);     /*m代表加密后的字符串*/

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	  /*解析control.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");

	ShowWebservicePage(lpublic,lcontrol);
  	release(lpublic);  
  	release(lcontrol);

	return 0;
}

int ShowWebservicePage(struct list *lpublic, struct list *lcontrol)
{ 
	 
	char *encry=(char *)malloc(BUF_LEN);				
	char *str;
	int ret = 0;
	int i=0;
	struct fast_fwd_info service_info;
	int retu=0;
	int flag=0;
	char   ip1[4]={0};
	char   ip2[4]={0};
	char   ip3[4]={0};
	char   ip4[4]={0};
	char clear_ip[32]={0};
	char cpu_type[10]={0};
	char tag_type[10]={0};
	char bucket_entry[10]={0};
	int op_ret=0;
	unsigned int max_entry = 0;
	char master_cpu[10]={0};
	char slave_cpu[10]={0};
	int op_ret1=0;
	int fast_icmp=0;
	int  pure_state = 0;

	  	 
	memset(encry,0,BUF_LEN);
	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
		return 0;
	}
	  
	ccgi_dbus_init();
	instance_parameter *paraHead2 = NULL;
	instance_parameter *p_q = NULL;
	list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
	char plotid[10] = {0};
	int pid = 0;
	cgiFormStringNoNewlines("plotid",plotid,sizeof(plotid));
	pid = atoi(plotid);
	if(0 == pid)
	{
		for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
		{
			pid = p_q->parameter.slot_id;
			break;
		}
	}
	
	  /***********************2008.5.26*********************/
	  cgiHeaderContentType("text/html");
	  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	  fprintf(cgiOut,"<title>%s</title>",search(lpublic,"fast_for_set"));
	  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
					"<style type=text/css>"\
						".a3{width:30;border:0; text-align:center}"\
					"</style>"\
					"<script language=javascript src=/ip.js>"\
					"</script>"\
					"</head>"\
					"<body>");

	  if(cgiFormSubmitClicked("fast_config") == cgiFormSuccess)
	  {
			ret=0;
			memset(ip1,0,4);
			memset(ip2,0,4);
			memset(ip3,0,4);
			memset(ip4,0,4);
			memset(clear_ip,0,32);
			memset(cpu_type,0,10);
			memset(tag_type,0,10);
			memset(bucket_entry,0,10);
			cgiFormStringNoNewlines("cpu_type",cpu_type,10);
			cgiFormStringNoNewlines("tag_type",tag_type,10);
			cgiFormStringNoNewlines("bucket_entry",bucket_entry,10);
			cgiFormStringNoNewlines("clear_ip1",ip1,4);
			cgiFormStringNoNewlines("clear_ip2",ip2,4);
			cgiFormStringNoNewlines("clear_ip3",ip3,4);
			cgiFormStringNoNewlines("clear_ip4",ip4,4);
			sprintf(clear_ip,"%ld.%ld.%ld.%ld",strtoul(ip1,0,10),strtoul(ip2,0,10),strtoul(ip3,0,10),strtoul(ip4,0,10));

			if((strcmp(ip1,"")!=0)&&(strcmp(ip2,"")!=0)&&(strcmp(ip3,"")!=0)&&(strcmp(ip4,"")!=0))
			{
				ret = ccgi_clear_rule_ip_cmd(clear_ip,pid);
				if(ret == 0)
				{
					ShowAlert(search(lcontrol,"suc_clear_ip"));
				}
				else
				{
					ShowAlert(search(lcontrol,"fail_clear_ip"));
				}
			}
			if(strcmp(bucket_entry,"")!=0)
			{
				ret = ccgi_set_fastfwd_bucket_entry_cmd(bucket_entry,pid);
				if(ret == 0)
				{
					ShowAlert(search(lcontrol,"suc_bucket_entry"));
				}
				else
				{
					ShowAlert(search(lcontrol,"fail_bucket_entry"));
				}
			}			
			if((strcmp(cpu_type,"none")!=0)&&(strcmp(tag_type,"none")!=0))
			{
				if(strcmp(cpu_type,"master")==0)
				{
					flag=0;
				}
				else
				{
					flag=1;
				}
				ret = ccgi_config_fast_forward_tag_type_cmd(tag_type,pid,flag);
				if(ret == 0)
				{
					ShowAlert(search(lcontrol,"suc_tag_type"));
				}
				else
				{
					ShowAlert(search(lcontrol,"fail_tag_type"));
				}
			}
	  }
	  if(cgiFormSubmitClicked("service_starts") == cgiFormSuccess)
	  {
		  retu=ccgi_config_fast_forward_enable_cmd("enable",pid);
		  show_error(lpublic,retu);
	  }
	  if(cgiFormSubmitClicked("service_ends") == cgiFormSuccess)
	  {
		  retu=ccgi_config_fast_forward_enable_cmd("disable",pid);
		  show_error(lpublic,retu);
	  }
	  if(cgiFormSubmitClicked("ICMP_starts") == cgiFormSuccess)
	  {
		  retu=ccgi_fastfwd_learned_icmp_enable_cmd("enable",pid);
		  show_error(lpublic,retu);
	  }
	  if(cgiFormSubmitClicked("ICMP_ends") == cgiFormSuccess)
	  {
		  retu=ccgi_fastfwd_learned_icmp_enable_cmd("disable",pid);
		  show_error(lpublic,retu);
	  }
	  if(cgiFormSubmitClicked("pure_ip_starts") == cgiFormSuccess)
	  {
		  retu=ccgi_fastfwd_pure_ip_enable_cmd(pid,"enable");
		  show_error(lpublic,retu);
	  }
	  if(cgiFormSubmitClicked("pure_ip_ends") == cgiFormSuccess)
	  {
		  retu=ccgi_fastfwd_pure_ip_enable_cmd(pid,"disable");
		  show_error(lpublic,retu);
	  }
	  if(cgiFormSubmitClicked("clear_old") == cgiFormSuccess)
	  {
		  retu=ccgi_clear_aging_rule_cmd(pid);
		  show_error(lpublic,retu);
	  }
	  if(cgiFormSubmitClicked("clear_all_mas") == cgiFormSuccess)
	  {
		  retu=ccgi_clear_rule_all_cmd(pid,0);
		  show_error(lpublic,retu);
	  }	  
	  if(cgiFormSubmitClicked("clear_all_sla") == cgiFormSuccess)
	  {
		  retu=ccgi_clear_rule_all_cmd(pid,1);
		  show_error(lpublic,retu);
	  }
	 
	  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>");  
	  fprintf(cgiOut,"<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lpublic,"fast_for_set"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
	    fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	 
			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");  
		fprintf(cgiOut,"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=fast_config style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));	
     	fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		fprintf(cgiOut,"</tr>"\
		"</table>"); 	
		fprintf(cgiOut,"</td>"\
		"<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
		"<tr>"); 
		
			fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
			"<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"); 
			  fprintf(cgiOut,"<tr height=4 valign=bottom>"\
				  "<td width=120>&nbsp;</td>"\
				  "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
			  "</tr>"\
			  "<tr>"\
				  "<td>");
			  fprintf(cgiOut,"<table width=120 border=0 cellspacing=0 cellpadding=0>");

			  fprintf(cgiOut,"<tr height=25>"\
				  "<td id=tdleft>&nbsp;</td>"\
				"</tr>"); 

			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"set_fast_for"));	/*突出显示*/
			fprintf(cgiOut,"</tr>");
			
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_fast_show.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"show_fast_for"));
			fprintf(cgiOut,"</tr>");
			for(i=0;i<27;i++)
			{
				fprintf(cgiOut,"<tr height=25>"\
				"<td id=tdleft>&nbsp;</td>"\
				"</tr>");
			}
			fprintf(cgiOut,"</table>"); 
			fprintf(cgiOut,"</td>"\
			"<td  align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">");

			fprintf(cgiOut,"<table width=600 border=0 cellspacing=0 cellpadding=0>"); 
			/****************设置槽号*************************/
			fprintf(cgiOut,"<tr height=20><td></td></tr>");
			fprintf(cgiOut,"<tr align=left>");
			fprintf(cgiOut,"<td colspan=10>SLOT ID:");
			fprintf( cgiOut, "<select name=insid onchange=slotid_change(this)>");
			for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
			{
				if(p_q->parameter.slot_id == pid)
				{
					fprintf(cgiOut,"<option value=\"%d\" selected>%d</option>",p_q->parameter.slot_id,p_q->parameter.slot_id);
				}
				else
				{
					fprintf(cgiOut,"<option value=\"%d\">%d</option>",p_q->parameter.slot_id,p_q->parameter.slot_id);
				}		
			}
			fprintf( cgiOut, "</select>\n");	
			fprintf(cgiOut,"</td>");
			fprintf(cgiOut,"</tr>");
			fprintf( cgiOut,"<script type=text/javascript>\n");
			fprintf( cgiOut,"function slotid_change( obj )\n"\
			"{\n"\
			"var slotid = obj.options[obj.selectedIndex].value;\n"\
			"var url = 'wp_fast_set.cgi?UN=%s&plotid='+slotid;\n"\
			"window.location.href = url;\n"\
			"}\n", encry);
			fprintf( cgiOut,"</script>\n" );
			/***************快转服务设置***************/
			op_ret=ccgi_show_fast_forward_info_cmd(pid,&service_info);
			if(op_ret != -4)
			{
			   fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
				 "<td width=500>"\
				   "<fieldset align=left>"\
					 "<legend><font color=Navy>%s%s</font></legend>",search(lcontrol,"set"),search(lcontrol,"fast_service"));
				 fprintf(cgiOut,"<table width=400 border=0 cellspacing=0 cellpadding=0>"\
				   "<tr height=30>");
					 fprintf(cgiOut,"<td width=200 align=left><input type=submit name=service_starts value=\"%s %s\"></td>",search(lcontrol,"enable_gate"),search(lcontrol,"fast_service"));
					 fprintf(cgiOut,"<td width=200 align=left><input type=submit name=service_ends	value=\"%s %s\"></td>",search(lcontrol,"stop"),search(lcontrol,"fast_service"));
					 fprintf(cgiOut,"</tr>");	 
					 fprintf(cgiOut,"<tr height=30>");
					 if((service_info.fast_fwd_enable == 1)&&(op_ret == 0))
					 {
						 fprintf(cgiOut,"<td colspan=2 align=left>%s:fast_forward is enable,coremask is 0x%02x</td>",search(lcontrol,"current_status"),service_info.fast_fwd_coremask);
					 }
					 else if(op_ret == 0)
					 {
						 fprintf(cgiOut,"<td colspan=2 align=left>%s:fast_forward is disable,coremask is 0x%02x</td>",search(lcontrol,"current_status"),service_info.fast_fwd_coremask);
					 }
					 else
					 {
						 fprintf(cgiOut,"<td colspan=2 align=left><font color=red>%s:error</font></td>",search(lcontrol,"current_status"));
					 }
				   fprintf(cgiOut,"</tr>");	   
				   fprintf(cgiOut,"</table>");
				   fprintf(cgiOut,"</fieldset>");
				   fprintf(cgiOut,"</td>");
				   fprintf(cgiOut,"</tr>");
				/***************设置快转流表中hash冲突时挂的链表项数***************/
				op_ret=ccgi_show_fastfwd_bucket_entry_cmd(pid,&max_entry);
				fprintf(cgiOut,"<tr height=20><td></td></tr>");
				fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
				  "<td width=500>"\
					"<fieldset align=left>"\
					  "<legend><font color=Navy>%s</font></legend>",search(lcontrol,"set_con_hash"));
					  fprintf(cgiOut,"<table width=400 border=0 cellspacing=0 cellpadding=0>"\
						"<tr height=30>");
							 fprintf(cgiOut,"<td width=300>%s:  <input name=bucket_entry size=15 maxLength=5><font color=red>(2--32767)</font></td>",search(lcontrol,"bucket_entry"));
					fprintf(cgiOut,"</tr>");	
					  fprintf(cgiOut,"<tr height=30>");
					  if(op_ret == 0)
					  {
						  fprintf(cgiOut,"<td align=left>%s:fastfwd rule bucket entry %lu</td>",search(lcontrol,"current_status"),max_entry);
					  }
					  else
					  {
						  fprintf(cgiOut,"<td colspan=2 align=left><font color=red>%s:error</font></td>",search(lcontrol,"current_status"));
					  }
					fprintf(cgiOut,"</tr>");	
					fprintf(cgiOut,"</table>");
					fprintf(cgiOut,"</fieldset>");
					fprintf(cgiOut,"</td>");
					fprintf(cgiOut,"</tr>");
		  		/*************设置快转标签类型*****************/
				op_ret=ccgi_show_fast_forward_tag_type_cmd(pid,0,master_cpu);
				op_ret1=ccgi_show_fast_forward_tag_type_cmd(pid,1,slave_cpu);
		 		 fprintf(cgiOut,"<tr height=20><td></td></tr>");
				fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
				  "<td width=500>"\
					"<fieldset align=left>"\
					  "<legend><font color=Navy>%s</font></legend>",search(lcontrol,"fast_tag_type"));
			  fprintf(cgiOut,"<table width=400 border=0 cellspacing=0 cellpadding=0>"\
				"<tr height=30>");
			  
			  	fprintf(cgiOut,"<td width=200>%s:<select name=cpu_type id=cpu_type>"\
							  "<option value=none></option>"\
							  "<option value=master>master</option>"\
							  "<option value=slave>slave</option>"\
						   "</select></td>",search(lcontrol,"cpu_type"));
				 fprintf(cgiOut,"<td width=200>%s:<select name=tag_type id=tag_type>"\
								 "<option value=none></option>"\
								 "<option value=atomic>atomic</option>"\
								 "<option value=ordered>ordered</option>"\
								 "<option value=null>null</option>"\
							  "</select></td>",search(lcontrol,"fast_tag_type"));
				fprintf(cgiOut,"</tr>");	
				
			  fprintf(cgiOut,"<tr height=20>");
			  if(op_ret==0)
			  {
				  fprintf(cgiOut,"<td colspan=2 align=left>%s:master cpu tag type is %s</td>",search(lcontrol,"current_status"),master_cpu);
			  }
			  else
			  {
				  fprintf(cgiOut,"<td colspan=2 align=left><font color=red>%s:error</font></td>",search(lcontrol,"current_status"));
			  }
			  fprintf(cgiOut,"</tr>");	
			  fprintf(cgiOut,"<tr height=20>");
			  if(op_ret1 ==0)
			  {
				  fprintf(cgiOut,"<td colspan=2 align=left>%s:slave cpu tag type is %s</td>",search(lcontrol,"current_status"),slave_cpu);
			  }
			  else if(op_ret1 ==-4)
			  {
				  fprintf(cgiOut,"<td colspan=2 align=left>%s:slave cpu tag type is none exist</td>",search(lcontrol,"current_status"));
			  }
			  else
			  {
				  fprintf(cgiOut,"<td colspan=2 align=left><font color=red>%s:slave cpu tag type is error</font></td>",search(lcontrol,"current_status"));
			  }
			  fprintf(cgiOut,"</tr>");	
				fprintf(cgiOut,"</table>");
				fprintf(cgiOut,"</fieldset>");
				fprintf(cgiOut,"</td>");
				fprintf(cgiOut,"</tr>");
			  
		  		/*************设置快转转发icmp报文功能*****************/
				op_ret=ccgi_show_fast_forward_running_config_cmd(pid,&fast_icmp);
				fprintf(cgiOut,"<tr height=20><td></td></tr>");
				fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
				  "<td width=500>"\
					"<fieldset align=left>"\
					  "<legend><font color=Navy>%s</font></legend>",search(lcontrol,"set_fast_icmp"));
					  fprintf(cgiOut,"<table width=400 border=0 cellspacing=0 cellpadding=0>"\
						"<tr height=30>");
						  fprintf(cgiOut,"<td width=200 align=left><input type=submit name=ICMP_starts value=\"%s %s\"></td>",search(lcontrol,"enable_gate"),search(lcontrol,"for_icmp"));
						  fprintf(cgiOut,"<td width=200 align=left><input type=submit name=ICMP_ends  value=\"%s %s\"></td>",search(lcontrol,"stop"),search(lcontrol,"for_icmp"));
						fprintf(cgiOut,"</tr>");	
						if((op_ret ==0)&&(fast_icmp ==1))
						{
							fprintf(cgiOut,"<td colspan=2 align=left>%s:fast_icmp is enable</td>",search(lcontrol,"current_status"));
						}
						else
						{
							fprintf(cgiOut,"<td colspan=2 align=left>%s:fast_icmp is disable</td>",search(lcontrol,"current_status"));
						}
						fprintf(cgiOut,"</table>");
						fprintf(cgiOut,"</fieldset>");
						fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"</tr>");
			   /*************设置用户流量净荷统计功能*****************/
			   op_ret=ccgi_show_fwd_pure_ip_enable_cmd(pid,&pure_state);
			   fprintf(cgiOut,"<tr height=20><td></td></tr>");
				fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
				  "<td width=500>"\
					"<fieldset align=left>"\
					  "<legend><font color=Navy>%s</font></legend>",search(lcontrol,"set_fast_pure_ip"));
					  fprintf(cgiOut,"<table width=400 border=0 cellspacing=0 cellpadding=0>"\
						"<tr height=30>");
						  fprintf(cgiOut,"<td width=200 align=left><input type=submit name=pure_ip_starts value=\"%s %s\"></td>",search(lcontrol,"enable_gate"),search(lcontrol,"pure_ip"));
						  fprintf(cgiOut,"<td width=200 align=left><input type=submit name=pure_ip_ends  value=\"%s %s\"></td>",search(lcontrol,"stop"),search(lcontrol,"pure_ip"));
						fprintf(cgiOut,"</tr>");	
						
						fprintf(cgiOut,"<tr height=30>");
						if((pure_state == 1)&&(op_ret==0))
						{
							fprintf(cgiOut,"<td  colspan=2 align=left>%s:fast_forward pure ip forward is enable</td>",search(lcontrol,"current_status"));
						}
						else if(op_ret==0)
						{
							fprintf(cgiOut,"<td  colspan=2 align=left>%s:fast_forward pure ip forward is disable</td>",search(lcontrol,"current_status"));
						}
						else
						{
							fprintf(cgiOut,"<td colspan=2 align=left><font color=red>%s:error</font></td>",search(lcontrol,"current_status"));
						}
						fprintf(cgiOut,"</tr>");  
						fprintf(cgiOut,"</table>");
						fprintf(cgiOut,"</fieldset>");
						fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"</tr>");
			   /*************三种清除快转配置的方法*****************/
			   fprintf(cgiOut,"<tr height=20><td></td></tr>");
				fprintf(cgiOut,"<tr valign=top style=\"padding-top:10px\">"\
				  "<td width=500>"\
					"<fieldset align=left>"\
					  "<legend><font color=Navy>%s</font></legend>",search(lcontrol,"set_clear"));
					  fprintf(cgiOut,"<table width=400 border=0 cellspacing=0 cellpadding=0>"\
						"<tr height=30>");
						  fprintf(cgiOut,"<td width=130 align=left><input type=submit name=clear_old value=\"%s\"></td>",search(lcontrol,"clear_old"));
						  fprintf(cgiOut,"</tr>");
						  fprintf(cgiOut,"<tr height=5><td></td></tr>");
						  fprintf(cgiOut,"<tr>");
						  fprintf(cgiOut,"<td width=130 align=left><input type=submit name=clear_all_mas	value=\"%s\"></td>",search(lcontrol,"clear_all_mas"));
						  fprintf(cgiOut,"<td width=130 align=left><input type=submit name=clear_all_sla	value=\"%s\"></td>",search(lcontrol,"clear_all_sla"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=5><td></td></tr>");
							fprintf(cgiOut,"<tr  height=30>"\
											"<td  width=200 align=left>%s:</td>",search(lcontrol,"clear_by_ip"));
							fprintf(cgiOut,"<td width=200 align=left>");
							fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:150;font-size:9pt\">");
							fprintf(cgiOut,"<input type=text  name=clear_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
							fprintf(cgiOut,"<input type=text  name=clear_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							fprintf(cgiOut,"<input type=text  name=clear_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							fprintf(cgiOut,"<input type=text  name=clear_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							fprintf(cgiOut,"</div></td>");
							fprintf(cgiOut,"</tr>\n");
							fprintf(cgiOut,"</table>");
							fprintf(cgiOut,"</fieldset>");
							fprintf(cgiOut,"</td>");
							fprintf(cgiOut,"</tr>");
			   fprintf(cgiOut,"<tr height=10><td></td></tr>");
			   fprintf(cgiOut,"<tr height=20><td><font color=red>%s</font></td></tr>",search(lcontrol,"explanation"));
			}
			else
			{
				fprintf(cgiOut,"<tr height=60>");
				  fprintf(cgiOut,"<td width=400 align=center><font size=4>%s</font></td>",search(lcontrol,"no_load_fast"));
				fprintf(cgiOut,"</tr>");	
			}
		/***************SET END*****************/					
	   fprintf(cgiOut,"</table>"); 
				  fprintf(cgiOut,"</td>"\
				  "</tr>"\
				  "<tr height=4 valign=top>"\
					  "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
					  "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
				  "</tr>"\
				"</table>");
			  fprintf(cgiOut,"</td>"\
			  "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
			"</tr>"\
		  "</table></td>");
		fprintf(cgiOut,"</tr>"\
		"<tr>"\
			"<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
		  "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
			"<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
		"</tr>"\
		"</table>");
		fprintf(cgiOut,"</div>"\
		"</form>"\
		"</body>"\
		"</html>");
		free(encry);

return 0;
}
void show_error(struct list *lpublic, int retu)
{
	if(retu==0)
	{
		ShowAlert(search(lpublic,"oper_succ"));
	}
	else
	{
		ShowAlert(search(lpublic,"oper_fail"));
	}

}
