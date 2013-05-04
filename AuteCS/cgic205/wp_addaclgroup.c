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
* wp_addaclgroup.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for add acl group list
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_dcli_acl.h"
#include "ws_dcli_portconf.h"
#include <sys/wait.h>

char *inType[]={
	"ingress",
	"egress"
};

int ShowAddAclGrpPage(); 
int addhand(struct list * lcontrol);



int cgiMain(struct list *lcontrol)
{
 ShowAddAclGrpPage();
 return 0;
}

int ShowAddAclGrpPage()
{
	ccgi_dbus_init();
	//**************add port show function*******************
	int slot_mun = 0;
	int ret = 0;
	ETH_SLOT_LIST  head,*p;
    	ETH_PORT_LIST *pp;
	//********************************************************
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	char *encry=(char *)malloc(BUF_LEN);			  
	//char *str;
	int i;
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	//cgiFormStringNoNewlines("GroupType",in_in,10);
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"acl_conf"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	  	"<style type=text/css>"\
	  ".a3{width:30;border:0; text-align:center}"\
	  "</style>"\
  "</head>"\
   "<script src=/ip.js>"\
  "</script>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_addaclgrp") == cgiFormSuccess)
  {
    	addhand(lcontrol);
  }

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>ACL</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_addaclgrp style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		 // if(cgiFormSubmitClicked("submit_addaclgrp") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		//  else                                         
     		//fprintf(cgiOut,"<td width=62 align=left><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry_addaclgrp,search(lpublic,"img_cancel"));
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
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_aclall.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"acl_list")); 				   
							fprintf(cgiOut,"</tr>"\
							
							"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_addaclrule.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_acl_rule"));
							fprintf(cgiOut,"</tr>");
							
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_aclgrplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"acl_grp_lsit"));
							fprintf(cgiOut,"</tr>");
							
							fprintf(cgiOut,"<tr height=26>"\
                 			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_acl_group"));   /*突出显示*/
                 			fprintf(cgiOut,"</tr>");


					  for(i=0;i<1;i++)
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
						  "<td align=left valign=top style=padding-top:10px>"\
						   "<table width=400 border=0 cellspacing=0 cellpadding=0>"\
						   "<tr align=left style=padding-top:8px>"\
						   "<td id=td1 width=70 style=padding-left:20px><font id=%s>%s</font></td>", search(lpublic,"menu_thead"),search(lcontrol,"group_index"));
						   fprintf(cgiOut,"<td align=left width=320 style=padding-left:10px><input type=text size=6 name=GroupIndex></td>"\
						   "</tr>"\

						   "<tr align=left style=padding-top:8px>"\
						   "<td id=td1 width=70 style=padding-left:20px><font id=%s>%s</font></td>", search(lpublic,"menu_thead"),search(lcontrol,"group_type"));
						   fprintf(cgiOut,"<td id=sec3 align=left width=320 style=padding-left:10px>");	
						   if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
						   {
             					   	fprintf(cgiOut,"<select name=\"GroupType\">"\
    						  					"<option value=\"ingress\">ingress\n");
                             				fprintf(cgiOut,"<option value=\"egress\">egress\n");	      
						   }
						   else
						   {
						   	fprintf(cgiOut, "<b>ingress</b><input type=hidden name=GroupType value=ingress>");
						   }
                   			       	fprintf(cgiOut,"</td>"\
						   "</tr>"\
						   "</table>"\
						   "<table width=400 border=0 cellspacing=0 cellpadding=0>");
						if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
						{
							fprintf(cgiOut,"<tr>"\
						  					"<td id=sec1 colspan=4 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lcontrol,"can_select"));
						  	fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
								fprintf(cgiOut,"<td width=20><input type=radio name=bindRadio value=%s checked></td>","PortNo");
								fprintf(cgiOut,"<td id=td1 width=70>%s:</td>",search(lcontrol,"bind_portNo"));
								fprintf(cgiOut,"<td  align=left width=50 style=padding-left:10px>");
									fprintf(cgiOut,"<select name = bindPort>");
									fprintf(cgiOut,"<option value="">--%s--</option>",search(lcontrol,"select"));
									ret = show_ethport_list(&head,&slot_mun);
									p = head.next;
									while(p!=NULL)
									{
										pp = p->port.next;
										while(pp!=NULL)
										{
											if(p->slot_no!=0)
											{
												fprintf(cgiOut,"<option value = %d/%d>%d/%d</option>\n",p->slot_no,pp->port_no,p->slot_no,pp->port_no);
											}
											pp = pp->next;
										}
										p = p->next;
									}
									fprintf(cgiOut,"</select>");
								fprintf(cgiOut,"</td>\n");
								fprintf(cgiOut,"<td  align=left style=color:red width=250>(SLOT/PORT)</td>"\
										"</tr>");

							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
								fprintf(cgiOut,"<td width=20><input type=radio name=bindRadio value=%s></td>","VlanId");
								fprintf(cgiOut,"<td id=td1 width=70>%s:</td>",search(lcontrol,"bind_Vlan"));
								fprintf(cgiOut,"<td  align=left width=50 style=padding-left:10px><input type=text size=6 name=bindVlan></td>");
								fprintf(cgiOut,"<td  align=left style=color:red width=250>(Vid)</td>");
							fprintf(cgiOut,"</tr>");
						}
						else
						{
								fprintf(cgiOut,"<tr>"\
							  					"<td id=sec1 colspan=3 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lcontrol,"can_select"));
							  	fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
									fprintf(cgiOut,"<td id=td1 width=70>%s:</td>",search(lcontrol,"bind_portNo"));
									fprintf(cgiOut,"<td  align=left width=50 style=padding-left:10px>");
										fprintf(cgiOut,"<select name = bindPort>");
											fprintf(cgiOut,"<option value="">--%s--</option>",search(lcontrol,"select"));
										ret = show_ethport_list(&head,&slot_mun);
										p = head.next;
										while(p!=NULL)
										{
											pp = p->port.next;
											while(pp!=NULL)
											{
												if(p->slot_no!=0)
												{
													fprintf(cgiOut,"<option value = %d/%d>%d/%d</option>\n",p->slot_no,pp->port_no,p->slot_no,pp->port_no);
												}
												pp = pp->next;
											}
											p = p->next;
										}
										fprintf(cgiOut,"</select>");
									fprintf(cgiOut,"</td>");
									fprintf(cgiOut,"<td  align=left style=color:red width=250>(SLOT/PORT)</td>"\
											"</tr>");


						}
						   fprintf(cgiOut,"</table>"\
						  "</td>"\
						"</tr>"\
						
						 "<tr>");
								fprintf(cgiOut,"<td><input type=hidden name=UN value=%s></td>",encry);
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
if((ret==0)&&(slot_mun>0))
{
	Free_ethslot_head(&head);
}
free(encry);
release(lpublic);  
release(lcontrol);
return 0;

}

int addhand(struct list * lcontrol)
{	
	char * bindRadio[] = {
		"PortNo",
		"VlanId"
	};	
	unsigned int temp=0,gu_index=0,dir=0;
	unsigned int nodetype=0;  //o stand for eth,1 stand for vlan
	char * Gourp_Index=(char *)malloc(10);
	char * Gourp_Type=(char *)malloc(10);
	int Choice;
	char bind[10];
	char * bind_choice=(char *)malloc(10);
	memset(Gourp_Index,0,10);
	memset(Gourp_Type,0,10);
	memset(bind_choice,0,10);
	memset(bind,0,10);
	cgiFormStringNoNewlines("GroupIndex",Gourp_Index,10);
	cgiFormStringNoNewlines("GroupType",Gourp_Type,10);
	unsigned int index_temp=0;
	if(strcmp(Gourp_Index,"")==0)
	{
		ShowAlert(search(lcontrol,"index_Not_NULL"));
		return -1;
	}
	else 
		index_temp=atoi(Gourp_Index);

	if(index_temp<1 || index_temp>1023)
	{
		ShowAlert(search(lcontrol,"illegal_input"));
		return -1;
	}
	
	if(strcmp(Gourp_Type,"ingress")==0)
		dir=0;
	else if(strcmp(Gourp_Type,"egress")==0)
		dir=1;

	if(strcmp(Gourp_Type,"ingress")==0)
		temp=0;
	else if(strcmp(Gourp_Type,"egress")==0)
		temp=1;

	if(index_temp!=0)
	{
		addacl_group(temp,index_temp);
	}
	if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
	{
		cgiFormRadio("bindRadio", bindRadio, 2, &Choice, 0);
		if(Choice==0)
		{
			nodetype=0;
			cgiFormStringNoNewlines("bindPort",bind_choice,10);
			if(strcmp(bind_choice,"")!=0)
			{
		    		get_one_port_index(bind_choice,&gu_index);
		    		if(index_temp!=0)
		    		{
		    			enable_aclgrp(Gourp_Type,"enable",gu_index,0,lcontrol);
		    			bind_acl_group(dir,nodetype,gu_index,index_temp);
		    		}
			}
		}
		else if(Choice==1)
		{
			nodetype=1;
			cgiFormStringNoNewlines("bindVlan",bind_choice,10);
			if(strcmp(bind_choice,"")!=0)
		     	{
		     		if(strcmp(bind_choice,"1")==0)
		     		{
		     			ShowAlert(search(lcontrol,"default_vlan_not_bind"));
		     		}
		     		else
		     		{
		        		gu_index=atoi(bind_choice);
		        		if(index_temp!=0)
					{
						enable_aclgrp(Gourp_Type,"enable",gu_index,1,lcontrol);
						bind_acl_group(dir,nodetype,gu_index,index_temp);
					}
				}
			}
		}
	}
	else
	{
		if(dir == 0)
		{
			nodetype=0;
			cgiFormStringNoNewlines("bindPort",bind_choice,10);
			if(strcmp(bind_choice,"")!=0)
			{
		    		get_one_port_index(bind_choice,&gu_index);
		    		if(index_temp!=0)
		    		{
		    			enable_aclgrp(Gourp_Type,"enable",gu_index,0,lcontrol);
		    			bind_acl_group(dir,nodetype,gu_index,index_temp);
		    		}
			}
		}
	}
	free(bind_choice);
	free(Gourp_Index);
	free(Gourp_Type);
	return 0;
}

 
