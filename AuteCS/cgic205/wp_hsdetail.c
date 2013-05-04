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
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"


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
    int i;
	char ulname[10];
	memset(ulname,0,10);
	char dlname[10];
	memset(dlname,0,10);
	char ulip[30];
	memset(ulip,0,30);
	char hbip[30];
	memset(hbip,0,30);
	char dlip[30];
	memset(dlip,0,30);
	char hip1[4],hip2[4],hip3[4],hip4[4];
	memset(hip1,0,4);
	memset(hip2,0,4);
	memset(hip3,0,4);
	memset(hip4,0,4);
	char uip1[4],uip2[4],uip3[4],uip4[4];
	memset(uip1,0,4);
	memset(uip2,0,4);
	memset(uip3,0,4);
	memset(uip4,0,4);
	char dip1[4],dip2[4],dip3[4],dip4[4];
	memset(dip1,0,4);
	memset(dip2,0,4);
	memset(dip3,0,4);
	memset(dip4,0,4);
	char hsprio[10];
	memset(hsprio,0,10);
	char hstime[10];
	memset(hstime,0,10);
	char hblink[30];
	memset(hblink,0,30);
	char vmac[10];
	memset(vmac,0,10);
	char paramalert[128];
	memset(paramalert,0,128);
	Z_VRRP zvrrp;
	memset(&zvrrp,0,sizeof(zvrrp));
	int hspro_num=0;
	//kehao add 20110519
    int numup,numdown ,numvgate;
	int j = 0;
	int k = 0;
	char *upstat[8];
	char *downstat[8];
	char *upname[8];
	char *downname[8];
	char *vgatename[8];
	char *vupip[8];
	char *vdownip[8];
	char *rupip[8];
	char *rdownip[8];
	char *vgip[8];

    for(j = 0; j < 8;j++)
	{
		upstat[j] = (char *)malloc(128);
		memset( upstat[j] ,0, 128 );

	}

    for(j = 0; j < 8;j++)
	{
		downstat[j] = (char *)malloc(128);
		memset( downstat[j] ,0, 128 );

	}

	
	for(j = 0; j < 8;j++)
	{
		upname[j] = (char *)malloc(128);
		memset( upname[j] ,0, 128 );

	}
	
	for(j = 0; j < 8;j++)
	{
		downname[j] = (char *)malloc(128);
		memset( downname[j] ,0, 128 );

	}
	for(j = 0;j < 8; j++)
	{
		vgatename[j] = (char *)malloc(128);
		memset( vgatename[j] ,0, 128 );

	}

	for(j = 0; j<8;j++)
	{
		vupip[j] = (char *)malloc(128);
		memset(vupip[j] ,0, 128 );
		
	}

	for(j = 0; j<8;j++)
	{
		vdownip[j] = (char *)malloc(128);
		memset(vdownip[j] ,0, 128 );
		
	}

	for(j = 0; j<8;j++)
	{
		rupip[j] = (char *)malloc(128);
		memset(rupip[j] ,0, 128 );
		
	}

	for(j = 0; j<8;j++)
	{
		rdownip[j] = (char *)malloc(128);
		memset(rdownip[j] ,0, 128 );
		
	}
    
	for(j = 0; j<8;j++)
	{
		vgip[j] = (char *)malloc(128);
		memset(vgip[j] ,0, 128 );
		
	}


	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>VRRP</title>");
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
	fprintf(cgiOut,"<style>"\
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
    //kehao modify  20110519
	//ccgi_show_hansi_profile_detail(&zvrrp, hspro_num); 

	//kehao modify  20110519
	ccgi_show_hansi_profile_detail(&zvrrp, hspro_num,&numup,&numdown,&numvgate,upstat,downstat,upname,downname,vgatename,vupip,vdownip,rupip,rdownip,vgip); 
	//////////////////////////////////////////////////////
	


	fprintf(cgiOut,"<table width=570 border=0 cellspacing=0 cellpadding=0>"\
	"<tr align=left height=30>");
	fprintf(cgiOut,"<td id=thead1>%s %s</td>","VRRP",search(lcontrl,"details"));
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
    "<td align=left style=\"padding-left:20px\">");
	fprintf(cgiOut,"<table valign=top rules=rows width=570 border=1>");
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>HS ID</td>"\
				  "<td id=td2>%s</td>",id);
    fprintf(cgiOut,"</tr>");
	//uplink ifname
	//kehao add  for circle
	for(k = 0; k < numup; k++)
	{
		#if 0
	    fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_uplink"),zvrrp.uplink_ifname);
		#endif
        fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_uplink"),upname[k]);
		//zvrrp.uplink_list = zvrrp.uplink_list->next;
	}

	
	//uplink virtual ip
	//kehao add for circle
	for(k=0; k<numup;k++)
	{ 
	   //kehao add 20110519
	   #if 0
	   fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s IP</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_uplinks"),zvrrp.uplink_ip);
	   #endif
       fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s IP</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_uplinks"),vupip[k]);
	   
	}
	#if 0
	//uplink mask
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s mask</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_uplinks"),zvrrp.uplink_ip);
	#endif
	//uplink ifname state
	//kehao add 20110520
	for(k = 0; k<numup;k++)
	{
	   #if 0
	   fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_uplinkst"),zvrrp.upstate);
	   #endif 

	   fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_uplinkst"),upstat[k]);
	}
	//uplink real ip
	//kehao add 20110520
	for(k = 0; k<numup;k++)
	{
	#if 0	
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_uprealip"),zvrrp.realipup);
	#endif 
    fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_uprealip"),rupip[k]);
	
	}
	
	//downlink ifname
	//kehao add for circle
	for(k = 0;k<numdown;k++)
	{
	   #if 0
	   fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_dlink"),zvrrp.downlink_ifname);
	   #endif

	   fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_dlink"),downname[k]);

	}
	//downlink ip	
	//kehao add for circle
	for(k = 0;k<numdown;k++)
	{
       //kehao modify 20110519
	   #if 0
	   fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s IP</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_downs"),zvrrp.downlink_ip);
	   #endif
	   fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s IP</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_downs"),vdownip[k]);
	   
	}
	#if 0
	//downlink mask
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s mask</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_downs"),zvrrp.downlink_ip);
	#endif
	//downlink ifname state
	//kehao modify 20110520
	for(k = 0;k<numdown;k++)
	{
	   #if 0
	   fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_dlinks"),zvrrp.downstate);
	   #endif 

	   fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_dlinks"),downstat[k]);
	}
	//downlink real ip
	//kahao add 20110520
	for(k = 0;k < numdown;k++)
	{
      #if 0
	  fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_dwrealip"),zvrrp.realipdown);
	  #endif
      fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_dwrealip"),rdownip[k]);
	  
	}
	
	//prio
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%d</td>",search(lcontrl,"prior"),zvrrp.priority);
	//heatbeat ifname
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s IFNAME</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_hblink"),zvrrp.hbinf);
	//heatbeat ip
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s IP</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_hblink"),zvrrp.hbip);
	//advertime
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%d</td>",search(lpublic,"hs_adtime"),zvrrp.advert);
	//virtual gateway
	fprintf(cgiOut,"<tr align=left valign=top>"\
				  "<td id=td1>%s Gateway</td>",search(lcontrl,"virtual"));
	if(0==zvrrp.gw_number)
	 	fprintf(cgiOut,"<td id=td2>%s</td>","not configured");
	else
		{
			int v_num=0;
			fprintf(cgiOut,"<td><table>");
			#if 0
			for(v_num=0;v_num<zvrrp.gw_number;v_num++)
				{
					fprintf(cgiOut,"<tr>");
					fprintf(cgiOut,"<td id=td2>%s</td>",zvrrp.gw[v_num].gwip);
					fprintf(cgiOut,"</tr>");		
					//kehao add for debug 
					fprintf(stderr,"vgate num = %s\n",zvrrp.gw_number);
					fprintf(stderr,"vgatewayip = %s\n",zvrrp.gw[v_num].gwip);

				}
			#endif
			for(k=0;k<numvgate;k++)
				{
					fprintf(cgiOut,"<tr>");
					fprintf(cgiOut,"<td id=td2>%s</td>",vgip[k]);
					fprintf(cgiOut,"</tr>");		
					//kehao add for debug 
					//fprintf(stderr,"vgate num = %s\n",zvrrp.gw_number);
					//fprintf(stderr,"vgatewayip = %s\n",zvrrp.gw[v_num].gwip);

				}
			fprintf(cgiOut,"</table></td>");
		}
	//virtual mac
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s MAC</td>"\
				  "<td id=td2>%s</td>",search(lcontrl,"virtual"),zvrrp.macstate);
	//if preemt
	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>"\
				  "<td id=td2>%s</td>",search(lpublic,"hs_preempt"),zvrrp.preempt);

	

	
	fprintf(cgiOut,"</table></td>"\
				"</tr>");
	fprintf(cgiOut,"<tr>");
	fprintf(cgiOut,"<td><input type=hidden name=UN value=%s></td>",m);
	fprintf(cgiOut,"<td><input type=hidden name=ID value=%s></td>",id);
	fprintf(cgiOut,"</tr>");
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
}
