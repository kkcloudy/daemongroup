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
#include "ws_dhcp_conf.h"
#include "ws_dcli_dhcp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic);
void  ConfIPAdd(struct list *lcontrol,struct list *lpublic); 


int cgiMain()
{
	
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	ccgi_dbus_init();
	ShowDhcpconPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	return 0;
}

int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic)
{ 
  char encry[BUF_LEN] = {0};
  char *str;
  char dhcp_encry[BUF_LEN] = {0};  
  int i = 0;   
  
   if(cgiFormSubmitClicked("dhcp_add") != cgiFormSuccess)
   {
		memset(encry,0,BUF_LEN);
	    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	    str=dcryption(encry);
	    if(str==NULL)
	    {
		      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
		}
		memset(dhcp_encry,0,BUF_LEN);                   /*清空临时变量*/
   }
   else
  	{
	  	cgiFormStringNoNewlines("encry_dhcp", dhcp_encry, BUF_LEN); 
	    str=dcryption(dhcp_encry);
	    if(str==NULL)
	    {
		      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
		}
		memset(dhcp_encry,0,BUF_LEN);                   /*清空临时变量*/
  	}
 
  cgiFormStringNoNewlines("encry_dhcp",dhcp_encry,BUF_LEN);

	instance_parameter *paraHead2 = NULL;
	instance_parameter *p_q = NULL;
	list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
	char allslotid[10] = {0};
	int allslot_id = 0;
	cgiFormStringNoNewlines("allslotid",allslotid,sizeof(allslotid));
	allslot_id = atoi(allslotid);
	if(0 == allslot_id)
	{
		for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
		{
			allslot_id = p_q->parameter.slot_id;
			break;
		}
	}	
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
 
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  	"<style type=text/css>\n"\
  	".a3{width:30;border:0; text-align:center}\n"\
  	"</style>\n"\
    "<script type=\"text/javascript\">");
  	 fprintf(cgiOut, "function if_mask_ranges()"\
                            "{\n"\
                               "var flag=0;\n"\
                               	"var un_mask=get_ip_addr(\"mask\");\n"\
                               	"if(un_mask==null)\n"\
                           		"{\n"\
                           			"alert(\"%s\");\n"\
                           			"return 0;\n"\
                           		"}\n"\
                               	"var mask_val = new Array(\n"\
                                                     "\"invalid\","\
                                                     "\"128.0.0.0\","\
                                                     "\"192.0.0.0\","\
                                                     "\"224.0.0.0\","\
                                                     "\"240.0.0.0\","\
                                                     "\"248.0.0.0\","\
                                                     "\"252.0.0.0\","\
                                                     "\"254.0.0.0\","\
                                                     "\"255.0.0.0\","\
                                                     "\"255.128.0.0\","\
                                                     "\"255.192.0.0\","\
                                                     "\"255.224.0.0\","\
                                                     "\"255.240.0.0\","\
                                                     "\"255.248.0.0\","\
                                                     "\"255.252.0.0\","\
                                                     "\"255.254.0.0\","\
                                                     "\"255.255.0.0\","\
                                                     "\"255.255.128.0\","\
                                                     "\"255.255.192.0\","\
                                                     "\"255.255.224.0\","\
                                                     "\"255.255.240.0\","\
                                                     "\"255.255.248.0\","\
                                                     "\"255.255.252.0\","\
                                                     "\"255.255.254.0\","\
                                                     "\"255.255.255.0\","\
                                                     "\"255.255.255.128\","\
                                                     "\"255.255.255.192\","\
                                                     "\"255.255.255.224\","\
                                                     "\"255.255.255.240\","\
                                                     "\"255.255.255.248\","\
                                                     "\"255.255.255.252\","\
                                                     "\"255.255.255.254\","\
                                                     "\"invalid\""
                                                     ");"
                                 "for(i=1;i<32;i++)\n"\
	                             	"{\n"\
	                             		"if(mask_val[i]==un_mask)\n"\
	                             			"{\n"\
	                             				"flag=1;\n"\
	                             				"break;\n"\
	                             			"}\n"\
	                             	"}\n"\
                                 "if(flag==0)\n"\
  	 								"{\n"\
  	 									"alert(\"%s\");\n"\
  	 									"return 2;\n"\
  	 								"}\n"\
  	 						"return 1;\n"\
                            "}",search(lpublic,"mask_not_null"),search(lcontrol,"dhcp_mask_err"));	 
	 fprintf(cgiOut, "function get_ip_addr(str_name)\n"\
								"{\n"\
								   "var str_para = \"\";"\
								   "var obj_temp;\n"\
								   "var str_temp = \"\";"\
								   "var iter = 1;\n"\
								   "for(; iter<=4; iter++)\n"\
								   "{\n"\
									  "str_temp = \"document.getElementById(\\\"\" +  str_name + iter + \"\\\")\";"\
									  "obj_temp = eval(str_temp);\n"\
									  "if(\"\" == obj_temp.value)"\
									  "{\n"\
										 "return null;\n"\
									  "}\n"\
									  "str_para += obj_temp.value + \".\";\n"\
								   "}\n"\
								   "str_temp = str_para.replace(/\\.$/,\"\");"\
								   "return str_temp;\n"\
								"}");
	 fprintf(cgiOut, "function get_sub_net(str_ip_addr, mask)"
                            "{\n"\
                               "var tmp = \"\";"\
                               "var ip_tmp = ip2val(str_ip_addr);\n"\
                               "var mask_tmp = ip2val(mask);\n"\
                               "var str_ip_addr = \"\";"\
                               "var tmp_ip = ip_tmp >>> 24;\n"\
                               "var tmp_mask = mask_tmp >>>24;\n"\
                               "tmp = tmp_ip&tmp_mask;\n"\
                               "str_ip_addr = tmp.toString(10);\n"\
	 						   "tmp_ip = ip_tmp << 8;\n"\
                               "tmp_ip = tmp_ip >>> 24;\n"\
                               "tmp_mask = mask_tmp << 8;\n"\
                               "tmp_mask = tmp_mask >>> 24;\n"\
							   "tmp = tmp_ip&tmp_mask;\n"\
							   "str_ip_addr += \".\" + tmp.toString(10);\n"\
							   "tmp_ip = ip_tmp << 16;\n"\
                               "tmp_ip = tmp_ip >>> 24;\n"\
							   "tmp_mask = mask_tmp << 16;\n"\
							   "tmp_mask = tmp_mask >>> 24;\n"\
	 							"tmp = tmp_ip&tmp_mask;\n"\
								"str_ip_addr += \".\" + tmp.toString(10);\n"\
								"tmp_ip = ip_tmp << 24;\n"\
                               "tmp_ip = tmp_ip >>> 24;\n"\
							   "tmp_mask = mask_tmp << 24;\n"\
							   "tmp_mask = tmp_mask >>> 24;\n"\
								"tmp = tmp_ip&tmp_mask;\n"\
								"str_ip_addr += \".\" + tmp.toString(10);\n"\
								"return str_ip_addr;\n"\
                            "}");	 
	  fprintf(cgiOut, "function ip2val(str_ip_addr)\n"\
                            "{\n"\
                               "var arr_ip_seg = str_ip_addr.split(\".\");\n"\
                               "var un_val = 0;\n"\
                               "for(var i=0; i<4; i++)\n"\
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
                               "for(; iter<=4; iter++)\n"\
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
	fprintf(cgiOut,"function dnKeyDown()\n"\
		"{\n"\
		" var   k=window.event.keyCode;\n"\
		"if ((k==46)||(k==8) || (k>=48 && k<=57)||(k>=65 && k <=90)||(k>=97 && k<=122)||(k>=37 && k<=40))\n"\
   		        " {}\n"\
    	" else if(k==13){\n"\
       		" window.event.keyCode = 9;}\n"\
   		" else{\n"\
      	 	 "window.event.returnValue = false;}\n"\
		"var length = 0;\n"\
		"length = document.all.dns_name.value.length;\n"\
		"if((k==46)||(k==8))\n"\
		"{\n"\
		"}\n"\
		"else\n"\
		"{\n"\
			"if(length >=20)\n"\
			"{\n"\
				"alert(\"%s\");\n"\
				"window.event.returnValue = false;\n"\
			"}\n"\
		"}\n"\
		"}",search(lcontrol,"name_dns"));   
	
		fprintf(cgiOut,"function add_dns()\n"\
			"{\n"\
				"var dnsip = get_ip_addr(\"dns_ip\");\n"\
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
				"clear_ip_addr(\"dns_ip\");\n"\
			"}",search(lpublic,"ip_not_null"));
				fprintf(cgiOut,"function add_opt()\n"\
			"{\n"\
				"var optip = get_ip_addr(\"opt_ip\");\n"\
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
				"clear_ip_addr(\"opt_ip\");\n"\
			"}",search(lpublic,"dhcp_exist"),search(lpublic,"ip_not_null"));
       fprintf(cgiOut,"function hexen()\n"\
	   		"{\n"\
		   		"document.getElementById(\"hex1\").style.display = 'block';\n"\
		   		"document.getElementById(\"hex2\").style.display = 'none';\n"\
		   		"document.getElementById(\"hex3\").style.display = 'none';\n"\
	   		"}");
       fprintf(cgiOut,"function opten()\n"\
	   		"{\n"\
		   		"document.getElementById(\"hex1\").style.display = 'none';\n"\
		   		"document.getElementById(\"hex2\").style.display = 'block';\n"\
		   		"document.getElementById(\"hex3\").style.display = 'block';\n"\
	   		"}\n");
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
	   					"var maskz = get_ip_addr(\"mask\");\n"\
	   					"if(maskz == null)\n"\
	   					"{\n"\
		   					"alert(\"%s\");\n"\
		   					"return false;\n"\
	   					"}\n"\
	   					"else\n"\
						"{\n"\
						    "flagz = if_mask_ranges();\n"\
						    "if(flagz == 1)\n"\
						    "{\n"\
								"document.all.ip_mask.value = maskz;\n"\
							"}\n"\
							"else\n"\
							"{\n"\
								"alert(\"%s\");\n"\
								"return false;\n"\
							"}\n"\
						"}\n"\
						"var startipz = get_ip_addr(\"beg_ip\");\n"\
						"var endipz = get_ip_addr(\"end_ip\");\n"\
						"if((startipz == null)||(endipz == null))\n"\
						"{\n"\
							"alert(\"%s\");\n"\
							"return false;\n"\
						"}\n"\
						"else if((startipz != null)&&(endipz != null))\n"\
						"{\n"\
							"var st_mask = get_sub_net(startipz, maskz);"\
							"var ed_mask = get_sub_net(endipz, maskz);"\
							"if(st_mask != ed_mask)\n"\
							"{\n"\
								"alert(\"%s\");\n"\
								"return false;\n"\
							"}\n"\
							"else\n"\
							"{\n"\
							    "un_temp = 0;\n"\
							    "sflag = 0;\n"\
								"un_temp = ip2val(startipz)&ip2val(maskz);\n"\
								"un_temp = ip2val(startipz)^un_temp;\n"\
								"sflag = un_temp;\n"\
								"un_temp = 0;\n"\
								"eflag = 0;\n"\
								"un_temp = ip2val(endipz)&ip2val(maskz);\n"\
								"un_temp = ip2val(endipz)^un_temp;\n"\
								"eflag = un_temp;\n"\
								"if(eflag < sflag)\n"\
	                            "{\n"\
	                            	"alert(\"%s\");\n"\
	                            	"return false;\n"\
	                            "}\n"\
	                            "else\n"\
	                            "{\n"\
									"document.all.startip.value = startipz;\n"\
									"document.all.endip.value = endipz;\n"\
								"}\n"\
							"}\n"\
						"}\n"\
		   		 		"var dnsnum = document.all.dnselect.options.length;\n"\
		   		 		"if(dnsnum>3)\n"\
		   		 		"{\n"\
		   		 			"alert(\"%s\");\n"\
		   		 			"return false;\n"\
		   		 		"}\n"\
		   		 		"var tt = \"\"; "\
						"for(var j=0; j<document.all.dnselect.options.length; j++)\n"\
						"{\n"\
							"tt = document.all.dnselect.options[j].value;\n"\
   							"if(j==0)\n"\
				   				"{\n"\
				   					"document.all.dns_ip.value = tt;\n"\
				   				"}\n"\
				   				"else{\n"\
				   					"document.all.dns_ip.value = document.all.dns_ip.value + \",\" + tt;\n"\
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
					   					"document.all.opt_ip.value = document.all.opt_ip.value + \" \" + tt;\n"\
					   				"}\n"\
					   "}\n"\
					 	"/* var tmp = \"\";"\
						"for(i=0; i<document.all.select.options.length; i++)\n"\
							"{\n"\
					   			"arr_ipr_list[i]=document.all.select.options[i].value;\n"\
								"tmp = arr_ipr_list[i];\n"\
	   							"tmp = tmp.replace(/-/,\" \");\n"\
	   							"if(i==0)\n"\
					   				"{\n"\
					   					"document.all.ip_s1.value = tmp;\n"\
					   				"}\n"\
					   				"else\n"\
					   				"{\n"\
					   					"document.all.ip_s1.value = document.all.ip_s1.value + \"^\" + tmp;\n"\
					   				"}\n"\
					   "} */\n"\
	   					"var routipz=get_ip_addr(\"route_ip\");\n"\
	   					"if(routipz != null)\n"\
						"{\n"\
							"document.all.routeip.value = routipz;\n"\
						"}\n"\
	   					"var winipz=get_ip_addr(\"win_ip\");\n"\
	   					"if(winipz != null)\n"\
						"{\n"\
							"document.all.winip.value = winipz;\n"\
						"}\n"\
		   		"}\n",search(lcontrol,"pool_arg_not_null"),search(lcontrol,"lease_d_error"),search(lcontrol,"lease_h_error"),search(lcontrol,"lease_m_error"),search(lcontrol,"lease_empty"),search(lcontrol,"lease_max"),search(lpublic,"mask_not_null"),search(lcontrol,"dhcp_mask_err"),search(lcontrol,"dhcp_ranges_empty"),search(lcontrol,"dhcp_subnet_fail"),search(lcontrol,"dhcp_endbeg_start"),search(lcontrol,"dns_max_num"));
  fprintf(cgiOut,"</script>\n"\
  "<script language=javascript src=/ip.js>\n"\
  "</script>\n"\
  "</head>\n"\
  "<body onLoad=\"hexen();\">");
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
				"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCP");
				fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
				fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>\n"\
				"<tr>");
				fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_add style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
				if(cgiFormSubmitClicked("dhcp_add") != cgiFormSuccess)
				fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
				else                                         
				fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,search(lpublic,"img_cancel"));
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
         		if(cgiFormSubmitClicked("dhcp_add") != cgiFormSuccess)
				{ 			
					fprintf(cgiOut,"<tr height=25>\n"\
					"<td align=left id=tdleft><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame class=top><font id=%s>DHCP</font><font id=%s> %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));
					fprintf(cgiOut,"</tr>");  
					fprintf(cgiOut,"<tr height=26>\n"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></a></td>",search(lpublic,"menu_san"),search(lcontrol,"add"));
					fprintf(cgiOut,"</tr>");         						
					fprintf(cgiOut,"<tr height=25>\n"\
					"<td align=left id=tdleft><a href=wp_dhcpinter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"interface_info"));
					fprintf(cgiOut,"</tr>"); 
					fprintf(cgiOut,"<tr height=25>\n"\
					"<td align=left id=tdleft><a href=wp_dhcpmac.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"bind"));
					fprintf(cgiOut,"</tr>"); 
					fprintf(cgiOut,"<tr height=25>\n"\
					"<td align=left id=tdleft><a href=wp_dhcplease.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"details"));
					fprintf(cgiOut,"</tr>"); 

					/*fprintf(cgiOut,"<tr height=25>\n"\
         			"<td align=left id=tdleft><a href=wp_dhcp_opt.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),"option");
         			fprintf(cgiOut,"</tr>"); 
					*/
						

				}
         		else if(cgiFormSubmitClicked("dhcp_add") == cgiFormSuccess) 			  
				{					   
					fprintf(cgiOut,"<tr height=25>\n"\
					"<td align=left id=tdleft><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame class=top><font id=%s>DHCP</font><font id=%s> %s</font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=26>\n"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"add"));   /*突出显示*/
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>\n"\
					"<td align=left id=tdleft><a href=wp_dhcpinter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"interface_info"));
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>\n"\
					"<td align=left id=tdleft><a href=wp_dhcpmac.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"bind"));
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>\n"\
					"<td align=left id=tdleft><a href=wp_dhcplease.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"details"));
					fprintf(cgiOut,"</tr>"); 
					
					/*fprintf(cgiOut,"<tr height=25>\n"\
         			"<td align=left id=tdleft><a href=wp_dhcp_opt.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),"option");
         			fprintf(cgiOut,"</tr>"); 
					*/
					
				}
				for(i=0;i<17;i++)
				{
					fprintf(cgiOut,"<tr height=25>\n"\
					"<td id=tdleft>&nbsp;</td>\n"\
					"</tr>");
				}
			 fprintf(cgiOut,"</table>\n"\
			              "</td>\n"\
			              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");

			fprintf(cgiOut,"<table width=500 border=0 cellspacing=0 cellpadding=0>");
			fprintf(cgiOut,"<tr>");
			fprintf(cgiOut,"<td>%s&nbsp;&nbsp;</td>","Slot ID:");
			fprintf(cgiOut,"<td><select name=allslot onchange=slotid_change(this)>");
			for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
			{
				if(p_q->parameter.slot_id == allslot_id)
				{
					fprintf(cgiOut,"<option value=\"%d\" selected>%d</option>",p_q->parameter.slot_id,p_q->parameter.slot_id);
				}
				else
				{
					fprintf(cgiOut,"<option value=\"%d\">%d</option>",p_q->parameter.slot_id,p_q->parameter.slot_id);
				}		
			}
			fprintf(cgiOut,"</select></td>");
			fprintf(cgiOut,"</tr>");
			fprintf( cgiOut,"<script type=text/javascript>\n");
		   	fprintf( cgiOut,"function slotid_change( obj )\n"\
		   	"{\n"\
		   	"var slotid = obj.options[obj.selectedIndex].value;\n"\
		   	"var url = 'wp_dhcpadd.cgi?UN=%s&allslotid='+slotid;\n"\
		   	"window.location.href = url;\n"\
		   	"}\n", encry);
		    fprintf( cgiOut,"</script>\n" );	
			free_instance_parameter_list(&paraHead2);	
			fprintf(cgiOut,"<input type=hidden name=allslotid value=\"%d\">",allslot_id);

			////pool name 
			/////
			#if 0
			fprintf(cgiOut,"<tr>\n"\
					"<td width=100 height=40>%s:</td>","POOL");
			#endif
			///////////////////////////////////////////////////////////////////////////////////////////////////////
			fprintf(cgiOut,"<tr>\n"\
					"<tr><td width=100 height=40>%s:</td>","POOL");
	        #if 0
			fprintf(cgiOut,"<td width=100><input type=text name=pool_name  size=21 maxLength=20/></td>\n"\
					"<td colspan=2></td>");
			#endif
            //////////////
			fprintf(cgiOut,"<td width=100><input type=text name=pool_name  size=21 maxLength=20/></td><font color=red>(%s)</font><td colspan=2></td></tr>",search(lcontrol,"invalid_char"));
			/////////////////
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
 			fprintf(cgiOut,"<td width=140>\n"\
 						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	  		fprintf(cgiOut,"<input type=text  name=dns_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	  	    fprintf(cgiOut,"<input type=text  name=dns_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
 		    fprintf(cgiOut,"<input type=text  name=dns_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		    fprintf(cgiOut,"<input type=text  name=dns_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	  	    fprintf(cgiOut,"</div></td>\n"\
				"<td>&nbsp;</td>\n"\
				"<td colspan=2 width=50 ><input type=button name=dns_add id=ranges_add onclick=add_dns() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_add"));
  			fprintf(cgiOut,"</tr>");
			
			fprintf(cgiOut,"<tr height=60>\n"\
						"<td>&nbsp;</td>\n"\
						"<td width=140 colspan=2 ><select name=dnselect  valus=\"\" size=\"6\" style=\"width:140px\"></select></td>"\
						"<td width=50 colspan=2 valign=top><input type=button name=dns_rm id=ranges_rm onclick=rm_dns() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_rm"));
			fprintf(cgiOut,"</tr>"); 			

			fprintf(cgiOut,"<tr height=30><td width=80>option43 select:</td>\n");
			fprintf(cgiOut,"<td><input type=radio name=showtype value=\"1\" checked onclick=\"hexen();\">\n");
			fprintf(cgiOut,"<input type=radio name=showtype value=\"2\" onclick=\"opten();\"></td>");
			fprintf(cgiOut,"</tr>\n");
			//////////option43 				
			#if 1
			fprintf(cgiOut,"<tr height=30 id=hex1>\n");
				fprintf(cgiOut,"<td width=80>\n");
				fprintf(cgiOut,"option43 HEX:");
				fprintf(cgiOut,"</td>\n");

				fprintf(cgiOut,"<td colspan=4>\n");
				fprintf(cgiOut,"<input type=text name=optype style=\"width:140px;\"><font color=red>(xx-%sxx-Numberxxxx-IP|120401010101)</font>",search(lpublic,"l_type"));
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"</tr>\n");
				#endif
				
			    fprintf(cgiOut,"<tr height=30 id=hex2>\n");
				fprintf(cgiOut,"<td width=80>\n");
				fprintf(cgiOut,"option43:");
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"<td width=140>\n"\
						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
				fprintf(cgiOut,"<input type=text  name=opt_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	  	        fprintf(cgiOut,"<input type=text  name=opt_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
 		        fprintf(cgiOut,"<input type=text  name=opt_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		        fprintf(cgiOut,"<input type=text  name=opt_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	  	        fprintf(cgiOut,"</div></td>");
				fprintf(cgiOut,"<td>&nbsp;</td>");
				fprintf(cgiOut,"<td colspan=2 width=50 ><input type=button name=optid_add id=ranges_add onclick=add_opt() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_add"));
				fprintf(cgiOut,"</tr>\n");

				fprintf(cgiOut,"<tr height=60 id=hex3>\n"\
					"<td>&nbsp;</td>\n"\
					"<td width=140 colspan=2 ><select name=optselect  valus=\"\" size=\"6\" style=\"width:140px\"></select></td>"\
					"<td width=50 colspan=2 valign=top><input type=button name=opt_rm id=ranges_rm onclick=rm_opt() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_rm"));
				fprintf(cgiOut,"</tr>");




				
			///dhcp router ip 
			fprintf(cgiOut,"<tr>\n"\
    				"<td width=100 height=40>%s:</td>","DHCP router ip");
			fprintf(cgiOut,"<td width=140>\n"\
							"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	  		fprintf(cgiOut,"<input type=text  name=route_ip1  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	  	    fprintf(cgiOut,"<input type=text  name=route_ip2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
 		    fprintf(cgiOut,"<input type=text  name=route_ip3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		    fprintf(cgiOut,"<input type=text  name=route_ip4  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	  	    fprintf(cgiOut,"</div></td>\n"\
					"<td width=10>&nbsp;</td>\n"\
					"<td width=140>&nbsp;</td>\n"\
					"<td width=50>&nbsp;</td>\n"\
				"</tr>");

			///dhcp win ip
			fprintf(cgiOut,"<tr>\n"\
    				"<td width=100 height=40>%s:</td>","DHCP win ip");
			fprintf(cgiOut,"<td width=140>\n"\
							"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	  		fprintf(cgiOut,"<input type=text  name=win_ip1  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	  	    fprintf(cgiOut,"<input type=text  name=win_ip2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
 		    fprintf(cgiOut,"<input type=text  name=win_ip3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		    fprintf(cgiOut,"<input type=text  name=win_ip4  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	  	    fprintf(cgiOut,"</div></td>\n"\
					"<td width=10>&nbsp;</td>\n"\
					"<td width=140>&nbsp;</td>\n"\
					"<td width=50>&nbsp;</td>\n"\
				"</tr>");

			fprintf(cgiOut,"<tr>\n"\
    				"<td width=100 height=40>%s:</td>",search(lcontrol,"dhcp_sub_mask"));
			fprintf(cgiOut,"<td width=140>\n"\
							"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	  		fprintf(cgiOut,"<input type=text  name=mask1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	  	    fprintf(cgiOut,"<input type=text  name=mask2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
 		    fprintf(cgiOut,"<input type=text  name=mask3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		    fprintf(cgiOut,"<input type=text  name=mask4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	  	    fprintf(cgiOut,"</div></td>\n"\
					"<td width=10>&nbsp;</td>\n"\
					"<td width=140>&nbsp;</td>\n"\
					"<td width=50>&nbsp;</td>\n"\
				"</tr>\n"\
				"<tr>\n"\
					"<td width=100 height=40>%s:</td>",search(lcontrol,"dhcp_pool"));

			//ip address pool,
 			fprintf(cgiOut,"<td width=140>\n"\
 							"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	  		fprintf(cgiOut,"<input type=text  name=beg_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	  	    fprintf(cgiOut,"<input type=text  name=beg_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
 		    fprintf(cgiOut,"<input type=text  name=beg_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		    fprintf(cgiOut,"<input type=text  name=beg_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	  	    fprintf(cgiOut,"</div></td>\n"\
 					"<td width=10 align=center >-</td>\n"\
					"<td width=150 >\n"\
						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	  		fprintf(cgiOut,"<input type=text  name=end_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	  	    fprintf(cgiOut,"<input type=text  name=end_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
 		    fprintf(cgiOut,"<input type=text  name=end_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		    fprintf(cgiOut,"<input type=text  name=end_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	  	    fprintf(cgiOut,"</div></td>\n"\
					"<td width=50 ></td>");
			fprintf(cgiOut,"</tr>");
			
			if(cgiFormSubmitClicked("dhcp_add") != cgiFormSuccess)
			{
				fprintf(cgiOut,"<input type=hidden name=encry_dhcp value=%s>",encry);
			}
		    else if(cgiFormSubmitClicked("dhcp_add") == cgiFormSuccess)
			{
				fprintf(cgiOut,"<input type=hidden name=encry_dhcp value=%s>",dhcp_encry);

			}
			fprintf(cgiOut,"<input type=hidden name=poolname>");
			fprintf(cgiOut,"<input type=hidden name=dns_ip>");
			fprintf(cgiOut,"<input type=hidden name=routeip>");
			fprintf(cgiOut,"<input type=hidden name=winip>");
			fprintf(cgiOut,"<input type=hidden name=ip_mask>");
			fprintf(cgiOut,"<input type=hidden name=startip>");
			fprintf(cgiOut,"<input type=hidden name=endip>");
			fprintf(cgiOut,"<input type=hidden name=opt_ip>");
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
	char lease[32] = {0};
	char day[10] = {0};
	char hour[10] = {0};
	char min[10] = {0};
	char dns_ip[128] = {0};
	char ip_mask[32] = {0};
	char startip[32] = {0};
	char endip[32] = {0};
	char domname[32] = {0};
	char routeip[32] = {0};
	char winip[32] = {0};
	char opt_ip[512] = {0};	
	char showtype[10] = {0};
	char optype[512] = {0};
	
	
	int ret1=-1,mode = 1,ret2 = 0;
	int retconfig = 0;
	unsigned int index = 0;
	unsigned int bkindex = 0;
	char allslotid[10] = {0};
	int allslot_id = 0;
	cgiFormStringNoNewlines("allslot",allslotid,sizeof(allslotid));
	allslot_id = atoi(allslotid);

	cgiFormStringNoNewlines("poolname",poolname,32);
	cgiFormStringNoNewlines("domname",domname,32);
	cgiFormStringNoNewlines("day",day,10);
	cgiFormStringNoNewlines("hour",hour,10);
	cgiFormStringNoNewlines("min",min,10);
	cgiFormStringNoNewlines("dns_ip",dns_ip,128);
	cgiFormStringNoNewlines("ip_mask",ip_mask,32);
	cgiFormStringNoNewlines("startip",startip,32);
	cgiFormStringNoNewlines("endip",endip,32);	
	cgiFormStringNoNewlines("opt_ip", opt_ip, 512);	
	cgiFormStringNoNewlines("routeip",routeip,32);
	cgiFormStringNoNewlines("winip",winip,32);
	cgiFormStringNoNewlines("showtype",showtype,10);
	cgiFormStringNoNewlines("optype",optype,512);

	int lease_t = 0;
	int lease_d = atoi(day);
	int lease_h = atoi(hour);
	int lease_m = atoi(min);
	//kehao add
	int i = 0;
	int flag = 0;
	//////////////////////

	///////////////
	lease_t = lease_d*24*3600+lease_h*3600+lease_m*60;
	sprintf(lease,"%d",lease_t);		 

	/*ip pool name*/
	if (0 != strcmp(poolname,""))
	{

        //kehao add for poolname
        for(i = 0; i < strlen(poolname); i++)
        {

           if( (poolname[i] >= 'a' && poolname[i] <= 'z') || (poolname[i] >= 'A' && poolname[i] <= 'Z')
		   	    || ( poolname[i] >= '0' && poolname[i] <= '9') 
		   	    || ( poolname[i] == '-') ||  (poolname[i] == '_') || (poolname[i] == '.') || (poolname[i] == '@') )

           {

			 flag = 0;

           }

		   else
		   {
		   	 flag = 1;
			 strcpy(poolname,"");
			 ShowAlert(search(lcontrol,"invalid_char"));
			 
		   }


        }
	  }

        //kehao  add  20110518
		if(flag == 0)
		{
	    //////////////
		ret1=create_ip_pool_name(poolname,&index,allslot_id);
		retconfig = config_ip_pool_name(poolname,&bkindex,allslot_id);
		
		if ((1 == ret1)||(1 == retconfig))
		{
			/*add ip pool range*/	
			if ((0 != strcmp(startip,"")) && (0 != strcmp(endip,"")) && (0 != strcmp(ip_mask,"")))
			{
				if(1 == retconfig)
				{
					index = bkindex;
				}
				ret2 = add_dhcp_pool_ip_range("add", startip,endip, ip_mask,index,allslot_id);
				
				if (1 == ret2)
				{
					/*add domain name*/
					if(strcmp(domname,"")!=0)
					{
						ip_dhcp_server_domain_name(domname,mode,index,allslot_id);
					}
					/*add dns*/	
					if(strcmp(dns_ip,"")!=0)
					{
						ip_dhcp_server_dns(mode, index, dns_ip,allslot_id);
					}
					/*add lease*/
					ip_dhcp_server_lease_default(lease, mode,index,allslot_id);
					/*option 43*/

					if (strcmp(showtype,"1") == 0)
					{
						if(strcmp(optype,"")!=0)
						{
							ip_dhcp_server_option43(optype, mode, index,allslot_id);
						}
					}
					else
					{
						if(strcmp(opt_ip,"")!=0)
						{
							char *pl = NULL;
							pl = strtok(opt_ip," ");
							while (NULL != pl)
							{
								ccgi_set_server_option43_veo(pl, mode, index, 0,allslot_id);
								pl = strtok(NULL," ");
							}
						}
					}
					/*add route ip*/
					if(strcmp(routeip,"")!=0)
					{
						ip_dhcp_server_routers_ip(routeip, index,mode,allslot_id);
					}
					
					/*add win ip*/
					if(strcmp(winip,"")!=0)
					{
						ip_dhcp_server_wins_ip(winip,mode, index,allslot_id);
					}

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
