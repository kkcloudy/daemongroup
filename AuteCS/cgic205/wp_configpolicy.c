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
* wp_configpolicy.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
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
#include "ws_init_dbus.h"
#include "ws_dcli_portconf.h"
#include "ws_dcli_qos.h"


char * Trust_mode[] = {
		"trust_l2",
		"trust_l3",
		"trust_l2andl3"
};

#define AMOUNT 512
#define MAX_PORTN 52
 
int ShowConfigPolicyPage(); 
int addvlan_hand(struct list *lpublic); 

int cgiMain()
{
	ShowConfigPolicyPage();
	return 0;
}

int ShowConfigPolicyPage()
{
	ccgi_dbus_init();
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	char * index=(char *)malloc(10);
	unsigned int indextemp=0;
	char *endptr = NULL;  
	int i,result,retz,ret;
	char configpolicy_encry[BUF_LEN];  
	unsigned int gu_index=0;

	char * Trust_Mode=(char * )malloc(20);
	memset(Trust_Mode,0,20);
	char * qos_maker=(char * )malloc(10);
	memset(qos_maker,0,10);
	char * up_enable=(char * )malloc(10);
	memset(up_enable,0,10);
	char * dscp_enable=(char * )malloc(10);
	memset(dscp_enable,0,10);
	char * remap_enable=(char * )malloc(10);
	memset(remap_enable,0,10);
	char * bindPort_mode=(char * )malloc(10);
	memset(bindPort_mode,0,10);
	//*********add show port**************
	int slot_mun = 0;
	int ret_p = 0;
	ETH_SLOT_LIST  head,*p;
    ETH_PORT_LIST *pp;
	int selectnum = 0;
	
	char *flavors[MAX_PORTN];
	int flavorChoices[MAX_PORTN];
	int invalid; 
    for(i=0;i<MAX_PORTN;i++)
	{
        flavors[i]=(char *)malloc(6);
        memset(flavors[i],0,6);
	}
	//**********************************
	char * mode =(char *)malloc(AMOUNT);
	memset(mode,0,AMOUNT);

	
	memset(encry,0,BUF_LEN);
	show_qos_mode(mode);
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
	{
		cgiFormStringNoNewlines("INDEX",index,10);
	  	str=dcryption(encry);
	  	if(str==NULL)
	 	{
			ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
			return 0;
	  	}
	  	memset(configpolicy_encry,0,BUF_LEN);	 /*清空临时变量*/

	}
    	else 
    	{
	        cgiFormStringNoNewlines("encry_addvlan",encry,BUF_LEN);
	        cgiFormStringNoNewlines("INDEX",index,10);
	        cgiFormStringNoNewlines("Trust_Mode",Trust_Mode,20);
	        cgiFormStringNoNewlines("qos_maker",qos_maker,10);
	        indextemp=strtoul(index,&endptr,10);	   
    	}
   	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");

    ret_p = show_ethport_list(&head,&slot_mun);
	p = head.next;
	i=0;
	while(p!=NULL)
	{
		pp = p->port.next;
		while(pp!=NULL)
		{
			if(p->slot_no!=0)
			{
				//fprintf(cgiOut,"<option value = %d/%d>%d/%d</option>\n",p->slot_no,pp->port_no,p->slot_no,pp->port_no);
				 i++;
				 selectnum++;
				 sprintf(flavors[i],"%d/%d",p->slot_no,pp->port_no);
			}
			pp = pp->next;
		}
		p = p->next;
	}
	if(cgiFormSubmitClicked("submit_configpolicy") == cgiFormSuccess)
	{
		cgiFormStringNoNewlines("up_enable",up_enable,10);
		cgiFormStringNoNewlines("dscp_enable",dscp_enable,10);
		cgiFormStringNoNewlines("remap_enable",remap_enable,10);
		cgiFormStringNoNewlines("bind_port",bindPort_mode,10);
		
		if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
		{
			if(strcmp(Trust_Mode,"trust_l2")==0)
			{
				result=trust_l2_up(up_enable,index,lcontrol);
				switch(result)
				{
					case 0:
						;
						break;
					case -3:
						ShowAlert(search(lcontrol,"trust_l2_fail"));
						break;
					default:
						ShowAlert(search(lpublic,"oper_fail"));
						break;
				}
			}
			else if(strcmp(Trust_Mode,"trust_l3")==0)
			{
				result=trust_l3_dscp(dscp_enable,remap_enable,index,lcontrol);
				if(result==CMD_SUCCESS)
				{
					;
				}
			}
			else if(strcmp(Trust_Mode,"trust_l2andl3")==0)
			{
				result=trust_l2andl3_mode(up_enable,dscp_enable,remap_enable,index,lcontrol);
				switch(result)
				{
					case 0:
						;
						break;
					case -3:
						ShowAlert(search(lcontrol,"trust_l2andl3_fail"));
						break;
					default:
						ShowAlert(search(lpublic,"oper_fail"));
						break;
				}
			}
		}
		
		retz=allow_qos_marker(qos_maker,index,lcontrol);
		if(retz==0)
			;
		else
			ShowAlert(search(lpublic,"oper_fail"));

		/////////////////////////////////////////////////////////////////////////////////////
		result = cgiFormSelectMultiple("bindPort", flavors, selectnum, 
		flavorChoices, &invalid);
		if (result == cgiFormNotFound)
		{
			ShowAlert(search(lcontrol,"Not_Select"));
		} 
		else
		{
			for (i=0; (i < selectnum); i++) 
			{
				if (flavorChoices[i]) 
				{
					if(strcmp(bindPort_mode,"bind")==0)
					{
						get_one_port_index_QOS(flavors[i],&gu_index);
						
						ret=bind_policy_port(index,gu_index);
						fprintf(stderr,"flavors[%d]=%s-gu_index=%d--ret=%d--index=%d", i, flavors[i], gu_index, ret, index);
						switch(ret)
						{
							case 0:
							{
								//ShowAlert(search(lcontrol,"bind_port_succ"));
								if (result==0 && retz==0)
									ShowAlert(search(lpublic,"oper_succ"));
								
								fprintf( cgiOut, "<script type='text/javascript'>\n" );
								fprintf( cgiOut, "window.location.href='wp_policymaplist.cgi?UN=%s';\n",encry);
								fprintf( cgiOut, "</script>\n" );
								break;
							}
							case -2:
								ShowAlert(search(lcontrol,"no_port"));
								break;
							case -3:
								ShowAlert(search(lcontrol,"no_suit_flow_mode"));
								break;
							case -5:
								ShowAlert(search(lcontrol,"no_qos_mode"));
								break;
							case -6:
								ShowAlert(search(lcontrol,"not_same_port_v3"));
								break;
							case -7:
								ShowAlert(search(lcontrol,"port_has_binded"));
								break;
							case -8:
								ShowAlert(search(lcontrol,"policy_map_not_exist"));	
								break;
							case -10:
								ShowAlert(search(lcontrol,"bind_policy_map_fail"));	
								break;
							case -11:
								ShowAlert(search(lcontrol,"bind_hw_error"));	
								break;
							default:
								ShowAlert(search(lpublic,"oper_fail"));
								break;
							}

					}
					else if(strcmp(bindPort_mode,"unbind")==0)
					{
						get_one_port_index_QOS(flavors[i],&gu_index);
						ret=unbind_policy_port(index,gu_index,lcontrol);
						switch(ret)
						{
							case 0:
							{
								ShowAlert(search(lcontrol,"ubind_succ"));
								fprintf( cgiOut, "<script type='text/javascript'>\n" );
								fprintf( cgiOut, "window.location.href='wp_policymaplist.cgi?UN=%s';\n",encry);
								fprintf( cgiOut, "</script>\n" );
								break;
							}
							case -2:
								ShowAlert(search(lcontrol,"no_such_port"));
								break;
							case -3:
								ShowAlert(search(lcontrol,"wrong_po_index"));
								break;
							case -4:
								ShowAlert(search(lcontrol,"no_map_info"));
								break;
							case -5:
								ShowAlert(search(lcontrol,"ubind_fail"));
								break;
							default :
								ShowAlert(search(lpublic,"oper_fail"));
								break;
						}
					}

				}
			}
		}
	}

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>QOS</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_configpolicy style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_configpolicy") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_policymaplist.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_policymaplist.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
        			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>QOS</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"list"));					   
        		  	fprintf(cgiOut,"</tr>"\
        		  	"<tr height=25>"\
        			"<td align=left id=tdleft><a href=wp_addqos.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>QOS Profile</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));
        		  	fprintf(cgiOut,"</tr>");
         			fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_addmap.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_map"));					   
         			fprintf(cgiOut,"</tr>");

				fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));					   
         			fprintf(cgiOut,"</tr>");
					
         			fprintf(cgiOut,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"config_policy"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
         			fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_createpolicy.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_policy"));					   
         			fprintf(cgiOut,"</tr>");


					for(i=0;i<5;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>");

						//acl_info=show_aclinfo_Byindex(indextemp);

     					   	fprintf(cgiOut,"<table frame=below rules=rows width=420 border=1>");	

						fprintf(cgiOut,"<tr>\n"\
										"<td colspan='7' align='left' style=\"font-size:14px\" height='30'><font color='red'><b>%s</b></font></td>\n"\
									"</tr>",search(lcontrol,mode));
     						if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
						{
							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
								fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"trust_mode"));
								fprintf(cgiOut,"<td  align=left style=padding-left:10px colspan=6>"\
												"<select name=Trust_Mode onchange=\"javascript:this.form.submit();\">");
							for(i=0;i<3;i++)
							{
								if(strcmp(Trust_mode[i],Trust_Mode)==0)				/*显示上次选中的*/
								{
										fprintf(cgiOut,"<option value=%s selected=selected>%s",Trust_mode[i],search(lcontrol,Trust_mode[i]));
								}
								else				
								{
										fprintf(cgiOut,"<option value=%s>%s",Trust_mode[i],search(lcontrol,Trust_mode[i]));
								}
							}
									fprintf(cgiOut,"</select>");
								fprintf(cgiOut,"</td>"\
										"</tr>");
							int Trust_modechoice=0;
							cgiFormSelectSingle("Trust_Mode", Trust_mode, 3, &Trust_modechoice, 0);
						if(Trust_modechoice==0)
						{
							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
								fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"UP_enable"));
								fprintf(cgiOut,"<td  align=left style=padding-left:10px colspan=6>"\
												"<select name=up_enable >"\
													"<option value=%s>%s","enable","enable");
										fprintf(cgiOut,"<option value=%s>%s","disable","disable");
								fprintf(cgiOut,"</td>"\
										"</tr>");

							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
								fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"allow_qos_maker"));
								fprintf(cgiOut,"<td  align=left style=padding-left:10px colspan=6>"\
												"<select name=qos_maker >"\
													"<option value=%s>%s","enable","enable");
										fprintf(cgiOut,"<option value=%s>%s","disable","disable");
								fprintf(cgiOut,"</td>"\
										"</tr>");
						}
						else if(Trust_modechoice==1)
						{
							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
								fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"DSCP_enable"));
								fprintf(cgiOut,"<td  align=left style=padding-left:10px>"\
												"<select name=dscp_enable >"\
													"<option value=%s>%s","enable","enable");
										fprintf(cgiOut,"<option value=%s>%s","disable","disable");
								fprintf(cgiOut,"</td>");

								fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"remap_enable"));
								fprintf(cgiOut,"<td  align=left style=padding-left:10px colspan=3>"\
												"<select name=remap_enable >"\
													"<option value=%s>%s","enable","enable");
										fprintf(cgiOut,"<option value=%s>%s","disable","disable");
								fprintf(cgiOut,"</td>"\
										"</tr>");

							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
								fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"allow_qos_maker"));
								fprintf(cgiOut,"<td  align=left style=padding-left:10px colspan=4>"\
												"<select name=qos_maker >"\
													"<option value=%s>%s","enable","enable");
										fprintf(cgiOut,"<option value=%s>%s","disable","disable");
								fprintf(cgiOut,"</td>"\
										"</tr>");
						}
						else if(Trust_modechoice==2)
						{
							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
								fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"UP_enable"));
								fprintf(cgiOut,"<td  align=left style=padding-left:10px>"\
												"<select name=up_enable >"\
													"<option value=%s>%s","enable","enable");
										fprintf(cgiOut,"<option value=%s>%s","disable","disable");
								fprintf(cgiOut,"</td>");

								fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"DSCP_enable"));
								fprintf(cgiOut,"<td  align=left style=padding-left:10px>"\
												"<select name=dscp_enable >"\
													"<option value=%s>%s","enable","enable");
										fprintf(cgiOut,"<option value=%s>%s","disable","disable");
								fprintf(cgiOut,"</td>");

								fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"remap_enable"));
								fprintf(cgiOut,"<td  align=left style=padding-left:10px>"\
												"<select name=remap_enable >"\
													"<option value=%s>%s","enable","enable");
										fprintf(cgiOut,"<option value=%s>%s","disable","disable");
								fprintf(cgiOut,"</td>"\
										"</tr>");

							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
								fprintf(cgiOut,"<td id=td1 colspan=2>%s:</td>",search(lcontrol,"allow_qos_maker"));
								fprintf(cgiOut,"<td  align=left style=padding-left:10px colspan=4>"\
												"<select name=qos_maker >"\
													"<option value=%s>%s","enable","enable");
										fprintf(cgiOut,"<option value=%s>%s","disable","disable");
								fprintf(cgiOut,"</td>"\
										"</tr>");
						}
						else
						{
							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
								fprintf(cgiOut,"<td id=td1 colspan=2>%s:</td>",search(lcontrol,"allow_qos_maker"));
								fprintf(cgiOut,"<td  align=left style=padding-left:10px>"\
												"<select name=qos_maker >"\
													"<option value=%s>%s","enable","enable");
										fprintf(cgiOut,"<option value=%s>%s","disable","disable");
								fprintf(cgiOut,"</td>"\
										"</tr>");
						}
					}
					else
					{
						fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
							fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"allow_qos_maker"));
							fprintf(cgiOut,"<td  align=left style=padding-left:10px  colspan=6>"\
											"<select name=qos_maker >"\
												"<option value=%s>%s","enable","enable");
									fprintf(cgiOut,"<option value=%s>%s","disable","disable");
							fprintf(cgiOut,"</td>"\
									"</tr>");
					}
						   
             				fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
						    fprintf(cgiOut,"<td id=td1 width=120>%s:</td>",search(lcontrol,"bind_portNo"));
							fprintf(cgiOut,"<td  align=left style=padding-left:10px><select name=bind_port >"\
						    "<option value=%s>%s","bind","bind");
						    fprintf(cgiOut,"<option value=%s>%s","unbind","unbind");
 						    fprintf(cgiOut,"</td>");
						    fprintf(cgiOut,"<td  align=left width=50 style=padding-left:10px>");
							fprintf(cgiOut,"<select name=\"bindPort\" style=\"width:140;height:160;background-color:#ebfff9\" multiple>\n");
							//fprintf(cgiOut,"<option value="">--%s--</option>",search(lcontrol,"select"));
							//ret_p = show_ethport_list(&head,&slot_mun);
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
							fprintf(cgiOut,"</select>\n");
							//fprintf(cgiOut,"<input type=text size=6 name=bindPort>");
							fprintf(cgiOut,"</td>");
						    fprintf(cgiOut,"<td  align=left style=color:red width=60>(SLOT/PORT)</td>"\
						    "<td>&nbsp;</td>"\
						    "<td>&nbsp;</td>"\
						    "<td>&nbsp;</td>"\
						   "</tr>"\
					  "</table>"\
						"</td>"\
						"</tr>");
						fprintf(cgiOut,"<tr>");
						
						fprintf(cgiOut,"<td><input type=hidden name=encry_addvlan value=%s></td>",encry);
						fprintf(cgiOut,"<td><input type=hidden name=INDEX value=%s></td>",index);

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
	if((ret_p==0)&&(slot_mun>0))
	{
		Free_ethslot_head(&head);
	}
	for(i=0;i<MAX_PORTN;i++)
	{
		free(flavors[i]);
	}
	free(encry);
	free(Trust_Mode);
	free(up_enable);
	free(remap_enable);
	free(dscp_enable);
	free(bindPort_mode);
	free(qos_maker);
	free(mode);
	release(lpublic);  
	release(lcontrol);
	return 0;
}

