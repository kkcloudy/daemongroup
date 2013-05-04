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
* wp_dhcpmac.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system cotrl for dhcp bind mac
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
#include <unistd.h>
#include "ws_dhcp_conf.h"
#include "ws_dcli_dhcp.h"
#include "ws_init_dbus.h"
#include "ws_returncode.h"
#include "ws_dbus_list_interface.h"

int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic);
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

	char dhcp_encry[BUF_LEN]; 
	char addn[N];         
	struct dhcp_static_show_st host ,*hq;
	memset(&host,0,sizeof(host));
	
	unsigned int  count = 0;
	int retu = 0,op_ret = 0;

	char *dhcpip = (char *)malloc(30);
	memset(dhcpip,0,30);
	char *dhcpmac = (char *)malloc(30);
	memset(dhcpmac,0,30);
	char *optype = (char *)malloc(10);
	memset(optype,0,10);
	char *ifname = (char *)malloc(20);
	memset(ifname,0,20);
	char *ifn = (char *)malloc(20);
	memset(ifn,0,20);
	cgiFormStringNoNewlines("TY", optype, 10); 
	cgiFormStringNoNewlines("DIP", dhcpip, 30); 
	cgiFormStringNoNewlines("DMAC", dhcpmac, 30);   
	cgiFormStringNoNewlines("IFN", ifn, 20); 
	char dip1[4];
	char dip2[4];
	char dip3[4];
	char dip4[4];
	memset(dip1,0,4);
	memset(dip2,0,4);
	memset(dip3,0,4);
	memset(dip4,0,4);
	ETHERADDR		macAddr;
	memset(&macAddr,0,sizeof(ETHERADDR));
	
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
	
    if(strcmp(optype,"1")==0)
	{
		retu = delete_dhcp_static_host(dhcpip, dhcpmac,ifn,allslot_id);
	}
  

	int i = 0;   
	if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
	{
		memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
			ShowErrorPage(search(lpublic,"ill_user")); 	 /*ó??§·?·¨*/
			return 0;
		}
		strcpy(addn,str);
		memset(dhcp_encry,0,BUF_LEN);                   /*????áùê±±?á?*/
	}
	else
	{
		cgiFormStringNoNewlines("encry_dhcp", dhcp_encry, BUF_LEN); 
		str=dcryption(dhcp_encry);
		if(str==NULL)
		{
			ShowErrorPage(search(lpublic,"ill_user")); 	 /*ó??§·?·¨*/
			return 0;
		}
		strcpy(addn,str);
		memset(dhcp_encry,0,BUF_LEN);                   /*????áùê±±?á?*/
	}
 
	cgiFormStringNoNewlines("encry_dhcp",dhcp_encry,BUF_LEN);
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");

	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
		"<style type=text/css>"\
		".a3{width:30;border:0; text-align:center}"\
		"</style>"\
		"<style type=text/css>"\
		"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
		"#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
		"#link{ text-decoration:none; font-size: 12px}"\
		".dhcplis {overflow-x:hidden;	overflow:auto; width: 500px; height: 220px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
		"</style>"\
		"</head>"\
		"<script language=javascript src=/ip.js>"\
		"</script>"\
		"<body>");

	
	if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
	{

		memset(dip1,0,4);
		memset(dip2,0,4);
		memset(dip3,0,4);
		memset(dip4,0,4);
		memset(dhcpmac,0,30);
		memset(ifname,0,20);
		cgiFormStringNoNewlines("bind_ip1", dip1, 4); 
		cgiFormStringNoNewlines("bind_ip2", dip2, 4); 
		cgiFormStringNoNewlines("bind_ip3", dip3, 4); 
		cgiFormStringNoNewlines("bind_ip4", dip4, 4); 
		cgiFormStringNoNewlines("dhcpmac", dhcpmac, 30); 
		cgiFormStringNoNewlines("ifname", ifname, 20); 
		memset(&macAddr,0,sizeof(ETHERADDR));
		
		if((strcmp(ifname,"")!=0)&&(strcmp(dip1,"")!=0)&&(strcmp(dip2,"")!=0)&&(strcmp(dip3,"")!=0)&&(strcmp(dip4,"")!=0)&&(strcmp(dhcpmac,"")!=0))
		{
		    op_ret = parse_mac_addr((char *)dhcpmac, &macAddr);
			if(op_ret!=0)
			{
				ShowAlert(search(lcontrol,"s_arp_mac_err_format"));
			}
			else
			{
				memset(dhcpip,0,30);
				sprintf(dhcpip,"%s.%s.%s.%s",dip1,dip2,dip3,dip4);
				retu=add_dhcp_static_host(dhcpip, dhcpmac,ifname,allslot_id);
				if(retu==1)
				{
					ShowAlert(search(lcontrol,"save"));			
				}
				else
				{
					ShowAlert(search(lcontrol, "no_save"));
				}
			}
		}
		else
		{
			ShowAlert(search(lcontrol,"arg_not_null"));
		}
	}

  
  fprintf(cgiOut,"<form method=post>"\
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
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
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
         		if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
         		{ 			
                  fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame class=top><font id=%s>DHCP</font><font id=%s> %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));
         			fprintf(cgiOut,"</tr>");  
					 fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpadd.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
         			fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpinter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"interface_info"));
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"bind"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
         				"<td align=left id=tdleft><a href=wp_dhcplease.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"details"));
         			    fprintf(cgiOut,"</tr>"); 

					/*
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_dhcp_opt.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),"option");
					fprintf(cgiOut,"</tr>"); */
				//fprintf(cgiOut,"<tr height=25>"\
     			//"<td align=left id=tdleft><a href=wp_dhcpview.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"stat"));
     			//fprintf(cgiOut,"</tr>");

         		}
         		else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess) 			  
         		{					   
         			fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame class=top><font id=%s>DHCP</font><font id=%s> %s</font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));
         			fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpadd.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
         			fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpinter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"interface_info"));
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"bind"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
         				"<td align=left id=tdleft><a href=wp_dhcplease.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"details"));
         			    fprintf(cgiOut,"</tr>"); 

					/*fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_dhcp_opt.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),"option");
					fprintf(cgiOut,"</tr>"); */
				//	fprintf(cgiOut,"<tr height=25>"\
     			//"<td align=left id=tdleft><a href=wp_dhcpview.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"stat"));
     			//fprintf(cgiOut,"</tr>");

         		}
				
				for(i=0;i<10;i++)
				{
					fprintf(cgiOut,"<tr height=25>"\
					"<td id=tdleft>&nbsp;</td>"\
					"</tr>");
				}
				
				fprintf(cgiOut,"</table>"\
					"</td>"\
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
		   	"var url = 'wp_dhcpmac.cgi?UN=%s&allslotid='+slotid;\n"\
		   	"window.location.href = url;\n"\
		   	"}\n", encry);
		    fprintf( cgiOut,"</script>\n" );	
			free_instance_parameter_list(&paraHead2);	
			fprintf(cgiOut,"<input type=hidden name=allslotid value=\"%d\">",allslot_id);
				
				fprintf(cgiOut,"<tr height=40>"\
					"<td width=80>%s:</td>",search(lcontrol,"ip_addr"));
				fprintf(cgiOut,"<td width=140 align=left colspan=2>"\
					"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
				fprintf(cgiOut,"<input type=text  name=bind_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
				fprintf(cgiOut,"<input type=text  name=bind_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
				fprintf(cgiOut,"<input type=text  name=bind_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
				fprintf(cgiOut,"<input type=text  name=bind_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
				fprintf(cgiOut,"</div></td>"\
					"</tr>"\
					"<tr height=40>"\
					"<td  align=left>MAC:</td>"\
					"<td width=140><input type=text name=dhcpmac  size=21/></td>");					
				fprintf(cgiOut,"<td align=left><font color=red>%s</font></td>",search(lpublic,"mac_format"));
				fprintf(cgiOut,"</tr>\n");
				fprintf(cgiOut,"<tr height=40>"\
					"<td  align=left>IFNAME:</td>"\
					"<td width=140 colspan=2><input type=text name=ifname  size=21/></td>");					
				fprintf(cgiOut,"</tr>\n");
				///////////////////////////////
				fprintf(cgiOut,"<tr>\n"\
				"<td colspan=2 style=\"padding-top:20px\">");
				fprintf(cgiOut,"<div class=dhcplis>\n");
				fprintf(cgiOut,"<table width=500 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
				fprintf(cgiOut,"<tr bgcolor=#eaeff9 style=font-size:14px align=left>\n");
				fprintf(cgiOut,"<th width=100 style=font-size:12px>%s</th>","IP");
				fprintf(cgiOut,"<th width=170 style=font-size:12px>%s</th>","MAC");
				fprintf(cgiOut,"<th width=100 style=font-size:12px>%s</th>","IFNAME");
				fprintf(cgiOut,"<th width=130 style=font-size:12px></th>");
				fprintf(cgiOut,"</tr>\n");

				ccgi_show_static_host(&host, &count,allslot_id);
				hq = host.next;
				while( hq != NULL )
				{
				    memset(dhcpip,0,30);
					memset(dhcpmac,0,30);
					sprintf(dhcpip,"%d.%d.%d.%d",(((hq->ipaddr) & 0xff000000) >> 24),(((hq->ipaddr) & 0xff0000) >> 16),	\
					(((hq->ipaddr) & 0xff00) >> 8),((hq->ipaddr) & 0xff));
					sprintf(dhcpmac,"%02x:%02x:%02x:%02x:%02x:%02x",hq->mac[0], hq->mac[1], hq->mac[2], hq->mac[3], hq->mac[4],hq->mac[5]);
					
					fprintf(cgiOut,"<tr height=25>\n");
					fprintf(cgiOut,"<td>%s</td>",dhcpip);
					fprintf(cgiOut,"<td>%s</td>",dhcpmac);
					fprintf(cgiOut,"<td>%s</td>",hq->ifname);
					
                    if(strcmp(encry,"")==0)
						strcpy(encry,dhcp_encry);
					fprintf(cgiOut,"<td><a href=wp_dhcpmac.cgi?UN=%s&TY=1&DIP=%s&DMAC=%s&IFN=%s&allslotid=%d target=mainFrame>%s</a></td>",encry,dhcpip,dhcpmac,hq->ifname,allslot_id,search(lpublic,"delete"));
					fprintf(cgiOut,"</tr>\n");

					hq = hq->next;
				}
				if(count>0)
					Free_ccgi_show_static_host(&host);

				fprintf(cgiOut,"</table></div>\n");
				fprintf(cgiOut,"</td></tr>\n");
				///////////////////////////////
				fprintf(cgiOut,"<tr>");
				if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
				{
					fprintf(cgiOut,"<td ><input type=hidden name=encry_dhcp value=%s></td>",encry);
				}
				else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
				{
					fprintf(cgiOut,"<td ><input type=hidden name=encry_dhcp value=%s></td>",dhcp_encry);
				}
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
free(dhcpip);
free(dhcpmac);
free(encry);
free(optype);
free(ifn);
free(ifname);
return 0;
}
