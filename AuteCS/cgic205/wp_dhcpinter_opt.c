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
* wp_dhcpinter.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system cotrl for dhcp inter
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
#include "ws_public.h"
#include "ws_dcli_dhcp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic);
void ShowIPaddr(struct list *lpublic,struct list *lcontrol,char *opt,char *vname);


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
  char *encry=(char *)malloc(BUF_LEN); 
  char *str;
  char addn[N];      
  char *pname = (char *)malloc(20);
  memset(pname,0,20);
  char *vname = (char *)malloc(20);
  memset(vname,0,20);
  char opt[10];
  memset(opt,0,10);
  cgiFormStringNoNewlines("TYPE", opt, 10); 
  cgiFormStringNoNewlines("PNAME", pname, 20); 
  cgiFormStringNoNewlines("VNAME", vname, 20); 

  int i = 0;  
  
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
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	"</head>"\
	"<body>");
  
	memset(encry,0,BUF_LEN);
	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	strcpy(addn,str);

	if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
	{
		ShowIPaddr(lpublic,lcontrol,opt,vname);
	}  
   
  fprintf(cgiOut,"<form method=post >"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCP");
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>");

		if(checkuser_group(addn)==0)  /*管理员*/
		{
			fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_conf style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		}
		else
		{
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpinter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		}
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpinter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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

			fprintf(cgiOut,"<tr height=26>"\
			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></a></td>",search(lpublic,"menu_san"),search(lcontrol,"interface_info")); /*突出显示*/
			fprintf(cgiOut,"</tr>");  
					
			for(i=0;i<6;i++)
			{
				fprintf(cgiOut,"<tr height=25>"\
				"<td id=tdleft>&nbsp;</td>"\
				"</tr>");
			}
			fprintf(cgiOut,"</table>"\
				"</td>"\
				"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
			fprintf(cgiOut,"<table width=650 border=0 cellspacing=0 cellpadding=0>");

			fprintf(cgiOut,"<tr>");
			fprintf(cgiOut,"<td>%s&nbsp;&nbsp;</td>","Slot ID:");
			fprintf(cgiOut,"<td>%d</td>",allslot_id);
			fprintf(cgiOut,"</tr>");
			free_instance_parameter_list(&paraHead2);	
			fprintf(cgiOut,"<input type=hidden name=allslotid value=\"%d\">",allslot_id);
			
			infi  interf;
			char *bindname = (char *)malloc(50);
			memset(bindname,0,50);
			interface_list_ioctl (0,&interf);
			 
			infi * q ;
			q = interf.next;
			while(q)
			{
				if(strcmp(q->if_name,"lo")!=0)
				{
				    if(strcmp(q->if_name,vname)==0)
				    {
						fprintf(cgiOut,"<tr height=25>");
						fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"intf_name"));
						fprintf(cgiOut,"<td style=font-size:14px align=left>%s</td>",q->if_name);
						fprintf(cgiOut,"</tr>\n");
						fprintf(cgiOut,"<tr height=25>\n");
						fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"ip_addr"));
						fprintf(cgiOut,"<td style=font-size:14px align=left>%s</td>",q->if_addr);
						fprintf(cgiOut,"</tr>\n");
						fprintf(cgiOut,"<tr height=25>\n");
						fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"mask"));
						fprintf(cgiOut,"<td style=font-size:14px align=left>%s</td>",q->if_mask);
						fprintf(cgiOut,"</tr>\n");
						fprintf(cgiOut,"<tr height=25>\n");
						fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"inter_stat"));
						fprintf(cgiOut,"<td style=font-size:14px align=left>%s</td>",q->if_stat);
						fprintf(cgiOut,"</tr>\n");
						fprintf(cgiOut,"<tr height=25>\n");
						fprintf(cgiOut,"<td>Pool %s</td>",search(lpublic,"name"));
						fprintf(cgiOut,"<td style=font-size:14px align=left><input type=text name=poolname value=\"%s\"></td>",pname);
						fprintf(cgiOut,"</tr>");
						
				    }
				}
				q = q->next;
			}
		    free_inf(&interf);
			free(bindname);

			
			fprintf(cgiOut,"<tr>");							
			fprintf(cgiOut,"<td><input type=hidden name=UN value=%s></td>",encry);
			fprintf(cgiOut,"<td><input type=hidden name=TYPE value=%s></td>",opt);
			fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr>");							
			fprintf(cgiOut,"<td><input type=hidden name=PNAME value=%s></td>",pname);
			fprintf(cgiOut,"<td><input type=hidden name=VNAME value=%s></td>",vname);
			fprintf(cgiOut,"</tr>");
			
			fprintf(cgiOut,"<tr>"\
			"<td colspan=2 id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
			fprintf(cgiOut,"</tr>"\
			"<tr>"\
			"<td colspan=2 >&nbsp;</td>"\
			"</tr>"\
			"<tr>");
			fprintf(cgiOut,"<td colspan=2><font color=red>%s</font></td>",search(lcontrol,"describe"));								
			fprintf(cgiOut,"</tr></table>");
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
free(encry);
free(pname);
free(vname);

return 0;
}
						 
void  ShowIPaddr(struct list *lpublic,struct list *lcontrol,char *opt,char *vname)
{
	char pname[20] = {0};
 	cgiFormStringNoNewlines("poolname", pname, 20); 
	int ret = 0;
	ccgi_dbus_init();
	char allslotid[10] = {0};
	int allslot_id = 0;
	cgiFormStringNoNewlines("allslotid",allslotid,sizeof(allslotid));
	allslot_id = atoi(allslotid);
	if(strcmp(opt,"1")==0)
	{
		ret=ccgi_set_interface_ip_pool(pname,vname, 1,BIND_POOL_ON_INTERFACE,allslot_id);
	}
	else if(strcmp(opt,"2")==0)
	{
		ret=ccgi_set_interface_ip_pool(pname, vname, 0,BIND_POOL_ON_INTERFACE,allslot_id);
	}

	switch(ret)
	{
		case 0:ShowAlert(search(lpublic,"oper_fail"));
			   break;
		case 1:ShowAlert(search(lpublic,"oper_succ"));
			   break;
		case -1:ShowAlert(search(lcontrol,"pool_name_too_long"));
			    break;
		case -2:ShowAlert(search(lcontrol,"pool_bind_if"));
			    break;
		case -3:ShowAlert(search(lcontrol,"pool_has_no_subnet"));
			    break;
		case -4:ShowAlert(search(lcontrol,"pool_not_exist"));
			    break;
		case -5:ShowAlert(search(lpublic,"error"));
			    break;
	}
}

