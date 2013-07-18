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
* wp_prtsur.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for port manage
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_nm_status.h"
#include "ws_dcli_portconf.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_dbus_list_interface.h"

#define Max_Port_Num 100    /*所有设备中的最大端口个数*/

void ShowPrtmagPage(char *m,char *n,struct list *lpublic,struct list *lcon);     /*m代表加密后的字符串*/

int cgiMain()
{
  char *encry=(char *)malloc(BUF_LEN);			 /*存储从wp_topFrame.cgi带入的加密字符串*/  
  char *str;	         /*存储解密后的当前登陆用户名*/  
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lcon;      /*解析control.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcon=get_chain_head("../htdocs/text/control.txt");
  memset(encry,0,BUF_LEN);  
  cgiFormStringNoNewlines("UN", encry, BUF_LEN);     
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user")); 	  /*用户非法*/
  else
    ShowPrtmagPage(encry,str,lpublic,lcon);
  free(encry);
  release(lpublic);  
  release(lcon);
  return 0;
}

void ShowPrtmagPage(char *m,char *n,struct list *lpublic,struct list *lcon)
{ 
  int i,k,cl=1,limit,retu,ret,num;                 /*cl标识表格的底色，1为#f9fafe，0为#ffffff*/
  char *menu_id=(char *)malloc(10);
  char *menu=(char *)malloc(15);
  char *prt_no=(char *)malloc(10);  
  ETH_SLOT_LIST  head,*p = NULL;
  ETH_PORT_LIST *pp = NULL;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcon,"prt_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
    "#div1_ch{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2_ch{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
    "#div1_en{ width:122px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2_en{ width:120px; height:15px; padding-left:5px; padding-top:3px}"\
    "#link{ text-decoration:none; font-size: 12px}"\
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
  "<body>"\
  "<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
      "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcon,"prt_manage"));
    fprintf(cgiOut,"<td width=692 align=right valign=bottom background=/images/di22.jpg>");
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
		  "</table>");
		}
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
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcon,"prt_sur"));     /*突出显示*/
                    fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_prtarp.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>ARP<font><font id=%s> %s<font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"survey"));                       
                    fprintf(cgiOut,"</tr>");
					retu=checkuser_group(n);
					if(retu==0)  /*管理员*/
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_static_arp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lcon,"prt_static_arp"));						 				
					  fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_prtcfg.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"prt_cfg"));                       
                      fprintf(cgiOut,"</tr>"\
					  "<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_prtfuncfg.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"func_cfg"));                       
                      fprintf(cgiOut,"</tr>");                    
					}
					//add by sjw  2008-10-9 17:14:37  for  subinterface
					fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_subintf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lcon,"title_subintf"));  					

					if(retu==0)
					{
					fprintf(cgiOut,"<tr height=25>"\
						  "<td align=left id=tdleft><a href=wp_interface_bindip.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lpublic,"config_interface"));	
					}
					fprintf(cgiOut,"<tr height=25>"\
  					    "<td align=left id=tdleft><a href=wp_all_interface.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",m,search(lpublic,"menu_san"),search(lcon,"interface"));				
				
				ccgi_dbus_init();		 //初始化dbus
				limit=count_eth_port_num();
 			  	limit-=3;
				if(retu==1)  /*普通用户*/
				  limit+=4;
				for(i=0;i<limit;i++)
	            {
  				  fprintf(cgiOut,"<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
	            }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=center valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-top:20px\">");
		    fprintf(cgiOut,"<table width=743 border=1 bordercolor=#cccccc cellspacing=0 cellpadding=0>");
              fprintf(cgiOut,"<tr align=center height=30 bgcolor=#eaeff9 style=font-size:13px>"\
              "<td width=60>%s</td>",search(lcon,"port_no"));
              fprintf(cgiOut,"<td width=60>%s</td>",search(lcon,"port_type"));
              fprintf(cgiOut,"<td width=60>%s</td>",search(lcon,"admin_status"));
              fprintf(cgiOut,"<td width=50>%s</td>",search(lcon,"link_status"));
              //fprintf(cgiOut,"<td width=50>%s</td>",search(lcon,"auto_nag"));
              fprintf(cgiOut,"<td width=50>%s</td>",search(lcon,"an_speed"));
	          fprintf(cgiOut,"<td width=60>%s</td>",search(lcon,"an_duplex"));
	          fprintf(cgiOut,"<td width=50>%s</td>",search(lcon,"an_flowctrl"));
              fprintf(cgiOut,"<td width=50>%s</td>",search(lcon,"duplex"));
              fprintf(cgiOut,"<td width=50>%s</td>",search(lcon,"flow_ctrl"));
              fprintf(cgiOut,"<td width=50>%s</td>",search(lcon,"back_pres"));
              fprintf(cgiOut,"<td width=50>%s</td>",search(lcon,"speed"));
			  fprintf(cgiOut,"<td width=60>%s</td>",search(lcon,"pre_media"));
              fprintf(cgiOut,"<td width=80>%s</td>",search(lcon,"mtu"));
   		      fprintf(cgiOut,"<td width=13>&nbsp;</td>");
            fprintf(cgiOut,"</tr>");
			k=1;
			ccgi_dbus_init();	
			ret=show_ethport_list(&head,&num);
			p=head.next;
			if(p!=NULL)
			{
				while(p!=NULL)
				{
					pp=p->port.next;
					while(pp!=NULL)
					{
                      fprintf(cgiOut,"<tr align=center height=25 bgcolor=%s style=font-size:12px>",setclour(cl));
                      fprintf(cgiOut,"<td>%d-%d</td>",p->slot_no,pp->port_no);
                      fprintf(cgiOut,"<td>%s</td>",eth_port_type_str[pp->porttype]);
				      fprintf(cgiOut,"<td>%s</td>",onoff_status_str[(pp->attr_map & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT]);
				      fprintf(cgiOut,"<td>%s</td>",link_status_str[(pp->attr_map& ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);
				      //fprintf(cgiOut,"<td>%s</td>",doneOrnot_status_str[(pp->attr_map & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT]);
				      fprintf(cgiOut,"<td>%s</td>",onoff_status_str[(pp->attr_map & ETH_ATTR_AUTONEG_SPEED) >> ETH_AUTONEG_SPEED_BIT]);
				      fprintf(cgiOut,"<td>%s</td>",onoff_status_str[(pp->attr_map & ETH_ATTR_AUTONEG_DUPLEX) >> ETH_AUTONEG_DUPLEX_BIT]);
					  fprintf(cgiOut,"<td>%s</td>",onoff_status_str[(pp->attr_map & ETH_ATTR_AUTONEG_FLOWCTRL) >> ETH_AUTONEG_FLOWCTRL_BIT]);
					  if(ETH_ATTR_LINKUP == ((pp->attr_map & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT))
					  {
						  fprintf(cgiOut,"<td>%s</td>",duplex_status_str[(pp->attr_map & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT]);
						  fprintf(cgiOut,"<td>%s</td>",onoff_status_str[(pp->attr_map & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT]);
					  }
					  else
					  {
						  fprintf(cgiOut,"<td>-</td>");
						  fprintf(cgiOut,"<td>-</td>");
					  }
				      fprintf(cgiOut,"<td>%s</td>",onoff_status_str[(pp->attr_map & ETH_ATTR_BACKPRESSURE) >> ETH_BACKPRESSURE_BIT]);
					  if(ETH_ATTR_LINKUP == ((pp->attr_map & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT))
					  {
						  fprintf(cgiOut,"<td>%s</td>",eth_speed_str[(pp->attr_map & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT]);				
					  }
					  else
					  {
						  fprintf(cgiOut,"<td>-</td>");
					  }
					  if(((pp->attr_map & ETH_ATTR_PREFERRED_COPPER_MEDIA) >> ETH_PREFERRED_COPPER_MEDIA_BIT))
						fprintf(cgiOut,"<td>COPPER</td>");	 
					  else if(((pp->attr_map & ETH_ATTR_PREFERRED_FIBER_MEDIA) >> ETH_PREFERRED_FIBER_MEDIA_BIT))
						fprintf(cgiOut,"<td>FIBER</td>"); 
					  else
						fprintf(cgiOut,"<td>-</td>");				  

					  if(ETH_ATTR_LINKUP == ((pp->attr_map & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT))
					  {
						  fprintf(cgiOut,"<td>%d</td>",pp->mtu);
					  }
					  else
					  {
						  fprintf(cgiOut,"<td>-</td>");
					  }
					  memset(menu,0,15);
				      strcat(menu,"menuLists");
		              sprintf(menu_id,"%d",k); 
		              strcat(menu,menu_id);
				      memset(prt_no,0,10);
				      sprintf(prt_no,"%d-%d",p->slot_no,pp->port_no);     /*int转成char*/
				      fprintf(cgiOut,"<td align=left>"\
				 	              "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(Max_Port_Num-k),menu,menu);
                                   fprintf(cgiOut,"<img src=/images/detail.gif>"\
                                   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
                                   fprintf(cgiOut,"<div id=%s>",search(lpublic,"div1"));
								   fprintf(cgiOut,"<div id=%s onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_prtarp.cgi?UN=%s&ID=%s target=mainFrame>ARP %s</a></div>",search(lpublic,"div2"),m,prt_no,search(lpublic,"survey"));
								   if(retu==0)	/*管理员*/
								   {
								     fprintf(cgiOut,"<div id=%s onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>%s</a></div>",search(lpublic,"div2"),m,prt_no,search(lcon,"prt_cfg"));
								     fprintf(cgiOut,"<div id=%s onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_prtfuncfg.cgi?UN=%s&ID=%s target=mainFrame>%s</a></div>",search(lpublic,"div2"),m,prt_no,search(lcon,"func_cfg"));
								   }
                                   fprintf(cgiOut,"</div>"\
                                   "</div>"\
                                   "</div>"\
				 	   "</td>");
				      fprintf(cgiOut,"</tr>");
                      cl=!cl;
					  k++;
					  pp=pp->next;
					}
					p=p->next;
				}
			 }
           fprintf(cgiOut,"</table>"\
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
free(menu_id);
free(menu);	
free(prt_no);
if((ret==0)&&(num>0))
{
	Free_ethslot_head(&head);
}
}          
