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
* wp_dhcp_opt.c
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

int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic);

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
  char *encry=(char *)malloc(BUF_LEN);
 
  char *str;
  char addn[N];        
  int i = 0;   

  char classname[30];
  memset(classname,0,30);

  char optname[30];
  memset(optname,0,30);

  char strip[30];
  memset(strip,0,30);

  char hexip[30];
  memset(hexip,0,30);

  char ip1[4];
  memset(ip1,0,4);
  
  char ip2[4];
  memset(ip2,0,4);
  
  char ip3[4];
  memset(ip3,0,4);
  
  char ip4[4];
  memset(ip4,0,4);

  int cl=1;

  struct optionsix o_head,*oq;
  int onum;
  int oflag=-1;
  
  
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
	      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	strcpy(addn,str);
 
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
 
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
    "#div1{ width:42px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:40px; height:15px; padding-left:5px; padding-top:3px}"\
    "#link{ text-decoration:none; font-size: 12px}"\
  	"</style>");
  	fprintf(cgiOut,"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  	"<script language=javascript src=/ip.js>"\
    "</script>"\
    "</head>"\
    "<body>");
  if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
  {
	 
	  memset(classname,0,30);
	  memset(optname,0,30);
	  memset(strip,0,30);
	  memset(hexip,0,30);
	  memset(ip1,0,4);
	  memset(ip2,0,4);
	  memset(ip3,0,4);
	  memset(ip4,0,4);

      cgiFormStringNoNewlines("optclass",classname,30);
	  cgiFormStringNoNewlines("iden",optname,30);
	  cgiFormStringNoNewlines("ip1",ip1,4);
	  cgiFormStringNoNewlines("ip2",ip2,4);
	  cgiFormStringNoNewlines("ip3",ip3,4);
	  cgiFormStringNoNewlines("ip4",ip4,4);     

	  int tip1,tip2,tip3,tip4;
	  tip1=strtoul(ip1,0,10);
	  tip2=strtoul(ip2,0,10);
	  tip3=strtoul(ip3,0,10);
	  tip4=strtoul(ip4,0,10);

	  if((strcmp(classname,"")!=0) &&(strcmp(optname,"")!=0) &&(strcmp(ip1,"")!=0) &&(strcmp(ip2,"")!=0) &&(strcmp(ip3,"")!=0) &&(strcmp(ip4,"")!=0))
	  {
	    sprintf(strip,"%d:%d:%d:%d",tip1,tip2,tip3,tip4);
		sprintf(hexip,"%X:%X:%X:%X",tip1,tip2,tip3,tip4);
	    add_dhcp_node_attr(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, classname);
		mod_dhcp_node(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, classname, DHCP_OPT_USRN, classname);
		mod_dhcp_node(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, classname, DHCP_OPT_IDEN, optname);
		mod_dhcp_node(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, classname, DHCP_OPT_VENDER, strip);
		mod_dhcp_node(DHCP_XML, DHCP_OPT_NAME, DHCP_NODE_ATTR, classname, DHCP_OPT_HEX, hexip);

		///////////write to conf
		int ret=-1;

		int bflag=-1,cflag=-1,oflag=-1,gflag=-1;

		struct bindipmac b_head;
		int bnum;

		struct optionsix o_head;
		int onum;

		struct confinfo c_head;
		int cnum;

		struct gbparam g_head;
		int gnum;

		bflag=read_bindinfo_xml(&b_head, &bnum);
		oflag=read_option_info(&o_head, &onum);
		cflag=read_confinfo_xml(&c_head, &cnum);
		gflag=read_gbparam_xml(&g_head,&gnum);
		
		if((bflag==0)&&(oflag==0)&&(cflag==0)&&(gflag==0))
		{
			ret=write_dhcp_conf(DHCP_CONF_PATH, &o_head, &c_head, &b_head,&g_head);
		}
		if((bflag==0 )&& (bnum > 0))
		Free_bindipmac_info(&b_head);

		if((cflag==0 )&& (cnum > 0))
		Free_confinfo_info(&c_head);

		if((oflag==0 )&& (onum > 0))
		Free_optionsix_info(&o_head);
		
		if((gflag==0 )&& (gnum > 0))
		Free_gbparam_info(&g_head);
		
		if(ret==0)
		   ShowAlert(search(lpublic,"oper_succ"));
		else
			ShowAlert(search(lpublic,"oper_fail"));
		}
		  else
		  	ShowAlert(search(lpublic,"param_null"));

	  
  }

  fprintf(cgiOut,"<form method=post >"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCP");
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

		  fprintf(cgiOut,"<input type=hidden name=UN value=\"%s\">",encry);
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
          if(checkuser_group(addn)==0)  /*管理员*/
          	{
				 fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_conf style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		  	}
		  else
		  	{
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		  	}
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
					"<td align=left id=tdleft><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame class=top><font id=%s>DHCP</font><font id=%s> %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));
					fprintf(cgiOut,"</tr>");  

         				
         			 if(checkuser_group(addn)==0)  /*管理员*/
                  		{
                  		fprintf(cgiOut,"<tr height=25>"\
         				"<td align=left id=tdleft><a href=wp_dhcpadd.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
         				fprintf(cgiOut,"</tr>");         						
						fprintf(cgiOut,"<tr height=25>"\
         				"<td align=left id=tdleft><a href=wp_dhcpinter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"interface_info"));
         			    fprintf(cgiOut,"</tr>"); 
						fprintf(cgiOut,"<tr height=25>"\
         				"<td align=left id=tdleft><a href=wp_dhcpmac.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"bind"));
         			    fprintf(cgiOut,"</tr>"); 
						fprintf(cgiOut,"<tr height=25>"\
         				"<td align=left id=tdleft><a href=wp_dhcplease.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"details"));
         			    fprintf(cgiOut,"</tr>"); 

						fprintf(cgiOut,"<tr height=26>"\
	         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),"option");   /*突出显示*/
	         			fprintf(cgiOut,"</tr>");
					//	fprintf(cgiOut,"<tr height=25>"\
		     		//	"<td align=left id=tdleft><a href=wp_dhcpview.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"stat"));
		     		//	fprintf(cgiOut,"</tr>");


					 }
				
			
				for(i=0;i<3;i++)
				{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
				}

				
				fprintf(cgiOut,"</table>"\
                 "</td>"\
                 "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
				fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>");					
				fprintf(cgiOut,"<tr>");
				
                ///////////////////////////add new content  identifier 

                fprintf(cgiOut,"<tr>\n");
				fprintf(cgiOut,"<td width=80>\n");
				fprintf(cgiOut,"%s:",search(lpublic,"dhcp_rname"));
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"<td>\n");
				fprintf(cgiOut,"<input type=text name=optclass value=\"\">");
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"</tr>\n");

				fprintf(cgiOut,"<tr height=7><td></td></tr>\n");

				fprintf(cgiOut,"<tr>\n");
				fprintf(cgiOut,"<td width=80>\n");
				fprintf(cgiOut,"option60:");
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"<td>\n");
				fprintf(cgiOut,"<input type=text name=iden value=\"\">");
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"</tr>\n");

				fprintf(cgiOut,"<tr height=7><td></td></tr>\n");

				fprintf(cgiOut,"<tr>\n");
				fprintf(cgiOut,"<td width=80>\n");
				fprintf(cgiOut,"option43:");
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"<td width=150 >"\
						"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	  		    fprintf(cgiOut,"<input type=text  name=ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	  	        fprintf(cgiOut,"<input type=text  name=ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
 		        fprintf(cgiOut,"<input type=text  name=ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
		        fprintf(cgiOut,"<input type=text  name=ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	  	        fprintf(cgiOut,"</div></td>");
				fprintf(cgiOut,"</tr>\n");

				fprintf(cgiOut,"<tr height=7><td></td></tr>\n");

				fprintf(cgiOut,"<tr>"\
				"<th width=120 align=left style=font-size:14px>%s</th>",search(lpublic,"dhcp_rname"));
				fprintf(cgiOut,"<th width=120 align=center style=font-size:14px>%s</th>","option60");
				fprintf(cgiOut,"<th width=120 align=center style=font-size:14px>%s</th>","option43");			 
				fprintf(cgiOut,"<th width=120 align=center style=font-size:14px></th>");
				fprintf(cgiOut,"<th width=120 align=center style=font-size:14px></th>");
				fprintf(cgiOut,"</tr>");
				
                oflag=read_option_info(&o_head, &onum);
				if(oflag==0)
				{
	                oq=o_head.next;

					while(oq !=NULL)
					{
					  fprintf(cgiOut,"<tr height=5 bgcolor=%s>\n",setclour(cl));
				      fprintf(cgiOut,"<td>%s</td>",oq->classname);
					  fprintf(cgiOut,"<td>%s</td>",oq->iden);
					  fprintf(cgiOut,"<td>%s</td>",oq->vendor);
					  fprintf(cgiOut,"<td><a href=wp_dhcp_optmod.cgi?UN=%s&ID=%s&TYPE=%s>%s</a></td>",encry,oq->classname,"1",search(lcontrol,"edit"));
					  fprintf(cgiOut,"<td><a href=wp_dhcp_optmod.cgi?UN=%s&ID=%s&TYPE=%s>%s</a></td>",encry,oq->classname,"2",search(lcontrol,"del"));
					  fprintf(cgiOut,"</tr>\n");
					  oq=oq->next;
					  cl = !cl;
					  
					}
				}
              ///////////////////////////end add new content
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

if((oflag==0 )&& (onum > 0))
	Free_optionsix_info(&o_head);
free(encry);

return 0;
}
						 






