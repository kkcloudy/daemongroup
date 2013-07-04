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
* wp_hsdetail.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for vrrp detail config
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
#include "ws_init_dbus.h"

#include "ws_dbus_list.h"
#include "ws_dbus_list_interface.h"
#include "ws_dcli_vrrp.h"
#include "ws_dcli_license.h"


void ShowSecdtaPage(char *m,char *id,struct list *lpublic,struct list *lcontrl);  

int cgiMain()
{
  char *encry=(char *)malloc(128);              
  char *str;               
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lcontrl;     /*解析security.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcontrl=get_chain_head("../htdocs/text/control.txt");
  memset(encry,0,128);
  char hsid[10];
  memset(hsid,0,10);
  cgiFormStringNoNewlines("UN", encry, 128); 
  cgiFormStringNoNewlines("ID",hsid,10);

  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
    ShowSecdtaPage(encry,hsid,lpublic,lcontrl);
  
  free(encry);
  release(lpublic);
  release(lcontrl);
  return 0;
}

void ShowSecdtaPage(char *m,char *id,struct list *lpublic,struct list *lcontrl)
{  
    int i = 0;
	Z_VRRP zvrrp;
	memset(&zvrrp,0,sizeof(zvrrp));
	int hspro_num=0;
	int retu = 0;
	vrrp_link_ip *uq=NULL;
	vrrp_link_ip *dq=NULL;
	vrrp_link_ip *vq=NULL;
	DBusConnection *connection = NULL;
	DBusConnection *master_connection = NULL;
	ccgi_dbus_init();
	instance_parameter *paraHead2 = NULL;
	instance_parameter *p_q = NULL;
	list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
	char plotid[10] = {0};
	int pid = 0,insid;
	int k = 0;
	cgiFormStringNoNewlines("plotid",plotid,sizeof(plotid));
	pid = atoi(plotid);
	insid = strtoul(id,NULL,10);
	if(0 == pid)
	{
		for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
		{
			pid = p_q->parameter.slot_id;
			connection = p_q->connection;
			break;
		}
	}
	else
	{
		for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
		{
			if(p_q->parameter.slot_id == pid)
			{
				connection = p_q->connection;
			}
		}
	}
	unsigned int active_master=0;
	FILE *fd = NULL;
	fd = fopen("/dbm/product/active_master_slot_id", "r");
	if (fd == NULL)
	{
		return -1;
	}
	fscanf(fd, "%d", &active_master);
	fclose(fd);
	for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
	{
	
		if(p_q->parameter.slot_id == active_master)
		{
			master_connection = p_q->connection;
		}
	}
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>VRRP</title>");
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
	fprintf(cgiOut,"<style type=text/css>"\
  	  "#div1{ width:80px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:78px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  ".divlis {overflow-x:hidden;	overflow:auto; width: 690; height: 400px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	  "</style>"\
	"</head>"\
	"<body>"\
	"<form>"\
	"<div align=center>"\
	"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
	"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"VRRP");
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>"\
	"<td width=62 align=center><a href=wp_hansilist.cgi?UN=%s&PN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,"",search(lpublic,"img_ok"));
	fprintf(cgiOut,"<td width=62 align=center><a href=wp_hansilist.cgi?UN=%s&PN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,"",search(lpublic,"img_cancel"));
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
	"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>VRRP%s<font></td>",search(lpublic,"menu_san"),search(lcontrl,"details"));   /*突出显示*/
	fprintf(cgiOut,"</tr>");

	
	for(i=0;i<15;i++)
	{
		fprintf(cgiOut,"<tr height=25>"\
		"<td id=tdleft>&nbsp;</td>"\
		"</tr>");
	}
	fprintf(cgiOut,"</table>"\
	"</td>"\
	"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
	"<table width=570 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
	"<tr valign=middle>"\
	"<td>");

	ccgi_dbus_init();
	hspro_num=strtoul(id,0,10);
//	retu = ccgi_show_hansi_profile(&zvrrp, hspro_num,pid,connection); 
	

	struct LicenseData *LicenseInfo = NULL;
	int license_count=0;
	int lic_ret=0;
	lic_ret=license_assign_show_cmd(master_connection,&license_count,&LicenseInfo);
	fprintf(stderr,"lic_ret=%d\n",lic_ret);
	fprintf(stderr,"license_count=%d\n",license_count);

	//////////////////////////////////////////////////////	

	fprintf(cgiOut,"<table width=570 border=0 cellspacing=0 cellpadding=0>"\
	"<tr align=left height=30>");
	fprintf(cgiOut,"<td id=thead1>%s %s</td>","VRRP",search(lcontrl,"details"));
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
    "<td align=left style=\"padding-left:20px\">");
	
	fprintf(cgiOut,"<div class=divlis><table valign=top rules=rows width=570 border=1>");
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>ID</td>"\
				  "<td id=td2>%d-%s</td>",pid,id);
    fprintf(cgiOut,"</tr>");
	//uplink ifname
	if(retu == 0)
	{
		uq = zvrrp.uplink_list;
		k = 0;
		while(NULL!=uq)
		{
			k++;
			if(k == 1)
			{
		        fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>%s/IP</td>"\
						  "<td id=td2>%s/%s</td>",search(lpublic,"hs_uplink"),uq->ifname,uq->link_ip);
				fprintf(cgiOut,"</tr>");	
			}
			else
			{
		        fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>&nbsp;</td>"\
						  "<td id=td2>%s/%s</td>",uq->ifname,uq->link_ip);
				fprintf(cgiOut,"</tr>");	
			}
			uq = uq->next;
		}
		
		//downlink ifname
		dq = zvrrp.downlink_list;
		k = 0;
		while(NULL!=dq)
		{

			k++;
			if(k == 1)
			{
			   fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>%s/IP</td>"\
						  "<td id=td2>%s/%s</td>",search(lpublic,"hs_dlink"),dq->ifname,dq->link_ip);
			   fprintf(cgiOut,"</tr>");
			}
			else
			{
			   fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>&nbsp;</td>"\
						  "<td id=td2>%s/%s</td>",dq->ifname,dq->link_ip);
			   fprintf(cgiOut,"</tr>");
			}
		   dq = dq->next;

		}
		
		//prio
		fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>%s</td>"\
					  "<td id=td2>%d</td>",search(lcontrl,"prior"),zvrrp.priority);
		fprintf(cgiOut,"</tr>");
		//heatbeat ifname
		fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>%s IFNAME</td>"\
					  "<td id=td2>%s</td>",search(lpublic,"hs_hblink"),zvrrp.hbinf);
		fprintf(cgiOut,"</tr>");
		//heatbeat ip
		fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>%s IP</td>"\
					  "<td id=td2>%s</td>",search(lpublic,"hs_hblink"),zvrrp.hbip);
		fprintf(cgiOut,"</tr>");
		//advertime
		fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>%s</td>"\
					  "<td id=td2>%d</td>",search(lpublic,"hs_adtime"),zvrrp.advert);
		fprintf(cgiOut,"</tr>");
		//virtual gateway
		vq = zvrrp.vgatewaylink_list;
		k = 0;
		while(NULL!=vq)
		{ 
			k++;
			if(k == 1)
			{
				fprintf(cgiOut,"<tr align=left valign=top>"\
						  "<td id=td1>%s Gateway/IP</td>",search(lcontrl,"virtual"));
				fprintf(cgiOut,"<td id=td2>%s/%s</td>",vq->ifname,vq->link_ip);
			   fprintf(cgiOut,"</tr>");
			}
			else
			{
				fprintf(cgiOut,"<tr align=left valign=top>"\
						  "<td id=td1>&nbsp;</td>");
				fprintf(cgiOut,"<td id=td2>%s/%s</td>",vq->ifname,vq->link_ip);
			   fprintf(cgiOut,"</tr>");
			}
		   
		   vq = vq->next;
		}
			
		//virtual mac
		fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>%s MAC</td>"\
					  "<td id=td2>%s</td>",search(lcontrl,"virtual"),zvrrp.macstate);
		fprintf(cgiOut,"</tr>");
		//if preemt
		fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>%s</td>"\
					  "<td id=td2>%s</td>",search(lpublic,"hs_preempt"),zvrrp.preempt);
		fprintf(cgiOut,"</tr>");

	}

	
	char buf[128] = {0};
	char *tmp = buf;
	int len = 0;
	
	char buf_num[128] = {0};
	char *tmp_num = buf_num;
	int len_num = 0;
	int j=0;
	int num=0;
	if(lic_ret==0)
	{
		if(LicenseInfo)
		{	
			fprintf(stderr,"license_count=%d\n",license_count);
			if(license_count!=0)
			{
				len += sprintf(tmp,"%14s","LicenseType:");
				tmp = buf + len;
				for(i = 1; i <= license_count; i++){
					len += sprintf(tmp,"%6d",i);
					tmp = buf + len;
				}
				fprintf(stderr,"buf=%s\n",buf);

				for(j = 0; j < license_count; j++)
				{
						num += LicenseInfo[j].r_assign_num[pid][insid];
				}
				fprintf(stderr,"num=%d\n",num);
//				if(num != 0)
//				{				
					len_num += sprintf(tmp_num,"%8s%2d-%2d:","Hansi",pid,insid);
					tmp_num = buf_num + len_num;
					for(j = 0; j < license_count; j++)
					{
						len_num += sprintf(tmp_num,"%6d",LicenseInfo[j].r_assign_num[pid][insid]);
						tmp_num = buf_num + len_num;
					}
					fprintf(stderr,"buf_num=%s\n",buf_num);
//				}
			}
		}
	}
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"max_ap_num"),buf);
	fprintf(cgiOut,"</tr>");
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>&nbsp;&nbsp;&nbsp;</td>"\
				  "<td id=td2>%s</td>",buf_num);
	fprintf(cgiOut,"</tr>");

	
	int oth_ret=0;
	char *info=NULL;
	char *p = NULL;
	char *str_strtok;
