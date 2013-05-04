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
* wp_config_mirror.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for tools mirror config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"
#include "ws_dcli_mirror.h"
#include "ws_dcli_portconf.h"

char *NodeType[] = {
	"Destination",
	"Mirror"
};

char * mirror_type[]={
	"VLAN",
	"ACL",
	"port",
	"FDB"
};


int ShowAddvlanPage(); 
int config_mirror_hand(char * nodeType_select,char * mirror_select,struct list * lcontrol);


int cgiMain()
{
 ShowAddvlanPage();
 return 0;
}

int ShowAddvlanPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	FILE *fp;
	char lan[3];
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	int i,show_mirror=0;
	char cofigmirror_encry[BUF_LEN]; 
	ETH_SLOT_LIST  head,*p;
    ETH_PORT_LIST *pp;	
	char *slot_port=(char *)malloc(10);
	int num,ret=-1;
	char * select_a=(char * )malloc(20);
	memset(select_a,0,20);
	char * select_b=(char * )malloc(20);
	memset(select_b,0,20);

	
	memset(encry,0,BUF_LEN);
	ccgi_dbus_init();
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
	{

	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(cofigmirror_encry,0,BUF_LEN);					 /*清空临时变量*/
	}
   else
   	{
      cgiFormStringNoNewlines("encry_configmirror",cofigmirror_encry,BUF_LEN);
      cgiFormStringNoNewlines("node_type",select_a,20);
      cgiFormStringNoNewlines("mirror_node",select_b,20);
	  
   	}
   cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_configmirror") == cgiFormSuccess)
  {
  	 //fprintf(stderr,"select_a=%s-select_b=%s",select_a,select_b);
     config_mirror_hand(select_a,select_b,lcontrol);
  }

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>MIRROR</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	    if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
		{
			ShowAlert(search(lpublic,"error_open"));
	    }
	    else
	    {
			fseek(fp,4,0);						/*将文件指针移到离文件首4个字节处，即lan=之后*/
			fgets(lan,3,fp);	   
			fclose(fp);
	    }
	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_configmirror style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));  
		  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_command.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_command.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",cofigmirror_encry,search(lpublic,"img_cancel"));
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
            		if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )
            		{
            		  fprintf(cgiOut,"<tr height=25>"\
            		  "<td align=left id=tdleft><a href=wp_mirror_tool.cgi?UN=%s target=mainFrame class=top><font id=%s>%s%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"show"),search(lcontrol,"mirror")); 
            		  fprintf(cgiOut,"</tr>");
            		  fprintf(cgiOut,"<tr height=26>"\
                             "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"config"),search(lcontrol,"mirror"));   //突出显示 
                      fprintf(cgiOut,"</tr>");
					  
            		}
            		else
            		{
            		  fprintf(cgiOut,"<tr height=25>"\
            		  "<td align=left id=tdleft><a href=wp_mirror_tool.cgi?UN=%s target=mainFrame class=top><font id=%s>%s%s</font></a></td>",cofigmirror_encry,search(lpublic,"menu_san"),search(lcontrol,"show"),search(lcontrol,"mirror")); 
            		  fprintf(cgiOut,"</tr>");
            		  fprintf(cgiOut,"<tr height=26>"\
                             "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"config"),search(lcontrol,"mirror"));   //突出显示 
                      fprintf(cgiOut,"</tr>");
					  
            		}
					for(i=0;i<7;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=115 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>"\
						  "<table width=260 border=0 cellspacing=0 cellpadding=0  style=padding-top:18px>");
				  			fprintf(cgiOut,"<tr align=left>");
							fprintf(cgiOut,"<td align=left width=80>%s:</td>",search(lcontrol,"operation"));
							fprintf(cgiOut,"<td align=left  width=180 colspan=2>");
							fprintf(cgiOut, "<select name=opera>"\
							"<option value=config>%s",search(lcontrol,"config"));
							fprintf(cgiOut,"<option value=delete_config>%s",search(lcontrol,"del"));
							fprintf(cgiOut,"</select>\n"\
                         	"</td>"\
                         	"</tr>");
							fprintf(cgiOut,"<tr align=left>");
							fprintf(cgiOut,"<td align=left width=80>%s:</td>",search(lcontrol,"node_type"));
							fprintf(cgiOut,"<td align=left  width=180 colspan=2>");
							fprintf(cgiOut, "<select name=\"node_type\" onchange=\"javascript:this.form.submit();\">");
							for(i=0;i<2;i++)
							if(strcmp(NodeType[i],select_a)==0)				/*显示上次选中的*/
								fprintf(cgiOut,"<option value=%s selected=selected>%s",NodeType[i],NodeType[i]);
							else				
								fprintf(cgiOut,"<option value=%s>%s",NodeType[i],NodeType[i]);

                         	fprintf(cgiOut, "</select>\n"\
                         	"</td>"\
                         	"</tr>");
							int NodeTypechoice=0,MirrorTypechoice=0;
                         	cgiFormSelectSingle("node_type", NodeType, 2, &NodeTypechoice, 0);
                         	switch(NodeTypechoice)
                         	{
                         		case 0:
									{
										
										fprintf(cgiOut,"<tr align=left><td>%s:</td>",search(lcontrol,"dest_port"));
										fprintf(cgiOut,"<td align=left>"\
										"<select name=dest_port_type>");
										fprintf(cgiOut,"<option value=ingress>%s",search(lcontrol,"ingress"));
										fprintf(cgiOut,"<option value=egress>%s",search(lcontrol,"egress"));
										fprintf(cgiOut,"<option value=bidirection>%s",search(lcontrol,"bidirection"));
										fprintf(cgiOut,"</select>"\
										"</td>");
										fprintf(cgiOut,"<td align=left>"\
										"<select name=dest_port>");
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
													  memset(slot_port,0,10);							
												      sprintf(slot_port,"%d-%d",p->slot_no,pp->port_no);
												      fprintf(cgiOut,"<option value=%s>%s",slot_port,slot_port);
													  pp=pp->next;
												  }
												  p=p->next;
											  }
										  }
										fprintf(cgiOut,"</select>"\
										"</td>"\
										"</tr>");
									}
									break;
                         		case 1:
									{
										show_mirror=1;
									}
									break;
                         	}

							if(show_mirror==1)
							{
								fprintf(cgiOut,"<tr align=left>");
    							fprintf(cgiOut,"<td align=left width=80>%s:</td>",search(lcontrol,"mirror_type"));
    							fprintf(cgiOut,"<td align=left  width=180 colspan=2>");
    							fprintf(cgiOut, "<select name=\"mirror_node\" onchange=\"javascript:this.form.submit();\">");
    							for(i=0;i<4;i++)
         							if(strcmp(mirror_type[i],select_b)==0)				/*显示上次选中的*/
         								fprintf(cgiOut,"<option value=%s selected=selected>%s",mirror_type[i],mirror_type[i]);
         							else				
         								fprintf(cgiOut,"<option value=%s>%s",mirror_type[i],mirror_type[i]);
    
                             	fprintf(cgiOut, "</select>\n"\
                             	"</td>"\
                             	"</tr>");

								cgiFormSelectSingle("mirror_node", mirror_type, 4, &MirrorTypechoice, 0);
								switch(MirrorTypechoice)
									{
										case 0:
											{
												fprintf(cgiOut,"<tr>");
												fprintf(cgiOut,"<td align=left width=80>%s:</td>","VLAN ID");
    											fprintf(cgiOut,"<td align=left  width=180 colspan=2><input type=text name=Vlanid size=8></td>");
												fprintf(cgiOut,"</tr>");
											}
										break;
										case 1:
											{
												fprintf(cgiOut,"<tr>");
												fprintf(cgiOut,"<td align=left width=80>%s:</td>","ACL Index");
    											fprintf(cgiOut,"<td align=left  width=180 colspan=2><input type=text name=ACl_index size=8></td>");
												fprintf(cgiOut,"</tr>");
											}
										break;
										case 2:
											{
												
         										fprintf(cgiOut,"<tr align=left><td width=80>%s:</td>",search(lcontrol,"mirror_port"));
     											fprintf(cgiOut,"<td align=left width=80>"\
     											"<select name=mirror_port_type>");
     											fprintf(cgiOut,"<option value=ingress>%s",search(lcontrol,"ingress"));
     											fprintf(cgiOut,"<option value=egress>%s",search(lcontrol,"egress"));
     											fprintf(cgiOut,"<option value=bidirection>%s",search(lcontrol,"bidirection"));
     											fprintf(cgiOut,"</select>"\
     											"</td>");
         										fprintf(cgiOut,"<td align=left width=100>"\
         										"<select name=mirror_port>");
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
															  memset(slot_port,0,10);							
														      sprintf(slot_port,"%d-%d",p->slot_no,pp->port_no);
														      fprintf(cgiOut,"<option value=%s>%s",slot_port,slot_port);
															  pp=pp->next;
														  }
														  p=p->next;
													  }
												  }
												fprintf(cgiOut,"</select>"\
     											"</td>"\
         										"</tr>");
											}
										break;
										case 3:
											{
												fprintf(cgiOut,"<tr>");
												fprintf(cgiOut,"<td align=left>%s:</td>",search(lcontrol,"Mac_address"));
    											fprintf(cgiOut,"<td align=left  width=180 colspan=2><input type=text name=MacAddress size=12></td>");
												fprintf(cgiOut,"</tr>");

												fprintf(cgiOut,"<tr>");
												fprintf(cgiOut,"<td align=left width=80>%s:</td>","VLAN ID");
    											fprintf(cgiOut,"<td align=left  width=180 colspan=2><input type=text name=Vlanid size=12></td>");
												fprintf(cgiOut,"</tr>");

												fprintf(cgiOut,"<tr>");
												fprintf(cgiOut,"<td align=left>%s:</td>",search(lcontrol,"FDB_port"));
    											fprintf(cgiOut,"<td align=left colspan=2>"\
         										"<select name=mirror_port>");
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
															  memset(slot_port,0,10);							
														      sprintf(slot_port,"%d-%d",p->slot_no,pp->port_no);
														      fprintf(cgiOut,"<option value=%s>%s",slot_port,slot_port);
															  pp=pp->next;
														  }
														  p=p->next;
													  }
												  }
     											fprintf(cgiOut,"</select>"\
     											"</td>"\
												"</tr>");
											}
										break;
										
									}

								
							}
						  fprintf(cgiOut,"<tr>");
						  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )
						  {
							fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_configmirror value=%s></td>",encry);
						  }
						  else
						  {
							fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_configmirror value=%s></td>",cofigmirror_encry);
						  }
						  fprintf(cgiOut,"</tr>"\
						  "</table>"\
						  "</td>"\
						  "</tr>"\
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
free(slot_port);
free(select_a);
free(select_b);
release(lpublic);  
release(lcontrol);
if((ret==0)&&(num>0))
{
	Free_ethslot_head(&head);
}
return 0;
}

															 
int config_mirror_hand(char * nodeType_select,char * mirror_select,struct list * lcontrol)
{
		char config_operation[20] = {0};
	char VlanID[10] = {0};
	char ACL_index[10] = {0};
	char dest_port_type[20] = {0};
	char dest_port[10] = {0};
	char mirror_port_type[20] = {0};
	char mirror_port_param[10] = {0};
	char MacAddress[50] = {0};

	int ret = 0;
	cgiFormStringNoNewlines("opera",config_operation,sizeof(config_operation));
	cgiFormStringNoNewlines("Vlanid",VlanID,sizeof(VlanID));
	cgiFormStringNoNewlines("ACl_index",ACL_index,sizeof(ACL_index));
	cgiFormStringNoNewlines("dest_port_type",dest_port_type,sizeof(dest_port_type));
	cgiFormStringNoNewlines("dest_port",dest_port,sizeof(dest_port));
	cgiFormStringNoNewlines("mirror_port_type",mirror_port_type,sizeof(mirror_port_type));
	cgiFormStringNoNewlines("mirror_port",mirror_port_param,sizeof(mirror_port_param));
	cgiFormStringNoNewlines("MacAddress",MacAddress,sizeof(MacAddress));
	config_mirror_profile();
	if(strcmp(config_operation,"config")==0)
	{
		if(strcmp(nodeType_select,"Destination")==0)
		{
			ret = Create_destPort(dest_port,dest_port_type,0);
			switch(ret)
			{
				case 1:
					ShowAlert(search(lcontrol,"Operation_Success"));
					break;
				default:
					ShowAlert(search(lcontrol,"opt_fail"));;
					break;
			}
		}
		else if(strcmp(nodeType_select,"Mirror")==0)
		{
			if(strcmp(mirror_select,"VLAN")==0)
			{
				ret = mirror_vlan(VlanID,0);
				switch(ret)
				{
					case 1:
						ShowAlert(search(lcontrol,"Operation_Success"));
						break;
					default:
						ShowAlert(search(lcontrol,"opt_fail"));;
						break;
				}
			}
			else if(strcmp(mirror_select,"ACL")==0)
			{
				ret = mirror_policy(ACL_index,0);
				switch(ret)
				{
					case 1:
						ShowAlert(search(lcontrol,"Operation_Success"));
						break;
					default:
						ShowAlert(search(lcontrol,"opt_fail"));;
						break;
				}
			}
			else if(strcmp(mirror_select,"port")==0)
			{
				ret = mirror_port(mirror_port_param,mirror_port_type,0);
				switch(ret)
				{
					case 1:
						ShowAlert(search(lcontrol,"Operation_Success"));
						break;
					default:
						ShowAlert(search(lcontrol,"opt_fail"));;
						break;
				}
			}
			else if(strcmp(mirror_select,"FDB")==0)
			{
				////// ?é?¤ mac μ??·ê?·?ê?o?·¨μ? ////////
				ret=mirror_fdb(MacAddress,VlanID,mirror_port_param,0);
				switch(ret)
				{
					case 1:
						ShowAlert(search(lcontrol,"Operation_Success"));
						break;
					case 2:
						ShowAlert(search(lcontrol,"mac_form"));
						break;
					default:
						ShowAlert(search(lcontrol,"opt_fail"));;
						break;
				}
			}
		}
	}
	else if(strcmp(config_operation,"delete_config")==0)
	{
		if(strcmp(nodeType_select,"Destination")==0)
		{
			ret = del_destPort(dest_port,dest_port_type,0);
			switch(ret)
			{
				case 1:
					ShowAlert(search(lcontrol,"Operation_Success"));
					break;
				default:
					ShowAlert(search(lcontrol,"opt_fail"));;
					break;
			}
		}
		else if(strcmp(nodeType_select,"Mirror")==0)
		{
			if(strcmp(mirror_select,"VLAN")==0)
			{
				ret = no_mirror_vlan(VlanID,0);
				switch(ret)
				{
					case 1:
						ShowAlert(search(lcontrol,"Operation_Success"));
						break;
					default:
						ShowAlert(search(lcontrol,"opt_fail"));;
						break;
				}
			}
			else if(strcmp(mirror_select,"ACL")==0)
			{
				ret = no_mirror_policy(ACL_index,0);
				switch(ret)
				{
					case 1:
						ShowAlert(search(lcontrol,"Operation_Success"));
						break;
					default:
						ShowAlert(search(lcontrol,"opt_fail"));;
						break;
				}
			}
			else if(strcmp(mirror_select,"port")==0)
			{
				ret = no_mirror_port(mirror_port_param,mirror_port_type,0);
				switch(ret)
				{
					case 1:
						ShowAlert(search(lcontrol,"Operation_Success"));
						break;
					default:
						ShowAlert(search(lcontrol,"opt_fail"));;
						break;
				}
			}
			else if(strcmp(mirror_select,"FDB")==0)
			{
				ret = no_mirror_fdb(MacAddress,VlanID,mirror_port_param,0);
				switch(ret)
				{
					case 1:
						ShowAlert(search(lcontrol,"Operation_Success"));
						break;
					default:
						ShowAlert(search(lcontrol,"opt_fail"));;
						break;
				}
			}
		}
	}
	return 1;
}



