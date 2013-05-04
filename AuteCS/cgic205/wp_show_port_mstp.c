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
* wp_show_port_mstp.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for multistp  port display 
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
#include "ws_stp.h"
#include "ws_dcli_portconf.h"



int ShowPortMSTPPage(struct list *lpublic,struct list *lcon);

static unsigned int productid = 1;//产品	ID

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcon;	   /*解析control.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcon=get_chain_head("../htdocs/text/control.txt");
	ccgi_dbus_init();
    ShowPortMSTPPage(lpublic,lcon);
	release(lpublic);  
	release(lcon);

 	return 0;
}

int ShowPortMSTPPage(struct list *lpublic,struct list *lcon)
{
	  char *encry=(char *)malloc(BUF_LEN);				/*存储从wp_usrmag.cgi带入的加密字符串*/
	  char *str=NULL;
	
	  char stp_encry[BUF_LEN]; 
	  int i;

      br_self_info aa;
	  memset(&aa,0,sizeof(aa));

	  //////////////
	  ETH_SLOT_LIST  head,*p;
      ETH_PORT_LIST *pp;
	  int port_num;
      //////////////
      
	  char instance[N];
	  memset(instance,0,N); 	  
	  cgiFormStringNoNewlines("instance",instance,N); 
	  int retu=0;
	  char * CheckUsr=(char * )malloc(10);
	  memset(CheckUsr,0,10);

      if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
      {
 			 memset(encry,0,BUF_LEN);
 			 cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
 			 str=dcryption(encry);
 			 if(str==NULL)
 			 {
	 			   ShowErrorPage(search(lpublic,"ill_user"));	 /*用户非法*/
	 			   return 0;
 			 }
 			 memset(stp_encry,0,BUF_LEN); 				  /*清空临时变量*/
	  }
		memset(stp_encry,0,BUF_LEN); 				  /*清空临时变量*/
		cgiFormStringNoNewlines("stp_encry",stp_encry,BUF_LEN);
		cgiFormStringNoNewlines("CheckUsr",CheckUsr,10);

		if(strcmp(CheckUsr,"")!=0)
			retu=atoi(CheckUsr);
		cgiHeaderContentType("text/html");
		fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
		fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
		fprintf(cgiOut,"<title>%s</title>",search(lcon,"stp_man"));
		fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
		  "<style type=text/css>"\
		  ".a3{width:30;border:0; text-align:center}"\
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
		if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
		{
			retu=checkuser_group(str);
		}

	
	
	  fprintf(cgiOut,"<form method=post >"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	  "<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
		"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>MSTP</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

		fprintf( cgiOut, "<input type=hidden name=UN value=%s />", encry );
		
		
				
			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
			  "<tr>"\
			  "<td width=62 align=center><input id=but type=submit name=submit_stp style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
			  if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
				fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
			  else										   
				fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",stp_encry,search(lpublic,"img_cancel"));
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
							if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
							{
								if(retu==0)  /*管理员*/
								{
     								fprintf(cgiOut,"<tr height=25>"\
     								  "<td align=left id=tdleft><a href=wp_mstp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"br_info"));	 
     								fprintf(cgiOut,"</tr>");
     								fprintf(cgiOut,"<tr height=25>"\
     								  "<td align=left id=tdleft><a href=wp_config_mstp_bridge.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"br_conf"));
     								fprintf(cgiOut,"</tr>"\
     								"<tr height=26>"\
     								"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"port_info"));//突出显示
     								fprintf(cgiOut,"</tr>"\
     								"<tr height=25>"\
     								  "<td align=left id=tdleft><a href=wp_config_mstp_port.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"port_conf"));
     								fprintf(cgiOut,"</tr>"\
     								"<tr height=25>"\
     								  "<td align=left id=tdleft><a href=wp_config_map_instance.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"conf_instance"));
     								fprintf(cgiOut,"</tr>");
								}
								else
								{
									fprintf(cgiOut,"<tr height=25>"\
     								  "<td align=left id=tdleft><a href=wp_mstp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"br_info"));	 
     								fprintf(cgiOut,"</tr>"\
     								"<tr height=26>"\
     								"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"port_info"));//突出显示
     								fprintf(cgiOut,"</tr>");
								}

							}
							else if(cgiFormSubmitClicked("submit_stp") == cgiFormSuccess)
							{
								if(retu==0)  /*管理员*/
								{
     								fprintf(cgiOut,"<tr height=25>"\
     								  "<td align=left id=tdleft><a href=wp_mstp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"br_info"));	 
     								fprintf(cgiOut,"</tr>");
     								fprintf(cgiOut,"<tr height=25>"\
     								  "<td align=left id=tdleft><a href=wp_config_mstp_bridge.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"br_conf"));
     								fprintf(cgiOut,"</tr>"\
     								"<tr height=26>"\
     								"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"port_info"));//突出显示
     								fprintf(cgiOut,"</tr>"\
     								"<tr height=25>"\
     								  "<td align=left id=tdleft><a href=wp_config_mstp_port.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"port_conf"));
     								fprintf(cgiOut,"</tr>"\
     								"<tr height=25>"\
     								  "<td align=left id=tdleft><a href=wp_config_map_instance.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"conf_instance"));
     								fprintf(cgiOut,"</tr>");
								}
								else
								{
									fprintf(cgiOut,"<tr height=25>"\
    								  "<td align=left id=tdleft><a href=wp_mstp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"br_info"));	 
    								fprintf(cgiOut,"</tr>"\
    								"<tr height=26>"\
    								"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"port_info"));//突出显示
    								fprintf(cgiOut,"</tr>");
								}

							}
                        
						int show_ret=-1;
						int line=0,l_num=0;
  	                    show_ret=show_ethport_list(&head,&port_num);
						p=head.next;
                		if(p!=NULL)
                		{
                			while(p!=NULL)
                			{
                				line +=p->port_num;
                				pp=p->port.next;
                				p=p->next;
                			}
                		}
						if(show_ret==0)
							l_num=line;
						else
							l_num=24;
						
						  for(i=0;i<l_num;i++)  //********************
						  {
							fprintf(cgiOut,"<tr height=25>"\
							  "<td id=tdleft>&nbsp;</td>"\
							"</tr>");
						  }
						  
						  if((show_ret==0)&&(port_num>0))
                             {
                           	  Free_ethslot_head(&head);
                             }
	
					  fprintf(cgiOut,"</table>"\
				  "</td>"\
				  "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:35px; padding-top:10px\">"\
						  "<table border=0 cellspacing=0 cellpadding=0>");
										fprintf(cgiOut,"<tr height=30>");
											fprintf(cgiOut,"<td>%s:</td>",search(lcon,"instance"));
											fprintf(cgiOut,"<td width=70><input type=text name=instance size=10 value=%s></td>"\
											"<td><font color=red>(0--63)</font></td>",instance);
										fprintf(cgiOut,"</tr></table>");
										
										if(strcmp(instance,"")!=0&&strtoul(instance,0,10)<=MAX_MST_ID&&strtoul(instance,0,10)>=MIN_MST_ID)
										{
											fprintf(cgiOut,"<table width=750 border=0 cellspacing=0 cellpadding=0>"\
																	"<tr height=35>");
																 
																	  fprintf(cgiOut,"<td align=center id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s%s%s</td>",search(lpublic,"stp1_des"),instance,search(lpublic,"stp3_des"));
																 
													 fprintf(cgiOut,"</tr>");
															fprintf(cgiOut,"<tr>"\
																				"<td align=left valign=top	style=\"padding-top:0px\">");
																  fprintf(cgiOut,"<div class=configvlan><table	width=750 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>"\
																					 "<tr height=25 bgcolor=#eaeff9 style=font-size:14px>"\
																					  "<th	style=font-size:12px>%s</th>",search(lcon,"slot_port_no"));
																					  fprintf(cgiOut,"<th  style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcon,"vID"));
																					  fprintf(cgiOut,"<th  style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcon,"prior"));
																					  fprintf(cgiOut,"<th  style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcon,"path_cost"));
																					  fprintf(cgiOut,"<th  style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcon,"port_rl"));
																					  fprintf(cgiOut,"<th  style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcon,"port_stat"));
																					  fprintf(cgiOut,"<th  style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcon,"link_status"));
																					  fprintf(cgiOut,"<th  style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcon,"link_type"));
																					  fprintf(cgiOut,"<th  style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcon,"edge_port"));
																					  fprintf(cgiOut,"<th  style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcon,"desi_br_id"));
																					  fprintf(cgiOut,"<th  style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcon,"desi_br_cost"));
																					  fprintf(cgiOut,"<th  style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcon,"desi_port"));
																					  fprintf(cgiOut,"</tr>");
																					  int br_stat=0;
																						  int stp_mode;
																						  if(ccgi_get_brg_g_state(&stp_mode)==1)
																						  {
																							  if(stp_mode==1)
																							  {
																								  int cl=1; 			  /*cl标识表格的底色，1为#f9fafe，0为#ffffff*/
											
											
																								  int ret,i,j;
																								  unsigned short *pvid = NULL,*tmp =NULL;
																								  unsigned int mstid = 0,num = 0;
																								  //portsmap = 0;
																								  PORT_MEMBER_BMP  portsmap;
																								  memset(&portsmap,0,sizeof(PORT_MEMBER_BMP));
																								  VLAN_PORTS_BMP ports_bmp ; 
																								  memset(&ports_bmp,0,sizeof(VLAN_PORTS_BMP));
																								  unsigned int tmpVal[2] = {0};
																								  
																								  //VLAN_PORTS_BMP ports_bmp = {0}; 
																								  
																								  unsigned int slot = 0,port = 0,count = 0,port_index = 0;
											
																								  
																								  mstid =  strtoul (instance,0,10);
																								  ccgi_get_broad_product_id(&productid);//2008.07.29取得产品id
																								   ret = ccgi_get_br_self_info(mstid, &pvid, &num, 0,&aa);
																								  //fprintf(stderr,"mstid=%d,proudctid=%d,ret=%d,pvid=%d,num=%d\n",mstid,productid,ret,*pvid,num);
																								  if(ret == 0)
																								  {
																									  //正常
																									  if(0 != mstid) 
																									  {
																										  tmp = pvid;
																										  for(j = 0; j < num; j++) 
																										  {
																											  ret |= ccgi_get_one_vlan_portmap(*tmp,&ports_bmp);
																											  //DCLI_DEBUG(("dcli_stp 1804:: show instance vid %d\n",*tmp));
																											  if(CMD_SUCCESS == ret) 
																											  {
																												 // portsmap = (ports_bmp.tagbmp | ports_bmp.untagbmp);
																												  
																													portsmap.portMbr[0]= (ports_bmp.tagbmp.portMbr[0]| ports_bmp.untagbmp.portMbr[0]);
                                                                                                                    portsmap.portMbr[1]= (ports_bmp.tagbmp.portMbr[1]| ports_bmp.untagbmp.portMbr[1]);

																												  
																												  //DCLI_DEBUG(("dcli_stp 1804:: show instance vid %d,portsmap %02x,\n",*tmp,portsmap));
																												  show_slot_port_by_productid(productid,&portsmap,*tmp,mstid);
																												  tmp++;
																												  memset(&ports_bmp,0,sizeof(VLAN_PORTS_BMP));
																											  }
																										  }
																									  }
																									  else 
																									  {
																										  if((ret |= ccgi_change_all_ports_to_bmp(&ports_bmp,&count)) < 0)
																										  {
																											  //DCLI_DEBUG(("execute command failed\n"));
																											  return CMD_FAILURE;
																										  }
																										  else
																										  {
																											 // portsmap = (ports_bmp.tagbmp | ports_bmp.untagbmp);
																											 
																											portsmap.portMbr[0]= (ports_bmp.tagbmp.portMbr[0]| ports_bmp.untagbmp.portMbr[0]);
                                                                                                            portsmap.portMbr[1]= (ports_bmp.tagbmp.portMbr[1]| ports_bmp.untagbmp.portMbr[1]);

																											  //DCLI_DEBUG(("dcli_stp 1804:: show instance vid %d,portsmap %02x,\n",*tmp,portsmap));
																											  for (i=0;i<64;i++) 
																											  {
																												  if(PRODUCT_ID_AX7K == productid)
																												  {
																													  slot = i/8 + 1;
																													  port = i%8;
																												  }
                                                                                                            	 else if((PRODUCT_ID_AX5K == productid) ||
																													(PRODUCT_ID_AX5K_I == productid) ||
																													(PRODUCT_ID_AU4K == productid) ||
																													(PRODUCT_ID_AU3K == productid) ||
																													(PRODUCT_ID_AU3K_BCM == productid) ||
																													(PRODUCT_ID_AU3K_BCAT == productid) || 
																													(PRODUCT_ID_AU2K_TCAT == productid)){
                                                                                                            		slot = 1;
                                                                                                            		port = i;
                                                                                                            	}

																												 tmpVal[i/32] = (1<<(i%32));
					                                                                                             if(portsmap.portMbr[i/32]& tmpVal[i/32]) {				  
																													  //vty_out(vty,"%-d/%-10d",slot,port);
																													  //vty_out(vty,"%-4s"," ");
																													  if((ret |= ccgi_get_one_port_index(slot,port,&port_index)) < 0) 
																													  {
																													  //DCLI_DEBUG(("execute command failed\n"));
																														  continue;
																													  }
																													  fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));
																													  ret |= ccgi_get_mstp_one_port_info(mstid,1,port_index,slot,port);
																													  cl=!cl;
																													  
																												  }
																											  }
																									  
																										  }
																									  }
																									  if(0 != num)
																										  free(pvid);	  
											
																								  }
																								  else if(ret == DCLI_STP_NO_SUCH_MSTID)
																								  {
																									  //实例不存在
																									  br_stat = 1;
																								  }
											
																							  }
																							  else
																							  {
																								  br_stat = -2;//rstp running
																							  }
																						  }
																						  else
																						  {
																							  br_stat = -1;
																						  } 																	  
											
											
																 fprintf(cgiOut,"</table></div>");
																					  if(br_stat == -1)
																						  fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"no_start_br"));
																					  if(br_stat == -2)
																						  fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"stp_run"));
																					  if(br_stat == 1)
																						  fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"instance_not_exist"));															  
																	  fprintf(cgiOut,"</td>"\
																				"</tr>"\
																				  "<tr>");
																					if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
																					{
																					  fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",encry);
																					  fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
																					}
																					else if(cgiFormSubmitClicked("submit_stp") == cgiFormSuccess)
																					{			   
																					  fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",stp_encry);
																					  fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
																					}
																  fprintf(cgiOut,"</tr>"\
																			  "</table>");

										}
										else if(strcmp(instance,"")==0)
										{
											fprintf(cgiOut,"<table><tr><td><font color=red>%s</font></td></tr></table>",search(lcon,"instance_not_null"));
										}
										else
										{
											ShowAlert(search(lcon,"instance_std"));
										}		
																		

										if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
										{
										  fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",encry);
										  fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
										}
										else if(cgiFormSubmitClicked("submit_stp") == cgiFormSuccess)
										{			   
										  fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",stp_encry);
										  fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
										}
	
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
	free(CheckUsr);												 
	return 0;

}


//显示端口信息
int ccgi_get_mstp_one_port_info
(
	int 				mstid,
	unsigned int vid,
	unsigned int port_index,
	unsigned int slot,
	unsigned int port
)
{
	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;
	DBusMessageIter	 stpIter_struct;
	DBusMessageIter	 stpIter_sub_struct;

	char buf[10] = {0};
	int n,ret;

	unsigned char  port_prio;
	unsigned int 	port_cost;
	int 					port_role;
	int 					port_state;
	int 					port_lk;
	int 					port_p2p;
	int 					port_edge;
	unsigned short br_prio;
	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int     br_cost;
	unsigned short br_dPort;

	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								MSTP_DBUS_METHOD_GET_PORT_INFO);

	dbus_message_append_args(stpQuery,
					 DBUS_TYPE_UINT32,&mstid,
					 DBUS_TYPE_UINT32,&(port_index),
					 DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	stpReply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,stpQuery,-1, &err);


	dbus_message_unref(stpQuery);
	if (NULL == stpReply) {
		//vty_out(vty,"failed get stp stpReply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(stpReply);
		return CMD_FAILURE;
	}

	dbus_message_iter_init(stpReply,&stpIter);				
	dbus_message_iter_get_basic(&stpIter,&ret);
	dbus_message_iter_next(&stpIter);
	if(DCLI_STP_OK == ret)
	{
	/*		
		Array of Port Infos.
		port no
		port prio
		port role
		port State
		port link
		port p2p
		port edge
		port Desi bridge
		port Dcost
		port D-port
	*/

		dbus_message_iter_recurse(&stpIter,&stpIter_struct);

		dbus_message_iter_get_basic(&stpIter_struct,&port_prio);
		
		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_cost);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_role);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_state);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_lk);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_p2p);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_edge);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&br_cost);			

		dbus_message_iter_next(&stpIter_struct);			
		dbus_message_iter_get_basic(&stpIter_struct,&br_dPort);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_recurse(&stpIter_struct,&stpIter_sub_struct);


		dbus_message_iter_get_basic(&stpIter_sub_struct,&br_prio);
		dbus_message_iter_next(&stpIter_sub_struct);

		for(n = 0; n <6; n++)
		{
			dbus_message_iter_get_basic(&stpIter_sub_struct,&mac[n]);
			dbus_message_iter_next(&stpIter_sub_struct);
		}						

		//vty_out(vty,"%-4d",port_prio);
		//vty_out(vty,"%-9d",port_cost);
		//vty_out(vty,"%-5s",stp_port_role[port_role]);
		//vty_out(vty,"%-13s",stp_port_state[port_state]);
		//vty_out(vty,"%-3s", port_lk ? "Y" : "N");
		fprintf(cgiOut,"<td style=font-size:12px align=left>%d/%d</td>",slot,port);
		if(mstid == 0)
			fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","");
		else
			fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",vid);		
		fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",port_prio);
		fprintf(cgiOut,"<td style=font-size:12px align=left>%d</td>",port_cost);
		fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",stp_port_role[port_role]);
		fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",stp_port_state[port_state]);
		fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",port_lk  ? "Y" : "N");	
		if(0 == port_p2p)
			fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","N");
		else if(1 == port_p2p)
			fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","Y");
		else if(2 == port_p2p)
			fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","A");
		
		//vty_out(vty,"%-4s",port_edge ? "Y" : "N");
		fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",port_edge ? "Y" : "N");
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",br_prio);
		if(port_state)
		{				
			fprintf(cgiOut,"<td style=font-size:12px align=left>%s:%02x%02x%02x%02x%02x%02x</td>",port_state ? buf : "",mac[0]\
				,mac[1],mac[2],mac[3],mac[4],mac[5]); 
		}
		else
		{
			fprintf(cgiOut,"<td style=font-size:12px align=left></td>");
		}
		
		memset(buf,0,sizeof(buf));
		sprintf(buf,"&nbsp;&nbsp;&nbsp;%d",br_cost);
		//vty_out(vty,"%-10s",port_state ? buf : "");
fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",port_state ? buf : "");		
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%#0x",br_dPort);
		//vty_out(vty,"%s",port_state ? buf : "");
fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",port_state ? buf : ""); 		
				
		//vty_out(vty,"\n");

		dbus_message_unref(stpReply);

		return CMD_SUCCESS;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(stpReply);
		return ret;
	}
	else
	{
		dbus_message_unref(stpReply);
		return ret;
	}	
		
}

//显示非0实例端口信息
int show_slot_port_by_productid
(
	unsigned int product_id,
	PORT_MEMBER_BMP* portBmp,
	unsigned int vid,
	unsigned int mstid
)
{

	unsigned int i,port_index = 0;
	unsigned int slot = 0,port = 0;
	int clor=1; 

	unsigned int tmpVal[2];
	memset(&tmpVal,0,sizeof(tmpVal));

	for (i=0;i<64;i++) {
		if(PRODUCT_ID_AX7K == product_id) {
			slot = i/8 + 1;
			port = i%8;
			
		}
		else if((PRODUCT_ID_AX5K == product_id) ||
				(PRODUCT_ID_AX5K_I == product_id) ||
				(PRODUCT_ID_AU4K == product_id) ||
				(PRODUCT_ID_AU3K == product_id) ||
				(PRODUCT_ID_AU3K_BCM == product_id) ||
				(PRODUCT_ID_AU3K_BCAT == product_id) || 
				(PRODUCT_ID_AU2K_TCAT == product_id)){
			slot = 1;
			port = i;
		}

		
		tmpVal[i/32] = (1<<(i%32));
		if(portBmp->portMbr[i/32] & tmpVal[i/32]) {				
			
			//vty_out(vty,"%-d/%-10d",slot,port);
			//vty_out(vty,"%-4d",vid);
			if(ccgi_get_one_port_index(slot,port,&port_index) < 0)	{
				//DCLI_DEBUG(("execute command failed\n"));
				continue;
			}
			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(clor));
			ccgi_get_mstp_one_port_info(mstid,vid,port_index,slot,port);
			clor = !clor;
		}
	}
		
	return 0;
}