//	oth_ret=show_vrrp_runconfig_by_hansi(pid,id,connection,&info);
	if((oth_ret==0)&&(info))
	{

		fprintf(stderr,"info=%s",info);
		p=info;
		str_strtok=strtok(p, "config hansi multi-link-detect on");
		if(str_strtok)
		{
			fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>%s</td>"\
						  "<td id=td2>%s</td>",search(lpublic,"link_detect_switch"),"on");
			fprintf(cgiOut,"</tr>");
		}
		else
		{
			p=info;
			str_strtok=strtok(p, "config hansi multi-link-detect off");
			if(str_strtok)
			{
				fprintf(cgiOut,"<tr align=left>"\
							  "<td id=td1>%s</td>"\
							  "<td id=td2>%s</td>",search(lpublic,"link_detect_switch"),"off");
				fprintf(cgiOut,"</tr>");
			}
			else
			{
				fprintf(cgiOut,"<tr align=left>"\
							  "<td id=td1>%s</td>"\
							  "<td id=td2>%s</td>",search(lpublic,"link_detect_switch"),"no configure");
				fprintf(cgiOut,"</tr>");
			}
		}	
		p=info;
		str_strtok=strtok(p, "config service enable");
		if(str_strtok)
		{
			fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>%s</td>"\
						  "<td id=td2>%s</td>",search(lpublic,"vrrp_service_state"),"enable");
			fprintf(cgiOut,"</tr>");
		}
		else
			{
				fprintf(cgiOut,"<tr align=left>"\
							  "<td id=td1>%s</td>"\
							  "<td id=td2>%s</td>",search(lpublic,"vrrp_service_state"),"disable");
				fprintf(cgiOut,"</tr>");
			}
	}
	
	


	
	fprintf(cgiOut,"</table></div></td>"\
				"</tr>");
	fprintf(cgiOut,"<input type=hidden name=UN value=%s>",m);
	fprintf(cgiOut,"<input type=hidden name=ID value=%s>",id);
	fprintf(cgiOut,"<input type=hidden name=plotid value=%d>",pid);
	fprintf(cgiOut,"</table>");
	fprintf(cgiOut,"</td>"\
	" </tr>"\
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
	free_ccgi_show_hansi_profile(&zvrrp);
	free_instance_parameter_list(&paraHead2);	
}
