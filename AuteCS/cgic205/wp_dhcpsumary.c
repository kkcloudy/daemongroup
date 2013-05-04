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
* wp_dhcpsumary.c
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
#include <sys/types.h>
#include <unistd.h>
#include <libxml/xpathInternals.h>
#include "ws_dcli_dhcp.h"
#include "ws_init_dbus.h"
#include "ws_public.h"
#include "ws_dbus_list_interface.h"


int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic);
void ShowIPaddr(struct list *lcontrol,struct list *lpublic,char* addn);


//void count_range();
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

	/*FILE *fp;
	char lan[3];*/

	char dhcp_encry[BUF_LEN]; 
	char addn[N];     
	int ret = -1;
	unsigned int node_num = 0;
	unsigned int option = 0;
	struct dhcp_relay_show_st rhead,*rq;
	memset(&rhead,0,sizeof(struct dhcp_relay_show_st));
	char relaystat[20] = {0};

	int i = 0;   
	struct dhcp_global_show_st global_show;
	memset(&global_show,0,sizeof(struct dhcp_global_show));
	global_show.domainname = (char *)malloc(30);
	global_show.option43 = (char *)malloc(256);
	memset(global_show.domainname,0,30);
	memset(global_show.option43,0,256);

 

	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	strcpy(addn,str);
	memset(dhcp_encry,0,BUF_LEN);                   /*清空临时变量*/

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
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
 		//下面三句话用于禁止页面缓存
  fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
  fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
  fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");

  
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  
  "<script type=\"text/javascript\">"\
   "function popMenu(objId)"\
   "{"\
	  "var obj = document.getElementById(objId);"\
	  "if (obj.style.display == 'none')"\
	  "{"\
		"obj.style.display = 'block';"\
	  "}"\
	  "else"\
	  "{"\
		"obj.style.display = 'none';"\
	  "}"\
  "}"\
  "</script>"\
  "<script language=javascript src=/ip.js>"\
  "</script>"\
  "<body>");

  
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
     
			 fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));	
		
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
		#if 0
         			fprintf(cgiOut,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"summary"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
 		#endif	
         			fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>DHCP</font><font id=%s> %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));
         			fprintf(cgiOut,"</tr>");
					
					fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcrelay.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>DHCP</font><font id=%s> %s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"dhcp_relay"));
         			fprintf(cgiOut,"</tr>"); 
					
					fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpv6con.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>DHCP IPV6</font><font id=%s><font></a></td>",encry,search(lpublic,"menu_san"));
         			fprintf(cgiOut,"</tr>"); 
				for(i=0;i<10;i++)
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}

			
			ret = ccgi_show_dhcp_relay(&rhead,&node_num,&option,allslot_id);
			if (option == 1)
			{
				strcpy(relaystat,"start");
			}
			else
			{
				strcpy(relaystat,"stop");
			}
			ccgi_show_ip_dhcp_server(&global_show,allslot_id);
			char dhcpsta[10] = { 0 };
			if( global_show.enable == 1 )
			{
				strcpy(dhcpsta,"start");
			}
			else
			{
				strcpy(dhcpsta,"stop");
			}
		 fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
	fprintf(cgiOut,"<table width=750 border=0 cellspacing=0 cellpadding=0>"\				
		"<tr height=30>"\
		"<td style=\"border-bottom:2px solid #163871\"><font id=%s1>%s</font></td>",search(lpublic,"menu_summary"),search(lpublic,"summary"));
	fprintf(cgiOut,"</tr>");

	fprintf(cgiOut,"<tr>"\
		"<td style=\"padding-left:30px; padding-top:20px\"><table width=600 border=0 cellspacing=0 cellpadding=0>");

	fprintf(cgiOut,"<tr>");
	fprintf(cgiOut,"<td colspan=2>%s&nbsp;&nbsp;","Slot ID");
	fprintf(cgiOut,"<select name=allslot onchange=slotid_change(this)>");
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
   	"var url = 'wp_dhcpsumary.cgi?UN=%s&allslotid='+slotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", encry);
    fprintf( cgiOut,"</script>\n" );	
	free_instance_parameter_list(&paraHead2);	
	fprintf(cgiOut,"<input type=hidden name=allslotid value=\"%d\">",allslot_id);

	
	fprintf(cgiOut,"<tr>"\
		"<td colspan=3 style=\"border-bottom:1px solid black; padding-top:25px\"><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),"DHCP");
	fprintf(cgiOut,"</tr>"\			  	
		"<tr align=left style=\"padding-top:10px; padding-left:10px\">"\
		"<td width=300><font id=%s3>%s</font></td>",search(lpublic,"menu_summary"),"Dhcp");
	fprintf(cgiOut,"<td width=150><font id=%s4>%s</font></td>",search(lpublic,"menu_summary"),search(lpublic,"l_state"));	
	fprintf(cgiOut,"<td width=150>%s</td>",dhcpsta);
	fprintf(cgiOut,"</tr>"\
		"<tr>"\
		"<td colspan=3 style=\"border-bottom:1px solid black; padding-top:15px\"><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),"DHCP RELAY");
	fprintf(cgiOut,"</tr>"\
		"<tr align=left style=\"padding-top:10px; padding-left:10px\">"\
		"<td width=300><font id=%s3>%s</font></td>",search(lpublic,"menu_summary"),"Dhcp relay");
	fprintf(cgiOut,"<td width=150><font id=%s4>%s</font></td>",search(lpublic,"menu_summary"),search(lpublic,"l_state"));	
	fprintf(cgiOut,"<td width=150>%s</td>",relaystat);
	fprintf(cgiOut,"</tr>");
	fprintf(cgiOut,"<tr>"\
		"<td colspan=3 style=\"border-bottom:1px solid black; padding-top:15px\"><font id=%s2>%s</font></td>",search(lpublic,"menu_summary"),"DHCP use ratio");
	fprintf(cgiOut,"</tr>");

	int retu = 0;
	int iprate = 0;
	unsigned int lease_count = 0;
	unsigned int lease_free = 0;
	struct lease_state  total_state;
	struct sub_lease_state sub_state[MAX_SUB_NET];
	unsigned int subnet_num = 0;	

	memset(&total_state, 0, sizeof(struct lease_state));

	for(i = 0; i < MAX_SUB_NET; ++i)
	{
		memset(sub_state, 0, sizeof(struct sub_lease_state));
	}

	retu = ccgi_show_lease_state(&total_state, sub_state, &subnet_num,allslot_id);	
					 
	fprintf(cgiOut,"<tr align=left style=\"padding-top:10px; padding-left:10px\">"\
	"<td width=300>%s</td>","IP total num:");
	fprintf(cgiOut,"<td width=150 colspan=2>%d</td>", total_state.total_lease_num);
	fprintf(cgiOut,"</tr>");

	fprintf(cgiOut,"<tr align=left style=\"padding-top:10px; padding-left:10px\">"\
	"<td width=300>%s</td>","IP active num:");
	fprintf(cgiOut,"<td width=150 colspan=2>%d</td>",total_state.active_lease_num);
	fprintf(cgiOut,"</tr>");

	fprintf(cgiOut,"<tr align=left style=\"padding-top:10px; padding-left:10px\">"\
	"<td width=300>%s</td>","IP free num:");
	fprintf(cgiOut,"<td width=150 colspan=2>%d</td>",total_state.free_lease_num);

	if (total_state.total_lease_num != 0)
	{
		iprate = ((total_state.active_lease_num * 100)/total_state.total_lease_num);
	}
	fprintf(cgiOut,"<tr align=left style=\"padding-top:10px; padding-left:10px\">"\
	"<td width=300>%s</td>","IP use ratio:");
	fprintf(cgiOut,"<td width=150 colspan=2>%d%</td>",iprate);
	fprintf(cgiOut,"</tr>");
	fprintf(cgiOut,"</table>"\	
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
	if (( ret == 1)&&(node_num>0))
	{
		Free_dhcprelay_info(&rhead);
	}
	free(global_show.domainname);
	free(global_show.option43);

return 0;
}



