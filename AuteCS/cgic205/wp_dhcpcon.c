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
* wp_dhcpcon.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system cotrl for dhcp config
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

#include "ws_dcli_dhcp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

#define IP_ADDR_PATH "/var/run/apache2/ip_addr.file"

int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic);
void ShowIPaddr(struct list *lcontrol,struct list *lpublic,char* addn);


int cgiMain()
{
	char *tmp=(char *)malloc(64);
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	
	ccgi_dbus_init();
	
	ShowDhcpconPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	free(tmp);
	
	return 0;
}



int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic)
{ 
  char *encry=(char *)malloc(BUF_LEN);
  char *str;
 
  struct dhcp_pool_show_st head,*q;
  memset(&head,0,sizeof(struct dhcp_pool_show_st));
  
  struct dhcp_sub_show_st *pq;
  
  unsigned int mode = 0, index = 0, count = 0;	
  int cflag=-1;
  
  char menu[21]="menulist";
  char i_char[10];
  
  char dhcp_encry[BUF_LEN]; 
  char addn[N];    
  int i = 0,j = 0;   
  int cl=1;
  
  struct dhcp_global_show_st global_show;
  memset(&global_show,0,sizeof(struct dhcp_global_show));
  global_show.domainname = (char *)malloc(30);
  global_show.option43 = (char *)malloc(256);
  memset(global_show.domainname,0,30);
  memset(global_show.option43,0,256);

  
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
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	  "<style type=text/css>"\
  	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
	  ".dhcplis {overflow-x:hidden;	overflow:auto; width: 716px; height: 420px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
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
	if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
	{
		ShowIPaddr(lcontrol,lpublic,addn);
	}

   
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
          if(checkuser_group(addn)==0)  /*管理员*/
          {
				 fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=dhcp_conf style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		  }
		  else
		  {
          	if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpsumary.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		   else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpsumary.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,search(lpublic,"img_ok"));	  
		  }
		  if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpsumary.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_dhcpsumary.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,search(lpublic,"img_cancel"));
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
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>DHCP</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
         		if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
         		{ 		
         			 if(checkuser_group(addn)==0)  /*管理员*/
					{
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_dhcpadd.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
						fprintf(cgiOut,"</tr>");         						
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_dhcpinter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"interface_info"));
						fprintf(cgiOut,"</tr>"); 
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_dhcpmac.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"bind"));
						fprintf(cgiOut,"</tr>"); 
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_dhcplease.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"details"));
						fprintf(cgiOut,"</tr>"); 
						//////////
						/*fprintf(cgiOut,"<tr height=25>"\
         				"<td align=left id=tdleft><a href=wp_dhcp_opt.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),"option");
						fprintf(cgiOut,"</tr>"); */

					}
				}
         		else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess) 			  
         		{			
         			if(checkuser_group(addn)==0)  /*管理员*/		   
         			{
	         			fprintf(cgiOut,"<tr height=25>"\
	         			"<td align=left id=tdleft><a href=wp_dhcpadd.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
	         			fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
	         			"<td align=left id=tdleft><a href=wp_dhcpinter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"interface_info"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
	         			"<td align=left id=tdleft><a href=wp_dhcpmac.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"bind"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
	         				"<td align=left id=tdleft><a href=wp_dhcplease.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"details"));
	         			fprintf(cgiOut,"</tr>");

						
						/////////////////////	
	                   /* fprintf(cgiOut,"<tr height=25>"\
	         				"<td align=left id=tdleft><a href=wp_dhcp_opt.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),"option");
	         			fprintf(cgiOut,"</tr>");
						*/
					//	fprintf(cgiOut,"<tr height=25>"\
         			//	"<td align=left id=tdleft><a href=wp_dhcpview.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"stat"));
         			//    fprintf(cgiOut,"</tr>");
					}
				}
			
			for(i=0;i<15;i++)
			{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
			}

				
				 
			fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");

				
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
		   	"var url = 'wp_dhcpcon.cgi?UN=%s&allslotid='+slotid;\n"\
		   	"window.location.href = url;\n"\
		   	"}\n", encry);
		    fprintf( cgiOut,"</script>\n" );	
			free_instance_parameter_list(&paraHead2);	
			fprintf(cgiOut,"<input type=hidden name=allslotid value=\"%d\">",allslot_id);

			ccgi_show_ip_dhcp_server(&global_show,allslot_id);
			
			fprintf(cgiOut,"<tr style='padding-top:10px'>"\
			"<td width=80>%s</td>",search(lcontrol,"dhcp_status"));
			if( global_show.enable == 1 )
			{
				fprintf(cgiOut,"<td width=420 align=left>");
				fprintf(cgiOut,"<select name=State onchange=StateChange(this)>");
				fprintf(cgiOut,"<option>stop</option>"\
				"<option selected=\"selected\">start</option>"\
				"</select>"\
				"</td>");
			}
			else
			{
				fprintf(cgiOut,"<td width=420 align=left>");
				fprintf(cgiOut,"<select name=State onchange=StateChange(this)>");
				fprintf(cgiOut,"<option >start</option>"\
				"<option selected=\"selected\">stop</option>"\
				"</select>"\
				"</td>");				
			}


			fprintf(cgiOut,"</tr>"\
			"<tr>"\
			"<td colspan=2 id=sec style=\"border-bottom:2px solid #53868b\">&nbsp;</td>"\
			"</tr>"\
			"<tr>"\
			"<td colspan=2 style=\"padding-top:20px\">");
			fprintf(cgiOut,"<div class=dhcplis>\n");
			fprintf(cgiOut,"<table width=700 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
			fprintf(cgiOut,"<tr >");
			//fprintf(cgiOut,"<th width=150 align=left style=font-size:14px>%s</th>",search(lpublic,"dhcp_rname"));
			fprintf(cgiOut,"<th width=150 align=left style=font-size:14px>%s</th>","POOL");
			fprintf(cgiOut,"<th width=150 align=left style=font-size:14px>%s</th>",search(lcontrol,"ip_start"));
			fprintf(cgiOut,"<th width=150 align=left style=font-size:14px>%s</th>",search(lcontrol,"ip_end"));
			fprintf(cgiOut,"<th width=150 align=left style=font-size:14px>%s</th>",search(lcontrol,"mask"));
			fprintf(cgiOut,"<th width=80 >&nbsp;</th>");
			fprintf(cgiOut,"<th width=80>&nbsp;</th>");
			fprintf(cgiOut,"</tr>");
            cflag=ccgi_show_ip_pool(mode, index, &head, &count,allslot_id);
			char tmp2[32];
			////////////////////////////////////////////modify
			 if(cflag==1)
			 {
				q = head.next;
				while( q != NULL )
				{
				    pq = q->sub_show.next;
					while( pq != NULL)
					{
						fprintf(cgiOut,"<tr bgcolor=%s height=25>\n",setclour(cl));
						fprintf(cgiOut,"<td width=150 style=font-size:14px align=left>%s</td>", q->poolname);
						memset(tmp2, 0, 32);
						fprintf(cgiOut,"<td width=150 style=font-size:14px align=left>%s</td>", pq->iplow ? ip_long2string(pq->iplow, tmp2) : "");
						memset(tmp2, 0, 32);
						fprintf(cgiOut,"<td width=150 style=font-size:14px align=left>%s</td>", pq->iphigh ? ip_long2string(pq->iphigh, tmp2) : "");
						memset(tmp2, 0, 32);
						fprintf(cgiOut,"<td width=150 style=font-size:14px align=left>%s</td>",pq->mask ? ip_long2string(pq->mask, tmp2) : "");
						if(checkuser_group(addn)==0)  /*管理员*/
						{
							memset(menu,0,21);
							memset(i_char,0,10);
							strcpy(menu,"menulist");
							sprintf(i_char,"%d",j+1);
							strcat(menu,i_char);
							
							if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
							{
								//fprintf(cgiOut,"<td  align=left><a href=wp_dhcpedit.cgi?UN=%s&NAME=%s&TYPE=%s target=mainFrame><font color=black>%s</font></td>",encry,poolshow[j].poolname,"1",search(lcontrol,"edit"));			
								//fprintf(cgiOut,"<td  align=left><a href=wp_dhcpedit.cgi?UN=%s&NAME=%s&TYPE=%s target=mainFrame><font color=black>%s</font></td>",encry,poolshow[j].poolname,"2",search(lcontrol,"del"));
								fprintf(cgiOut,"<td colspan=2>");
								fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(500-j),menu,menu);
								fprintf(cgiOut,"<img src=/images/detail.gif>"\
								"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
								fprintf(cgiOut,"<div id=div1>");
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_dhcpedit.cgi?UN=%s&NAME=%s&TYPE=%s&allslotid=%d target=mainFrame>%s</a></div>",encry,q->poolname,"1",allslot_id,search(lcontrol,"edit"));
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_dhcpedit.cgi?UN=%s&NAME=%s&TYPE=%s&allslotid=%d target=mainFrame>%s</a></div>",encry,q->poolname,"2",allslot_id,search(lcontrol,"del"));
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_dhcpdta.cgi?UN=%s&NAME=%s&TYPE=%s&allslotid=%d target=mainFrame>%s</a></div>",encry,q->poolname,"3",allslot_id,search(lcontrol,"qos_map_detail"));
								fprintf(cgiOut,"</div>"\
								"</div>"\
								"</div>");
								fprintf(cgiOut,"</td>");
							}
							else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
							{
								//fprintf(cgiOut,"<td  align=left><a href=wp_dhcpedit.cgi?UN=%s&NAME=%s&TYPE=%s target=mainFrame><font color=black>%s</font></td>",dhcp_encry,poolshow[j].poolname,"1",search(lcontrol,"edit"));			
								//fprintf(cgiOut,"<td  align=left><a href=wp_dhcpedit.cgi?UN=%s&NAME=%s&TYPE=%s target=mainFrame><font color=black>%s</font></td>",dhcp_encry,poolshow[j].poolname,"2",search(lcontrol,"del"));
								fprintf(cgiOut,"<td colspan=2>");
								fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(500-j),menu,menu);
								fprintf(cgiOut,"<img src=/images/detail.gif>"\
								"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
								fprintf(cgiOut,"<div id=div1>");
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_dhcpedit.cgi?UN=%s&NAME=%s&TYPE=%s&allslotid=%d target=mainFrame>%s</a></div>",dhcp_encry,q->poolname,"1",allslot_id,search(lcontrol,"edit"));
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_dhcpedit.cgi?UN=%s&NAME=%s&TYPE=%s&allslotid=%d target=mainFrame>%s</a></div>",dhcp_encry,q->poolname,"2",allslot_id,search(lcontrol,"del"));
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_dhcpdta.cgi?UN=%s&NAME=%s&TYPE=%s&allslotid=%d target=mainFrame>%s</a></div>",dhcp_encry,q->poolname,"3",allslot_id,search(lcontrol,"qos_map_detail"));
								fprintf(cgiOut,"</div>"\
								"</div>"\
								"</div>");
								fprintf(cgiOut,"</td>");
							}
						}
						else
						{
							if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
							{
								fprintf(cgiOut,"<td style=font-size:14px align=left></td>");			
								fprintf(cgiOut,"<td style=font-size:14px align=left></td>");			
							}
							else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
							{
								fprintf(cgiOut,"<td style=font-size:14px align=left></td>");			
								fprintf(cgiOut,"<td style=font-size:14px align=left></td>");			
							}

						}
						fprintf(cgiOut,"</tr>");
						cl=!cl;	
						j++;
						pq = pq->next;
						
					}

					q = q->next;
				}				
		 	}			 

			if( cflag == 1 )
			{
				Free_show_dhcp_pool(&head);
			}
			////////////////////////////////////////////
			fprintf(cgiOut,"</table>");
			fprintf(cgiOut,"</div>\n");
			fprintf(cgiOut,"</td>"\
			"</tr>"
			"<tr>");
			if(cgiFormSubmitClicked("dhcp_conf") != cgiFormSuccess)
			{
				fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_dhcp value=%s></td>",encry);
			}
			else if(cgiFormSubmitClicked("dhcp_conf") == cgiFormSuccess)
			{
				fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_dhcp value=%s></td>",dhcp_encry);
				fprintf( cgiOut,"<script type=text/javascript>\n");
					   	fprintf( cgiOut,"function slotid_change( obj )\n"\
					   	"{\n"\
					   	"var slotid = obj.options[obj.selectedIndex].value;\n"\
					   	"var url = 'wp_dhcplease.cgi?UN=%s&allslotid='+slotid;\n"\
					   	"window.location.href = url;\n"\
					   	"}\n", dhcp_encry);
						fprintf(stderr,"encry!!!!!!!!8888!!!!!!!!!!!=%s\n",dhcp_encry);
					    fprintf( cgiOut,"</script>\n" );
			}
			fprintf(cgiOut,"</tr>"\
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

free(encry);
free(global_show.domainname);
free(global_show.option43);
return 0;
}
						 
void  ShowIPaddr(struct list *lcontrol,struct list *lpublic,char*addn)
{
	if(checkuser_group(addn)==0)
	{
		char * status = (char *)malloc(10);	
		memset(status,0,10);		
		cgiFormStringNoNewlines("State",status,10);
		char allslot_str[10] = {0};
		int allslot_id = 0;
		cgiFormStringNoNewlines("allslot",allslot_str,sizeof(allslot_str));
		allslot_id = atoi(allslot_str);
		int sflag = -1;
		ccgi_dbus_init();
		if(!strcmp(status,"start"))
		{	
			sflag=ip_dhcp_server_enable("enable",allslot_id);		
            if(sflag==1)
            {
				ShowAlert(search(lcontrol,"DHCP_STAT1"));	 
            }
			else
			{
				ShowAlert(search(lcontrol,"DHCP_STAT10"));
			}
		}
		else
		{
			sflag=ip_dhcp_server_enable("disable",allslot_id);	
			if(sflag==1)
			{
				ShowAlert(search(lcontrol,"DHCP_STAT0"));
			}
			else
			{
				ShowAlert(search(lcontrol,"DHCP_STAT01"));
			}

		}
		free(status);
	}
}




