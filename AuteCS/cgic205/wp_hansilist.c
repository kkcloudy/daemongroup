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
* wp_hansilist.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for vrrp list
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

int ShowhansilistPage(char *m,char *n,struct list *lpublic,struct list *lcontrol);    


int cgiMain()
{  
  char *encry=(char *)malloc(BUF_LEN);  
  char *str;   
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lcontrol;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcontrol=get_chain_head("../htdocs/text/control.txt");
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
    str=dcryption(encry);
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else 
	  ShowhansilistPage(encry,str,lpublic,lcontrol);
  }
  else                    
  {      
   cgiFormStringNoNewlines("encry_newvrrp",encry,BUF_LEN);
	str=dcryption(encry);	
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else 
	  ShowhansilistPage(encry,str,lpublic,lcontrol);
  } 
  free(encry);
  release(lpublic);  
  release(lcontrol);
  return 0;
}

int ShowhansilistPage(char *m,char *n,struct list *lpublic,struct list *lcontrol)
{  
  int i = 0,cl=1; 
  int ret=-1;

  Z_VRRP zvrrp;
  memset(&zvrrp,0,sizeof(zvrrp));

  char hspro[10];
  memset(hspro,0,10);

  int retu;

  char *menu_id=(char *)malloc(10);
  char *menu=(char *)malloc(15);
  char paramalert[128];
  memset(paramalert,0,128);
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>VRRP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
    "<style>"\
    "#div1{ width:82px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:80px; height:15px; padding-left:5px; padding-top:3px}"\
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
    "<body>");


	fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>VRRP</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=hansi_list style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_er></font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"hs_list"));   /*突出显示*/
					fprintf(cgiOut,"</tr>");


					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_hansiadd.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"hs_create"));
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
              "<table width=720 border=0 cellspacing=0 cellpadding=0>");

               
				/////////////////////////
				fprintf(cgiOut,"<tr>"\
			    "<td align=center valign=top style=\"padding-top:5px; padding-bottom:10px\">");
				fprintf(cgiOut,"<table width=720 valign=top border=0 cellspacing=0 cellpadding=0>");
					fprintf(cgiOut,"<tr>"\
					"<td colspan=2 id=sec style=\"border-bottom:2px solid #53868b\">&nbsp;</td>"\
					"</tr>"\
					"<tr>");

					fprintf(cgiOut,"<td align=left colspan=2>");     

					fprintf(cgiOut,"<table frame=below valign=top rules=rows width=720 border=1>"\
					"<tr align=left>");
				    fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),"ID");
					fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"hs_state"));
					fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"hs_uplink"));
					fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),"IP");
					fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"hs_dlink"));
					fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),"IP");
                    fprintf(cgiOut,"<th width=80><font id=%s>%s IFNAME</font></th>",search(lpublic,"menu_thead"),search(lpublic,"hs_hblink"));
					fprintf(cgiOut,"<th width=80><font id=%s>%s IP</font></th>",search(lpublic,"menu_thead"),search(lpublic,"hs_hblink"));
					fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"prior"));
					fprintf(cgiOut,"<th width=80></th>");
					fprintf(cgiOut,"</tr>");

                    ccgi_dbus_init();
                    for(i=1;i<17;i++)
                	{
	                    memset(&zvrrp,0,sizeof(zvrrp));
	                    ret=ccgi_show_hansi_profile(&zvrrp, i);
						
						if(ret==DCLI_VRRP_RETURN_CODE_OK)
	          	        {

							fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
							fprintf(cgiOut,"<td>%d</td>",i);
							fprintf(cgiOut,"<td>%s</td>",zvrrp.state);
							fprintf(cgiOut,"<td>%s</td>",zvrrp.uplink_ifname);
							fprintf(cgiOut,"<td>%s</td>",zvrrp.uplink_ip);
							fprintf(cgiOut,"<td>%s</td>",zvrrp.downlink_ifname);
							fprintf(cgiOut,"<td>%s</td>",zvrrp.downlink_ip);
						    fprintf(cgiOut,"<td>%s</td>",zvrrp.hbinf);
							fprintf(cgiOut,"<td>%s</td>",zvrrp.hbip);
							fprintf(cgiOut,"<td>%d</td>",zvrrp.priority);

							memset(menu,0,15);
						    strcat(menu,"menuLists");
							sprintf(menu_id,"%d",i+1); 
							strcat(menu,menu_id);
				  
							

							retu=checkuser_group(n);
							if(retu==0)/*管理员*/
							{
									fprintf(cgiOut,"<td>"\
									"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(17-i),menu,menu);
									fprintf(cgiOut,"<img src=/images/detail.gif>"\
									"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
									fprintf(cgiOut,"<div id=div1>");
									fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_hansimod.cgi?UN=%s&ID=%d&TYPE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,i,"2",search(lpublic,"confirm_delete"),search(lpublic,"delete"));                             
									fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_hansimod.cgi?UN=%s&ID=%d&TYPE=%s target=mainFrame>%s</a></div>",m,i,"1",search(lpublic,"configure"));
									fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_hsdetail.cgi?UN=%s&ID=%d&TYPE=%s target=mainFrame>%s</a></div>",m,i,"1",search(lpublic,"details"));
									fprintf(cgiOut,"</div>"\
									"</div>"\
									"</div>"\
									"</td>");
							}
							
							
							fprintf(cgiOut,"</tr>");
							cl =!cl;
		               }
            	    }
				    
					fprintf(cgiOut,"</table>");
					fprintf(cgiOut,"</td></tr>");
					fprintf(cgiOut,"</table>");
	fprintf(cgiOut,"</td>"\
  "</tr>");
  fprintf(cgiOut,"<tr>"\
    "<td colspan=3><input type=hidden name=encry_newvrrp value=%s></td>",m);
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
 
free(menu_id);
free(menu);
 
return 0;
}






