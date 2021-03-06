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
* wp_dhcpvs_edit.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system cotrl for dhcp  
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

#include "ws_dhcp_conf.h"

int ShowDhcpEditPage(char *encry,char *eid,struct list *lcontrol,struct list *lpublic);
void ConfIPAdd(struct list *lcontrol,struct list *lpublic);
int ShowConfClearPage(char *m,char *id,struct list *lcontrol,struct list *lpublic);

int cgiMain()
{
	char *encry=(char *)malloc(BUF_LEN);  
	char *str;   

	struct list *lpublic;   /*解析public.txt文件的链表头*/
    struct list *lcontrol;     /*解析control.txt文件的链表头*/  
    lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt");


	///////////////
	char eid[30];
	memset(eid,0,30);

	char etype[10];
	memset(etype,0,10);


	cgiFormStringNoNewlines("NAME",eid,30);
	cgiFormStringNoNewlines("TYPE",etype,10);


	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
	{
		str=dcryption(encry);
		if(str==NULL)
		ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
		else 
		{
			if(strcmp(etype,"2")==0)
			   ShowConfClearPage(encry,eid,lcontrol,lpublic);
			else
			   ShowDhcpEditPage(encry,eid,lcontrol,lpublic);
		}

	}
	else                    
	{      
		cgiFormStringNoNewlines("encry_dhcp",encry,BUF_LEN);
		str=dcryption(encry);	
		if(str==NULL)
		ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
		else 
		{
			if(strcmp(etype,"2")==0)
			   ShowConfClearPage(encry,eid,lcontrol,lpublic);
			else
			   ShowDhcpEditPage(encry,eid,lcontrol,lpublic);
		}

	} 


	free(encry);
	////////////////

	release(lpublic);  
    release(lcontrol);

	return 0;
}


