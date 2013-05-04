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
* wp_config_igmp.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for igmp config
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
#include "ws_igmp_snp.h"


int ShowIGMPConfigPage(struct list *lpublic,struct list *lcon);
int config_igmp(struct list *lcon);


int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcon;	   /*解析control.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcon=get_chain_head("../htdocs/text/control.txt");
	ccgi_dbus_init();
    ShowIGMPConfigPage(lpublic,lcon);  
	release(lpublic);  
	release(lcon);

 	return 0;
}

int ShowIGMPConfigPage(struct list *lpublic,struct list *lcon)
{
	  char *encry=(char *)malloc(BUF_LEN);				/*存储从wp_usrmag.cgi带入的加密字符串*/
	  char *str;
	  
	  char igmp_encry[BUF_LEN]; 
	  int i,retu;
	  
	  char igmp[N];
	  int op_ret;
	  int vlan_ret;
	  
	  igmp_timer test;
	  memset(&test,0,sizeof(test));

      if(cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess)
      {
 			 memset(encry,0,BUF_LEN);
 			 cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
 			 str=dcryption(encry);
			 retu = checkuser_group(str);
 			 if(str==NULL)
 			 {
	 			   ShowErrorPage(search(lpublic,"ill_user"));	 /*用户非法*/
	 			   return 0;
 			 }
 			 memset(igmp_encry,0,BUF_LEN); 				  /*清空临时变量*/
	  }
		memset(igmp_encry,0,BUF_LEN); 				  /*清空临时变量*/
		cgiFormStringNoNewlines("igmp_encry",igmp_encry,BUF_LEN);
		
		if(cgiFormSubmitClicked("submit_igmp") == cgiFormSuccess)
      		{
				str=dcryption(igmp_encry);
				retu = checkuser_group(str);
			}
		
		cgiHeaderContentType("text/html");
		fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
		fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
		fprintf(cgiOut,"<title>%s</title>","IGMP SNOOPING");
		fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
		  "<style type=text/css>"\
		  ".a3{width:30;border:0; text-align:center}"\
		  "</style>"\
		"</head>"\
		"<script src=/ip.js>"\
		"</script>"\
		"<body>");
		
		memset(igmp, 0, N);
		cgiFormStringNoNewlines("igmp",igmp,N);
		if(cgiFormSubmitClicked("submit_igmp") == cgiFormSuccess)
		{
				int ret = -1;
				ret = igmp_snooping_able(igmp);
				//show_igmp_snp_time_interval_new(&test);
				if(strcmp(igmp,"disable")==0)
				{
					if(ret == 0)
					{
						ShowAlert(search(lcon,"stop_succ"));
					}
					else if(ret == 2)
					{
						ShowAlert(search(lcon,"igmp_no_sta"));
					}
					else if(ret == -1)
					{
						ShowAlert(search(lcon,"stop_fail"));
					}
						
				}
				else if(strcmp(igmp,"enable")==0)
				{
					if(ret == 0 || ret == 1)
					{
						if(config_igmp(lcon)==0)
							ShowAlert(search(lpublic,"oper_succ"));
						//else
						//	ShowAlert(search(lpublic,"oper_fail"));
					}
					else if(ret == -1)
					{
						ShowAlert(search(lcon,"start_fail"));
					}

				}

	    //配置IGMP SNOOPING
		}	
	
	  fprintf(cgiOut,"<form method=post>"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	  "<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
		"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>IGMP SNOOPING</font></td>");
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
			

			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
			  "<tr>"\
			  "<td width=62 align=center><input id=but type=submit name=submit_igmp style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
			  if(cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess)
				fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry ,search(lpublic,"img_cancel"));
			  else										   
				fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",igmp_encry ,search(lpublic,"img_cancel"));
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
						if(cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess)
						{

						    //search(lcon,"config")
							fprintf(cgiOut,"<tr height=26>"\
							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san></font></td>",search(lpublic,"menu_san"),search(lcon,"igmp_time_para"));
							fprintf(cgiOut,"</tr>");

							//新增条目
					    fprintf(cgiOut,"<tr height=25>"\
					     "<td align=left id=tdleft><a href=wp_igmp_vlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"igmp_conf"));
					    fprintf(cgiOut,"</tr>");

                         /*
						//新增条目
					    fprintf(cgiOut,"<tr height=25>"\
					     "<td align=left id=tdleft><a href=wp_igmp_view.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"IGMP信息");
					    fprintf(cgiOut,"</tr>");
                        */
						}
						else if(cgiFormSubmitClicked("submit_igmp") == cgiFormSuccess)
						{
							fprintf(cgiOut,"<tr height=26>"\
							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san></font></td>",search(lpublic,"menu_san"),search(lcon,"igmp_time_para"));
							fprintf(cgiOut,"</tr>");

							//新增条目
					    fprintf(cgiOut,"<tr height=25>"\
					     "<td align=left id=tdleft><a href=wp_igmp_vlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",igmp_encry,search(lpublic,"menu_san"),search(lcon,"igmp_conf"));
					    fprintf(cgiOut,"</tr>");

                           /*
						//新增条目
					    fprintf(cgiOut,"<tr height=25>"\
					     "<td align=left id=tdleft><a href=wp_igmp_view.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",igmp_encry,search(lpublic,"menu_san"),"IGMP信息");
					    fprintf(cgiOut,"</tr>");
							*/
						}
						for(i=0;i<9;i++)
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
														  "<td align=left valign=top  style=\"padding-top:10px\">");
								fprintf(cgiOut,"<table border=0 cellspacing=0 cellpadding=0>");
									fprintf(cgiOut,"<tr height=30>");
									  fprintf(cgiOut,"<td>%s:</td>","IGMP SNOOPING");
									  if(retu == 0)
									  	{
										  	fprintf(cgiOut,"<td width=70><select name=igmp style=width:138px><option value=enable>start<option value=disable>stop</select></td>"\
										 	 "<td><font color=red>(%s)</font></td>",search(lcon,"start_stop"));
									  	}
									  else
									  	{
									  		fprintf(cgiOut,"<td width=70><select name=igmp disabled style=width:138px><option value=enable>start<option value=disable>stop</select></td>"\
										 	 "<td><font color=red>(%s)</font></td>",search(lcon,"start_stop"));
									  	}
									 
									  //可设置开启状态

                                    op_ret = igmp_snooping_able(igmp);
									show_igmp_snp_time_interval_new(&test);
                                    if(op_ret==1){
									    
									fprintf(cgiOut,"<tr height=30>");
									  	fprintf(cgiOut,"<td>%s:</td>",search(lcon,"vlan_lifetime"));
									  	fprintf(cgiOut,"<td width=140><input type=text name=vlan_lifetime size=21 value=\"%d\"></td>"\
									  	"<td><font color=red>(10000--100000)</font></td>",test.vlanlife);
									fprintf(cgiOut,"<tr height=30>");
										fprintf(cgiOut,"<td>%s:</td>",search(lcon,"group_lifetime"));
										fprintf(cgiOut,"<td width=140><input type=text name=group_lifetime size=21 value=\"%d\"></td>"\
										"<td><font color=red>(1000--50000)</font></td>",test.grouplife);		
									fprintf(cgiOut,"<tr height=30>");
										fprintf(cgiOut,"<td>%s:</td>",search(lcon,"query_interval"));
										fprintf(cgiOut,"<td width=140><input type=text name=query_interval size=21 value=\"%d\"></td>"\
										"<td><font color=red>(1000--10000)</font></td>",test.queryinterval);
									fprintf(cgiOut,"<tr height=30>");
										fprintf(cgiOut,"<td>%s:</td>",search(lcon,"roust"));
										fprintf(cgiOut,"<td width=140><input type=text name=robust size=21 value=\"%d\"></td>"\
										"<td><font color=red>(1--100)</font></td>",test.robust);
										fprintf(cgiOut,"<tr height=30>");
										fprintf(cgiOut,"<td>%s:</td>",search(lcon,"response"));
										fprintf(cgiOut,"<td width=140><input type=text name=response size=21 value=\"%d\"></td>"\
										"<td><font color=red>(100--1000)</font></td>",test.respinterval);

								    fprintf(cgiOut,"<tr height=5><td></td></tr>");
									int vlan_count=0;
     								vlan_ret = iShow_igmp_vlan_count(&vlan_count);
     								
     								if(vlan_ret == 0)
     								{
     									fprintf(cgiOut,"<tr>"\
     										"<td>%s</td>",search(lcon,"igmp_vlan_count")); 
     										fprintf(cgiOut,"<td>%d</td>"\
     									"</tr>",vlan_count);
     								}
     								fprintf(cgiOut,"<tr height=10><td></td></tr>");
									}
									//  if(op_ret==2)
									 // 	 ShowAlert("未开启");

										
								fprintf(cgiOut,"</table>");


								fprintf(cgiOut,"</td>"\
										  "</tr>"\
											"<tr>");
											  if(cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess)
											  {
												fprintf(cgiOut,"<td><input type=hidden name=igmp_encry value=%s></td>",encry);
											  }
											  else if(cgiFormSubmitClicked("submit_igmp") == cgiFormSuccess)
											  { 			 
												fprintf(cgiOut,"<td><input type=hidden name=igmp_encry value=%s></td>",igmp_encry);
											  }
        									fprintf(cgiOut,"</tr>"\
        												"</table>"\
														"<table width=700 border=0 cellspacing=0 cellpadding=0>"
															"<tr>"\
																"<td  id=sec1 style=\"padding-left:0px;width=700; border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
																fprintf(cgiOut,"</tr>");
																
																fprintf(cgiOut,"<tr height=25 style=\"padding-left:0px; padding-top:2px\">"\
																  "<td style=font-size:14px;color:#FF0000>%s</td>"\
															"</tr></table>",search(lpublic,"life_des"));							
																
																	
	
	
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


int config_igmp(struct list *lcon)
{
	char vlan_lifetime[N],group_lifetime[N],robust[N],query_interval[N],response[N];
	int vlan_lifetime_call=-1,group_lifetime_call=-1,robust_call=-1,query_interval_call=-1,response_call=-1;
	int vlan_lifetime_arg,group_lifetime_arg,robust_arg,query_interval_arg,response_arg;
	int flag = 0;
	char arg_std[512];
	
	memset(vlan_lifetime,0,N);
	memset(group_lifetime,0,N);
	memset(robust,0,N);
	memset(query_interval,0,N);

	//参数合法性检查
	if(cgiFormStringNoNewlines("vlan_lifetime",vlan_lifetime,N)==cgiFormSuccess)
	{
		vlan_lifetime_arg = strtoul(vlan_lifetime,0,10);
		if(vlan_lifetime_arg < 10000 || vlan_lifetime_arg > 100000){
			flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"vlan_lifetime_err"));
			//strcat(arg_std,search(lcon,"arg_std"));
			ShowAlert(arg_std);			
			//ShowAlert("prio_arg参数不符合标准");
		}
		if(flag != -1)
			vlan_lifetime_call = 0;
	}

	if(cgiFormStringNoNewlines("group_lifetime",group_lifetime,N)==cgiFormSuccess)
	{
		group_lifetime_arg = strtoul(group_lifetime,0,10);
		if(group_lifetime_arg < 1000 || group_lifetime_arg > 50000){
			flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"group_lifetime_err"));
			//strcat(arg_std,search(lcon,"arg_std"));
			ShowAlert(arg_std);			
			//ShowAlert("prio_arg参数不符合标准");
		}
		if(flag != -1)
			group_lifetime_call = 0;
	}

	if(cgiFormStringNoNewlines("robust",robust,N)==cgiFormSuccess)
	{
		robust_arg = strtoul(robust,0,10);
		if(robust_arg < 1 || robust_arg > 100){
			flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"roust_err"));
			//strcat(arg_std,search(lcon,"arg_std"));
			ShowAlert(arg_std);			
			//ShowAlert("prio_arg参数不符合标准");
		}
		if(flag != -1)
			robust_call = 0;
	}

	if(cgiFormStringNoNewlines("query_interval",query_interval,N)==cgiFormSuccess)
	{
		query_interval_arg = strtoul(query_interval,0,10);
		if(query_interval_arg < 1000 || query_interval_arg > 10000){
			flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"query_interval_err"));
			//strcat(arg_std,search(lcon,"arg_std"));
			ShowAlert(arg_std);			
			//ShowAlert("prio_arg参数不符合标准");
		}
		if(flag != -1)
			query_interval_call = 0;
	}

	if(cgiFormStringNoNewlines("response",response,N)==cgiFormSuccess)
	{
		response_arg = strtoul(response,0,10);
		if(response_arg < 100 || response_arg > 1000){
			flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"response_err"));
			//strcat(arg_std,search(lcon,"arg_std"));
			ShowAlert(arg_std);			
			//ShowAlert("prio_arg参数不符合标准");
		}
		if(flag != -1)
			response_call = 0;
	}

	//调用配置函数
	if(flag == 0)
	{
		if(vlan_lifetime_call == 0)
		{
			config_igmp_snooping(1,vlan_lifetime);
		}
		if(group_lifetime_call == 0)
		{
			config_igmp_snooping(2,group_lifetime);
		}	
		if(robust_call == 0)
		{
			config_igmp_snooping(3,robust);
		}
		if(query_interval_call == 0)
		{
			config_igmp_snooping(4,query_interval);
		}
		if(response_call == 0)
		{
			config_igmp_snooping(5,response);
		}
	}
	
	if(flag == -1)
		return -2;
	return 0;

}

