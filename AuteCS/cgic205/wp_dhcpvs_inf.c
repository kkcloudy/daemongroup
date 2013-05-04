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
* wp_dhcpvs_inf.c
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

#define IP_ADDR_PATH "/var/run/apache2/dhcpvs.option"

int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic);
void ShowIPaddr(struct list *lcontrol,struct list *lpublic);


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
 

  char dhcp_encry[BUF_LEN]; 
  char addn[N];         
 
  int i = 0;   
  int cl=1;

  struct substringz s_head,*sq;
  int subnum=0,retflag=-1;

  
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
 
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	  "<style type=text/css>"\
  	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  ".dhcplis {overflow-x:hidden;	overflow:auto; width: 750px; height: 420px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	  "</style>"\
  "</head>"\
  "<body>");
  if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
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
  
  cgiFormStringNoNewlines("encry_dhcp",dhcp_encry,BUF_LEN);

  
  if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
  {
    ShowIPaddr(lcontrol,lpublic);
  }  
   
  fprintf(cgiOut,"<form method=post >"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCPV6");
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
			if(checkuser_group(addn)==0)  /*管理员*/
			{
				fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_conf style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
			}
			else
			{
				if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
					fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpvs_conf.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
				else                                         
					fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpvs_conf.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,search(lpublic,"img_ok"));	  
			}
			if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
				fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpvs_conf.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
			else                                         
				fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpvs_conf.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,search(lpublic,"img_cancel"));
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
				"<td align=left id=tdleft><a href=wp_dhcpvs_conf.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>DHCPV6 </font><font id=%s> %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));
				fprintf(cgiOut,"</tr>");  
				fprintf(cgiOut,"<tr height=25>"\
				"<td align=left id=tdleft><a href=wp_dhcpvs_add.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>DHCPV6 </font><font id=%s>%s<font></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));  
				fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"<tr height=26>"\
				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_er>DHCPV6 </font><font id=%s>%s<font></a></td>",search(lpublic,"menu_san"),search(lcontrol,"interface_info")); /*突出显示*/
				fprintf(cgiOut,"</tr>");  

			}
			else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess) 			  
			{	
				fprintf(cgiOut,"<tr height=25>"\
				"<td align=left id=tdleft><a href=wp_dhcpvs_conf.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>DHCPV6 </font><font id=%s> %s</font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));
				fprintf(cgiOut,"</tr>");  
				fprintf(cgiOut,"<tr height=25>"\
				"<td align=left id=tdleft ><a href=wp_dhcpvs_add.cgi?UN=%s target=mainFrame class=top><font id=yingwen_er>DHCPV6 </font><font id=%s>%s<font></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"add"));  
				fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"<tr height=26>"\
				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_er>DHCPV6 </font><font id=%s>%s<font></a></td>",search(lpublic,"menu_san"),search(lcontrol,"interface_info")); /*突出显示*/
				fprintf(cgiOut,"</tr>");
			}		
			
			FILE *fd;
			char buf[512];
			for(i=0;i<16;i++)
			{
				fprintf(cgiOut,"<tr height=25>"\
				"<td id=tdleft>&nbsp;</td>"\
				"</tr>");
			}


				 
			fprintf(cgiOut,"</table>"\
			"</td>"\
			"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");

			fprintf(cgiOut,"<table width=600 border=0 cellspacing=0 cellpadding=0>");
			fprintf(cgiOut,"<tr>"\
			"<td colspan=2 style=\"padding-top:20px\">");

            fprintf(cgiOut,"<div class=dhcplis>\n");
			fprintf(cgiOut,"<table width=600 border=1 frame=below rules=rows  cellspacing=0 bordercolor=#cccccc cellpadding=0>");
			fprintf(cgiOut,"<tr>"\
			"<th width=120 align=center style=font-size:14px>%s</th>",search(lcontrol,"intf_name"));
			fprintf(cgiOut,"<th width=120 align=center style=font-size:14px>%s</th>",search(lcontrol,"ip_addr"));
			fprintf(cgiOut,"<th width=120 align=center style=font-size:14px>%s</th>",search(lcontrol,"mask"));			 
			fprintf(cgiOut,"<th width=120 align=center style=font-size:14px>%s</th>",search(lcontrol,"inter_stat"));
			fprintf(cgiOut,"<th width=120 align=center style=font-size:14px>%s</th>",search(lcontrol,"listen"));
			fprintf(cgiOut,"<th width=70 align=center style=font-size:14px>&nbsp;</th>");
			fprintf(cgiOut,"</tr>");

			char *buf_inter=(char*)malloc(256);
			int check = 0;
			memset(buf,0,512);
			memset(buf_inter,0,256);
			strcpy(buf_inter,"sudo find_infvs.sh > /dev/null");
			system(buf_inter);
			fd = fopen(IP_ADDR_PATH,"r");
			if(fd != NULL)
			{
				fgets(buf,512,fd);
			}
            retflag=string_linksep_list(&s_head, &subnum,buf," ");  
			
			
			
			infi  interf;
			interface_list_ioctl (0,&interf);
			infi * q ;
			q = interf.next;
			while(q)
			{
				check=0;
				if(retflag==0)
				{   
					sq=s_head.next;					
					while(sq != NULL)
					{
						if(strcmp(sq->substr,q->if_name)==0)
						{
							check = 1;
							break;
						}				  			  			  
						sq=sq->next;				
					}
				}
				if(strcmp(q->if_name,"lo")!=0)
				{
					fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
					fprintf(cgiOut,"<td style=font-size:14px align=center>%s</td>",q->if_name);
					fprintf(cgiOut,"<td style=font-size:14px align=center>%s</td>",q->if_addr);
					fprintf(cgiOut,"<td style=font-size:14px align=center>%s</td>",q->if_mask);
					fprintf(cgiOut,"<td style=font-size:14px align=center>%s</td>",q->if_stat);

					if(check==1)
					{
						if(q->upflag==1)
							fprintf(cgiOut, "<td align=center><input type=\"checkbox\" value=%s name=vote checked></td>",q->if_name);
						else
							fprintf(cgiOut, "<td align=center><input type=\"checkbox\" value=%s name=vote checked disabled></td>",q->if_name);
					}
					else
					{
						if(q->upflag==1)
							fprintf(cgiOut, "<td align=center><input type=\"checkbox\" value=%s name=vote ></td>",q->if_name);
						else
							fprintf(cgiOut, "<td align=center><input type=\"checkbox\" value=%s name=vote disabled ></td>",q->if_name);
					}
				}

				fprintf(cgiOut,"</tr>");
				cl=!cl;								
				q = q->next;
			}
		    free_inf(&interf);
			if((retflag==0 )&& (subnum > 0))
				Free_substringz_all(&s_head);
           
			fprintf(cgiOut,"</table></div></td></tr>");
			if(fd != NULL)
			{
				fclose(fd);
			}
			free(buf_inter);
			fprintf(cgiOut,"<tr>");							
			if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
			{
				fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_dhcp value=%s></td>",encry);
			}
			else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
			{
				fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_dhcp value=%s></td>",dhcp_encry);
			}
			
			fprintf(cgiOut,"</tr>"
			"<tr>"\
			"<td colspan=2 id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
			fprintf(cgiOut,"</tr>"\
			"<tr>"\
			"<td colspan=2 >&nbsp;</td>"\
			"</tr>"\
			"<tr>");

			fprintf(cgiOut,"<td colspan=2>%s</td>",search(lcontrol,"describe"));								
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


return 0;
}
						 
void  ShowIPaddr(struct list *lcontrol,struct list *lpublic)
{
	FILE* fd_op;
	int result2;
	char ** responses2;
	char *inte = (char*)malloc(512);
	memset(inte,0,512);
	strcpy(inte,"INTERFACES=\"");
	result2 = cgiFormStringMultiple("vote", &responses2);
	int i = 0;
	i= 0;       
	while (responses2[i]) 
	{
		strcat(inte,responses2[i]);
		strcat(inte," ");
		i++;
	}
	strcat(inte,"\"");
	cgiStringArrayFree(responses2);
	fd_op = fopen(DHCPVS_OPT_PATH,"w+");
	if(fd_op == NULL)
	{
		fprintf(stderr,"can not open FILE!\n");
	}
	else
	{
		fputs(inte,fd_op);
	}
	ShowAlert(search(lcontrol,"save"));
	if(fd_op != NULL)
	{
		fclose(fd_op);
	}

	free(inte);


}