int ShowDhcpEditPage(char *encry,char *eid,struct list *lcontrol,struct list *lpublic)
{ 

	char *spool=(char *)malloc(1024);
	char *slease=(char *)malloc(50);
	char *ssubset=(char *)malloc(50);
	char *snserver=(char *)malloc(50);
	char *snetmask=(char *)malloc(50);
	char sauth[10];
	char sprefix[10];

	memset(spool,0,1024);
	memset(slease,0,50);
	memset(ssubset,0,50);
	memset(snserver,0,50);
	memset(snetmask,0,50);
	memset(sauth,0,10);  
	memset(sprefix,0,10);
	//////////

	struct dvsconfz confs;
	memset(&confs,0,sizeof(struct dvsconfz));
	confs.spool=(char *)malloc(1024);
	int conflag;

	struct substringz s_head,*sq;
	int subnum=0,retflag=-1;

    char subn_ip1[5];
    char subn_ip2[5];
    char subn_ip3[5];
    char subn_ip4[5];
    char subn_ip5[5];
    char subn_ip6[5];
    char subn_ip7[5];
    char subn_ip8[5];

	memset(subn_ip1,0,5);
	memset(subn_ip2,0,5);
	memset(subn_ip3,0,5);
	memset(subn_ip4,0,5);
	memset(subn_ip5,0,5);
	memset(subn_ip6,0,5);
	memset(subn_ip7,0,5);
	memset(subn_ip8,0,5);

	char ns_ip1[5];
	char ns_ip2[5];
	char ns_ip3[5];
	char ns_ip4[5];
	char ns_ip5[5];
	char ns_ip6[5];
	char ns_ip7[5];
	char ns_ip8[5];

	memset(ns_ip1,0,5);
	memset(ns_ip2,0,5);
	memset(ns_ip3,0,5);
	memset(ns_ip4,0,5);
	memset(ns_ip5,0,5);
	memset(ns_ip6,0,5);
	memset(ns_ip7,0,5);
	memset(ns_ip8,0,5);

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
 
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
    "<script type=\"text/javascript\">");
  			fprintf(cgiOut,"var arr_ipr_sval = new Array();"\
						   "var arr_ipr_eval = new Array();"\
                           "var str_port_subnet = \"\";"\
  						   "var arr_temp = new Array();"\
                           "arr_temp[\"sip\"] = null;"\
                           "arr_temp[\"eip\"] = null;"\
						   "var len=0;"\
						   "var arr_ipr_list = new Array();");
  	 fprintf(cgiOut, "function add_ip_ranges()"\
                            "{"\
                               "var value = \"\";"\
                               "var str_start_ip = get_ip_addr(\"beg_ip\");"\
   							   "var str_end_ip = get_ip_addr(\"end_ip\");"\
                               "if(null == str_start_ip || null == str_end_ip)"\
                               "{"\
                                  "alert(\"%s\");"\
                                  "clear_ip_addr(\"beg_ip\");"\
							      "clear_ip_addr(\"end_ip\");"\
                                  "return ;"\
                               "}"\
   							   "var hex_start_ip = get_iphex_addr(\"beg_ip\");"\
			                   "var hex_end_ip = get_iphex_addr(\"end_ip\");"\
   							   "if(hex_start_ip!=hex_end_ip)"\
   							   "{"\
	   							   "alert(\"%s\");"\
	   							   "clear_ip_addr(\"beg_ip\");"\
							       "clear_ip_addr(\"end_ip\");"\
	   							   "return ;"\
   							   "}"\
   							   "else"\
   							   "{"\
	   							   "len = document.all.select.options.length;"\
	   							   "if(len==0)"\
	   							   "{"\
	   							            "value = str_start_ip + \" \" + str_end_ip;"\
								            "document.all.select.options.add("\
									        "new Option(value,value,false,false)"\
									        ");"\
	   							   "}"\
	   							   "else if(len != 0)"\
	   							   "{"\
		   							   "var first_ip = document.all.select.options[0].value;"\
		   							   "var arr_ip_seg = first_ip.split(\" \");"\
		   							   "var first_arr = arr_ip_seg[0];"\
		   							   "var match_net = get_matchnet(first_arr);"\
		   							   "if(hex_start_ip == match_net )"\
		   							   "{"\
			   							    "value = str_start_ip + \" \" + str_end_ip;"\
								            "document.all.select.options.add("\
									        "new Option(value,value,false,false)"\
									        ");"\
		   							   "}"\
		   							   "else"\
		   							   "{"\
			   							   "alert(\"%s\");"\
			   							   "clear_ip_addr(\"beg_ip\");"\							    
							               "clear_ip_addr(\"end_ip\");"\
			   							   "return ;"\
		   							   "}"\
	   							   "}"\
   							   "}"\
							   "clear_ip_addr(\"beg_ip\");"\
							   "clear_ip_addr(\"end_ip\");"\
                               "}",search(lcontrol, "ip_null"),search(lpublic,"dhcpvs_nmatch"),search(lpublic,"dhcpvs_nmatch"));
	 fprintf(cgiOut, "function get_ip_addr(str_name)"\
								"{"\
								   "var str_para = \"\";"\
								   "var obj_temp;"\
								   "var str_temp = \"\";"\
								   "var iter = 1;"\
								   "for(; iter<=8; iter++)"\
								   "{"\
									  "str_temp = \"document.getElementById(\\\"\" +  str_name + iter + \"\\\")\";"\
									  "obj_temp = eval(str_temp);"\
									  "if(\"\" == obj_temp.value)"\
									  "{"\
										 "return null;"\
									  "}"\
									  "str_para += obj_temp.value + \":\";"\
								   "}"\
								   "str_temp = str_para.replace(/\\:$/,\"\");"\
								   "return str_temp;"\
								"}");
	 fprintf(cgiOut, "function get_iphex_addr(str_name)"\
		"{"\
			"var str_para = \"\";"\
			"var obj_temp;"\
			"var str_temp = \"\";"\
			"var iter = 1;"\
			"for(; iter<=7; iter++)"\
			"{"\
				"str_temp = \"document.getElementById(\\\"\" +  str_name + iter + \"\\\")\";"\
				"obj_temp = eval(str_temp);"\
				"if(\"\" == obj_temp.value)"\
				"{"\
					"return null;"\
				"}"\
				"str_para += parseInt(obj_temp.value,16).toString(10) + \":\";"\
			"}"\
			"str_temp = str_para.replace(/\\:$/,\"\");"\
			"return str_temp;"\
		"}");	 
	  fprintf(cgiOut, "function clear_ip_addr(str_name)"\
                            "{"\
                               "var obj_temp;"\
                               "var str_temp = \"\";"\
                               "var iter = 1;"\
                               "for(; iter<=8; iter++)"\
                               "{"\
                                  "str_temp = \"document.getElementById(\\\"\" +  str_name + iter + \"\\\")\";"\
                                  "obj_temp = eval(str_temp);"\
                                  "obj_temp.value = \"\";"\
                               "}"\
                            "}");
	  fprintf(cgiOut, "function ip2val(str_ip_addr)"\
                            "{"\
                               "var arr_ip_seg = str_ip_addr.split(\":\");"\
                               "var un_val = 0;"\
                               "for(var i=0; i<8; i++)"\
                               "{"\
                                  "un_val = (un_val << 8) + parseInt(arr_ip_seg[i]);"\
                               "}"\
                               "return un_val;"\
                            "}");
	  fprintf(cgiOut, "function get_matchnet(str_ip_addr)"\
                            "{"\
                               "var arr_ip_seg = str_ip_addr.split(\":\");"\
                               "var un_val = \"\";"\
                               "var str_temp = \"\";"\
                               "for(var i=0; i<7; i++)"\
                               "{"\
                                  "un_val +=parseInt(arr_ip_seg[i],16).toString(10) + \":\";"\
                               "}"\
                               "str_temp = un_val.replace(/\\:$/,\"\");"\
                               "return str_temp;"\
                            "}");
	  fprintf(cgiOut, "function get_subnet(str_ip_addr)"\
                            "{"\
                               "var arr_ip_seg = str_ip_addr.split(\":\");"\
                               "var un_val = \"\";"\
                               "var str_temp = \"\";"\
                               "for(var i=0; i<4; i++)"\
                               "{"\
                                  "un_val += arr_ip_seg[i] + \":\";"\
                               "}"\
                               "str_temp = un_val.replace(/\\:$/,\"\");"\
                               "str_temp = str_temp + \":0:0:0:0\";"\
                               ""\
                               "return str_temp;"\
                            "}");	  
	   fprintf(cgiOut, "function clear_ip_addr(str_name)"\
                            "{"\
                               "var obj_temp;"\
                               "var str_temp = \"\";"\
                               "var iter = 1;"\
                               "for(; iter<=8; iter++)"\
                               "{"\
                                  "str_temp = \"document.getElementById(\\\"\" +  str_name + iter + \"\\\")\";"\
                                  "obj_temp = eval(str_temp);"\
                                  "obj_temp.value = \"\";"\
                               "}"\
                            "}");
	 fprintf(cgiOut,"function rm_ip_ranges(){"\
		  "for(var i=0; i<document.all.select.options.length; i++){"\
		  "if(document.all.select.options[i].selected==true){"\
		  "var optionIndex ;"\
		  "for(var j=i;j<len;j++)"\
		  	"{"\
		  		"arr_ipr_sval[j]=arr_ipr_sval[j+1];"\
		  		"arr_ipr_eval[j]=arr_ipr_eval[j+1];"\
		  	"}"\
		  "len=len-1;"\
		  "optionIndex = i;"\
		  "}"\
		  "if(optionIndex!=null){"\
		  "document.all.select.remove(optionIndex);"\
		  "}"\
		  "}"\
          "}");	
		fprintf(cgiOut, "function mysubmit()"\
					  "{"\
					    "var pool_len = document.all.select.options.length;"\
					    "if(pool_len ==0)"\
					    "{"\
					    "alert(\"%s\");"\
					    "return false;"\
					    "}"\
					 	"var tmp = \"\";"\
						"for(var i=0; i<document.all.select.options.length; i++)"\
							"{"\
					   			"arr_ipr_list[i]=document.all.select.options[i].value;"\
								"tmp = arr_ipr_list[i];"\
	   							"tmp = tmp.replace(/-/,\" \");"\
	   							"if(i==0)"\
					   				"{"\
					   					"document.all.spool.value = tmp;"\
					   				"}"\
					   				"else"\
					   				"{"\
					   					"document.all.spool.value = document.all.spool.value + \"^\" + tmp;"\
					   				"}"\
					   "}"\
					   "var v1=document.all.day.value;"\
		   				"if(v1>999)"\
		   	    			 "{"\
		   	     				"alert(\"%s\");"\
		   	      				"return false;"\
		   		 		"}"\
		   		 		"var v2=document.all.hour.value;"\
		   				"if(v2>23)"\
		   	    			 "{"\
		   	     				"alert(\"%s\");"\
		   	      				"return false;"\
		   		 		"}"\
		   		 		"var v3=document.all.min.value;"\
		   				"if(v3>59)"\
		   	    			 "{"\
		   	     				"alert(\"%s\");"\
		   	      				"return false;"\
		   		 		"}"\
		   		 		"var v4=v1 + v2 + v3;"\
		   				"if(v4 == \"\")"\
		   	    			 "{"\
		   	     				"alert(\"%s\");"\
		   	      				"return false;"\
		   		 		"}"\
		   		 		"var ip1 = document.all.dns_ip1.value;"\
		   		 		"var ip2 = document.all.dns_ip2.value;"\
		   		 		"var ip3 = document.all.dns_ip3.value;"\
		   		 		"var ip4 = document.all.dns_ip4.value;"\
		   		 		"var ip5 = document.all.dns_ip5.value;"\
		   		 		"var ip6 = document.all.dns_ip6.value;"\
		   		 		"var ip7 = document.all.dns_ip7.value;"\
		   		 		"var ip8 = document.all.dns_ip8.value;"\
						"var value1 = \"\";"\
	   					"if(ip1==\"\"&&ip2==\"\"&&ip3==\"\"&&ip4==\"\"&&ip5==\"\"&&ip6==\"\"&&ip7==\"\"&&ip8==\"\")"\
	   					"{"\
	   					"}"\
		   		 		"else if(ip1!=\"\"&&ip2!=\"\"&&ip3!=\"\"&&ip4!=\"\"&&ip5!=\"\"&&ip6!=\"\"&&ip7!=\"\"&&ip8!=\"\")"\
	   					"{"\
	   							"value1 = ip1 + \":\" + ip2 + \":\" + ip3 + \":\" + ip4 + \":\" +ip5 + \":\" + ip6 + \":\" + ip7 + \":\" + ip8;"\
	   					"}"\
		   		 		"else"\
		   		 		"{"\
		   		 			"alert(\"%s\");"\
		   		 			"return false;"\
		   		 		"}"\
		   		 		"document.all.snserver.value = value1;"\
		   		 		"var z_subnet = get_subnet(document.all.select.options[0].value);"\
					    "document.all.ssubset.value = z_subnet;"\
					    "var num_nm=document.all.nm_num.value;"\
					    "document.all.snetmask.value=num_nm;"\
					 "}",search(lcontrol,"dhcp_ranges_empty"),search(lcontrol,"lease_d_error"),search(lcontrol,"lease_h_error"),search(lcontrol,"lease_m_error"),search(lcontrol,"lease_empty"),search(lpublic,"err_ip_format"));
  fprintf(cgiOut,"</script>"\
  "<script language=javascript src=/ipv6.js>"\
  "</script>"\
  "</head>"\
  "<body>");

	if(cgiFormSubmitClicked("dhcp_add") == cgiFormSuccess)
	{	  
	ConfIPAdd(lcontrol,lpublic);
	}

	conflag=get_dvsconf_struct(DHCPVS_XML, DHCPVS_SC_CONF, DHCP_NODE_ATTR, eid, &confs);
    int i;
	int lease_t;
	char day[5];
	char hour[5];
	char min[5];
	int ttmp;
	////lease

	lease_t = atoi(confs.slease);
	ttmp = lease_t/86400;
	sprintf(day,"%d",ttmp);
	ttmp = lease_t%86400;
	ttmp = ttmp/3600;
	sprintf(hour,"%d",ttmp);
	ttmp = lease_t%3600;
	ttmp = ttmp/60;
	sprintf(min,"%d",ttmp);

	char *tmp;
	
	i = 0;
	if(strcmp(confs.ssubnet,"")!=0)
	{
	  i=0;
	  tmp = strtok(confs.ssubnet,":");
	  while(tmp != NULL)
	  	{
	  		i++;
			if(i==1)
				strcpy(subn_ip1,tmp);
			else if(i ==2 )
				strcpy(subn_ip2,tmp);
			else if(i==3)
				strcpy(subn_ip3,tmp);
			else if(i==4)
				strcpy(subn_ip4,tmp);
			else if(i==5)
				strcpy(subn_ip5,tmp);
			else if(i==6)
				strcpy(subn_ip6,tmp);
			else if(i==7)
				strcpy(subn_ip7,tmp);
			else if(i==8)
				strcpy(subn_ip8,tmp);
			tmp = strtok(NULL,":");	
	  	}
	 
	}
	i = 0;
	if(strcmp(confs.snserver,"")!=0)
	{
	  i=0;
	  tmp = strtok(confs.snserver,":");
	  while(tmp != NULL)
	  	{
	  		i++;
			if(i==1)
				strcpy(ns_ip1,tmp);
			else if(i ==2 )
				strcpy(ns_ip2,tmp);
			else if(i==3)
				strcpy(ns_ip3,tmp);
			else if(i==4)
				strcpy(ns_ip4,tmp);
			else if(i==5)
				strcpy(ns_ip5,tmp);
			else if(i==6)
				strcpy(ns_ip6,tmp);
			else if(i==7)
				strcpy(ns_ip7,tmp);
			else if(i==8)
				strcpy(ns_ip8,tmp);
			tmp = strtok(NULL,":");	
	  	}
	 
	}

	fprintf(cgiOut,"<form method=post  onsubmit=\"return mysubmit()\">"\
	"<div align=center>"\
	"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
	"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCPV6");
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>");
	fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_add style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	fprintf(cgiOut,"<td width=62 align=center><a href=wp_dhcpvs_conf.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
	if(cgiFormSubmitClicked("dhcp_add") != cgiFormSuccess)
	{ 		  
	fprintf(cgiOut,"<tr height=26>"\
	"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_er>DHCPV6 </font><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"edit"));   /*突出显示*/
	fprintf(cgiOut,"</tr>");	
	}
	else if(cgiFormSubmitClicked("dhcp_add") == cgiFormSuccess) 			  
	{					   
	fprintf(cgiOut,"<tr height=26>"\
	"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_er>DHCPV6 </font><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"edit"));   /*突出显示*/
	fprintf(cgiOut,"</tr>");
	}
	for(i=0;i<17;i++)
	{
	fprintf(cgiOut,"<tr height=25>"\
	"<td id=tdleft>&nbsp;</td>"\
	"</tr>");
	}
	fprintf(cgiOut,"</table>"\
	"</td>"\
	"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");

	fprintf(cgiOut,"<table width=750 border=0 cellspacing=0 cellpadding=0>");

	///////////////ruler name,signle designed
	fprintf(cgiOut,"<tr>\n");
	fprintf(cgiOut,"<td width=100 height=40 >%s:</td>",search(lpublic,"dhcp_auth"));
	fprintf(cgiOut,"<td>\n");
	if(strcmp(confs.sauth,"yes")==0)
	fprintf(cgiOut,"<input type=checkbox name=sauth value=yes checked/>");
	else
	fprintf(cgiOut,"<input type=checkbox name=sauth value=yes />");
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"</tr>\n");		


	///////////////prefix
	fprintf(cgiOut,"<tr>\n");
	fprintf(cgiOut,"<td width=100 height=40 >%s:</td>",search(lpublic,"dhcpvs_prefix"));
	fprintf(cgiOut,"<td>\n");
	if(strcmp(confs.sprefix,"yes")==0)
	fprintf(cgiOut,"<input type=checkbox name=sprefix value=yes checked/>");
	else
	fprintf(cgiOut,"<input type=checkbox name=sprefix value=yes />");
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"</tr>\n");		


	fprintf(cgiOut,"<tr>"\
	"<td width=100 height=40 >%s:</td>",search(lcontrol,"lease duration"));
	fprintf(cgiOut,"<td colspan=4 width=350 align=left><input type=text name=day onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
	onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
	ondragenter=\"return  false;\"\
	style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" size=5 value=\"%s\"/><font color=red>(%s)</font>",day,search(lcontrol,"day"));
	fprintf(cgiOut,"<input type=text name=hour onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
	onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
	ondragenter=\"return  false;\"\
	style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" size=5 value=\"%s\"/><font color=red>(%s)</font>",hour,search(lcontrol,"hour"));
	fprintf(cgiOut,"<input type=text name=min onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
	onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
	ondragenter=\"return  false;\"\
	style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" size=5 value=\"%s\"/><font color=red>(%s)</font></td>",min,search(lcontrol,"time_min"));
	fprintf(cgiOut,"</tr>");			

	fprintf(cgiOut,"<tr>"\
	"<td width=100 height=40>%s:</td>",search(lpublic,"dhcpvs_server"));
	fprintf(cgiOut,"<td width=140>"\
	"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:290;font-size:9pt\">");
	fprintf(cgiOut,"<input type=text  name=dns_ip1 value=\"%s\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",ns_ip1,search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=dns_ip2 value=\"%s\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",ns_ip2,search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=dns_ip3 value=\"%s\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",ns_ip3,search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=dns_ip4 value=\"%s\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",ns_ip4,search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=dns_ip5 value=\"%s\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",ns_ip5,search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=dns_ip6 value=\"%s\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",ns_ip6,search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=dns_ip7 value=\"%s\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",ns_ip7,search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=dns_ip8 value=\"%s\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",ns_ip8,search(lpublic,"ip_error"));
	fprintf(cgiOut,"</div></td>"\
	"<td>&nbsp;</td>");
	fprintf(cgiOut,"</tr>");		

	fprintf(cgiOut,"<tr>"\
	"<td width=100 height=40>%s:</td>",search(lpublic,"dhcpvs_sbnet"));
	fprintf(cgiOut,"<td width=140>"\
	"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:290;font-size:9pt\">");
	fprintf(cgiOut,"<input type=text  name=nm_ip1  maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\" disabled>:",search(lpublic,"ip_error"),subn_ip1); 
	fprintf(cgiOut,"<input type=text  name=nm_ip2  maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\" disabled>:",search(lpublic,"ip_error"),subn_ip2);
	fprintf(cgiOut,"<input type=text  name=nm_ip3  maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\" disabled>:",search(lpublic,"ip_error"),subn_ip3);
	fprintf(cgiOut,"<input type=text  name=nm_ip4  maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\" disabled>:",search(lpublic,"ip_error"),subn_ip4);
	fprintf(cgiOut,"<input type=text  name=nm_ip5  maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\" disabled>:",search(lpublic,"ip_error"),subn_ip5);
	fprintf(cgiOut,"<input type=text  name=nm_ip6  maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\" disabled>:",search(lpublic,"ip_error"),subn_ip6);
	fprintf(cgiOut,"<input type=text  name=nm_ip7  maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\" disabled>:",search(lpublic,"ip_error"),subn_ip7);
	fprintf(cgiOut,"<input type=text  name=nm_ip8  maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\" disabled>",search(lpublic,"ip_error"),subn_ip8);
	fprintf(cgiOut,"</div></td>"\
	"<td>&nbsp;</td>");
	fprintf(cgiOut,"</tr>");		

	fprintf(cgiOut,"<tr>"\
	"<td width=100 height=40>%s:</td>",search(lpublic,"dhcpvs_domsea"));
	fprintf(cgiOut,"<td width=100><input type=text name=ssearch  size=21 value=\"%s\"/></td>",confs.ssearch);
	fprintf(cgiOut,"</tr>");


	fprintf(cgiOut,"<tr>"\
	"<td width=100 height=40>%s:</td>",search(lpublic,"netmask"));
	fprintf(cgiOut,"<td>\n");
	fprintf(cgiOut,"<input type=text name=nm_num maxlength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
	onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
	ondragenter=\"return  false;\"\
	style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" value=\"%s\" size=7 disabled/><font color=red></font>",confs.snetmask);
	fprintf(cgiOut,"</td>\n");			
	fprintf(cgiOut,"</tr>");	


	fprintf(cgiOut,"<tr>"\
	"<td width=100 height=40>%s:</td>",search(lcontrol,"dhcp_pool"));

	//ip address pool,
	fprintf(cgiOut,"<td width=140>"\
	"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:290;font-size:9pt\">");
	fprintf(cgiOut,"<input type=text  name=beg_ip1 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=beg_ip2 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=beg_ip3 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=beg_ip4 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=beg_ip5 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=beg_ip6 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=beg_ip7 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=beg_ip8 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	fprintf(cgiOut,"</div></td>"\
	"<td width=10 align=center >-</td>"\
	"<td width=150 >"\
	"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:290;font-size:9pt\">");
	fprintf(cgiOut,"<input type=text  name=end_ip1 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=end_ip2 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=end_ip3 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=end_ip4 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=end_ip5 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=end_ip6 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=end_ip7 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=end_ip8 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	fprintf(cgiOut,"</div></td>"\
	"<td width=50 ><input type=button name=ranges_add id=ranges_add onclick=add_ip_ranges() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_add"));
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
	"<td width=100 height=40>&nbsp;</td>"\
	"<td colspan=3 width=600><select name=select  size=\"6\" style=\"width:590px\">");
	
	retflag=string_link_list(&s_head, &subnum, confs.spool);  
			if(retflag==0)
			{
			    sq=s_head.next;
			   
			    while(sq != NULL)
			    {
				  
				  fprintf(cgiOut,"<option value=\"%s\">%s</option>",sq->substr,sq->substr);
				  sq=sq->next;
			    }
			}
	if((retflag==0 )&& (subnum > 0))
		Free_substringz_all(&s_head);
	
	fprintf(cgiOut,"</select></td>"\
	"<td width=50 valign=top><input type=button name=ranges_rm id=ranges_rm onclick=rm_ip_ranges() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_rm"));
	fprintf(cgiOut,"</tr>"\
	"<tr>");
	fprintf(cgiOut,"<td ><input type=hidden name=encry_dhcp value=%s></td>",encry);
	fprintf(cgiOut,"<td ><input type=hidden name=NAME value=%s></td>",eid);  

	//spool
	fprintf(cgiOut,"<td><input type=hidden name=spool value=%s></td>",spool);	
	//server name ip address
	fprintf(cgiOut,"<td><input type=hidden name=snserver value=%s></td>",snserver);
	//subnet ip address
	fprintf(cgiOut,"<td><input type=hidden name=ssubset value=%s></td>",ssubset);
	fprintf(cgiOut,"</tr>");
	//net mask current is 64
	fprintf(cgiOut,"<tr><td><input type=hidden name=snetmask value=%s></td>",snetmask);
	fprintf(cgiOut,"<td><input type=hidden name=sprefix value=%s></td>",sprefix);	
	///
	fprintf(cgiOut,"</tr>"\
	"</table>");		


	fprintf(cgiOut,"</td>"\
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

	free(confs.spool);
	free(spool);
	free(slease);
	free(ssubset);
	free(snserver);
	free(snetmask);


	return 0;
	}
						 
