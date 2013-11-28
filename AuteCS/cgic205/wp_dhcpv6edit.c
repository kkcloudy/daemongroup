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
* wp_dhcpedit.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system cotrl for dhcp edit
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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ws_dhcpv6.h"
#include "ws_dcli_dhcp.h"

int ShowDhcpEditPage(char *encry,char *eid,struct list *lcontrol,struct list *lpublic);
void ConfIPAdd(struct list *lcontrol,struct list *lpublic,char *poolname);
int ShowConfClearPage(char *m,char *id,struct list *lcontrol,struct list *lpublic);

int cgiMain()
{
	char encry[BUF_LEN] = {0};  
	char *str;   

	struct list *lpublic;
    struct list *lcontrol;
    lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt");

    ccgi_dbus_init();
	///////////////
	char eid[30];
	memset(eid,0,30);

	char etype[10];
	memset(etype,0,10);
	cgiFormStringNoNewlines("NAME",eid,30);
	cgiFormStringNoNewlines("TYPE",etype,10);
	cgiFormStringNoNewlines("UN",encry,BUF_LEN);
	
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user"));
	}
	else 
	{
		if(strcmp(etype,"2")==0)
		{
		   ShowConfClearPage(encry,eid,lcontrol,lpublic);
		}
		else
		{
		   ShowDhcpEditPage(encry,eid,lcontrol,lpublic);
		}
	}  
	release(lpublic);  
    release(lcontrol);

	return 0;
}


