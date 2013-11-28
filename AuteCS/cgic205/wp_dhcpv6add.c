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
* wp_dhcpadd.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system cotrl for dhcp  add
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
#include "ws_init_dbus.h"
#include "ws_dhcpv6.h"

int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic);
void  ConfIPAdd(struct list *lcontrol,struct list *lpublic); 


int cgiMain()
{
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	
	ShowDhcpconPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	return 0;
}

int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic)
{ 
	char encry[BUF_LEN] = {0};
	char *str;
	int i = 0;   
	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}	  
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
 
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  	"<style type=text/css>\n"\
  	".a3{width:30;border:0; text-align:center}\n"\
  	"</style>\n"\
    "<script type=\"text/javascript\">");
	 fprintf(cgiOut, "function get_ipv6_addr(str_name)\n"\
								"{\n"\
								   "var str_para = \"\";"\
								   "var obj_temp;\n"\
								   "var str_temp = \"\";"\
								   "var iter = 1;\n"\
								   "for(; iter<=8; iter++)\n"\
								   "{\n"\
									  "str_temp = \"document.getElementById(\\\"\" +  str_name + iter + \"\\\")\";"\
									  "obj_temp = eval(str_temp);\n"\
									  "if(\"\" == obj_temp.value)"\
									  "{\n"\
										 "return null;\n"\
									  "}\n"\
									  "str_para += obj_temp.value + \":\";\n"\
								   "}\n"\
								   "str_temp = str_para.replace(/\\:$/,\"\");"\
								   "return str_temp;\n"\
								"}");
	  fprintf(cgiOut, "function ip2val(str_ip_addr)\n"\
                            "{\n"\
                               "var arr_ip_seg = str_ip_addr.split(\":\");\n"\
                               "var un_val = 0;\n"\
                               "for(var i=0; i<8; i++)\n"\
                               "{\n"\
                                  "un_val = (un_val << 8) + parseInt(arr_ip_seg[i]);\n"\
                               "}\n"\
                               "return un_val;\n"\
                            "}");
	   fprintf(cgiOut, "function clear_ip_addr(str_name)\n"\
                            "{\n"\
                               "var obj_temp;\n"\
                               "var str_temp = \"\";"\
                               "var iter = 1;\n"\
                               "for(; iter<=8; iter++)\n"\
                               "{\n"\
                                  "str_temp = \"document.getElementById(\\\"\" +  str_name + iter + \"\\\")\";"\
                                  "obj_temp = eval(str_temp);\n"\
                                  "obj_temp.value = \"\";"\
                               "}\n"\
                            "}");
	  fprintf(cgiOut,"function rm_dns(){\n"\
	  	"for(var i=0; i<document.all.dnselect.options.length; i++){\n"\
		  "if(document.all.dnselect.options[i].selected==true){\n"\
		  "var optionIndex ;\n"\
		  "optionIndex = i;\n"\
		  "}\n"\
		  "if(optionIndex!=null){\n"\
		  "document.all.dnselect.remove(optionIndex);\n"\
		  "}\n"\
		  "}\n"\
          "}");	
	  fprintf(cgiOut,"function rm_opt(){\n"\
	  	"for(var i=0; i<document.all.optselect.options.length; i++){\n"\
		  "if(document.all.optselect.options[i].selected==true){\n"\
		  "var optionIndex ;\n"\
		  "optionIndex = i;\n"\
		  "}\n"\
		  "if(optionIndex!=null){\n"\
		  "document.all.optselect.remove(optionIndex);\n"\
		  "}\n"\
		  "}\n"\
          "}");
	
		fprintf(cgiOut,"function add_dns()\n"\
			"{\n"\
   		 		"var dnsnum = document.all.dnselect.options.length;\n"\
   		 		"if (dnsnum > 2)"\
   		 		"{"\
	   		 		"alert(\"%s\");"\
	   		 		"return false;"\
   		 		"}"\
   		 		"else"\
				"{"\
					"var dnsip = get_ipv6_addr(\"dns_ip\");\n"\
					"if(dnsip != null)\n"\
					"{\n"\
						"document.all.dnselect.options.add(\n"\
						"new Option(dnsip,dnsip,false,false)\n"\
						");\n"\
					"}\n"\
					"else\n"\
					"{\n"\
						"alert(\"%s\");\n"\
						"window.event.returnValue = false;\n"\
					"}\n"\
				"}"\
				"clear_ip_addr(\"dns_ip\");\n"\
			"}",search(lcontrol,"dns_max_num"),search(lpublic,"ip_not_null"));
				fprintf(cgiOut,"function add_opt()\n"\
			"{\n"\
   		 		"var optnum = document.all.optselect.options.length;\n"\
   		 		"if (optnum > 7)"\
   		 		"{"\
	   		 		"alert(\"%s\");"\
	   		 		"clear_ip_addr(\"opt_ip\");\n"\
	   		 		"return false;"\
   		 		"}"\
   		 		"else"\
				"{"\
					"var optip = get_ipv6_addr(\"opt_ip\");\n"\
					"for(var k=0; k<document.all.optselect.options.length; k++)\n"\
					"{\n"\
	                  "if(optip == document.all.optselect.options[k].value)\n"\
	                  "{\n"\
		                  "alert(\"%s\");\n"\
		                  "clear_ip_addr(\"opt_ip\");\n"\
		                  "return;\n"\
	                  "}\n"\
					"}\n"\
					"if(optip != null)\n"\
					"{\n"\
						"document.all.optselect.options.add(\n"\
						 "new Option(optip,optip,false,false)\n"\
						 ");\n"\
					"}\n"\
					"else\n"\
					"{\n"\
						"alert(\"%s\");\n"\
						"window.event.returnValue = false;\n"\
					"}\n"\
				"}"\
				"clear_ip_addr(\"opt_ip\");\n"\
			"}","option52 num is more than 8",search(lpublic,"dhcp_exist"),search(lpublic,"ip_not_null"));
	   fprintf(cgiOut, "function mysubmit()\n"\
					 "{\n"\
					    "var un_temp = 0;\n"\
		   		 		"var flagz = \"\";"\
		   		 		"var sflag = 0;\n"\
		   		 		"var eflag =0;\n"\
		   		 		"var poolname=document.all.pool_name.value;\n"\
		   		 		"if(poolname == \"\")"\
		   		 		"{\n"\
			   		 		"alert(\"%s\");\n"\
			   		 		"return false;\n"\
		   		 		"}\n"\
		   		 		"else\n"\
		   		 		"{\n"\
		   		 			"document.all.poolname.value=poolname;\n"\
		   		 		"}\n"\
					   "var v1=document.all.day.value;\n"\
		   				"if(v1>365)\n"\
	   	    			 "{\n"\
	   	     				"alert(\"%s\");\n"\
	   	      				"return false;\n"\
		   		 		"}\n"\
		   		 		"var v2=document.all.hour.value;\n"\
		   				"if(v2>23)\n"\
	   	    			 "{\n"\
	   	     				"alert(\"%s\");\n"\
	   	      				"return false;\n"\
		   		 		"}\n"\
		   		 		"var v3=document.all.min.value;\n"\
		   				"if(v3>59)\n"\
	   	    			 "{\n"\
	   	     				"alert(\"%s\");\n"\
	   	      				"return false;\n"\
		   		 		"}\n"\
		   		 		"var v4=v1 + v2 + v3;\n"\
		   				"if(v4 == \"\")"\
		   	    		"{\n"\
	   	     				"alert(\"%s\");\n"\
	   	      				"return false;\n"\
		   		 		"}\n"\
		   		 		"var leaseall = v1*86400 + v2*3600 + v3*60;\n"\
		   		 		"if(leaseall>31536000)\n"\
		   		 		"{\n"\
			   		 		"alert(\"%s\");\n"\
			   		 		"return false;\n"\
		   		 		"}\n"\
		   		 		"var prefixnum=document.all.prefixl.value;\n"\
		   		 		"if((prefixnum > 16)|| (prefixnum < 1) || (prefixnum == \"\"))\n"\
	   	    			 "{\n"\
	   	     				"alert('prefix is form 1 to 16');\n"\
	   	      				"return false;\n"\
		   		 		"}\n"\
						"var startipz = get_ipv6_addr(\"beg_ip\");\n"\
						"var endipz = get_ipv6_addr(\"end_ip\");\n"\
						"if((startipz == null)||(endipz == null))\n"\
						"{\n"\
							"alert(\"%s\");\n"\
							"return false;\n"\
						"}\n"\
						"document.all.startip.value = startipz;"\
						"document.all.endip.value = endipz;"\
		   		 		"var tt = \"\"; "\
						"for(var j=0; j<document.all.dnselect.options.length; j++)\n"\
						"{\n"\
							"tt = document.all.dnselect.options[j].value;\n"\
   							"if(j==0)\n"\
				   				"{\n"\
				   					"document.all.dnsipstr.value = tt;\n"\
				   				"}\n"\
				   				"else{\n"\
				   					"document.all.dnsipstr.value = document.all.dnsipstr.value + \",\" + tt;\n"\
				   				"}\n"\
					   "}\n"\
					   "tt = \"\"; "\
					   "for(var k=0; k<document.all.optselect.options.length; k++)\n"\
							"{\n"\
								"tt = document.all.optselect.options[k].value;\n"\
	   							"if(k==0)\n"\
					   				"{\n"\
					   					"document.all.opt_ip.value = tt;\n"\
					   				"}\n"\
					   				"else{\n"\
					   					"document.all.opt_ip.value = document.all.opt_ip.value + \",\" + tt;\n"\
					   				"}\n"\
					   "}\n"\
		   		"}\n",search(lcontrol,"pool_arg_not_null"),search(lcontrol,"lease_d_error"),search(lcontrol,"lease_h_error"),search(lcontrol,"lease_m_error"),search(lcontrol,"lease_empty"),search(lcontrol,"lease_max"),"ip is not null");
	fprintf(cgiOut,"</script>\n"\
	"<script language=javascript src=/ipv6.js>\n"\
	"</script>\n"\
	"</head>\n"\
	"<body>");
	if(cgiFormSubmitClicked("dhcp_add") == cgiFormSuccess)
	{
		ConfIPAdd(lcontrol,lpublic);
	}
  
	fprintf(cgiOut,"<form method=post onsubmit=\"return mysubmit()\">\n"\
	"<div align=center>\n"\
	"<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
	"<tr>\n"\
	"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>\n"\
	"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>\n"\
	"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCP IPV6");
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>\n"\
	"<tr>");
	fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_add style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpv6con.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	fprintf(cgiOut,"</tr>\n"\
	"</table>");


	fprintf(cgiOut,"</td>\n"\
	"<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>\n"\
	"</tr>\n"\
	"<tr>\n"\
	"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>\n"\
	"<tr>\n"\
	"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>\n"\
	"<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>\n"\
	"<tr height=4 valign=bottom>\n"\
	"<td width=120>&nbsp;</td>\n"\
	"<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>\n"\
	"</tr>\n"\
	"<tr>\n"\
	"<td><table width=120 border=0 cellspacing=0 cellpadding=0>\n"\
	"<tr height=25>\n"\
	"<td id=tdleft>&nbsp;</td>\n"\
	"</tr>");
	fprintf(cgiOut,"<tr height=25>\n"\
	"<td align=left id=tdleft><a href=wp_dhcpv6con.cgi?UN=%s target=mainFrame class=top><font id=%s>DHCP</font><font id=%s> %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));
	fprintf(cgiOut,"</tr>");  
	fprintf(cgiOut,"<tr height=26>\n"\
	"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></a></td>",search(lpublic,"menu_san"),search(lcontrol,"add"));
	fprintf(cgiOut,"</tr>");         						

	for(i=0;i<19;i++)
	{
		fprintf(cgiOut,"<tr height=25>\n"\
		"<td id=tdleft>&nbsp;</td>\n"\
		"</tr>");
	}
	fprintf(cgiOut,"</table>\n"\
	"</td>\n"\
	"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");

	fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>");
	////pool name 
	fprintf(cgiOut,"<tr>\n"\
	"<td width=100 height=40>%s:</td>","POOL");
	fprintf(cgiOut,"<td width=100><input type=text name=pool_name  size=21 maxLength=20/></td>\n"\
	"<td colspan=2></td>");
	fprintf(cgiOut,"</tr>");
	//domain name
	fprintf(cgiOut,"<tr>\n");
	fprintf(cgiOut,"<td width=100 height=40 >%s:</td>","Domain");
	fprintf(cgiOut,"<td><input type=text name=domname value=\"\" size=21 maxLength=20></td></tr>\n");

	fprintf(cgiOut,"<tr>\n"\
	"<td width=100 height=40 >%s:</td>",search(lcontrol,"lease duration"));
	fprintf(cgiOut,"<td colspan=4 width=350 align=left><input type=text name=day onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
	onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
	ondragenter=\"return  false;\"\
	style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" size=5/><font color=red>(%s)</font>",search(lcontrol,"day"));
	fprintf(cgiOut,"<input type=text name=hour onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
	onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
	ondragenter=\"return  false;\"\
	style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" size=5/><font color=red>(%s)</font>",search(lcontrol,"hour"));
	fprintf(cgiOut,"<input type=text name=min onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
	onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
	ondragenter=\"return  false;\"\
	style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" size=5/><font color=red>(%s)</font></td>",search(lcontrol,"time_min"));
	fprintf(cgiOut,"</tr>");
	fprintf(cgiOut,"<tr>\n"\
	"<td width=100 height=40>%s:</td>",search(lcontrol,"dnserver"));

	fprintf(cgiOut,"<td width=290>\n"\
	"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:290;font-size:9pt\">");
	fprintf(cgiOut,"<input type=text  name=dns_ip1 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=dns_ip2 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=dns_ip3 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=dns_ip4 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=dns_ip5 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=dns_ip6 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=dns_ip7 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=dns_ip8 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	fprintf(cgiOut,"</div></td>\n"\
	"<td>&nbsp;</td>\n"\
	"<td colspan=2 width=50 ><input type=button name=dns_add id=ranges_add onclick=add_dns() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_add"));
	fprintf(cgiOut,"</tr>");

	fprintf(cgiOut,"<tr height=60>\n"\
	"<td>&nbsp;</td>\n"\
	"<td width=140 colspan=2 ><select name=dnselect  valus=\"\" size=\"3\" style=\"width:290px\"></select></td>"\
	"<td width=50 colspan=2 valign=top><input type=button name=dns_rm id=ranges_rm onclick=rm_dns() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_rm"));
	fprintf(cgiOut,"</tr>"); 			
	//////////option52 				
	fprintf(cgiOut,"<tr height=30 id=hex2>\n");
	fprintf(cgiOut,"<td width=80>\n");
	fprintf(cgiOut,"option52:");
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"<td width=290>\n"\
	"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:290;font-size:9pt\">");
	fprintf(cgiOut,"<input type=text  name=opt_ip1 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=opt_ip2 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=opt_ip3 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=opt_ip4 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=opt_ip5 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=opt_ip6 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=opt_ip7 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=opt_ip8 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	fprintf(cgiOut,"</div></td>");
	fprintf(cgiOut,"<td>&nbsp;</td>");
	fprintf(cgiOut,"<td colspan=2 width=50 ><input type=button name=optid_add id=ranges_add onclick=add_opt() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_add"));
	fprintf(cgiOut,"</tr>\n");

	fprintf(cgiOut,"<tr height=60 id=hex3>\n"\
	"<td>&nbsp;</td>\n"\
	"<td width=140 colspan=2 ><select name=optselect  valus=\"\" size=\"8\" style=\"width:290px\"></select></td>"\
	"<td width=50 colspan=2 valign=top><input type=button name=opt_rm id=ranges_rm onclick=rm_opt() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_rm"));
	fprintf(cgiOut,"</tr>");


	///dhcp router ip 
	fprintf(cgiOut,"<tr>\n"\
	"<td width=100 height=40>%s:</td>","DHCP prefix-length");
	fprintf(cgiOut,"<td colspan=4><input type=text name=prefixl value=\"\" maxLength=2><font color=red>(1-16></font></td>\n"\
	"</tr>");

	fprintf(cgiOut,"<tr>\n"\
	"<td width=100 height=40>%s:</td>",search(lcontrol,"dhcp_pool"));

	//ip address pool,
	fprintf(cgiOut,"<td width=290>\n"\
	"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:290;font-size:9pt\">");
	fprintf(cgiOut,"<input type=text  name=beg_ip1 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=beg_ip2 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=beg_ip3 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=beg_ip4 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=beg_ip5 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=beg_ip6 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=beg_ip7 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=beg_ip8 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	fprintf(cgiOut,"</div></td>\n"\
	"<td width=10 align=center >-</td>\n"\
	"<td width=290>\n"\
	"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:290;font-size:9pt\">");
	fprintf(cgiOut,"<input type=text  name=end_ip1 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=end_ip2 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=end_ip3 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=end_ip4 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=end_ip5 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text  name=end_ip6 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=end_ip7 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text  name=end_ip8 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	fprintf(cgiOut,"</div></td>\n"\
	"<td width=50 ></td>");
	fprintf(cgiOut,"</tr>");

	fprintf(cgiOut,"<input type=hidden name=UN value=%s>",encry);
	fprintf(cgiOut,"<input type=hidden name=poolname>");
	fprintf(cgiOut,"<input type=hidden name=dnsipstr>");
	fprintf(cgiOut,"<input type=hidden name=startip>");
	fprintf(cgiOut,"<input type=hidden name=endip>");
	fprintf(cgiOut,"<input type=hidden name=opt_ip>");
	fprintf(cgiOut,"<input type=hidden name=option52str>");
	fprintf(cgiOut,"</table>");		


fprintf(cgiOut,"</td>\n"\
            "</tr>\n"\
            "<tr height=4 valign=top>\n"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>\n"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>\n"\
            "</tr>\n"\
          "</table>\n"\
        "</td>\n"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>\n"\
      "</tr>\n"\
    "</table></td>\n"\
  "</tr>\n"\
  "<tr>\n"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>\n"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>\n"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>\n"\
  "</tr>\n"\
"</table>\n"\
"</div>\n"\
"</form>\n"\
"</body>\n"\
"</html>");

return 0;
}

void  ConfIPAdd(struct list *lcontrol,struct list *lpublic)
{
    char poolname[32] = {0};	
	//char lease[32] = {0};
	char day[10] = {0};
	char hour[10] = {0};
	char min[10] = {0};
	char dnsipstr[512] = {0};
	char startip[128] = {0};
	char endip[128] = {0};
	char domname[128] = {0};
	char opt_ip[512] = {0};	
	char predixstr[10] = {0};
	char option52str[512] = {0};
	
	int ret1=-1,mode = 1,ret2 = 0;
	int retconfig = 0;
	unsigned int index = 0;
	unsigned int bkindex = 0;
	
	cgiFormStringNoNewlines("poolname",poolname,sizeof(poolname));
	cgiFormStringNoNewlines("domname",domname,sizeof(domname));
	cgiFormStringNoNewlines("day",day,sizeof(day));
	cgiFormStringNoNewlines("hour",hour,sizeof(hour));
	cgiFormStringNoNewlines("min",min,sizeof(min));
	cgiFormStringNoNewlines("dnsipstr",dnsipstr,sizeof(dnsipstr));
	cgiFormStringNoNewlines("startip",startip,sizeof(startip));
	cgiFormStringNoNewlines("endip",endip,sizeof(endip));	
	cgiFormStringNoNewlines("opt_ip", opt_ip, sizeof(opt_ip));	
	cgiFormStringNoNewlines("prefixl", predixstr, sizeof(predixstr));
	cgiFormStringNoNewlines("option52str", option52str, sizeof(option52str));

	int lease_t = 0;
	int lease_d = atoi(day);
	int lease_h = atoi(hour);
	int lease_m = atoi(min);
	lease_t = lease_d*24*3600+lease_h*3600+lease_m*60;
	//sprintf(lease,"%d",lease_t);	

	ccgi_dbus_init();
	/*ip pool name*/
	if (0 != strcmp(poolname,""))
	{
		ret1 = 0;//ccgi_create_ipv6_pool_name(ADD_OPT, poolname, &index);		
		retconfig =0;// ccgi_config_ipv6_pool_name(poolname,&bkindex);

		if ((1 == ret1)||(1 == retconfig))
		{
			/*add ip pool range*/	
			if ((0 != strcmp(startip,"")) && (0 != strcmp(endip,"")))
			{
				if(1 == retconfig)
				{
					index = bkindex;
				}
				ret2 = 0;//ccgi_addordel_ipv6pool("add", startip, endip, predixstr, index);
				if (1 == ret2)
				{
					/*add domain name*/
					if(strcmp(domname,"")!=0)
					{
						ccgi_set_server_domain_search_ipv6(domname,mode, index, ADD_OPT);
					}
					/*add dns*/	
					if (0 != strcmp(dnsipstr,""))
					{
						//ip_dhcp_server_dns(mode, index, dnsipstr);
					}
					/*add lease*/
					//ccgi_set_server_lease_default_ipv6(lease_t,mode,index,ADD_OPT);
					/*option 52*/
					if (0 != strcmp(option52str,""))
					{
						ccgi_set_server_option52_ipv6(opt_ip,mode,index, ADD_OPT);
					}
					//ccgi_set_server_name_servers_ipv6(dnsipstr,mode, index, ADD_OPT);
					ccgi_set_server_option52_ipv6(opt_ip, mode,index,ADD_OPT);

					ShowAlert(search(lcontrol,"add_suc"));
				}
				else
				{
				 	ShowAlert(search(lcontrol,"add_err"));
				}
			}
		}
		else
		{
			ShowAlert(search(lcontrol,"add_err"));
		}
	}
}

