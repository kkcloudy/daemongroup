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
#include "ws_dhcpv6.h"


void ShowDhcpdtAPage(char *m,char *n,char *ins_id,struct list *lpublic,struct list *lsecu);  

int cgiMain()
{
	char encry[BUF_LEN] = {0};    
	char eid[30] = {0};
	char *str;               
	struct list *lpublic;
	struct list *lsecu;
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsecu=get_chain_head("../htdocs/text/security.txt");
	cgiFormStringNoNewlines("UN", encry, sizeof(encry)); 
	cgiFormStringNoNewlines("NAME",eid,sizeof(eid));
	ccgi_dbus_init();
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user"));
	}
	else
	{
		ShowDhcpdtAPage(encry,str,eid,lpublic,lsecu);
	}
	release(lpublic);  
	release(lsecu);
	return 0;
}

void ShowDhcpdtAPage(char *m,char *n,char *ins_id,struct list *lpublic,struct list *lsecu)
{  
	int i = 0;	
	unsigned int mode = 0, index = 0, count = 0;	
	int cflag=-1;
	int lease_t = 0,day=0,hour=0,min=0;
	struct dhcp6_pool_show_st head,*q;
	memset(&head,0,sizeof(struct dhcp6_pool_show_st));
	struct dhcp6_sub *sq;
	int j = 0;

	
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>DHCP IPV6</title>");
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	"</head>"\
	"<body>"\
	"<form>"\
	"<div align=center>"\
	"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
	"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCP IPV6");
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>"\
	"<td width=62 align=center><a href=wp_dhcpv6con.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
	fprintf(cgiOut,"<td width=62 align=center><a href=wp_dhcpv6con.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),"DHCP");   /*Õª≥ˆœ‘ æ*/
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

				fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1 width=200>POOL %s</td>",search(lpublic,"name"));
				fprintf(cgiOut,"<td id=td2 width=370>%s</td>",ins_id);
				fprintf(cgiOut,"</tr>");
								
				cflag = 0;//ccgi_show_ipv6_pool(mode,index, &head, &count);
				if(cflag==1)
				{
				    q = head.next;
					while( q != NULL )
					{
					  sq = q->ipv6_subnet.next;
					  while( sq != NULL )
					  {
							if(strcmp(ins_id,q->poolname)!=0)
							{
								break;
							}
							else
							{
								if (q->domain_name) 
								{
									fprintf(cgiOut,"<tr align=left>"\
									"<td id=td1 width=200>%s</td>","DHCP domain name");
									fprintf(cgiOut,"<td id=td2 width=370>%s</td>",q->domain_name);
									fprintf(cgiOut,"</tr>");
								}
								for(j = 0; j < q->option_adr_num; j++)
								{
									fprintf(cgiOut,"<tr align=left>"\
									"<td id=td1 width=200>%s [%d]</td>","DHCP option52",j+1);
									fprintf(cgiOut,"<td id=td2 width=370 style=\"word-break:break-all\">%s</td>\n",q->option52[j]);
									fprintf(cgiOut,"</tr>");
								}
								for (j = 0; j < q->dnsnum; j++) 
								{				
									fprintf(cgiOut,"<tr align=left>"\
									"<td id=td1 width=200>%s [%d]</td>","DHCP dns",j+1);
									fprintf(cgiOut,"<td id=td2 width=370 style=\"word-break:break-all\">%s</td>\n",q->dnsip[j]);
									fprintf(cgiOut,"</tr>");
								}
								lease_t = (q->defaulttime? q->defaulttime : 86400);
								day = lease_t/86400;
								hour = lease_t%86400;
								hour = hour/3600;
								min = lease_t%3600;
								min = min/60;
								fprintf(cgiOut,"<tr align=left>"\
								"<td id=td1 width=200>%s</td>","DHCP lease");
								fprintf(cgiOut,"<td id=td2 width=370>%d day,%d hour,%d min</td>",day,hour,min);
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr align=left>"\
								"<td id=td1 width=200>%s</td>","DHCP start IP");
								fprintf(cgiOut,"<td id=td2 width=370>%s</td>",sq->range_low_ip);
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr align=left>"\
								"<td id=td1 width=200>%s</td>","DHCP end IP");
								fprintf(cgiOut,"<td id=td2 width=370>%s</td>",sq->range_high_ip);
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr align=left>"\
								"<td id=td1 width=200>%s</td>","DHCP Prefix");
								fprintf(cgiOut,"<td id=td2 width=370>%d</td>",sq->prefix_length);
								fprintf(cgiOut,"</tr>");
							}
					     sq = sq->next;
					  }					
                      q = q->next;
					}
					
				}			
				if( cflag == 1 )
				{
					Free_ccgi_show_ipv6_pool(&head);
				}

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



