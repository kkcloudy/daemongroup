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
* wp_show_port.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for stp port display 
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_err.h"
#include "ws_init_dbus.h"
#include "ws_stp.h"
#include "ws_dcli_portconf.h"


int ShowPortPage(struct list *lpublic,struct list *lcon);
static unsigned int productid = 1;

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcon;	   /*解析control.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcon=get_chain_head("../htdocs/text/control.txt");
	ccgi_dbus_init();
    ShowPortPage(lpublic,lcon);
	release(lpublic);  
	release(lcon);

 	return 0;
}

int ShowPortPage(struct list *lpublic,struct list *lcon)
{
	char *encry=(char *)malloc(BUF_LEN);				/*存储从wp_usrmag.cgi带入的加密字符串*/
	char *str=NULL;

	//////////////
	ETH_SLOT_LIST  head,*p;
	ETH_PORT_LIST *pp;
	int port_num;
	//////////////

	char stp_encry[BUF_LEN]; 
	int retu=0;
	int i, ret = -1;
	char * CheckUsr=(char *)malloc(10);
	memset(CheckUsr,0,10);
	if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
	{
		memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
			ShowErrorPage(search(lpublic,"ill_user"));	 /*用户非法*/
			return 0;
		}
		memset(stp_encry,0,BUF_LEN); 				  /*清空临时变量*/
	}

	cgiFormStringNoNewlines("stp_encry",stp_encry,BUF_LEN);
	cgiFormStringNoNewlines("CheckUsr",CheckUsr,BUF_LEN);
	if(strcmp(CheckUsr,"")!=0)
	retu=atoi(CheckUsr);
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lcon,"stp_man"));
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
	"<body>");

	if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
	{
		retu=checkuser_group(str);
	}
	fprintf(cgiOut,"<form method=post >"\
	"<div align=center>"\
	"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
	"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>RSTP</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");


	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>"\
	"<td width=62 align=center><input id=but type=submit name=submit_stp style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
	if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	else										   
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",stp_encry,search(lpublic,"img_cancel"));
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
	if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
	{
		if(retu==0)  /*管理员*/
		{
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_stp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"br_info"));	 
			fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_config_stp_bridge.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"br_conf"));
			fprintf(cgiOut,"</tr>"\
			"<tr height=26>"\
			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"port_info"));//突出显示
			fprintf(cgiOut,"</tr>"\
			"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_config_stp_port.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"port_conf"));
			fprintf(cgiOut,"</tr>");
		}
		else
		{
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_stp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"br_info"));	 
			fprintf(cgiOut,"</tr>"\
			"<tr height=26>"\
			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"port_info"));//突出显示
			fprintf(cgiOut,"</tr>");
		}
	}
	else if(cgiFormSubmitClicked("submit_stp") == cgiFormSuccess)
	{
		if(retu==0)  /*管理员*/
		{
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_stp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"br_info"));	 
			fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_config_stp_bridge.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"br_conf"));
			fprintf(cgiOut,"</tr>"\
			"<tr height=26>"\
			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"port_info"));//突出显示
			fprintf(cgiOut,"</tr>"\
			"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_config_stp_port.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"port_conf"));
			fprintf(cgiOut,"</tr>");
		}
		else
		{
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_stp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"br_info"));	 
			fprintf(cgiOut,"</tr>"\
			"<tr height=26>"\
			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"port_info"));//突出显示
			fprintf(cgiOut,"</tr>");
		}
	}

	//////////////根据端口数目来调节框的长度
	int show_ret=-1;
	int line=0,l_num=0;
	show_ret=show_ethport_list(&head,&port_num);
	p=head.next;
	if(p!=NULL)
	{
		while(p!=NULL)
		{
			line +=p->port_num;
			pp=p->port.next;
			p=p->next;
		}
	}
	if(show_ret==0)
		l_num=line;
	else
		l_num=24;

	for(i=0;i<l_num;i++)  //********************
	{
		fprintf(cgiOut,"<tr height=25>"\
		"<td id=tdleft>&nbsp;</td>"\
		"</tr>");
	}

	if((show_ret==0)&&(port_num>0))
	{
		Free_ethslot_head(&head);
	}

	fprintf(cgiOut,"</table>"\
	"</td>"\
	"<td align=center valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:0px; padding-top:10px\">"\
	"<table width=720 border=0 cellspacing=0 cellpadding=0>"\
	"<tr height=35>");

	fprintf(cgiOut,"<td align=center id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lpublic,"port_des"));

	fprintf(cgiOut,"</tr>"\
	"<tr>"\
	"<td align=left valign=top  style=\"padding-top:0px\">");




	fprintf(cgiOut,"<div class=configvlan><table width=720 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>"\
	"<tr height=25 bgcolor=#eaeff9 style=font-size:14px  align=left>"\
	"<th  style=font-size:12px>%s</th>",search(lcon,"slot_port_no"));
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>",search(lcon,"prior"));
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>",search(lcon,"path_cost"));
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>",search(lcon,"port_rl"));
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>",search(lcon,"port_stat"));
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>",search(lcon,"link_status"));
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>",search(lcon,"link_type"));
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>",search(lcon,"edge_port"));
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>",search(lcon,"desi_br_id"));
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>",search(lcon,"desi_br_cost"));
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>",search(lcon,"desi_port"));
	fprintf(cgiOut,"</tr>");
	int br_stat=0;
		int stp_mode;
		if(ccgi_get_brg_g_state(&stp_mode)==1)
		{
			if(stp_mode==0)
			{
				unsigned char slot = 0,port = 0;
				unsigned port_index = 0;

				unsigned int tmpval[2] = {0};
				PORT_MEMBER_BMP portbmp;
				memset(&portbmp,0,sizeof(PORT_MEMBER_BMP));

				port_info port_information;
				char buf[10] = {0};
				int cl=1;
				if((ret = ccgi_get_all_ports_index(&portbmp)) <0)
				{
					ShowAlert(search(lcon,"execute_fail"));
				}
				ccgi_get_broad_product_id(&productid);
				for (i = 0; i < 64; i++) 
				{	
					if(PRODUCT_ID_AX7K == productid) 
					{
						slot = i/8 + 1;
						port = i%8;
					}
					else if((PRODUCT_ID_AX5K == productid) ||
					(PRODUCT_ID_AX5K_I == productid) ||
					(PRODUCT_ID_AU4K == productid) ||
					(PRODUCT_ID_AU3K == productid) ||
					(PRODUCT_ID_AU3K_BCM == productid) ||
					(PRODUCT_ID_AU3K_BCAT == productid) || 
					(PRODUCT_ID_AU2K_TCAT == productid))
					{
						slot = 1;
						port = i;
					}
					tmpval[i/32] = (1<<(i%32));

					if(portbmp.portMbr[i/32]& tmpval[i/32]) 
					{
						if(ccgi_get_one_port_index(slot,port,&port_index) < 0)	
						{
							continue;
						}
						if((ret = ccgi_get_one_port_info(port_index,productid,&port_information))!=0)
						{

							if(STP_DISABLE == ret)
							{
								br_stat = -1;
								break;
							}
							else 
							{
								ShowAlert(search(lcon,"execute_fail"));
								break;
							}
						}
						else
						{
							fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));
							fprintf(cgiOut,"<td style=font-size:12px align=left>%d/%d</td>",slot,port);
							fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",port_information.port_prio);
							fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",port_information.port_cost);
							fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",stp_port_role[port_information.port_role]);
							fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",stp_port_state[port_information.port_state]);
							fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",port_information.port_lk  ? "Y" : "N");	
							if(0 == port_information.port_p2p)
								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","N");
							else if(1 == port_information.port_p2p)
								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","Y");
							else if(2 == port_information.port_p2p)
								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","A");

							fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",port_information.port_edge ? "Y" : "N");
							memset(buf,0,sizeof(buf));
							sprintf(buf,"%d",port_information.br_prio);
							if(port_information.port_state)
							{				
								fprintf(cgiOut,"<td style=font-size:12px align=left>%s:%02x%02x%02x%02x%02x%02x</td>",port_information.port_state ? buf : "",port_information.mac[0]\
								,port_information.mac[1],port_information.mac[2],port_information.mac[3],port_information.mac[4],port_information.mac[5]); 
							}
							else
							{
								fprintf(cgiOut,"<td style=font-size:12px align=left></td>");
							}
							memset(buf,0,sizeof(buf));
							sprintf(buf,"&nbsp;&nbsp;&nbsp;%d",port_information.br_cost); 											
							fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",port_information.port_state ? buf : "");
							memset(buf,0,sizeof(buf));
							sprintf(buf,"%#0x",port_information.br_dPort);
							fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",port_information.port_state ? buf : "");																			
							cl=!cl;
						}
					}	
				}	
			}
			else
			{	
				br_stat = -2;//mstp running
			}
		}
		else
		{
			br_stat = -1;
		}
	fprintf(cgiOut,"</table></div>");
	if(br_stat == -1)
		fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"no_start_br"));										   
	if(br_stat == -2)
		fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"mstp_run"));
	fprintf(cgiOut,"</td>"\
	"</tr>"\
	"<tr>");
	if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
	{
		fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",encry);
		fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
	}
	else if(cgiFormSubmitClicked("submit_stp") == cgiFormSuccess)
	{ 			 
		fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",stp_encry);
		fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
	}
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

	free(encry);
	free(CheckUsr);													 
	return 0;

}

