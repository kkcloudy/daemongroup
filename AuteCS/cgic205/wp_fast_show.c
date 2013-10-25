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
* wp_fast_show.c
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
	int ret = 0,i=0;
	int op_ret=-1,op_ret1=-1;
	char plotid[10] = {0};
	int pid = 0;
	struct fast_fwd_info service_info;
	rule_stats_t statistics;
	struct to_linux_flow_r data;
	float d_use_rate=0;
	float b_use_rate=0;
	int flag=0;
	char sorce_ip[32]={0};
	char sip1[4]={0};
	char sip2[4]={0};
	char sip3[4]={0};
	char sip4[4]={0};
	char des_ip[32]={0};
	char dip1[4]={0};
	char dip2[4]={0};
	char dip3[4]={0};
	char dip4[4]={0};
	char usr_ip[32]={0};
	char uip1[4]={0};
	char uip2[4]={0};
	char uip3[4]={0};
	char uip4[4]={0};
	char time_range[10]={0};
	char pro_type[10]={0};
	char cpu_type[10]={0};
	char d_port[10]={0};
	char s_port[10]={0};
	unsigned long long dynamic_a=0;
	unsigned long long static_a=0;
	se_interative_t  cmd_data;
	int p_buff[10];

	instance_parameter *paraHead2 = NULL;
	instance_parameter *p_q = NULL;
	  	 
	memset(encry,0,BUF_LEN);
	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
		return 0;
	}
	  
	ccgi_dbus_init();
	char choice[5]={0}; 
	memset(choice,0,5);
	cgiFormStringNoNewlines("SZ",choice,10);
	if(strcmp(choice,"")==0)
		strcpy(choice,"0");
	memset(plotid,0,10);
	cgiFormStringNoNewlines("plotid",plotid,sizeof(plotid));
	pid = atoi(plotid);
	list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
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
			if(strcmp(choice,"0")==0)
			{
				dynamic_a=0;
				static_a=0;
				op_ret=-1;
				memset(time_range,0,10);
				cgiFormStringNoNewlines("time_range",time_range,10);
				if(strcmp(time_range,"")!=0)
					op_ret=ccgi_show_aging_rule_cnt_cmd(time_range,pid,&dynamic_a,&static_a);
				if(op_ret!=0)
				{
					ShowAlert(search(lpublic,"oper_fail"));
				}
			}
		  
		  if(strcmp(choice,"1")==0)
		  {
			  memset(pro_type,0,10);
			  memset(d_port,0,10);
			  memset(s_port,0,10);
			  memset(sip1,0,4);
			  memset(sip2,0,4);
			  memset(sip3,0,4);
			  memset(sip4,0,4);
			  memset(dip1,0,4);
			  memset(dip2,0,4);
			  memset(dip3,0,4);
			  memset(dip4,0,4);
			  memset(sorce_ip,0,32);
			  memset(des_ip,0,32);
			  op_ret=-1;
			  cgiFormStringNoNewlines("pro_type",pro_type,10);
			  cgiFormStringNoNewlines("sip1",sip1,4);
			  cgiFormStringNoNewlines("sip2",sip2,4);
			  cgiFormStringNoNewlines("sip3",sip3,4);
			  cgiFormStringNoNewlines("sip4",sip4,4);
			  cgiFormStringNoNewlines("dip1",dip1,4);
			  cgiFormStringNoNewlines("dip2",dip2,4);
			  cgiFormStringNoNewlines("dip3",dip3,4);
			  cgiFormStringNoNewlines("dip4",dip4,4);
			  cgiFormStringNoNewlines("d_port",d_port,10);
			  cgiFormStringNoNewlines("s_port",s_port,10);
			  sprintf(sorce_ip,"%ld.%ld.%ld.%ld:%ld",strtoul(sip1,0,10),strtoul(sip2,0,10),strtoul(sip3,0,10),strtoul(sip4,0,10),strtoul(s_port,0,10));
			  sprintf(des_ip,"%ld.%ld.%ld.%ld:%ld",strtoul(dip1,0,10),strtoul(dip2,0,10),strtoul(dip3,0,10),strtoul(dip4,0,10),strtoul(d_port,0,10));
			if((strcmp(pro_type,"")!=0)&&(strcmp(sip1,"")!=0)&&(strcmp(sip2,"")!=0)&&(strcmp(sip3,"")!=0)\
				&&(strcmp(sip4,"")!=0)&&(strcmp(dip1,"")!=0)&&(strcmp(dip2,"")!=0)&&(strcmp(dip3,"")!=0)&&(strcmp(dip4,"")!=0)\
				&&(strcmp(d_port,"")!=0)&&(strcmp(s_port,"")!=0))
			{
				op_ret=ccgi_show_rule_five_tuple_cmd(pro_type,sorce_ip,des_ip,pid,&cmd_data);
				fprintf(stderr,"--------------------op_ret=%d",op_ret);
				if(op_ret!=0)
				{
					switch(op_ret)
					{
						case -3:
							ShowAlert(search(lpublic,"s_ip_port"));
							break;
						case -4:
							ShowAlert(search(lpublic,"d_ip_port"));
							break;
						case -5:
							ShowAlert(search(lpublic,"fail_com"));
							break;
						case -6:
							ShowAlert(search(lpublic,"agent_reply"));
							break;
						case -7:
							ShowAlert("No this rule");
							break;
						default:
							ShowAlert(search(lpublic,"oper_fail"));
							break;
					}
				}
			}
		  }
		  if(strcmp(choice,"2")==0)
		  {		
		  		memset(cpu_type,0,10);
				op_ret=-1;
			    cgiFormStringNoNewlines("cpu_type",cpu_type,10);
				if(strcmp(cpu_type,"master")==0)
				{
					flag=0;
				}
				else
				{
					flag=1;
				}
				op_ret=ccgi_show_rule_stats_cmd(pid,flag,&statistics);
				if(op_ret!=0)
				{
					ShowAlert(search(lpublic,"oper_fail"));
				}
		  }
		  if(strcmp(choice,"4")==0)
		  {
			  memset(uip1,0,4);
			  memset(uip2,0,4);
			  memset(uip3,0,4);
			  memset(uip4,0,4);
			  memset(usr_ip,0,32);
			  op_ret=-1;
			  cgiFormStringNoNewlines("uip1",uip1,4);
			  cgiFormStringNoNewlines("uip2",uip2,4);
			  cgiFormStringNoNewlines("uip3",uip3,4);
			  cgiFormStringNoNewlines("uip4",uip4,4);
			  sprintf(usr_ip,"%ld.%ld.%ld.%ld",strtoul(uip1,0,10),strtoul(uip2,0,10),strtoul(uip3,0,10),strtoul(uip4,0,10));
			if((strcmp(uip1,"")!=0)&&(strcmp(uip2,"")!=0)&&(strcmp(uip3,"")!=0)&&(strcmp(uip4,"")!=0))
			{
				op_ret=ccgi_show_user_acl_stats_cmd(usr_ip,pid,&statistics);
				if(op_ret!=0)
				{
					ShowAlert(search(lpublic,"oper_fail"));
				}
			}
		  }
		  if(strcmp(choice,"5")==0)
		  {		
		  		memset(cpu_type,0,10);
				op_ret=-1;
			    cgiFormStringNoNewlines("cpu_type",cpu_type,10);
				if(strcmp(cpu_type,"master")==0)
				{
					flag=0;
				}
				else
				{
					flag=1;
				}
				op_ret=ccgi_show_fpa_buff_counter_cmd(pid,flag,p_buff);
				if(op_ret!=0)
				{
					ShowAlert(search(lpublic,"oper_fail"));
				}
		  }
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
	    fprintf(cgiOut,"<input type=hidden name=plotid value=%d />",pid);
	    fprintf(cgiOut,"<input type=hidden name=SZ value=%s />",choice);
	 
			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");  
		fprintf(cgiOut,"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=fast_config style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));	
     	fprintf(cgiOut,"<td width=62 align=left><a href=wp_fast_set.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
			"<td align=left id=tdleft><a href=wp_fast_set.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"set_fast_for"));
			fprintf(cgiOut,"</tr>");
			
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"show_fast_for"));	/*突出显示*/
			fprintf(cgiOut,"</tr>");
			for(i=0;i<32;i++)
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
			fprintf(cgiOut,"<td width=60>SLOT ID:</td>");
			fprintf(cgiOut,"<td width=170  align=left><select name=insid onchange=slotid_change(this)>");
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
			fprintf( cgiOut, "</select>");	
			fprintf(cgiOut,"</td>");
			fprintf(cgiOut,"</tr>");
			fprintf( cgiOut,"<script type=text/javascript>\n");
			fprintf( cgiOut,"function slotid_change( obj )\n"\
			"{\n"\
			"var slotid = obj.options[obj.selectedIndex].value;\n"\
			"var url = 'wp_fast_show.cgi?UN=%s&plotid='+slotid;\n"\
			"window.location.href = url;\n"\
			"}\n", encry);
			fprintf( cgiOut,"</script>\n" );
			/*******************设置选项******************/
			op_ret1=ccgi_show_fast_forward_info_cmd(pid,&service_info);
			if(op_ret1 != -4)
			{
				fprintf(cgiOut,"<tr height=30>\n");
				fprintf(cgiOut,"<td width=60>%s:</td>",search(lcontrol,"show_type"));
				fprintf(cgiOut,"<td width=170  align=left>\n");
				fprintf(cgiOut,"<select name=udtype onchange=port_sel_change(this)>");
				if(strcmp(choice,"0")==0)
					fprintf(cgiOut,"<option value='0' selected=selected>%s</option>",search(lcontrol,"show_aging"));
				else
					fprintf(cgiOut,"<option value='0'>%s</option>",search(lcontrol,"show_aging"));
				if(strcmp(choice,"1")==0)
					fprintf(cgiOut,"<option value='1' selected=selected>%s</option>\n",search(lcontrol,"show_by_five"));
				else
					fprintf(cgiOut,"<option value='1'>%s</option>\n",search(lcontrol,"show_by_five"));

				if(strcmp(choice,"2")==0)
					fprintf(cgiOut,"<option value='2' selected=selected>%s</option>\n",search(lcontrol,"show_use_info"));
				else
					fprintf(cgiOut,"<option value='2'>%s</option>\n",search(lcontrol,"show_use_info"));

				if(strcmp(choice,"3")==0)
					fprintf(cgiOut,"<option value='3' selected=selected>%s</option>\n",search(lcontrol,"show_rec_linux"));
				else
					fprintf(cgiOut,"<option value='3'>%s</option>\n",search(lcontrol,"show_rec_linux"));

				if(strcmp(choice,"4")==0)
					fprintf(cgiOut,"<option value='4' selected=selected>%s</option>\n",search(lcontrol,"show_use_flow"));
				else
					fprintf(cgiOut,"<option value='4'>%s</option>\n",search(lcontrol,"show_use_flow"));

				if(strcmp(choice,"5")==0)
					fprintf(cgiOut,"<option value='5' selected=selected>%s</option>\n",search(lcontrol,"show_buff"));
				else
					fprintf(cgiOut,"<option value='5'>%s</option>\n",search(lcontrol,"show_buff"));

				fprintf(cgiOut,"</select>\n");
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"</tr>\n");
				fprintf(cgiOut,"<script type=text/javascript>\n");
				fprintf(cgiOut,"function port_sel_change( obj )\n"\
				   	"{\n"\
				       	"var selectz = obj.options[obj.selectedIndex].value;\n"\
				       	"var url = 'wp_fast_show.cgi?UN=%s&plotid=%d&SZ='+selectz;\n"\
				       	"window.location.href = url;\n"\
				   	"}\n", encry,pid);
				fprintf(cgiOut,"</script>\n" );
				/****************根据选项显示输入表单******************/
			if((strcmp(choice,"0")==0))
				{	
					memset(time_range,0,10);
					cgiFormStringNoNewlines("time_range",time_range,10);
					fprintf(cgiOut,"<tr height=30>");
					fprintf(cgiOut,"<td width=60>%s:</td>",search(lcontrol,"time_range"));
						 fprintf(cgiOut,"<td width=170 align=left><input name=time_range size=15 maxLength=6 value=%s><font color=red>(1--100000)</font></td>",time_range);
					fprintf(cgiOut,"</tr>");	
				}
				if((strcmp(choice,"1")==0))
				{
					  memset(pro_type,0,10);
					  memset(d_port,0,10);
					  memset(s_port,0,10);
					  memset(sip1,0,4);
					  memset(sip2,0,4);
					  memset(sip3,0,4);
					  memset(sip4,0,4);
					  memset(dip1,0,4);
					  memset(dip2,0,4);
					  memset(dip3,0,4);
					  memset(dip4,0,4);
					  cgiFormStringNoNewlines("pro_type",pro_type,10);
					  cgiFormStringNoNewlines("sip1",sip1,4);
					  cgiFormStringNoNewlines("sip2",sip2,4);
					  cgiFormStringNoNewlines("sip3",sip3,4);
					  cgiFormStringNoNewlines("sip4",sip4,4);
					  cgiFormStringNoNewlines("dip1",dip1,4);
					  cgiFormStringNoNewlines("dip2",dip2,4);
					  cgiFormStringNoNewlines("dip3",dip3,4);
					  cgiFormStringNoNewlines("dip4",dip4,4);
					  cgiFormStringNoNewlines("d_port",d_port,10);
					  cgiFormStringNoNewlines("s_port",s_port,10);
					fprintf(cgiOut,"<tr align=left>");
					fprintf(cgiOut,"<td width=60>%s:</td>",search(lcontrol,"pro_type"));
					fprintf(cgiOut,"<td width=170  align=left><select name=pro_type>");
					if(strcmp(pro_type,"udp") == 0)
					{
						fprintf(cgiOut,"<option value=tcp>tcp</option>");
						fprintf(cgiOut,"<option value=udp selected=selected>udp</option>");
						fprintf(cgiOut,"<option value=icmp>icmp</option>");
					}
					else if(strcmp(pro_type,"icmp") == 0)
					{
						fprintf(cgiOut,"<option value=tcp>tcp</option>");
						fprintf(cgiOut,"<option value=udp >udp</option>");
						fprintf(cgiOut,"<option value=icmp  selected=selected>icmp</option>");
					}
					else
					{
						fprintf(cgiOut,"<option value=tcp selected=selected>tcp</option>");
						fprintf(cgiOut,"<option value=udp >udp</option>");
						fprintf(cgiOut,"<option value=icmp>icmp</option>");
					}
					fprintf( cgiOut, "</select>\n");	
					fprintf(cgiOut,"</td>");
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=30>");
						 fprintf(cgiOut,"<td width=60>%s:</td>",search(lcontrol,"sip"));
						 fprintf(cgiOut,"<td width=170  align=left>");
							 fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:150;font-size:9pt\">");
							 fprintf(cgiOut,"<input type=text  name=sip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",sip1,search(lpublic,"ip_error")); 
							 fprintf(cgiOut,"<input type=text  name=sip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",sip2,search(lpublic,"ip_error"));
							 fprintf(cgiOut,"<input type=text  name=sip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",sip3,search(lpublic,"ip_error"));
							 fprintf(cgiOut,"<input type=text  name=sip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",sip4,search(lpublic,"ip_error"));
							 fprintf(cgiOut,"</div>"\
							 			"</td>");
							 fprintf(cgiOut,"<td width=50>%s:</td>",search(lcontrol,"port_no"));
							fprintf(cgiOut,"<td width=170 align=left><input name=s_port size=15 maxLength=5 value=%s><font color=red>(1--65535)</font></td>",s_port);
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=30>");
						 fprintf(cgiOut,"<td width=60 >%s:</td>",search(lcontrol,"dip"));
						 fprintf(cgiOut,"<td width=170  align=left>");
							 fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:150;font-size:9pt\">");
							 fprintf(cgiOut,"<input type=text  name=dip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",dip1,search(lpublic,"ip_error")); 
							 fprintf(cgiOut,"<input type=text  name=dip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",dip2,search(lpublic,"ip_error"));
							 fprintf(cgiOut,"<input type=text  name=dip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",dip3,search(lpublic,"ip_error"));
							 fprintf(cgiOut,"<input type=text  name=dip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",dip4,search(lpublic,"ip_error"));
							 fprintf(cgiOut,"</div>"\
							 			"</td>");
							 fprintf(cgiOut,"<td width=50>%s:</td>",search(lcontrol,"port_no"));
							fprintf(cgiOut,"<td width=170 align=left><input name=d_port size=15 maxLength=5  value=%s><font color=red>(1--65535)</font></td>",d_port);
					fprintf(cgiOut,"</tr>");	
				}
				if((strcmp(choice,"2")==0)||(strcmp(choice,"5")==0))
				{		  		
					memset(cpu_type,0,10);
				    cgiFormStringNoNewlines("cpu_type",cpu_type,10);
					fprintf(cgiOut,"<tr height=30>");
					fprintf(cgiOut,"<td width=60>%s:</td>",search(lcontrol,"cpu_type"));
					fprintf(cgiOut,"<td width=170  align=left><select name=cpu_type>");
					if(strcmp(cpu_type,"slave")==0)
					{
						fprintf(cgiOut,"<option value=master>master</option>");
						fprintf(cgiOut,"<option value=slave selected=selected>slave</option>");
					}
					else
					{
						fprintf(cgiOut,"<option value=master selected=selected>master</option>");
						fprintf(cgiOut,"<option value=slave>slave</option>");
					}
					fprintf( cgiOut, "</select>\n");	
					fprintf(cgiOut,"</td>");
					fprintf(cgiOut,"</tr>");	
				}
				if((strcmp(choice,"4")==0))
				{			  
 				  memset(uip1,0,4);
 				  memset(uip2,0,4);
 				  memset(uip3,0,4);
 				  memset(uip4,0,4);
 				  memset(usr_ip,0,32);
 				  cgiFormStringNoNewlines("uip1",uip1,4);
 				  cgiFormStringNoNewlines("uip2",uip2,4);
 				  cgiFormStringNoNewlines("uip3",uip3,4);
 				  cgiFormStringNoNewlines("uip4",uip4,4);
					fprintf(cgiOut,"<tr height=30>");
					fprintf(cgiOut,"<td width=60>IP %s:</td>",search(lpublic,"addr"));
					fprintf(cgiOut,"<td width=170  align=left>");
						fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:150;font-size:9pt\">");
						fprintf(cgiOut,"<input type=text  name=uip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",uip1,search(lpublic,"ip_error")); 
						fprintf(cgiOut,"<input type=text  name=uip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",uip2,search(lpublic,"ip_error"));
						fprintf(cgiOut,"<input type=text  name=uip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",uip3,search(lpublic,"ip_error"));
						fprintf(cgiOut,"<input type=text  name=uip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",uip4,search(lpublic,"ip_error"));
						fprintf(cgiOut,"</div>"\
								   "</td>");
					fprintf(cgiOut,"</tr>");	
				}
				/*************************显示结果******************************/
				fprintf(cgiOut, "<tr><td><hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
				"<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
				"<hr width=100%% size=1 color=#fff align=center noshade /></td><td>"\
				"<hr width=100%% size=1 color=#fff align=center noshade /></td>"\
				"</tr>" );
				if(strcmp(choice,"0")==0)
				{
					if(op_ret==0)
					{
						fprintf(cgiOut,"<tr height=30>");
							fprintf(cgiOut,"<td colspan=4>The number of total aging rule :%llu</td>",dynamic_a+static_a);
							fprintf(cgiOut,"</tr>");	
							fprintf(cgiOut,"<tr  height=30>");	
							fprintf(cgiOut,"<td colspan=4>The number of dynamic aging rule :%llu</td>",dynamic_a);
							fprintf(cgiOut,"</tr>");	
							fprintf(cgiOut,"<tr  height=30>");	
							fprintf(cgiOut,"<td colspan=4>The number of static aging rule  :%llu</td>",static_a);
						fprintf(cgiOut,"</tr>");	
					}
				}
				if(strcmp(choice,"1")==0)
				{
					if(op_ret==0)
					{
						fprintf(cgiOut,"<tr height=30>");
							fprintf(cgiOut,"<td colspan=4>%u.%u.%u.%u:%u  ==> %u.%u.%u.%u:%u %s</td>",\
												IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.sip),
												cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.sport,
												IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.dip),
												cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.dport,
												PROTO_STR(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.protocol));
							fprintf(cgiOut,"</tr>");
							if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.rule_state == RULE_IS_LEARNING)
							{
								if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.time_stamp==RULE_IS_AGE)
								{
									fprintf(cgiOut,"<tr>"); 
									fprintf(cgiOut,"<td colspan=4>rule_state = LEARNING,and the rule is age</td>");
									fprintf(cgiOut,"</tr>");	
								}
								else if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.time_stamp==RULE_IS_NEW)
								{
									fprintf(cgiOut,"<tr>"); 
									fprintf(cgiOut,"<td colspan=4>rule_state = LEARNING,and the rule is new</td>");
									fprintf(cgiOut,"</tr>");	
								}
								else 
								{
									fprintf(cgiOut,"<tr>"); 
									fprintf(cgiOut,"<td colspan=4>rule_state = LEARNING</td>");
									fprintf(cgiOut,"</tr>");	
								}

							}\
							else if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.action_type == FLOW_ACTION_DROP)
							{
								fprintf(cgiOut,"<tr>"); 
								fprintf(cgiOut,"<td colspan=4>action_type = FLOW_ACTION_DROP</td>");
								fprintf(cgiOut,"</tr>");	
							}
							else if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.action_type == FLOW_ACTION_TOLINUX)
							{
								fprintf(cgiOut,"<tr>"); 
								fprintf(cgiOut,"<td colspan=4>action_type = FLOW_ACTION_TOLINUX</td>");
								fprintf(cgiOut,"</tr>");	
							}
							else
							{
								fprintf(cgiOut,"<tr>"); 
								fprintf(cgiOut,"<td colspan=4>smac: %02x-%02x-%02x-%02x-%02x-%02x</td>", MAC_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.ether_shost));
								fprintf(cgiOut,"</tr>");	
								fprintf(cgiOut,"<tr>"); 
								fprintf(cgiOut,"<td colspan=4>dmac: %02x-%02x-%02x-%02x-%02x-%02x</td>", MAC_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.ether_dhost));
								fprintf(cgiOut,"</tr>");	
								fprintf(cgiOut,"<tr>"); 
								fprintf(cgiOut,"<td colspan=4>eth protocol: %04x</td>", cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.ether_type);
								fprintf(cgiOut,"</tr>");	
								switch(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.action_type)
								{
									case FLOW_ACTION_ETH_FORWARD:
										fprintf(cgiOut,"<tr>"); 
										fprintf(cgiOut,"<td colspan=4>	 action_type = FLOW_ACTION_ETH_FORWARD</td>");
										fprintf(cgiOut,"</tr>");	
										break;
									case FLOW_ACTION_CAP802_3_FORWARD:
										fprintf(cgiOut,"<tr>"); 
										fprintf(cgiOut,"<td colspan=4>	 action_type = FLOW_ACTION_CAP802_3_FORWARD</td>");
										fprintf(cgiOut,"</tr>");	
										fprintf(cgiOut,"<tr>"); 
										fprintf(cgiOut,"<td colspan=4>	 capwap use_num = %d</td>", cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.use_num);
										fprintf(cgiOut,"</tr>");	
										fprintf(cgiOut,"<tr>"); 
										fprintf(cgiOut,"<td colspan=4>	 capwap tunnel: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d  tos = 0x%02x</td>",\
																			IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.sip), cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.sport,
																			IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.dip), cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.dport,  
																			cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.tos);
										fprintf(cgiOut,"</tr>");	
										break;
									case FLOW_ACTION_CAPWAP_FORWARD:
										fprintf(cgiOut,"<tr>"); 
										fprintf(cgiOut,"<td colspan=4>	 action_type = FLOW_ACTION_CAPWAP_FORWARD</td>");
										fprintf(cgiOut,"</tr>");	
										fprintf(cgiOut,"<tr>"); 
										fprintf(cgiOut,"<td colspan=4>	 capwap use_num = %d</td>", cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.use_num);
										fprintf(cgiOut,"</tr>");	
										fprintf(cgiOut,"<tr>"); 
										fprintf(cgiOut,"<td colspan=4>	 capwap tunnel: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d  tos = 0x%02x</td>",\
																								IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.sip), cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.sport,
																								IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.dip), cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.dport,  
																								cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.tos);
										fprintf(cgiOut,"</tr>");	
										break;
									default:
										fprintf(cgiOut,"<tr>"); 
										fprintf(cgiOut,"<td colspan=4>	 action_type = UNKNOWN</td>");
										fprintf(cgiOut,"</tr>");	
										break;
								}
								
								if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.time_stamp == RULE_IS_AGE)
								{
									fprintf(cgiOut,"<tr>"); 
									fprintf(cgiOut,"<td colspan=4>	 forward port = %d , and the rule is age</td>", cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.forward_port);
									fprintf(cgiOut,"</tr>");	
								}
								else
								{
									fprintf(cgiOut,"<tr>"); 
									fprintf(cgiOut,"<td colspan=4>	 forward port = %d , and the rule is new</td>", cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.forward_port);
									fprintf(cgiOut,"</tr>");	
								}
								if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.rule_state == RULE_IS_STATIC)
								{
									fprintf(cgiOut,"<tr>"); 
									fprintf(cgiOut,"<td colspan=4>  rule_state = STATIC</td>", cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.forward_port);
									fprintf(cgiOut,"</tr>");	
								}
								else
								{
									if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.rule_state == RULE_IS_LEARNED)
									{
										fprintf(cgiOut,"<tr>"); 
										fprintf(cgiOut,"<td colspan=4>	 rule_state = LEARNED</td>", cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.forward_port);
										fprintf(cgiOut,"</tr>");	
									}
								}
								fprintf(cgiOut,"<tr>"); 
								fprintf(cgiOut,"<td colspan=4>	 dsa_info: 0x%08x</td>", cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.dsa_info);
								fprintf(cgiOut,"</tr>");
								
								fprintf(cgiOut,"<tr>"); 
								fprintf(cgiOut,"<td colspan=4>	 out_type:0x%02x   out_tag:0x%02x	in_type:0x%02x	 in_tag:0x%02x</td>",\
																						cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.out_ether_type, 
																						cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.out_tag, 
																						cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.in_ether_type, 
																						cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.in_tag);
								fprintf(cgiOut,"</tr>");	
								fprintf(cgiOut,"<tr>"); 
								fprintf(cgiOut,"<td colspan=4>	 action mask = 0x%x</td>", cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.action_mask);
								fprintf(cgiOut,"</tr>");
						}
					}
				}
				if(strcmp(choice,"2")==0)
				{	
					if(op_ret==0)
					{
						d_use_rate=(float)(statistics.d_tbl_used_rule-statistics.d_tbl_aged_rule)/(float)(statistics.acl_dynamic_tbl_size)*100;
						b_use_rate=(float)(statistics.s_tbl_used_rule-statistics.s_tbl_aged_rule)/(float)(statistics.acl_static_tbl_size)*100;
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>====================acl_bucket_tbl count=====================</td>");
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>entry num: %u</td>",statistics.acl_static_tbl_size);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>free num: %u</td>",statistics.acl_static_tbl_size-statistics.s_tbl_used_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>used num: %u</td>",statistics.s_tbl_used_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>aged rules num: %u</td>",statistics.s_tbl_aged_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>new rules num: %u</td>",statistics.s_tbl_used_rule-statistics.s_tbl_aged_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>use rate: %02f%%</td>",b_use_rate);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>learned rules num: %u</td>",statistics.s_tbl_learned_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>learning rules num: %u</td>",statistics.s_tbl_learning_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>static insert rules num: %u</td>",statistics.s_tbl_static_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>===================acl_dynamic_tbl count======================</td>");
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>entry num: %u</td>",statistics.acl_dynamic_tbl_size);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>free num: %u</td>",statistics.acl_dynamic_tbl_size-statistics.d_tbl_used_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>used num: %u</td>",statistics.d_tbl_used_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>aged rules num: %u</td>",statistics.d_tbl_aged_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>new rules num: %u</td>",statistics.d_tbl_used_rule-statistics.d_tbl_aged_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>use rate: %02f%%</td>",d_use_rate);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>learned rules num: %u</td>",statistics.d_tbl_learned_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>learning rules num: %u</td>",statistics.d_tbl_learning_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>static insert rules num: %u</td>",statistics.d_tbl_static_rule);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>====================capwap cache table=======================</td>");
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>capwap table entry num: %u</td>",statistics.capwap_cache_tbl_size);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>capwap table used entry num: %u</td>",statistics.cw_tbl_used);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>capwap table 802.3 entry num: %u</td>",statistics.cw_tbl_802_3_num);
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td colspan=4>capwap table 802.11 entry num: %u</td>",statistics.cw_tbl_802_11_num);
						fprintf(cgiOut,"</tr>");
					}
				}
				if((strcmp(choice,"3")==0))
				{
					op_ret=ccgi_show_tolinux_flow_cmd(pid,&data);
					if(op_ret==0)
					{
						fprintf(cgiOut,"<tr height=30>");
							fprintf(cgiOut,"<td colspan=4>To linux flow  %llu bps</td>",data.to_linux_bps);
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<td colspan=4>To linux flow  %llu pps</td>",data.to_linux_pps);
						fprintf(cgiOut,"</tr>");
					}
				}
				if(strcmp(choice,"4")==0)
				{
					if(op_ret==0)
					{
						fprintf(cgiOut,"<tr height=30>");
							fprintf(cgiOut,"<td colspan=4>total used num : %lu, (static: %lu, dynamic: %lu)</td>",statistics.s_tbl_used_rule+statistics.d_tbl_used_rule, \
																												statistics.s_tbl_used_rule,\
																												statistics.d_tbl_used_rule);
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=30>");
							fprintf(cgiOut,"<td colspan=4>uplink rules num : %lu, (static: %lu, dynamic: %lu)</td>",statistics.s_tbl_uplink_rule+statistics.d_tbl_uplink_rule, \
																												statistics.s_tbl_uplink_rule,\
																												statistics.d_tbl_uplink_rule);
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=30>");
							fprintf(cgiOut,"<td colspan=4>downlink rules num : %lu, (static: %lu, dynamic: %lu)</td>",statistics.s_tbl_downlink_rule+statistics.d_tbl_downlink_rule, \
																												statistics.s_tbl_downlink_rule,\
																												statistics.d_tbl_downlink_rule);
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=30>");
							
							fprintf(cgiOut,"<td colspan=4>new rules num : %lu, (static: %lu, dynamic: %lu)</td>",statistics.s_tbl_used_rule-statistics.s_tbl_aged_rule+statistics.d_tbl_used_rule-statistics.d_tbl_aged_rule, \
																												statistics.s_tbl_used_rule-statistics.s_tbl_aged_rule,\
																												statistics.d_tbl_used_rule-statistics.d_tbl_aged_rule);
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=30>");
							fprintf(cgiOut,"<td colspan=4>aged rules num  : %lu, (static: %lu, dynamic: %lu)</td>",statistics.s_tbl_aged_rule+statistics.d_tbl_aged_rule, \
																												statistics.s_tbl_aged_rule,\
																												statistics.d_tbl_aged_rule);
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=30>");
							fprintf(cgiOut,"<td colspan=4>learned rules num  : %lu, (static: %lu, dynamic: %lu)</td>",statistics.s_tbl_learned_rule+statistics.d_tbl_learned_rule, \
																												statistics.s_tbl_learned_rule,\
																												statistics.d_tbl_learned_rule);
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=30>");
							fprintf(cgiOut,"<td colspan=4>learning rules num : %lu, (static: %lu, dynamic: %lu)</td>",statistics.s_tbl_learning_rule+statistics.d_tbl_learning_rule, \
																												statistics.s_tbl_learning_rule,\
																												statistics.d_tbl_learning_rule);
						fprintf(cgiOut,"</tr>");
					}
				}
				if((strcmp(choice,"5")==0))
				{
					if(op_ret==0)
					{
						for(i=0;i<CVMX_FPA_NUM_POOLS;i++)
						{
							fprintf(cgiOut,"<tr height=30>");
								fprintf(cgiOut,"<td colspan=4>pool %d available buff is %d</td>",i,p_buff[i]);
							fprintf(cgiOut,"</tr>");	
						}
					}
				}				
			}
			else
			{
				fprintf(cgiOut,"<tr height=60>");
				  fprintf(cgiOut,"<td colspan=4 align=center><font size=4>%s</font></td>",search(lcontrol,"no_load_fast"));
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

