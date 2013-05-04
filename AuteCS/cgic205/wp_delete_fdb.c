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
* wp_delete_fdb.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for delete fdb
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
#include "ws_fdb.h"
#include "ws_dcli_portconf.h"



int ShowAddBlacklistPage(struct list *lpublic,struct list *lcon);

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcon;	   /*解析control.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcon=get_chain_head("../htdocs/text/control.txt");

    ShowAddBlacklistPage(lpublic,lcon);
	release(lpublic);  
	release(lcon);

 	return 0;
}

int ShowAddBlacklistPage(struct list *lpublic,struct list *lcon)
{
	char *encry=(char *)malloc(BUF_LEN);				/*存储从wp_usrmag.cgi带入的加密字符串*/
	char *str;
	
	char fdb_encry[BUF_LEN]; 
	char vlanid[N],portno[N],trunk[N],showtype[N];

	int i,ret;

    ETH_SLOT_LIST  head,*p;
    ETH_PORT_LIST *pp;
    int result,p_num;

	char paramalert[128];
    memset(paramalert,0,128);

	if((cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_del") != cgiFormSuccess))
	{
		memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
		   ShowErrorPage(search(lpublic,"ill_user"));	 /*用户非法*/
		   return 0;
		}
		//memset(fdb_encry,0,BUF_LEN); 				  /*清空临时变量*/
	}
	memset(fdb_encry,0,BUF_LEN); 				  /*清空临时变量*/
	cgiFormStringNoNewlines("fdb_encry",fdb_encry,BUF_LEN);
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lcon,"fdb_man"));
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	  "<style type=text/css>"\
	  ".a3{width:30;border:0; text-align:center}"\
	  "</style>"\
	"</head>"\
	"<script src=/ip.js>"\
	"</script>"\
	"<body>");


	if(cgiFormSubmitClicked("submit_del") == cgiFormSuccess)		//删除黑名单fdb
	{
		memset(vlanid,0,N);
		memset(portno,0,N);
		memset(trunk,0,N);
		memset(showtype,0,N);
		cgiFormStringNoNewlines("showtype",showtype,N);	
		
		
		if(strcmp(showtype,"1")==0)
		{
			memset(vlanid,0,N);
			cgiFormStringNoNewlines("vlanid",vlanid,N);	
			if(strcmp(vlanid,"")!=0)
			{
				ret=-1;
				ccgi_dbus_init();
				
				ret=delete_fdb_by_vid(vlanid);
				switch(ret)
				{

					case -5:
					ShowAlert(search(lcon,"vlan_out_range"));
					break;

					case -1:
					ShowAlert(search(lpublic,"oper_fail"));
					break;

					case NPD_FDB_ERR_VLAN_NONEXIST:
					ShowAlert(search(lcon,"vlan_not_exist"));
					break;

					case 0 :
					ShowAlert(search(lpublic,"oper_succ"));
					break;

					default:
					ShowAlert(search(lpublic,"oper_fail"));
					break;

				}

			}
			else
			{

				memset(paramalert,0,128);
				sprintf(paramalert,"Vlan id %s",search(lpublic,"param_null"));
				ShowAlert(paramalert);

			}
				
		}

		
		if(strcmp(showtype,"2")==0)
		{
			memset(trunk,0,N);
			cgiFormStringNoNewlines("trunkid",trunk,N);
			if(strcmp(trunk,"")!=0)
			{
	             ret=-1;
				 ccgi_dbus_init();

				 ret=delete_fdb_by_trunk(trunk);
				 switch(ret)
				 {
					
					case -1:
					ShowAlert(search(lpublic,"oper_fail"));
					break;
					
					case 0 :
					ShowAlert(search(lpublic,"oper_succ"));
					break;

					default:
					ShowAlert(search(lpublic,"oper_fail"));
					break;

				}

			}
			else
			{

				memset(paramalert,0,128);
				sprintf(paramalert,"Trunk id %s",search(lpublic,"param_null"));
				ShowAlert(paramalert);
			}
		}

		if(strcmp(showtype,"3")==0)
		{
			memset(portno,0,N);
			ccgi_dbus_init();


			cgiFormStringNoNewlines("portno",portno,N);
			ret=-1;

			ret=delete_fdb_by_port(portno);
			
			switch(ret)
			{

				case -1:
				ShowAlert(search(lpublic,"oper_fail"));
				break;
					
				case 0 :
				ShowAlert(search(lpublic,"oper_succ"));
				break;

				default:
				ShowAlert(search(lpublic,"oper_fail"));
				break;

			}
		}
		
	}
	fprintf(cgiOut,"<form method=post>"\
	"<div align=center>"\
	"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
	"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>FDB</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
		
			
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_fdb style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if((cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_del") != cgiFormSuccess))
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else										   
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",fdb_encry,search(lpublic,"img_cancel"));
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
						if((cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_del") != cgiFormSuccess))
						{
							fprintf(cgiOut,"<tr height=25>"\
    						"<td align=left id=tdleft><a href=wp_show_fdb.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"show_fdb"));   /*突出显示*/
    						fprintf(cgiOut,"</tr>");
    						fprintf(cgiOut,"<tr height=25>"\
    						  "<td align=left id=tdleft><a href=wp_config_agingtime.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"fdb_age"));
    						fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							  "<td align=left id=tdleft><a href=wp_add_static_fdb.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> FDB</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"fdb_sta"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							  "<td align=left id=tdleft><a href=wp_add_blacklist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"fdb_bla"));
							fprintf(cgiOut,"</tr>");
							
							fprintf(cgiOut,"<tr height=25>"\
    						"<td align=left id=tdleft><a href=wp_config_fdb_limit.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"fdb_limit"));
    						fprintf(cgiOut,"</tr>");
						
							//add new web page for delete fdb
							fprintf(cgiOut,"<tr height=26>"\
							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s FDB</font></td>",search(lpublic,"menu_san"),search(lcon,"del"));
							fprintf(cgiOut,"</tr>");
						}
						else //if(cgiFormSubmitClicked("submit_fdb") == cgiFormSuccess)
						{
							fprintf(cgiOut,"<tr height=25>"\
    						  "<td align=left id=tdleft><a href=wp_show_fdb.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"show_fdb"));   /*突出显示*/
    						fprintf(cgiOut,"</tr>");
    						fprintf(cgiOut,"<tr height=25>"\
    						  "<td align=left id=tdleft><a href=wp_config_agingtime.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"fdb_age"));
    						fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							  "<td align=left id=tdleft><a href=wp_add_static_fdb.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> FDB</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"fdb_sta"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							  "<td align=left id=tdleft><a href=wp_add_blacklist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"fdb_bla"));
							fprintf(cgiOut,"</tr>");
							
							fprintf(cgiOut,"<tr height=25>"\
    						  "<td align=left id=tdleft><a href=wp_config_fdb_limit.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"fdb_limit"));
    						fprintf(cgiOut,"</tr>");

							//add new web page for delete fdb
							fprintf(cgiOut,"<tr height=26>"\
							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s FDB</font></td>",search(lpublic,"menu_san"),search(lcon,"del"));
							fprintf(cgiOut,"</tr>");
						}
						for(i=0;i<4;i++)
						{
							fprintf(cgiOut,"<tr height=25>"\
							  "<td id=tdleft>&nbsp;</td>"\
							"</tr>");
						}
	
					  fprintf(cgiOut,"</table>"\
				  "</td>"\
				  "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
						  "<table width=700 border=0 cellspacing=0 cellpadding=0>"\
														"<tr>"\
														  "<td align=left valign=top  style=\"padding-top:18px\">");
															fprintf(cgiOut,"<table border=0 cellspacing=0 cellpadding=0>");
															//add blacklist
															fprintf(cgiOut,"<tr height=30>");
															fprintf(cgiOut,"<td><input type=\"radio\" name=\"showtype\" value=\"1\"  checked>%s:</td>",search(lcon,"vID"));
															fprintf(cgiOut,"<td width=140><input type=text name=vlanid size=21 maxLength=4></td>"\
															"<td><font color=red>(1--4094)</font></td>");
															
															fprintf(cgiOut,"<tr height=30>");
															fprintf(cgiOut,"<td><input type=\"radio\" name=\"showtype\" value=\"2\">%s:</td>","Trunk ID");
															fprintf(cgiOut,"<td width=140><input type=text name=trunkid size=21 maxLength=3></td>"\
															"<td><font color=red>%s</font></td>","(1--127)");

															fprintf(cgiOut,"<tr height=30>");
															fprintf(cgiOut,"<td><input type=\"radio\" name=\"showtype\" value=\"3\" >%s:</td>",search(lcon,"_port"));
															fprintf(cgiOut,"<td width=70><select name=portno style=width:138px>");
															ccgi_dbus_init();
															///////////////不用此函数，统统修改为 show_eth_port_list之类的
															result=show_ethport_list(&head,&p_num);
															p=head.next;
															if(p!=NULL)
															{
																while(p!=NULL)
																{
																	pp=p->port.next;
																	while(pp!=NULL)
																	{
																		if(p->slot_no!=0)
																		{                        					
																			fprintf(cgiOut,"<option value=%d/%d>%d/%d</option>",p->slot_no,pp->port_no,p->slot_no,pp->port_no);
																		}
																		pp=pp->next;
																	}
																	p=p->next;
																}
															}
															///////////////
															fprintf(cgiOut,"</select></td>"\
															"<td></td>");

															fprintf(cgiOut,"</table>");
															fprintf(cgiOut,"<table border=0 cellspacing=0 cellpadding=0 style=\"padding-top:18px\">");
															fprintf(cgiOut,"<tr height=30>");
															fprintf(cgiOut,"<td width=60><input type=submit style= height:22px;width=60  border=0 name=submit_del  value=\"%s\"></td>",search(lcon,"del"));
															fprintf(cgiOut,"</table>");

															fprintf(cgiOut,"</td>"\
															"</tr>"\
															"<tr>");
        													  if((cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)&&(cgiFormSubmitClicked("submit_del") != cgiFormSuccess))
        													  {
        														fprintf(cgiOut,"<td><input type=hidden name=fdb_encry value=%s></td>",encry);
        													  }
        													  else //if(cgiFormSubmitClicked("submit_fdb") == cgiFormSuccess)
    														  { 			 
    															fprintf(cgiOut,"<td><input type=hidden name=fdb_encry value=%s></td>",fdb_encry);
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
	
	if((result==0)&&(p_num>0))
    {
    	Free_ethslot_head(&head);
    }	
																 
	return 0;

}