int ShowDhcpEditPage(char *encry,char *eid,struct list *lcontrol,struct list *lpublic)
{ 

	int i = 0;
	char startip[128] = {0};
	char endip[128] = {0};
	char domname[128] = {0};
	unsigned int mode = 0, index = 0, count = 0;	
	int cflag=-1;
	struct dhcp6_pool_show_st head,*q;
	memset(&head,0,sizeof(struct dhcp6_pool_show_st));
	struct dhcp6_sub *sq;
	int j = 0;
    int prefix_num = 0;
	int lease_t = 0;
	int day = 0;
	int hour = 0;
	int min = 0;
	int opt_num = 0;
	int dns_num = 0;

	char *optarray[8];
	char *dnsarray[3];
	for (i = 0;i < 8;i++)
	{
		optarray[i] = (char *)malloc(128);
		memset(optarray[i],0,128);

	}
	for (i = 0; i < 3; i++)
	{
		dnsarray[i] = (char *)malloc(128);
		memset(dnsarray[i],0,128);
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
			"}","dns ip is beyond",search(lpublic,"ip_not_null"));
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
			"}","option52 ip is beyond",search(lpublic,"dhcp_exist"),search(lpublic,"ip_not_null"));
	   fprintf(cgiOut, "function mysubmit()\n"\
					 "{\n"\
					    "var j = 0;"\
		   		 		"var tt = \"\";"\
		   		 		"tt = \"\";"\
						"for(j=0; j<document.all.dnselect.options.length; j++)\n"\
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
					   "tt = \"\";"\
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
		   				"if(v4 == \"\")\n"\
		   	    		"{\n"\
	   	     				"alert(\"%s\");\n"\
	   	      				"return false;\n"\
		   		 		"}\n"\
		   		 		"var leaseall = v1*86400 + v2*3600 + v3*60;"\
		   		 		"if(leaseall>31536000)"\
		   		 		"{"\
		   		 		"alert(\"%s\");"\
		   		 		"return false;"\
		   		 		"}"\
						"var startipz = get_ip_addr(\"beg_ip\");\n"\
						"var endipz = get_ip_addr(\"end_ip\");\n"\
						"if((startipz == null)&&(endipz == null))\n"\
						"{\n"\
							"alert(\"%s\");\n"\
							"return false;\n"\
						"}\n"\
		   		"}",search(lcontrol,"lease_d_error"),search(lcontrol,"lease_h_error"),search(lcontrol,"lease_m_error"),search(lcontrol,"lease_empty"),search(lcontrol,"lease_max"),search(lcontrol,"dhcp_ranges_empty"));
	fprintf(cgiOut,"</script>\n"\
	"<script language=javascript src=/ipv6.js>\n"\
	"</script>\n"\
	"</head>\n"\
	"<body>");  
	if(cgiFormSubmitClicked("dhcp_add") == cgiFormSuccess)
	{	  	    
		ConfIPAdd(lcontrol,lpublic,eid);		
	}
	  /////////////////dcli_dhcp
	 //cflag=ccgi_show_ip_pool(mode, index, &head, &count);
	cflag = 0;//ccgi_show_ipv6_pool(mode,index, &head, &count);
	if(cflag==1)
	{
		q = head.next;
		while( q != NULL )
		{
		  sq = q->ipv6_subnet.next;
		  while( sq != NULL )
		  {
				if(strcmp(eid,q->poolname)!=0)
				{
					break;
				}
				else
				{
					strncpy(domname,q->domain_name,sizeof(domname));
					opt_num = q->option_adr_num;
					dns_num = q->dnsnum;
					for(j = 0; j < q->option_adr_num; j++)
					{

						strncpy(optarray[j],q->option52[j],128);
					}
					for (j = 0; j < q->dnsnum; j++) 
					{				
						strncpy(dnsarray[j],q->dnsip[j],128);
					}
					lease_t = (q->defaulttime? q->defaulttime : 86400);
					day = lease_t/86400;
					hour = lease_t%86400;
					hour = hour/3600;
					min = lease_t%3600;
					min = min/60;

					strncpy(startip,sq->range_low_ip,sizeof(startip));
					strncpy(endip,sq->range_high_ip,sizeof(endip));
					prefix_num = sq->prefix_length;					
				}
		     sq = sq->next;
		  }					
		  q = q->next;
		}
	}
	////lease  
	fprintf(cgiOut,"<form method=post onsubmit=\"return mysubmit()\">"\
	"<div align=center>"\
	"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
	"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCP IPV6");
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>");
	fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_add style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	fprintf(cgiOut,"<td width=62 align=center><a href=wp_dhcpv6con.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"edit"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");	
         		}
         		else if(cgiFormSubmitClicked("dhcp_add") == cgiFormSuccess) 			  
         		{					   
					fprintf(cgiOut,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"edit"));   /*突出显示*/
					fprintf(cgiOut,"</tr>");
         		}
				for(i=0;i<20;i++)
				{
					fprintf(cgiOut,"<tr height=25>"\
					  "<td id=tdleft>&nbsp;</td>"\
					"</tr>");
				}
				fprintf(cgiOut,"</table>"\
				"</td>"\
				"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
				fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>");

				/////////////////////////////////////////////////
				fprintf( cgiOut, "<tr height=10><td colspan=5></td></tr>");
				fprintf( cgiOut,"<tr><td colspan=5>DHCP %s</td></tr>\n",search(lpublic,"dhcp_pconfig"));
				fprintf( cgiOut, "<tr><td colspan=5><hr width=100%% size=1 color=#fff align=center noshade /></td></tr>");

				////////////////pool name
	            fprintf(cgiOut,"<tr>\n");
				fprintf(cgiOut,"<td width=100 height=40 >%s:</td>","POOL");
				fprintf(cgiOut,"<td>%s</td></tr>\n",eid);

				//domain name
	            fprintf(cgiOut,"<tr>\n");
				fprintf(cgiOut,"<td width=100 height=40 >%s:</td>","Domain");
				fprintf(cgiOut,"<td><input type=text name=domname value=\"%s\" size=21 maxLength=20></td></tr>\n",domname);

				fprintf(cgiOut,"<tr>"\
				 					"<td width=100 height=40 >%s:</td>",search(lcontrol,"lease duration"));
				fprintf(cgiOut,"<td colspan=4 width=350 align=left><input type=text name=day onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
									onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
									ondragenter=\"return  false;\"\
									style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" value=\"%d\" size=5/><font color=red>(%s)</font>",day,search(lcontrol,"day"));
				fprintf(cgiOut,"<input type=text name=hour onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
									onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
									ondragenter=\"return  false;\"\
									style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" value=\"%d\" size=5/><font color=red>(%s)</font>",hour,search(lcontrol,"hour"));
				fprintf(cgiOut,"<input type=text name=min onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"\
									onpaste=\"var s=clipboardData.getData('text');   if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\"\
									ondragenter=\"return  false;\"\
									style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" value=\"%d\" size=5/><font color=red>(%s)</font></td>",min,search(lcontrol,"time_min"));
	 			fprintf(cgiOut,"</tr>");				
				
	 			fprintf(cgiOut,"<tr>"\
 					"<td width=100 height=40>%s:</td>",search(lcontrol,"dnserver"));
	 			fprintf(cgiOut,"<td width=290>"\
	 						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:290;font-size:9pt\">");
		  		fprintf(cgiOut,"<input type=text  name=dns_ip1 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text  name=dns_ip2 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text  name=dns_ip3 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
			    fprintf(cgiOut,"<input type=text  name=dns_ip4 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
		  		fprintf(cgiOut,"<input type=text  name=dns_ip5 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error")); 
		  	    fprintf(cgiOut,"<input type=text  name=dns_ip6 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
	 		    fprintf(cgiOut,"<input type=text  name=dns_ip7 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>:",search(lpublic,"ip_error"));
			    fprintf(cgiOut,"<input type=text  name=dns_ip8 value=\"\" maxlength=4 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
				fprintf(cgiOut,"</div></td>"\
					"<td>&nbsp;</td>"\
					"<td colspan=2 width=50 ><input type=button name=dns_add id=ranges_add onclick=add_dns() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_add"));
	  			fprintf(cgiOut,"</tr>");
				
				fprintf(cgiOut,"<tr height=60>"\
							"<td>&nbsp;</td>"\
							"<td width=140 colspan=2 ><select name=dnselect  valus=\"\" size=\"3\" style=\"width:290px\">");
				for (i = 0;i < dns_num;i++)
				{
					fprintf(cgiOut,"<option value=\"%s\">%s</option>",dnsarray[i],dnsarray[i]);	
				}
				fprintf(cgiOut,"</select></td>"\
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
					"<td width=140 colspan=2 ><select name=optselect  valus=\"\" size=\"8\" style=\"width:290px\">\n");
				///////////////	
				for (i = 0;i < opt_num;i++)
				{
					fprintf(cgiOut,"<option value=\"%s\">%s</option>",optarray[i],optarray[i]);	
				}
				//////////////
				fprintf(cgiOut,"</select></td>"\
					"<td width=50 colspan=2 valign=top><input type=button name=opt_rm id=ranges_rm onclick=rm_opt() style=\"width:50px\" value=\"%s\"></input></td>", search(lcontrol, "dhcp_rm"));
				fprintf(cgiOut,"</tr>");
				

				fprintf(cgiOut,"<tr>"\
	    				"<td width=100 height=40>%s:</td>","prefix length");
				fprintf(cgiOut,"<td colspan=4><input type=text name=prefixstr value=\"%d\"></td>",prefix_num);
				fprintf(cgiOut,"</tr>"\
					"<tr>"\
					"<td width=100 height=40>%s:</td>",search(lcontrol,"dhcp_pool"));
	 			fprintf(cgiOut,"<td width=140><input type=text name=startip value=\"%s\" style='width:280px'></td>",startip);
				fprintf(cgiOut,"<td align=center>-</td>"\
						"<td width=150 colspan=2><input type=text name=endip value=\"%s\" style='width:280px'></td>",endip);
				fprintf(cgiOut,"</tr>");	
				
				fprintf(cgiOut,"<input type=hidden name=UN value=%s>",encry);	
				fprintf(cgiOut,"<input type=hidden name=NAME value=%s>",eid);				
				fprintf(cgiOut,"<input type=hidden name=opt_ip>");
				fprintf(cgiOut,"<input type=hidden name=defaultsip value=%s>",startip);
				fprintf(cgiOut,"<input type=hidden name=defaulteip value=%s>",endip);
				fprintf(cgiOut,"<input type=hidden name=dns_ip>");				
				fprintf(cgiOut,"<input type=hidden name=defprefix value=\"%d\">",prefix_num);
				fprintf(cgiOut,"</table>");		
				
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
	if( cflag == 1 )
	{
		Free_ccgi_show_ipv6_pool(&head);
	}
	for (i = 0;i < 8;i++)
	{
		free(optarray[i]);

	}
	for (i = 0; i < 3; i++)
	{
		free(dnsarray[i]);
	}

	return 0;
}
						 
void  ConfIPAdd(struct list *lcontrol,struct list *lpublic,char *poolname)
{
	char lease[32] = {0};
	char day[10] = {0};
	char hour[10] = {0};
	char min[10] = {0};
	char dns_ip[128] = {0};
	char startip[128] = {0};
	char endip[128] = {0};
	char domname[128] = {0};
	char defsip[128] = {0};
	char defeip[128] = {0};
	char opt_ip[512] = {0};
	char defprefix[10] = {0};
	char prefixstr[10] = {0};
	int flag = 0;	


	int ret1=-1,mode = 1,ret2 = 0;
	unsigned int index = 0;
	//unsigned int isenable = 0;

	cgiFormStringNoNewlines("domname",domname,sizeof(domname));
	cgiFormStringNoNewlines("day",day,sizeof(day));
	cgiFormStringNoNewlines("hour",hour,sizeof(hour));
	cgiFormStringNoNewlines("min",min,sizeof(min));
	cgiFormStringNoNewlines("dns_ip",dns_ip,sizeof(dns_ip));
	cgiFormStringNoNewlines("startip",startip,sizeof(startip));
	cgiFormStringNoNewlines("endip",endip,sizeof(endip));	
	cgiFormStringNoNewlines("defaultsip",defsip,sizeof(defsip));
	cgiFormStringNoNewlines("defaulteip",defeip,sizeof(defeip));	
	cgiFormStringNoNewlines("opt_ip", opt_ip, sizeof(opt_ip));	
	cgiFormStringNoNewlines("defprefix", defprefix, sizeof(defprefix));
	cgiFormStringNoNewlines("prefixstr", prefixstr, sizeof(prefixstr));
	
	int lease_t = 0;
	int lease_d = atoi(day);
	int lease_h = atoi(hour);
	int lease_m = atoi(min);
	lease_t = lease_d*24*3600+lease_h*3600+lease_m*60;
	sprintf(lease,"%d",lease_t);	

	ccgi_dbus_init();
	/*ip pool name*/
	ret1 = 0;//ccgi_config_ipv6_pool_name(poolname, &index);
	if (1 == ret1)
	{
		/*modify ip pool range*/	
		if ((strcmp(defsip,startip) == 0) && (strcmp(defeip,endip) == 0))
		{
		}
		else
		{
			//ccgi_addordel_ipv6pool("delete", defsip, defeip, defprefix, index);
			//ccgi_addordel_ipv6pool("add", startip,endip, prefixstr, index);
			if(ret2==1)
			{				
				ShowAlert(search(lcontrol,"add_suc"));
			}
			else
			{
				flag = 1;
				ShowAlert(search(lpublic,"oper_fail"));
			}
		}
		/*modify domain name*/	
		int domret = 0;
		if (strcmp(domname,"") != 0)
		{
			//ccgi_set_server_domain_search_ipv6(domname,mode,index, ADD_OPT);		    
		}
		else
		{
			domret = 0;//ccgi_set_server_domain_search_ipv6(domname,mode,index,DEL_OPT);
		}
		//ccgi_set_server_lease_default_ipv6(lease_t,mode,index,ADD_OPT);

		if (0 != strcmp(dns_ip,""))
		{
			//ccgi_set_server_name_servers_ipv6(dns_ip,mode, index, ADD_OPT);
		}
		else
		{
			//ccgi_set_server_name_servers_ipv6(dns_ip,mode, index, DEL_OPT);
		}
		
		if(0 != strcmp(opt_ip,""))
		{
			//ccgi_set_server_option52_ipv6(opt_ip, mode,index,ADD_OPT);
		}
		else
		{
			//ccgi_set_server_option52_ipv6(opt_ip, mode,index,DEL_OPT);
		}
		if (0 == flag)
		{
			ShowAlert(search(lpublic,"oper_succ"));
		}
	}
}
int ShowConfClearPage(char *m,char *id,struct list *lcontrol,struct list *lpublic)
{

	cgiHeaderContentType("text/html");	
	fprintf( cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( cgiOut, "<head> \n" );
	fprintf( cgiOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );
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
	unsigned int index = 0;
	ret = 0;//ccgi_create_ipv6_pool_name(1,id,&index);

	if( ret == 1 )
	{
		ShowAlert(search(lpublic,"oper_succ"));
	}
	else
	{
		ShowAlert(search(lpublic,"oper_fail"));
	}

	fprintf( cgiOut, "<input type=hidden name=poolname value=%s>",m);
	fprintf( cgiOut, "<script type='text/javascript'>\n" );
	fprintf( cgiOut, "window.location.href='wp_dhcpv6con.cgi?UN=%s';\n", m);
	fprintf( cgiOut, "</script>\n");
	fprintf( cgiOut, "</body>\n");
	fprintf( cgiOut, "</html>\n");

	return 0;
}



