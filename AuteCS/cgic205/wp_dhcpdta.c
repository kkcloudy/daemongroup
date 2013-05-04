/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* wp_dhcpdta.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
*
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
#include "ws_dcli_dhcp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

void ShowDhcpdtAPage(char *m,char *n,char *ins_id,struct list *lpublic,struct list *lsecu);  

int cgiMain()
{
  char *encry=(char *)malloc(BUF_LEN);    
  char *eid=(char *)malloc(30);
  char *str;               
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lsecu;     /*解析security.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsecu=get_chain_head("../htdocs/text/security.txt");
  memset(encry,0,BUF_LEN);
  memset(eid,0,30);
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  cgiFormStringNoNewlines("NAME",eid,30);
  ccgi_dbus_init();
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
    ShowDhcpdtAPage(encry,str,eid,lpublic,lsecu);
  free(encry);
  free(eid);
  release(lpublic);  
  release(lsecu);
  return 0;
}

void ShowDhcpdtAPage(char *m,char *n,char *ins_id,struct list *lpublic,struct list *lsecu)
{  
	int i = 0;
  	struct dhcp_pool_show_st head,*q;
	memset(&head,0,sizeof(struct dhcp_pool_show_st));
	struct dhcp_sub_show_st *pq;
	
	unsigned int mode = 0, index = 0, count = 0;	
	int cflag=-1;
	char *tmp1=(char *)malloc(50);
	memset(tmp1,0,50);
	char *tmp2=(char *)malloc(32);
	memset(tmp2,0,32);
	char *tmp3=(char *)malloc(32);
	memset(tmp3,0,32);
	int lease_t = 0,day=0,hour=0,min=0;
	
	struct dhcp_global_show_st global_show;
	memset(&global_show,0,sizeof(struct dhcp_global_show));
	global_show.domainname = (char *)malloc(30);
	memset(global_show.domainname,0,30);
	global_show.option43 = (char *)malloc(256);	
	memset(global_show.option43,0,256);

	char *gdom = (char *)malloc(32);
    char *groute = (char *)malloc(32);
	char *gwin = (char *)malloc(32);
	char *glease = (char *)malloc(32);
	char *gdns1 = (char *)malloc(32);
	char *gdns2 = (char *)malloc(32);
	char *gdns3 = (char *)malloc(32);

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
  fprintf(cgiOut,"<title>DHCP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
    "</head>"\
	"<body>"\
	"<form>"\
	"<div align=center>"\
	"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCP");
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),"DHCP");   /*突出显示*/
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
	           "<tr align=left height=10 valign=top>");
			   fprintf(cgiOut,"<td id=thead1>%s %s</td>","",search(lsecu,"detail"));
	           fprintf(cgiOut,"</tr>"\
               "<tr>"\
               "<td align=left style=\"padding-left:20px\">");
			    fprintf(cgiOut,"<table frame=below rules=rows width=570 border=1>");
				fprintf(cgiOut,"<tr>");
				fprintf(cgiOut,"<td id=td1>%s&nbsp;&nbsp;</td>","Slot ID:");
				fprintf(cgiOut,"<td id=td2>%d</td>",allslot_id);
				fprintf(cgiOut,"</tr>");
				free_instance_parameter_list(&paraHead2);	
				fprintf(cgiOut,"<input type=hidden name=allslotid value=\"%d\">",allslot_id);
				
				ccgi_show_ip_dhcp_server(&global_show,allslot_id);
				
				fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1 width=200>Global DHCP server</td>");
				fprintf(cgiOut,"<td id=td2 width=370>%s</td>",(global_show.enable)?"enable":"disable");
				fprintf(cgiOut,"</tr>");
				
				fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1 width=200>Global DHCP static arp</td>");
				fprintf(cgiOut,"<td id=td2 width=370>%s</td>",(global_show.staticarp)?"enable":"disable");
				fprintf(cgiOut,"</tr>");
				
				fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1 width=200>Global DHCP domain</td>");
				fprintf(cgiOut,"<td id=td2 width=370>%s</td>",global_show.domainname);				
				fprintf(cgiOut,"</tr>");
                memset(gdns1,0,32);
                memset(gdns2,0,32);
                memset(gdns3,0,32);
				ip_long2string(global_show.dns[0], gdns1);
				ip_long2string(global_show.dns[1], gdns2);
				ip_long2string(global_show.dns[2], gdns3);
				fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1 width=200>Global DHCP dns</td>");
				fprintf(cgiOut,"<td id=td2 width=370>%s %s %s %s %s</td>",(global_show.dns[0])?gdns1:"",(global_show.dns[1])?",":"",(global_show.dns[1])?gdns2:"",(global_show.dns[2])?",":"",(global_show.dns[2])?gdns3:"");				
				fprintf(cgiOut,"</tr>");
                memset(groute,0,32);
				ip_long2string(global_show.routers, groute);
				fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1 width=200>Global DHCP route</td>");
				fprintf(cgiOut,"<td id=td2 width=370>%s</td>",groute);					
				fprintf(cgiOut,"</tr>");
				 memset(gwin,0,32);
				ip_long2string(global_show.wins, gwin);
				fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1 width=200>Global DHCP Winip</td>");
				fprintf(cgiOut,"<td id=td2 width=370>%s</td>",gwin);					
				fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1 width=200>Global DHCP lease</td>");
				fprintf(cgiOut,"<td id=td2 width=370>%d</td>",global_show.defaulttime);					
				fprintf(cgiOut,"</tr>");				
				/////////////////////////////////
				fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1 width=200>POOL %s</td>",search(lpublic,"name"));
				fprintf(cgiOut,"<td id=td2 width=370>%s</td>",ins_id);
				fprintf(cgiOut,"</tr>");
				
				cflag=ccgi_show_ip_pool(mode, index, &head, &count,allslot_id);
				if(cflag==1)
				{
				    q = head.next;
					while( q != NULL )
					{
					  pq = q->sub_show.next;
					  while( pq != NULL )
					  {
							if(strcmp(ins_id,q->poolname)!=0)
							{
								break;
							}
							else
							{
							    if (q->domainname) 
								{
									fprintf(cgiOut,"<tr align=left>"\
									"<td id=td1 width=200>%s</td>","DHCP domain name");
									fprintf(cgiOut,"<td id=td2 width=370>%s</td>",q->domainname);
									fprintf(cgiOut,"</tr>");
								}
								if (q->option43) 
								{
									fprintf(cgiOut,"<tr align=left>"\
									"<td id=td1 width=200>%s</td>","DHCP option43");
									fprintf(cgiOut,"<td id=td2 width=370 style=\"word-break:break-all\">%s</td>\n",q->option43);
									fprintf(cgiOut,"</tr>");
								}
								lease_t = (q->defaulttime ? q->defaulttime : 86400);
								day = lease_t/86400;
								hour = lease_t%86400;
								hour = hour/3600;
								min = lease_t%3600;
								min = min/60;
								memset(tmp1,0,50);
								memset(tmp3,0,32);
								fprintf(cgiOut,"<tr align=left>"\
								"<td id=td1 width=200>%s</td>","DHCP lease");
								fprintf(cgiOut,"<td id=td2 width=370>%d day,%d hour,%d min</td>",day,hour,min);
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr align=left>"\
								"<td id=td1 width=200>%s</td>","DHCP dns");
								fprintf(cgiOut,"<td id=td2 width=370>%s</td>",(q->dns[0]&q->dns[1]) ? (strcat(strcat(ip_long2string(q->dns[0], tmp1), ip_long2string(q->dns[1], tmp2)), (q->dns[2] ? ip_long2string(q->dns[2], tmp3) : ""))) : ((q->dns[0] ? ip_long2string(q->dns[0], tmp1) : "0.0.0.0")));
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr align=left>"\
								"<td id=td1 width=200>%s</td>","DHCP route IP");
								fprintf(cgiOut,"<td id=td2 width=370>%s</td>",q->routers ? ip_long2string(q->routers, tmp2) : "0.0.0.0");
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr align=left>"\
								"<td id=td1 width=200>%s</td>","DHCP win IP");
								fprintf(cgiOut,"<td id=td2 width=370>%s</td>",q->wins ? ip_long2string(q->wins, tmp2) : "0.0.0.0");
								fprintf(cgiOut,"</tr>");
								memset(tmp2,0,32);
								fprintf(cgiOut,"<tr align=left>"\
								"<td id=td1 width=200>%s</td>","DHCP mask");
								fprintf(cgiOut,"<td id=td2 width=370>%s</td>",pq->mask ? ip_long2string(pq->mask, tmp2) : "0.0.0.0");
								fprintf(cgiOut,"</tr>");
								memset(tmp2,0,32);
								fprintf(cgiOut,"<tr align=left>"\
								"<td id=td1 width=200>%s</td>","DHCP start IP");
								fprintf(cgiOut,"<td id=td2 width=370>%s</td>",pq->iplow ? ip_long2string(pq->iplow, tmp2) : "0.0.0.0");
								fprintf(cgiOut,"</tr>");
								memset(tmp2,0,32);
								fprintf(cgiOut,"<tr align=left>"\
								"<td id=td1 width=200>%s</td>","DHCP end IP");
								fprintf(cgiOut,"<td id=td2 width=370>%s</td>",pq->iphigh ? ip_long2string(pq->iphigh, tmp2) : "0.0.0.0");
								fprintf(cgiOut,"</tr>");
							}
					     pq = pq->next;
					  }
					
                      q = q->next;
					}
					
				}			
				if( cflag == 1 )
					Free_show_dhcp_pool(&head);

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
free(tmp2);
free(tmp1);
free(tmp3);
free(global_show.domainname);
free(global_show.option43);
free(glease);
free(gwin);
free(groute);
free(gdom);
free(gdns1);
free(gdns2);
free(gdns3);
}


