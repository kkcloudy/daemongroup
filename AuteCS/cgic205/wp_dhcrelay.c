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
* wp_dhcrelay.c
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
#include "ws_public.h"
#include "ws_dcli_dhcp.h"
#include "ws_init_dbus.h"
#include "ws_returncode.h"
#include "ws_intf.h"
#include "ws_dbus_list_interface.h"

int dhcp_relayShowDhcrelayPage(struct list *lcontrol,struct list *lpublic);
void dhcp_relay_status(struct list *lcontrol,struct list *lpublic,char*addn);
void  dhcp_relay_config(char *addn,struct list *lpublic);

int cgiMain()
{
	
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	ccgi_dbus_init();
	dhcp_relayShowDhcrelayPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	return 0;
}



int dhcp_relayShowDhcrelayPage(struct list *lcontrol,struct list *lpublic)
{ 
	char *encry=(char *)malloc(BUF_LEN); 
	char *str;
	char dhcp_encry[BUF_LEN]; 
	char addn[N];        

	int i = 0;
	int cl=1;
	char vip[32] = {0};
	int ret = -1;
	unsigned int node_num = 0;
	unsigned int option = 0;
	char getupname[20] = {0};
	char getdname[20] = {0};
	char getip[32] = {0};

	struct dhcp_relay_show_st rhead,*rq;
	memset(&rhead,0,sizeof(struct dhcp_relay_show_st));
  
	if(cgiFormSubmitClicked("dhcp_relay") != cgiFormSuccess)
	{
		memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
			ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
		}
		strcpy(addn,str);
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
		strcpy(addn,str);
		memset(dhcp_encry,0,BUF_LEN);                   /*清空临时变量*/

	}
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
	cgiFormStringNoNewlines("encry_dhcp",dhcp_encry,BUF_LEN);
	cgiFormStringNoNewlines("UPNAME",getupname,20);
	cgiFormStringNoNewlines("DNAME",getdname,20);
	cgiFormStringNoNewlines("IP",getip,32);
	int get_ip = 0;
	int result2 = -1;
	if ((strcmp(getupname,"") != 0) && (strcmp(getdname,"") != 0) && (strcmp(getip,"") != 0))
	{
	    get_ip = strtoul(getip,0,10);
		result2 = ccgi_set_interface_ip_relay(getupname, getdname, get_ip, 0,allslot_id);

	}

	if(cgiFormSubmitClicked("dhcp_relay") == cgiFormSuccess)
	{
		dhcp_relay_status(lcontrol,lpublic,addn);
		dhcp_relay_config(addn,lpublic);

	}  
		
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
	"<style type=text/css>\n"\
	".a3{width:30;border:0; text-align:center}"\
	"</style>\n"\
	"<style type=text/css>\n"\
	"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	"#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	"#link{ text-decoration:none; font-size: 12px}"\
	".dhcplis {overflow-x:hidden;	overflow:auto; width: 750px; height: 300px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	"</style>\n"\
	"</head>\n"\
	"<script src=/ip.js>\n"\
	"</script>\n"\
	"<body>");

  fprintf(cgiOut,"<form method=post>");
  fprintf(cgiOut,"<div align=center>\n"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
  "<tr>\n"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>\n"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>\n"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCP");
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>\n"\
		"<tr>");
	if(checkuser_group(addn)==0)  /*管理员*/
	{
		fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_relay style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	}
	else
	{
		if(cgiFormSubmitClicked("dhcp_relay") != cgiFormSuccess)
		{
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpview.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		}
		else  
		{
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpview.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,search(lpublic,"img_ok"));	  
		}
	}
	if(cgiFormSubmitClicked("dhcp_relay") != cgiFormSuccess)
	{
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpsumary.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	}
	else  
	{
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpsumary.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,search(lpublic,"img_cancel"));
	}
	fprintf(cgiOut,"</tr>\n"
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
		if(cgiFormSubmitClicked("dhcp_relay") != cgiFormSuccess)
		{ 			
			fprintf(cgiOut,"<tr height=26>\n"\
				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></a></td>",search(lpublic,"menu_san"),search(lcontrol,"dhcp_relay")); /*突出显示*/
			fprintf(cgiOut,"</tr>");  
		}
		else if(cgiFormSubmitClicked("dhcp_relay") == cgiFormSuccess) 			  
		{	
			fprintf(cgiOut,"<tr height=26>\n"\
				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></a></td>",search(lpublic,"menu_san"),search(lcontrol,"dhcp_relay")); /*突出显示*/
			fprintf(cgiOut,"</tr>");

		}
		for(i=0;i<19;i++)
		{
			fprintf(cgiOut,"<tr height=25>\n"\
			"<td id=tdleft>&nbsp;</td>\n"\
			"</tr>");
		}
		fprintf(cgiOut,"</table>\n"\
			"</td>\n"\
			"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">\n");
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
	   	"var url = 'wp_dhcrelay.cgi?UN=%s&allslotid='+slotid;\n"\
	   	"window.location.href = url;\n"\
	   	"}\n", encry);
	    fprintf( cgiOut,"</script>\n" );	
		free_instance_parameter_list(&paraHead2);
		fprintf(cgiOut,"<input type=hidden name=allslotid value=\"%d\">",allslot_id);

		ret = ccgi_show_dhcp_relay(&rhead,&node_num,&option,allslot_id);
		fprintf(cgiOut,"<tr style='padding-top:10px'>\n"\
			"<td align=left>%s:</td>",search(lcontrol,"dhcrelay_stat"));
		fprintf(cgiOut,"<td align=left width=400>\n"\
			"<select name=State>");
		if (option == 1)
		{
			fprintf(cgiOut,"<option value=\"start\" selected=\"selected\">start</option>\n"\
			"<option value=\"stop\" >stop</option>");
		}
		else
		{
			fprintf(cgiOut,"<option value=\"start\">start</option>\n"\
				"<option value=\"stop\" selected=\"selected\">stop</option>");
		}
		fprintf(cgiOut,"</select>\n"\
			"</td>\n"\
			"</tr>");
		fprintf(cgiOut,"<tr><td colspan=2>&nbsp;</td></tr>"); 
		
		struct flow_data_list * alldata = NULL;
		struct flow_data_list * temp = NULL;	
		struct flow_data_list * tempz = NULL;	

		intflib_getdata(&alldata);
		temp = alldata;
		tempz = alldata;
		//local interface
		fprintf(cgiOut,"<tr height=30>\n"\
			"<td align=left>%s:</td>", search(lpublic,"dhcp_clientname"));
		fprintf(cgiOut,"<td>\n");
		fprintf(cgiOut,"<select name=localvname style='width:100px;height:auto'>\n");
		for(temp; temp; temp = temp->next)
		{
			if(strncmp(temp->ltdata.ifname,"radio",5)==0 
			|| strncmp(temp->ltdata.ifname,"pimreg",6)==0 
			|| strncmp(temp->ltdata.ifname,"sit0",4)==0
			|| strncmp(temp->ltdata.ifname,"lo",2)==0)
			{
				continue;
			}
			fprintf(cgiOut,"<option value='%s'>%s</option>",temp->ltdata.ifname,temp->ltdata.ifname);
			
		}		
		fprintf(cgiOut,"</select></td></tr>");
		//remote interface
		fprintf(cgiOut,"<tr height=30>\n"\
			"<td align=left>%s:</td>", search(lpublic,"dhcp_servername"));
		fprintf(cgiOut,"<td>\n");
		fprintf(cgiOut,"<select name=remotevname style='width:100px;height:auto'>\n");
		for(tempz; tempz; tempz = tempz->next)
		{
			if(strncmp(tempz->ltdata.ifname,"radio",5)==0 
			|| strncmp(tempz->ltdata.ifname,"pimreg",6)==0 
			|| strncmp(tempz->ltdata.ifname,"sit0",4)==0
			|| strncmp(tempz->ltdata.ifname,"lo",2)==0)
			{
				continue;
			}
			fprintf(cgiOut,"<option value='%s'>%s</option>",tempz->ltdata.ifname,tempz->ltdata.ifname);
			
		}		
		fprintf(cgiOut,"</select></td></tr>");		
		intflib_memfree(&alldata);
		
		//remote server ip
		fprintf(cgiOut,"<tr height=30>\n"\
			"<td align=left>%s:</td>", search(lcontrol, "dhcrelay_addr"));
		fprintf(cgiOut,"<td>\n"\
			"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">\n"\
			"<input type=text  name=serv_ip1 value=\"\" id=serv_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.</input>",search(lpublic,"ip_error"));
		fprintf(cgiOut, "<input type=text  name=serv_ip2 value=\"\" id=serv_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.</input>",search(lpublic,"ip_error"));
		fprintf(cgiOut, "<input type=text  name=serv_ip3 value=\"\" id=serv_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.</input>",search(lpublic,"ip_error")); 
		fprintf(cgiOut, "<input type=text  name=serv_ip4 value=\"\" id=serv_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()></input>", search(lpublic,"ip_error"));
		fprintf(cgiOut, "</div></td></tr>\n");

		fprintf(cgiOut,"<tr><td colspan=2></td></tr>\n"); 
		fprintf(cgiOut,"<tr><td colspan=2 id=sec style=\"border-bottom:2px solid #53868b\">%s</td></tr>\n",search(lcontrol,"intf_list"));


		fprintf(cgiOut,"<tr>\n"\
			"<td colspan=2 style=\"padding-top:20px\">\n"\
			"<div class=dhcplis>\n"\
			"<table width=750 border=1 frame=below rules=rows  cellspacing=0 bordercolor=#cccccc cellpadding=0>");

		fprintf(cgiOut,"<tr>\n"\
			"<th width=200 align=center style=font-size:14px>%s</th>",search(lpublic,"dhcp_servername"));
		fprintf(cgiOut,"<th width=200 align=center style=font-size:14px>%s</th>",search(lpublic,"dhcp_clientname"));
		fprintf(cgiOut,"<th width=200 align=center style=font-size:14px>%s</th>",search(lcontrol,"ip_addr"));	
		fprintf(cgiOut,"<th width=150 align=center style=font-size:14px></th>");	
		fprintf(cgiOut,"</tr>");


		if ((ret == 1)&&(node_num>0))
		{
			rq = rhead.next;
			while (rq != NULL)
			{
				memset(vip,0,32);
				INET_NTOA_T(rq->ipaddr, vip);
				fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
				fprintf(cgiOut,"<td style=font-size:14px align=center>%s</td>",rq->downifname);
				fprintf(cgiOut,"<td style=font-size:14px align=center>%s</td>",rq->upifname);
				fprintf(cgiOut,"<td style=font-size:14px align=center>%s</td>",vip);
				if (strcmp(encry,"") == 0)
				{
					strcpy(encry,dhcp_encry);
				}
				fprintf(cgiOut,"<td style=font-size:14px align=center>\n"\
					"<a id=link href=wp_dhcrelay.cgi?UN=%s&UPNAME=%s&DNAME=%s&IP=%d target=mainFrame>%s</a></td>",encry,rq->upifname,rq->downifname,rq->ipaddr,search(lpublic,"delete"));
				fprintf(cgiOut,"</tr>");
				rq = rq->next;
				cl=!cl;			
			}				
		}
		if (( ret == 1)&&(node_num>0))
		{
			Free_dhcprelay_info(&rhead);
		}
		
		fprintf(cgiOut,"</table></div></td></tr>");
		fprintf(cgiOut,"<tr>");							
		if(cgiFormSubmitClicked("dhcp_relay") != cgiFormSuccess)
		{
			fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_dhcp value=%s></td>",encry);
		}
		else if(cgiFormSubmitClicked("dhcp_relay") == cgiFormSuccess)
		{
			fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_dhcp value=%s></td>",dhcp_encry);
		}

		fprintf(cgiOut,"</tr>\n"\
			"<tr>\n"\
			"<td colspan=2 >&nbsp;</td>\n"\
			"</tr>\n"\
			"<tr>");
		fprintf(cgiOut,"</tr></table>");
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
		
free(encry);
return 0;
}
						 
void  dhcp_relay_status(struct list *lcontrol,struct list *lpublic,char *addn)
{
	if(checkuser_group(addn)==0)
	{
		char allslotid[10] = {0};
		int allslot_id = 0;
		cgiFormStringNoNewlines("allslot",allslotid,sizeof(allslotid));
		allslot_id = atoi(allslotid);
		char status[20] = {0};
		cgiFormStringNoNewlines("State",status,20);
		unsigned int ifenable = 0;
		int ret = -1;
		/////enable or disable 
		if (strcmp(status,"stop") == 0)
		{
			ifenable = 0;
		}
		else if (strcmp(status,"start") == 0)
		{
			ifenable = 1;
		}
		ret = ccgi_set_relay_enable(ifenable,allslot_id);
		if (ret == 1)
		{
			ShowAlert(search(lpublic,"oper_succ"));
		}
		else
		{
			ShowAlert(search(lpublic,"oper_fail"));
		}	
	}
}
						 
void  dhcp_relay_config(char *addn,struct list *lpublic)
{
	char serv_ip1[4] = {0};
	char serv_ip2[4] = {0};
	char serv_ip3[4] = {0};
	char serv_ip4[4] = {0};
	char downname[20] = {0};
	char upname[20] = {0};
	char strip[32] = {0};
	int intip = 0;
	int result1 = -1;
	char allslotid[10] = {0};
	int allslot_id = 0;
	cgiFormStringNoNewlines("allslot",allslotid,sizeof(allslotid));
	allslot_id = atoi(allslotid);
	if(checkuser_group(addn)==0)
	{
		cgiFormStringNoNewlines("remotevname",downname,20);
		cgiFormStringNoNewlines("localvname",upname,20);
		cgiFormStringNoNewlines("serv_ip1",serv_ip1,4);
		cgiFormStringNoNewlines("serv_ip2",serv_ip2,4);
		cgiFormStringNoNewlines("serv_ip3",serv_ip3,4);
		cgiFormStringNoNewlines("serv_ip4",serv_ip4,4);

		if((strcmp(downname,"") != 0)&&(strcmp(upname,"") != 0)&&(strcmp(serv_ip1,"") != 0)&&(strcmp(serv_ip2,"") != 0)&&(strcmp(serv_ip3,"") != 0)&&(strcmp(serv_ip4,"") != 0))
		{
			sprintf(strip,"%s.%s.%s.%s",serv_ip1,serv_ip2,serv_ip3,serv_ip4);
			INET_ATON_T(intip, strip);
			result1 = ccgi_set_interface_ip_relay(upname, downname, intip, 1,allslot_id);
		}
		if (result1 == 1)
		{
			ShowAlert(search(lpublic,"oper_succ"));
		}
		else
		{
			ShowAlert(search(lpublic,"oper_fail"));
		}
	}
}

