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
* wp_dhcp_sub_para.c
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
#include <sys/types.h>
#include <unistd.h>
#include "ws_ec.h"

#define MSG_LEN 256
#define INTF_LEN 64


static char* LANGUAGE_PATH = "../htdocs/text/public.txt";


int ShowDhcp_para_Page(struct list *lcontrol,struct list *lpublic); 
void dhcp_para_config(struct list *lcontrol,struct list *lpublic,char *na,char *nb,char *nm); 

int cgiMain()
{


 	struct list *lcontrol;
	 struct list *lpublic;
	 lcontrol = get_chain_head("../htdocs/text/control.txt");
	 lpublic= get_chain_head("../htdocs/text/public.txt");
	 ShowDhcp_para_Page(lcontrol,lpublic);
	 release(lcontrol);
	 release(lpublic); 
	 return 0;
}
void strallcut(char *str)
{
      int  i,j=0;
      char sp[512];
      for (i = 0; *(str + i) != '\0'; i++) {
           if (*(str + i) == ' ' )
                   continue;
              sp[j++]=*(str + i);
     }
    sp[j] = 0;
    strcpy(str, sp);
}
int ShowDhcp_para_Page(struct list *lcontrol,struct list *lpublic)
{ 
  FILE *fp;
  char lan[3];
  char *str_encry =(char *)malloc(BUF_LEN);				
  char *str_uid = NULL; 
  memset(str_encry,0,BUF_LEN);
  char * select1=(char*)malloc(512);
  char * select2=(char*)malloc(512);
  char ip_addr_gate[16];
  char ip_addr_dns[16];
  char buf[256];
   char str_para_buf[MSG_LEN];
   char str_intf_name[INTF_LEN];
    char str_intf_addr[INTF_LEN];
    char str_mask[INTF_LEN]; 
	char str_mask1[INTF_LEN];
	memset(str_para_buf, 0, MSG_LEN);
   memset(ip_addr_gate, 0, 16);
   memset(ip_addr_dns, 0, 16);
  memset(select1,0,512);
  memset(select2,0,512);
  

 
	
  if(cgiFormSubmitClicked("dhcp_sub_para") != cgiFormSuccess)
  {
    memset(str_encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", str_encry, BUF_LEN);
	cgiFormStringNoNewlines("Na", str_intf_name, INTF_LEN);
   cgiFormStringNoNewlines("Nb", str_intf_addr, INTF_LEN);
   cgiFormStringNoNewlines("Nm", str_mask, INTF_LEN);
    cgiFormStringNoNewlines("Nm1", str_mask1, INTF_LEN);
   cgiFormStringNoNewlines("pool", str_para_buf, MSG_LEN);
  }
  else
  {
      cgiFormStringNoNewlines("encry_addusr",str_encry,BUF_LEN);
	   cgiFormStringNoNewlines("na", str_intf_name, INTF_LEN);
   cgiFormStringNoNewlines("nb", str_intf_addr, INTF_LEN);
   cgiFormStringNoNewlines("nm", str_mask, INTF_LEN);
   cgiFormStringNoNewlines("nm1", str_mask1, INTF_LEN);
   cgiFormStringNoNewlines("pool", str_para_buf, MSG_LEN);
 
  }
  if(!strcmp(str_para_buf,""))
  	{
  		strcpy(str_para_buf,"0.0.0.0 0.0.0.0");
		fprintf(stderr,"%s",str_para_buf);
  	}
		str_uid = dcryption(str_encry);
    if(NULL == str_uid)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	     
      return 0;
	}
  //fprintf(stderr,"sssssssssssssssssspool=%s",str_para_buf);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>DHCP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  	
	  "<script type=\"text/javascript\">"\
		  "function getValue(ip1,ip2,ip3,ip4,obj2){"\
		  "if(ip1.value==\"\" || ip2.value==\"\" || ip3.value==\"\" || ip4.value==\"\"){"\
		  "alert(\"%s\");"\
		  "}else{"\
		  "var value1 = ip1.value;"\
		  "var value2 = ip2.value;"\
		  "var value3 = ip3.value;"\
		  "var value4 = ip4.value;"\
		  "var value = value1 + \".\" + value2 + \".\" + value3 + \".\" + value4;"\
		  "obj2.options.add("\
			  "new Option(value,value,false,false)"\
			  ");"\
		  "}"\
		  "ip1.value=\"\";"\
		   "ip2.value=\"\";"\
		   "ip3.value=\"\";"\
		   "ip4.value=\"\";"\
		  "}",search(lcontrol,"ip_null"));
		  fprintf(cgiOut,"function removeValue(obj){"\
		  "var selectObj = obj;"\
		  "var optionIndex = null;"\
		  "for(var i=0; i<selectObj.options.length; i++){"\
		  "if(selectObj.options[i].selected==true){"\
		  "optionIndex = i;"\
		  "}"\
		  "}"\
		  "if(optionIndex!=null){"\
		  "selectObj.remove(optionIndex);"\
		  "}"\
	  "}"\
	  "function mysubmit()"\
		"{"\
		 "document.all.ip_s1.value=\"\";"\
		"for(var i=0; i<document.all.select1.options.length; i++){"\
		   "var value = document.all.select1.options[i].value;"\
		    "if(i==0){"\
		    	"document.all.ip_s1.value=value;}"\
		    "else{"\
		    	"document.all.ip_s1.value = document.all.ip_s1.value + \",\" + value;}"\
  			"}"\
       "document.all.ip_s2.value=\"\";"\
		"for(var i=0; i<document.all.select2.options.length; i++){"\
		   "var value = document.all.select2.options[i].value;"\
		   "if(i==0){"\
		   		"document.all.ip_s2.value=value;}"\
		   	"else{"\
		   		"document.all.ip_s2.value = document.all.ip_s2.value + \",\" + value;}"\
		   		"}"\
		   	"var v1=document.all.lease.value;"\
		   	"if(v1<2||v1>590000)"\
		   	     "{"\
		   	     "alert(\"%s\");"\
		   	      "window.event.returnValue = false;"\
		   		 "}"\
		   		"}",search(lcontrol,"lease error"));
		  fprintf(cgiOut,"function myKeyDown()"\
		"{"\
		" var   k=window.event.keyCode;"\
		"if ((k==46)||(k==8)||(k==189)||(k==109) || (k>=48 && k<=57)||(k>=96 && k<=105)||(k>=37 && k<=40))"\
   		        " {}"\
    	" else if(k==13){"\
       		" window.event.keyCode = 9;}"\
   		" else{"\
      	 	 "window.event.returnValue = false;}"\
		"}"\
  	"</script>"\
  	"<script language=javascript src=/ip.js>"\
  	"</script>"\
  "</head>"\
  "<body>"); 
  

  if(cgiFormSubmitClicked("dhcp_sub_para") == cgiFormSuccess)
  {
      dhcp_para_config(lcontrol,lpublic,str_intf_name,str_intf_addr,str_mask1);
  }
  fprintf(stderr,"sdfasdfasdfas");
  fprintf(cgiOut,"<form method=post encType=multipart/form-data  onsubmit=mysubmit()>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcontrol, "dhcp_sub_para"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

  if((fp=fopen(LANGUAGE_PATH,"r"))==NULL)	  
  {
      ShowAlert(search(lcontrol,"error_open"));
  }
  else
  {
	  fseek(fp,4,0);					  
	  fgets(lan,3,fp);
	  fclose(fp);
  }
 
  if(strcmp(lan,"ch")==0)
    	{	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
		  if(checkuser_group(str_uid)==0)
		  	{
          	fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_sub_para style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  	}
		  else
		  	{
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",str_encry,search(lpublic,"img_ok"));
		  	}
		  if(cgiFormSubmitClicked("dhcp_sub_para") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",str_encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",str_encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
		}		
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>");
		  if(checkuser_group(str_uid)==0)
		  	{
		  	fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_sub_para style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		  }
		  else{
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/ok-en.jpg border=0 width=62 height=20/></a></td>",str_encry);
		  	}
		  if(cgiFormSubmitClicked("dhcp_sub_para") != cgiFormSuccess)
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",str_encry);
		  else
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",str_encry);
		  fprintf(cgiOut,"</tr>"\
		  "</table>");
		}
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
 					"<td align=left id=tdleft>"\
 					    "<a href=wp_dhcp_addpool.cgi?UN=%s&Na=%s&Nb=%s&Nm=%s"\
 					      " target=mainFrame"\
 					      " class=top"\
 					      "><font id=yingwen_san>DHCP</font><font id=%s>%s</font></a>"\
 					"</td>"\
  			     "</tr>",str_encry,str_intf_name,str_intf_addr,str_mask,search(lpublic,"menu_san"),search(lcontrol,"dhcp_pool"));

  fprintf(cgiOut,"<tr height=26>"\
				     "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol, "dhcp_sub_para"));    
  fprintf(cgiOut,"</tr>");

   int i=0;
   for(;i<12;i++)
   {
     fprintf(cgiOut,"<tr height=25>"\
   	"<td id=tdleft>&nbsp;</td>"\
     "</tr>");
   }
   fprintf(cgiOut, "</table></td>");

   
 	
   
    
   //display error message or error page
   
   //get dhcp relay service message from system  
    
   char * tmp = (char*)malloc(128);
   char * lease = (char*)malloc(15);
   char * gateways = (char *)malloc(128);
   char * dnserver = (char*)malloc(128);
   char * dnsname = (char *)malloc(30);
   memset(tmp,0,128);
   memset(lease,0,15);
   memset(gateways,0,128);
   memset(dnserver,0,128);
   memset(dnsname,0,30);
   int sta = system("dhcp_read.sh");
   int t = 0;
   if(sta != 0)
   	  fprintf(stderr,"exec dhcp_read.sh error!");
   FILE* re = fopen("/var/run/apache2/dhcp.sub.r","r");
   if(re == NULL)
   	{
	  fprintf(stderr,"can not open dhcp.sub.r");
   	}else{
    	while(fgets(buf,256,re))
		{
           t = 0;
		tmp = strtok(buf,"/");
		  while(tmp != NULL)
		  	{
			  t++;
			  if(t ==1)
			  	{
				strcpy(gateways,tmp);
			  	}
			  else if(t == 2)
			  	{
				 strcpy(dnserver,tmp);
			  	}
			  else if(t == 3)
			  	{
					strcpy(dnsname,tmp);
			    }
			  else if(t == 4)
			  	   strcpy(lease,tmp);
			tmp = strtok(NULL,"/");  
		  	}
	
		}
  
    strallcut(lease);
	strallcut(dnsname);
	strallcut(gateways);
	strallcut(dnserver);
	
    int lease_t = atoi(lease);
	 lease_t = lease_t/3600;
	// fprintf(cgiOut,"lease_t===%d",lease_t);
	 sprintf(lease,"%d",lease_t);
	// fprintf(cgiOut,"lease=====%s",lease);
	// strallcut(lease);
	 //fprintf(cgiOut,"%s",lease);
	 if(lease_t<2)
	 	{
	 		memset(lease,0,15);
	 		strcpy(lease,"");
	 	}
	// fprintf(cgiOut,"aaa=%s",lease);
   if(!strcmp(dnsname,"_"))
   	   strcpy(dnsname,"");
   	}
 //display dhcp relay page 
 fprintf(cgiOut, "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
	  	           "<table width=450 height=350 border=0 cellspacing=0 cellpadding=10>"\
	  		         "<tr height=35>");
   fprintf(cgiOut,"<td width=125 >%s:</td>",search(lcontrol,"lease duration"));
             fprintf(cgiOut,"<td ><input name=lease type=text style=ime-mode:disabled  onkeydown=myKeyDown() value=\"%s\" size=10/><font color=red>(%s)</font></td>",lease,search(lcontrol,"hour"));
				fprintf(cgiOut,"</tr>");	            
		   fprintf(cgiOut,"<tr height=35><td width=120 >%s:</td>",search(lcontrol,"default gateway"));
           fprintf(cgiOut,"<td width=150>"\
	  			         "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
	  				       "<input type=text  name=gate_ip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",ip_addr_gate,search(lpublic,"ip_error")); 
	  	    fprintf(cgiOut,"<input type=text  name=gate_ip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.", ip_addr_gate+4,search(lpublic,"ip_error"));
 		    fprintf(cgiOut,"<input type=text  name=gate_ip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.", ip_addr_gate+8,search(lpublic,"ip_error"));
		    fprintf(cgiOut,"<input type=text  name=gate_ip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>", ip_addr_gate+12,search(lpublic,"ip_error"));
	  	    fprintf(cgiOut,"</div>"\
						"</td>");
			if(checkuser_group(str_uid)==0)
				{
				fprintf(cgiOut,"<td ><input type=button name=Submit value= %s style=width:50px;height:20px onclick=getValue(gate_ip1,gate_ip2,gate_ip3,gate_ip4,select1); /> </td>",search(lcontrol,"dhcp_add"));
				}
			else{
			     fprintf(cgiOut,"<td ><input type=button name=Submit value= %s style=width:50px;height:20px disabled onclick=getValue(gate_ip1,gate_ip2,gate_ip3,gate_ip4,select1); /> </td>",search(lcontrol,"dhcp_add"));
				}
			fprintf(cgiOut,"</tr>"\
   					"<tr height=90>"\
   						"<td>&nbsp;</td>"
   						"<td ><select name=select1 style=width:140px size=5>");
					  if(strstr(gateways,"_")== NULL)
					  	{
					    	tmp = strtok(gateways,",");
		                  while(tmp != NULL)
						 {
							fprintf(cgiOut,"<option value=%s>%s</option>",tmp,tmp);
							tmp = strtok(NULL,",");
					    	} 
					  	}
				fprintf(cgiOut,"</select></td>");
				if(checkuser_group(str_uid)==0)
					{
                    fprintf(cgiOut,"<td valign=left><input type=button name=Submit2 value=%s style=width:50px;height:20px onclick=removeValue(select1); ></td>",search(lcontrol,"dhcp_rm"));
					}
				else{
					fprintf(cgiOut,"<td valign=left><input type=button name=Submit2 disabled value=%s style=width:50px;height:20px onclick=removeValue(select1); ></td>",search(lcontrol,"dhcp_rm"));
					}
				 fprintf(cgiOut,"</tr>"\
   					"<tr height=35>"\
   						"<td >%s:</td>",search(lcontrol,"dnserver"));
   						
     					 fprintf(cgiOut,"<td width=150>"\
	  			           "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">"\
	  				       "<input type=text  name=dns_ip1 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",ip_addr_dns,search(lpublic,"ip_error")); 
	  	    fprintf(cgiOut,"<input type=text  name=dns_ip2 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.", ip_addr_dns+4,search(lpublic,"ip_error"));
 		    fprintf(cgiOut,"<input type=text  name=dns_ip3 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.", ip_addr_dns+8,search(lpublic,"ip_error"));
		    fprintf(cgiOut,"<input type=text  name=dns_ip4 value=\"%s\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>", ip_addr_dns+12,search(lpublic,"ip_error"));
	  	    fprintf(cgiOut,"</div>"\
						"</td>");
				if(checkuser_group(str_uid)==0)
					{
						fprintf(cgiOut,"<td ><input type=button  name=Submit2 value= %s style=width:50px height:36px style=background-image:url(/images/SubBackGif.gif) onclick=getValue(dns_ip1,dns_ip2,dns_ip3,dns_ip4,select2); > </td>",search(lcontrol,"dhcp_add"));
					}
				else{
						fprintf(cgiOut,"<td ><input type=button  name=Submit2 value= %s style=width:50px height:36px style=background-image:url(/images/SubBackGif.gif) disabled onclick=getValue(dns_ip1,dns_ip2,dns_ip3,dns_ip4,select2); > </td>",search(lcontrol,"dhcp_add"));
					}
					fprintf(cgiOut,"</tr>"\
					"<tr height=90>"
						"<td>&nbsp;</td>"
						"<td><select name=select2 style=width:140px size=5>");
		 				 if(strstr(dnserver,"_") ==NULL)
		 				 	{
			            	tmp = strtok(dnserver,",");
							while(tmp != NULL)
							{
								fprintf(cgiOut,"<option value=%s>%s</option>",tmp,tmp);
								tmp = strtok(NULL,",");
							}
		 				 	}
						 if(checkuser_group(str_uid)==0)
						 	{
   							fprintf(cgiOut,"<td valign=top ><input  type=button name=Submit2 value=%s style=width:50px height:36px style=background-image:url(/images/SubBackGif.gif) onclick=removeValue(select2); /></td>",search(lcontrol,"dhcp_rm"));
						 	}
						 else{
							fprintf(cgiOut,"<td valign=top ><input  type=button name=Submit2 disabled style=width:50px height:36px style=background-image:url(/images/SubBackGif.gif) value=%s onclick=removeValue(select2); /></td>",search(lcontrol,"dhcp_rm"));
						 	}
					fprintf(cgiOut,"</td>"\
		   			"</tr>"\
		   			"<tr height=35><td>%s:</td>",search(lcontrol,"dns_name"));
      				fprintf(cgiOut,"<td><input name=dns type=text value=\"%s\" size=21/></td>",dnsname);
      			fprintf(cgiOut,	"</tr>"\
      				 			  "<td ><input type=hidden name=encry_addusr value=%s></td>",str_encry);	
         		fprintf(cgiOut,"<td><input type=hidden name=ip_s1 value=\"%s\"></td>",select1);	
         		
         		fprintf(cgiOut,"<td><input type=hidden name=ip_s2 value=\"%s\"></td>",select2);	
         		fprintf(cgiOut,"<td><input type=hidden name=na value=%s></td>",str_intf_name);
         		fprintf(cgiOut,"<td><input type=hidden name=nb value=%s></td>",str_intf_addr);
         		fprintf(cgiOut,"<td><input type=hidden name=nm value=%s></td>",str_mask);
         		fprintf(cgiOut,"<td><input type=hidden name=nm1 value=%s></td>",str_mask1);
         		fprintf(cgiOut,"<td><input type=hidden name=pool value=\"%s\"></td>",str_para_buf);
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

free(tmp);
free(lease); 
free(gateways); 
free(dnserver); 
free(dnsname);
free(select1);
free(select2);
free(str_encry);
if(re != NULL)
{
	fclose(re);
}
return 0;
}

void dhcp_para_config(struct list *lcontrol,struct list *lpublic,char*na,char*nb,char *nm)
{ 
// fprintf(cgiOut,"aa");
	char *select = (char*)malloc(256);
	char * select2=(char *)malloc(256);
	char * tmp =(char*)malloc(256);
	char * lease = (char *)malloc(32);
	char * dns= (char*)malloc(32);
	char * cmd = (char*)malloc(512);
	char str_para_buf[MSG_LEN];
   
   memset(tmp,0,256);
	memset(select,0,256);
	memset(select2,0,256);
	memset(lease,0,32);
	memset(dns,0,32);
		memset(cmd,0,512);
	cgiFormStringNoNewlines("ip_s1",select,256);
	cgiFormStringNoNewlines("ip_s2",select2,256);
	cgiFormStringNoNewlines("lease",lease,10);
	cgiFormStringNoNewlines("dns",dns,20);
 	cgiFormStringNoNewlines("pool",str_para_buf,MSG_LEN);
	//fprintf(stderr,"1111na=%s",na);
	//fprintf(stderr,"%s",nb);
	//fprintf(stderr,"%s",nm);
  // fprintf(cgiOut,"dddddddddddddddddddddd=%s",str_para_buf);	
	if(!strcmp(lease,""))
		{
		 strcpy(lease,"2"); 			
		}
	int lease_t = atoi(lease);
	//    fprintf(cgiOut,"%d",lease_t);
	lease_t = lease_t*60*60;
	//fprintf(cgiOut,"%d",lease_t);
	sprintf(lease,"%d",lease_t);
	//fprintf(cgiOut,"%s",lease);
	strcat(cmd,"dhcp_sub_write.sh ");
	strcat(cmd,na);
	strcat(cmd," ");
	strcat(cmd,nb);
	strcat(cmd," ");
	strcat(cmd,nm);
	strcat(cmd," \"");
	strcat(cmd,str_para_buf);
	strcat(cmd,"\" ");
	strcpy(tmp,select);
	strallcut(tmp);
	 if(!strcmp(tmp,""))
		{
		  strcpy(select," \"# routers _");
		  strcat(cmd,select);
	 	}
	 else{
		  strcat(cmd,"\"option routers ");
		  strcat(cmd,select);
	 }
	strcat(cmd," ^");
	memset(tmp,0,256);
	strcpy(tmp,select2);
    strallcut(tmp);
	 if(!strcmp(tmp,""))
		  {
		  strcpy(select2,"# domain-name-servers _");
		  strcat(cmd,select2);
	 	}
	 else{
		  strcat(cmd,"option domain-name-servers ");
		  strcat(cmd,select2);
	 	}
	 strcat(cmd," ^");
	 if(!strcmp(dns,""))
	 	{
		   strcpy(dns,"# domain-name \\\" _ \\\" ");
		   strcat(cmd,dns);
	 	}
	 else{
		  strcat(cmd,"option domain-name \\\" ");
		  strcat(cmd,dns);
		  strcat(cmd," \\\" ");
	 	}
	strcat(cmd," ^");
	strcat(cmd,"default-lease-time ");
	strcat(cmd,lease);
	strcat(cmd," ^");
	strcat(cmd,"max-lease-time ");
	strcat(cmd,lease);
	strcat(cmd," \"");
    //fprintf(cgiOut,"%s",cmd);
   
		int rec = system(cmd);
		if(rec == 0)
		{
		  ShowAlert(search(lcontrol,"save"));
		}
	    else
		   ShowAlert(search(lcontrol,"no_save"));
	//fprintf(cgiOut,"%d",rec);
     
	       memset(cmd,0,512);
		   strcpy(cmd,"dhcp_sub_read.sh ");
		   strcat(cmd,na);
		   int substa = system(cmd);
		   if(substa != 0 )
				fprintf(stderr,"%s","exec dhcp_sub_read.sh failed!");

	free(cmd);
	free(select);
   free(tmp);
	free(select2);
	free(lease);
	free(dns);
}

