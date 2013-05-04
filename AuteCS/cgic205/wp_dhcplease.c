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
* wp_dhcplease.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system cotrl for dhcp lease
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
#include "ws_returncode.h"
#include "ws_dhcp_conf.h"
#include "ws_dbus_list_interface.h"

int ShowDhcpconPage(struct list *lcontrol,struct list *lpublic);

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
	char *str = NULL;	
	//int ret = 0;
	int i = 0;
	int cl = 1;/*contrl color of the table*/
	char dhcp_encry[BUF_LEN]; 
	fprintf(stderr,"dhcp_encry[BUF_LEN]==%s\n",dhcp_encry);
	char addn[N]; 

    char showtype[10];
	memset(showtype,0,10);
	struct dhcp_lease_st  head,*dq = NULL;
	//struct dhcp_lease_st iphead,*iq = NULL;
	memset(&head,0,sizeof(head));
	//memset(&iphead,0,sizeof(iphead));

	char *leaseip = (char *)malloc(30);
	memset(leaseip,0,30);
	
	char *leasemac = (char *)malloc(30);
	memset(leasemac,0,30);

	char *inputip = (char *)malloc(30);
	memset(inputip,0,30);

	char *inputmac = (char *)malloc(30);
	memset(inputmac,0,30);

    int lnum = 0;
	int ret_show = 0;
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
  fprintf( cgiOut ,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf( cgiOut ,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf( cgiOut ,"<link rel=stylesheet href=/style.css type=text/css>"\
		"<style type=text/css>"\
		"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
		"#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
		"#link{ text-decoration:none; font-size: 12px}"\
		".dhcplis {overflow-x:hidden;	overflow:auto; width: 716px; height: 400px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
		"</style>"\
		"</head>"\
		"<body>");
  if(cgiFormSubmitClicked("dhcp_lease") != cgiFormSuccess)
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	fprintf(stderr,"UN=(dhcp_lease) != cgiFormSuccess==%s\n",encry);
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
	fprintf(stderr,"encry_dhcp=(dhcp_lease) = cgiFormSuccess==%s\n",dhcp_encry);
    str=dcryption(dhcp_encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	strcpy(addn,str);
	memset(dhcp_encry,0,BUF_LEN);                   /*清空临时变量*/
	
  	}
  
  cgiFormStringNoNewlines("encry_dhcp",dhcp_encry,BUF_LEN);
   fprintf(stderr,"encry_dhcp = ss==%s\n",dhcp_encry);
  fprintf( cgiOut ,"<form method=post name=form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"DHCP");
    fprintf( cgiOut ,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	   
	fprintf( cgiOut ,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>");
	if(checkuser_group(addn)==0)  /*管理员*/
	{
		fprintf( cgiOut ,"<td width=62 align=center><input id=but type=submit name=dhcp_lease style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
	}
	else
	{
		if(cgiFormSubmitClicked("dhcp_lease") != cgiFormSuccess)
		{
			fprintf( cgiOut ,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
		}
		else   
		{
			fprintf(stderr,"encry_dhcp = s11s==%s\n",dhcp_encry);
			fprintf( cgiOut ,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,search(lpublic,"img_ok"));	  
		}
	}
	if(cgiFormSubmitClicked("dhcp_lease") != cgiFormSuccess)
	{
		fprintf(stderr,"encry_dhcp = s11s==%s\n",encry);
		fprintf( cgiOut ,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	}
	else      
	{
		fprintf(stderr,"dhcp_encry = s1441s==%s\n",dhcp_encry);
		fprintf( cgiOut ,"<td width=62 align=left><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",dhcp_encry,search(lpublic,"img_cancel"));
	}
	fprintf( cgiOut ,"</tr>"\
		"</table>");
				
		
	fprintf( cgiOut ,"</td>"\
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
         		if(cgiFormSubmitClicked("dhcp_lease") != cgiFormSuccess)
         		{ 			
         			fprintf(stderr,"encry = s1441saaaaaaaa==%s\n",encry);
                     fprintf( cgiOut ,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame class=top><font id=%s>DHCP</font><font id=%s> %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));
         			fprintf( cgiOut ,"</tr>");  
					 fprintf( cgiOut ,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpadd.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
         			fprintf( cgiOut ,"</tr>");
					fprintf( cgiOut ,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpinter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"interface_info"));
					fprintf( cgiOut ,"</tr>");
					fprintf( cgiOut ,"<tr height=25>"\
         				"<td align=left id=tdleft><a href=wp_dhcpmac.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"bind"));
         			    fprintf( cgiOut ,"</tr>"); 
					fprintf( cgiOut ,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></a></td>",search(lpublic,"menu_san"),search(lcontrol,"details")); /*突出显示*/
         			fprintf( cgiOut ,"</tr>"); 

					/*fprintf( cgiOut ,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcp_opt.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),"option");
         			fprintf( cgiOut ,"</tr>"); */
				//	fprintf( cgiOut ,"<tr height=25>"\
     			//"<td align=left id=tdleft><a href=wp_dhcpview.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"stat"));
     			//fprintf( cgiOut ,"</tr>");

					
         		}
         		else if(cgiFormSubmitClicked("dhcp_lease") == cgiFormSuccess) 			  
         		{	
         			fprintf(stderr,"dhcp_encry = s1441saaaaaaaafff==%s\n",dhcp_encry);
         		   fprintf( cgiOut ,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpcon.cgi?UN=%s target=mainFrame class=top><font id=%s>DHCP</font><font id=%s> %s</font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lpublic,"menu_san"),search(lcontrol,"dhcp_con"));
         			fprintf( cgiOut ,"</tr>");  
					 fprintf( cgiOut ,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpadd.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
         			fprintf( cgiOut ,"</tr>");
					fprintf( cgiOut ,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcpinter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"interface_info"));
					fprintf( cgiOut ,"</tr>");
					fprintf( cgiOut ,"<tr height=25>"\
         				"<td align=left id=tdleft><a href=wp_dhcpmac.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"bind"));
         			    fprintf( cgiOut ,"</tr>"); 
					fprintf( cgiOut ,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></a></td>",search(lpublic,"menu_san"),search(lcontrol,"details")); /*突出显示*/
         			fprintf( cgiOut ,"</tr>"); 

					/*fprintf( cgiOut ,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_dhcp_opt.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),"option");
         			fprintf( cgiOut ,"</tr>"); */
					//fprintf( cgiOut ,"<tr height=25>"\
	     			//"<td align=left id=tdleft><a href=wp_dhcpview.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",dhcp_encry,search(lpublic,"menu_san"),search(lcontrol,"stat"));
	     			//fprintf( cgiOut ,"</tr>");
         			
         		}
				
				for(i=0;i<15;i++)
				{
						fprintf( cgiOut ,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
				}
				
				  fprintf( cgiOut ,"</table>"\
               "</td>"\
               "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
				
				fprintf( cgiOut ,"<table width=750 border=0 cellspacing=0 cellpadding=0>");

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
		   	"var url = 'wp_dhcplease.cgi?UN=%s&allslotid='+slotid;\n"\
		   	"window.location.href = url;\n"\
		   	"}\n", encry);
			fprintf(stderr,"encry!!!!!!!!!!!!!!!!!!!=%s\n",encry);
		    fprintf( cgiOut,"</script>\n" );	
			free_instance_parameter_list(&paraHead2);	
			fprintf(cgiOut,"<input type=hidden name=allslotid value=\"%d\">",allslot_id);

				
				#if 0
				fprintf( cgiOut ,"<tr>\n");
				fprintf( cgiOut ,"<td colspan=2>\n");
				fprintf( cgiOut ,"&nbsp;&nbsp;%s:<input type=text name=scontent onKeyDown='if (event.keyCode==13)  return false; '>\n",search(lpublic,"log_key"));
				fprintf( cgiOut ,"<font color=red>&nbsp;( IP/MAC/... )</font></td>\n");
				fprintf( cgiOut ,"</tr>\n");
				#endif
				fprintf( cgiOut ,"<tr height=15><td colspan=2></td></tr>\n");
				
				fprintf( cgiOut ,"<tr>"\
                          "<td colspan=2 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lcontrol,"details"));
                fprintf( cgiOut ,"</tr>");
			    ////////////end  
			    #if 0 
			    fprintf( cgiOut ,"<tr>"\
			    "<td><input type=radio name=showtype value=\"1\" />%s</td>"\
			    "<td><input type=text name=scip value=\"111\" maxLength=30 /></td>"\
			    "</tr>\n",search(lpublic,"dhcp_byip"));
				#endif
				memset(showtype,0,10);
				cgiFormStringNoNewlines("showtype", showtype, 10); 

				
			    fprintf( cgiOut ,"<tr>");
				if(strcmp(showtype,"2")==0)
			    	fprintf( cgiOut ,"<td><input type=radio name=showtype value=\"2\" checked/>%s</td>",search(lpublic,"dhcp_bymac"));
				else
			    	fprintf( cgiOut ,"<td><input type=radio name=showtype value=\"2\"/>%s</td>",search(lpublic,"dhcp_bymac"));
					
			    fprintf( cgiOut ,"<td><input type=text name=scmac value=\"\" maxLength=30 /><font color=red>(xx:xx:xx:xx:xx:xx)</font></td>"\
			    "</tr>\n");
				
			    fprintf( cgiOut ,"<tr>");
				if((strcmp(showtype,"3")==0)||(strcmp(showtype,"")==0))
			    	fprintf( cgiOut ,"<td colspan=2><input type=radio name=showtype value=\"3\" checked/>%s</td>",search(lpublic,"dhcp_byall"));
				else
			    	fprintf( cgiOut ,"<td colspan=2><input type=radio name=showtype value=\"3\"/>%s</td>",search(lpublic,"dhcp_byall"));
			    fprintf( cgiOut ,"</tr>\n");
				fprintf( cgiOut ,"<tr>"\
							   "<td colspan=2 style=\"padding-top:20px\">");				
				fprintf( cgiOut ,"<div class=dhcplis>\n");
				fprintf( cgiOut ,"<table width=700 border=0 frame=below rules=rows  cellspacing=0 bordercolor=#cccccc cellpadding=0>");
				fprintf( cgiOut ,"<tr bgcolor=#eaeff9 style=font-size:14px align=left>"\
				"<th width=300 align=left style=font-size:14px>%s</th>","IP");
				fprintf( cgiOut ,"<th width=300 align=left style=font-size:14px>%s</th>","MAC");
				fprintf( cgiOut ,"</tr>");
				//ret = save_dhcp_lease();
				if(cgiFormSubmitClicked("dhcp_lease") == cgiFormSuccess)
				{
					memset(showtype,0,10);
					cgiFormStringNoNewlines("showtype", showtype, 10); 
					#if 0 
					if( strcmp(showtype,"1") == 0 )
					{
					    memset(inputip,0,30);
						cgiFormStringNoNewlines("scip", inputip, 30); 						
						ret = show_dhcp_lease_by_ip("2.3.3.3", &iphead);	
						iq = iphead.next;
						while(iq  != NULL)
						{
							fprintf( cgiOut ,"<tr bgcolor=%s height=20>",setclour(cl));
							fprintf( cgiOut ,"<td style=font-size:12px>%s</td>",iq->leaseip);
							fprintf( cgiOut ,"<td style=font-size:12px>%s</td>\n",iq->leasemac );
							fprintf( cgiOut ,"</tr>\n");
							cl =  !cl;
							iq = iq->next;
						}
						if( ret == 1 )
							Free_show_dhcp_lease(&iphead);
					}
					#endif
					if( strcmp( showtype,"2" ) == 0 )
					{
					    memset(inputmac,0,30);
						cgiFormStringNoNewlines("scmac", inputmac, 30); 
						if(strcmp(inputmac,"")==0)
						{
							ShowAlert(search(lcontrol,"arg_not_null"));
						}
						else
						{
							show_dhcp_lease_by_mac(inputmac, leaseip, leasemac,allslot_id)	;					
							fprintf( cgiOut ,"<tr height=20>");
							fprintf( cgiOut ,"<td style=font-size:12px>%s</td>",leaseip);
							fprintf( cgiOut ,"<td style=font-size:12px>%s</td>\n",leasemac);
							fprintf( cgiOut ,"</tr>\n");
						}
					}
					if( strcmp( showtype,"3" ) == 0 )
					{
						memset(&head,0,sizeof(head));
						ret_show  = show_dhcp_lease(&head, &lnum,allslot_id);					
						if (1 == ret_show)
						{
							dq = head.next;
							while(dq != NULL)
							{
								fprintf( cgiOut ,"<tr bgcolor=%s height=20>",setclour(cl));
								fprintf( cgiOut ,"<td style=font-size:12px>%s</td>",dq->leaseip);
								fprintf( cgiOut ,"<td style=font-size:12px>%s</td>\n",dq->leasemac );
								fprintf( cgiOut ,"</tr>\n");

								cl =  !cl;
								dq = dq->next;
							}
						}
						if((lnum>0)&&(1 == ret_show))
						{
							Free_show_dhcp_lease(&head);
						}
						
					}
				}
			    fprintf( cgiOut ,"</table></div></td></tr>");
				fprintf( cgiOut ,"<tr>");							
				if(cgiFormSubmitClicked("dhcp_lease") != cgiFormSuccess)
				{
					fprintf(stderr,"encry@@@@@@@@@@@=%s\n",encry);
					fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_dhcp value=%s></td>",encry);
				}
				else if(cgiFormSubmitClicked("dhcp_lease") == cgiFormSuccess)
				{
					fprintf(stderr,"dhcp_encry~~~~~@@@@@@@@@@@=%s\n",dhcp_encry);
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

				fprintf( cgiOut ,"</tr>"\
				"</table>");

             fprintf( cgiOut ,"</td>"\
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
free(leaseip);
free(leasemac);
free(inputip);
free(inputmac);
return 0;
}
						 