void  ConfIPAdd(struct list *lcontrol,struct list *lpublic)
{
	char * day = (char*)malloc(10);
	char * hour = (char*)malloc(10);
	char * min = (char*)malloc(10);

	char *spool=(char *)malloc(1024);
	char *slease=(char *)malloc(50);
	char *ssubset=(char *)malloc(50);
	char *snserver=(char *)malloc(50);
	char *ssearch=(char *)malloc(50);
	char *snetmask=(char *)malloc(50);
	char sauth[10];
	char sprefix[10];

	memset(spool,0,1024);
	memset(slease,0,50);
	memset(ssubset,0,50);
	memset(snserver,0,50);
	memset(ssearch,0,50);
	memset(snetmask,0,50);
	memset(sauth,0,10);  
	memset(sprefix,0,10);
	memset(day,0,10);
	memset(hour,0,10);
	memset(min,0,10);
	
	cgiFormStringNoNewlines("day",day,10);
	cgiFormStringNoNewlines("hour",hour,10);
	cgiFormStringNoNewlines("min",min,10);
	cgiFormStringNoNewlines("spool",spool,1024);
	cgiFormStringNoNewlines("slease",slease,50);
	cgiFormStringNoNewlines("ssubset",ssubset,50);
	cgiFormStringNoNewlines("snserver",snserver,50);	
	cgiFormStringNoNewlines("ssearch",ssearch,50);
	cgiFormStringNoNewlines("snetmask",snetmask,50);	
	cgiFormStringNoNewlines("sauth",sauth,10);
	cgiFormStringNoNewlines("sprefix",sprefix,10);

	int lease_t = 0;
	int lease_d = atoi(day);
	int lease_h = atoi(hour);
	int lease_m = atoi(min);
	lease_t = lease_d*24*3600+lease_h*3600+lease_m*60;
	sprintf(slease,"%d",lease_t);	

	if(strcmp(sauth,"")==0)
		strcpy(sauth,"no");

	if(strcmp(sprefix,"")==0)
		strcpy(sprefix,"no");

	add_dhcp_node_attr(DHCPVS_XML, DHCPVS_SC_CONF, DHCP_NODE_ATTR, ssubset);

	//lease
	mod_dhcp_node(DHCPVS_XML, DHCPVS_SC_CONF, DHCP_NODE_ATTR, ssubset, DHCPVS_TC_LEASE, slease);

	//name service name
	mod_dhcp_node(DHCPVS_XML, DHCPVS_SC_CONF, DHCP_NODE_ATTR, ssubset, DHCPVS_TC_NSERVER, snserver);

	//domain search
	mod_dhcp_node(DHCPVS_XML, DHCPVS_SC_CONF, DHCP_NODE_ATTR, ssubset, DHCPVS_TC_DSEARCH, ssearch);

	//netmask
	mod_dhcp_node(DHCPVS_XML, DHCPVS_SC_CONF, DHCP_NODE_ATTR, ssubset, DHCPVS_TC_NETMASK, "64");

	//pools
	mod_dhcp_node(DHCPVS_XML, DHCPVS_SC_CONF, DHCP_NODE_ATTR, ssubset, DHCPVS_TC_POOL, spool);

	//subnet
	mod_dhcp_node(DHCPVS_XML, DHCPVS_SC_CONF, DHCP_NODE_ATTR, ssubset, DHCPVS_TC_SUBNET, ssubset);

	//prefix
	mod_dhcp_node(DHCPVS_XML, DHCPVS_SC_CONF, DHCP_NODE_ATTR, ssubset, DHCPVS_TC_PREFIX,sprefix);

	//auth
	mod_dhcp_node(DHCPVS_XML, DHCPVS_SC_CONF, DHCP_NODE_ATTR, ssubset, DHCPVS_TC_AUTH,sauth);

	///////////write to conf
	int cflag=-1,ret=-1;

	struct dvsconfz c_head;
	int cnum;


	cflag=read_dvsconf_xml(DHCPVS_XML,&c_head, &cnum);

	if(cflag==0)
	{
		ret=write_dhcpvs_conf(DHCPVS_CONF_PATH, &c_head);
	}

	if((cflag==0 )&& (cnum > 0))
		Free_dvsconfz_info(&c_head);

	if(ret == 0)
	{
		ShowAlert(search(lcontrol,"add_suc"));
	}
	else
		ShowAlert(search(lcontrol,"add_err"));

	free(day);
	free(hour);
	free(min);
	free(spool);
	free(slease);
	free(ssubset);
	free(snserver);
	free(ssearch);
	free(snetmask);
}





int ShowConfClearPage(char *m,char *id,struct list *lcontrol,struct list *lpublic)
{

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


		int ret=-1;
		ret=delete_dhcp_onelevel(DHCPVS_XML, DHCPVS_SC_CONF, DHCP_NODE_ATTR, id);
						///////////write to conf
		int cflag=-1;

		struct dvsconfz c_head;
		int cnum;


		cflag=read_dvsconf_xml(DHCPVS_XML,&c_head, &cnum);

		if(cflag==0)
		{
			ret=write_dhcpvs_conf(DHCPVS_CONF_PATH, &c_head);
		}

		if((cflag==0 )&& (cnum > 0))
			Free_dvsconfz_info(&c_head);		
		
		if(ret==0)
		   ShowAlert(search(lpublic,"oper_succ"));
		else
			ShowAlert(search(lpublic,"oper_fail"));

		
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_dhcpvs_conf.cgi?UN=%s';\n", m);
		fprintf( cgiOut, "</script>\n" );	

		fprintf( cgiOut, "</body>\n" );
		fprintf( cgiOut, "</html>\n" );

	return 0;
}



