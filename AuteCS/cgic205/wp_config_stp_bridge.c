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
* wp_config_stp_bridge.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for stp brige config 
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


int ShowSTPbridge(struct list *lpublic,struct list *lcon);
int config_bridge(struct list *lpublic,struct list *lcon,bridge_info br_info);


int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcon;	   /*解析control.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcon=get_chain_head("../htdocs/text/control.txt");
	ccgi_dbus_init();
	ShowSTPbridge(lpublic,lcon);
	release(lpublic);  
	release(lcon);

	return 0;
}

int ShowSTPbridge(struct list *lpublic,struct list *lcon)
{
	  char *encry=(char *)malloc(BUF_LEN);				//存储从wp_usrmag.cgi带入的加密字符串
	  char *str;
	 
	  char stp_encry[BUF_LEN]; 
	  char rstp[N];
	  int i;
	  int stpmode1;
	  int ret = -1;
      bridge_info br_info;
	  memset(br_info.root_br_mac,0,sizeof(br_info.root_br_mac));
	  memset(br_info.design_br_mac,0,sizeof(br_info.design_br_mac));

      if((cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)&&(cgiFormSubmitClicked("default") != cgiFormSuccess))
      {
 			 memset(encry,0,BUF_LEN);
 			 cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
 			 str=dcryption(encry);
 			 if(str==NULL)
 			 {
	 			   ShowErrorPage(search(lpublic,"ill_user"));	 //用户非法
	 			   return 0;
 			 }
 			 memset(stp_encry,0,BUF_LEN); 				  //清空临时变量
	  }

		cgiFormStringNoNewlines("stp_encry",stp_encry,BUF_LEN);
		cgiHeaderContentType("text/html");
		fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
		fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
		fprintf(cgiOut,"<title>%s</title>",search(lcon,"stp_man"));
		fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
		  "<style type=text/css>"\
		  ".a3{width:30;border:0; text-align:center}"\
		  "</style>"\
		"</head>"\
		"<script src=/ip.js>"\
		"</script>"\
		"<body>");
		memset(rstp,0,N); 
		cgiFormStringNoNewlines("rstp",rstp,N);
		if(cgiFormSubmitClicked("default") == cgiFormSuccess)
		{
				stpmode1 = config_spanning_tree_mode("stp");//选择RSTP模式
				if(stpmode1 == 0 || stpmode1 ==1)
				{
					if(config_spanning_tree(rstp,ccgi_dbus_connection)==0)//开启或关闭桥配置
					{
						if(ccgi_get_brg_g_state(&stpmode1)==1)
						{
							ret = config_spanning_tree_default();//恢复默认配置
							if(ret == 0)
							{
								ShowAlert(search(lpublic,"oper_succ"));
							}
							else
							{
								ShowAlert(search(lpublic,"oper_fail"));
							}
						}
						else
						{
							ShowAlert(search(lcon,"no_start_br"));
						}
					}
					else
					{
						ShowAlert(search(lpublic,"oper_fail"));
					}
				}
				else if(stpmode1 == 2)
				{
					ShowAlert(search(lcon,"mstp_run"));
				}
				else
				{
					ShowAlert(search(lcon,"sel_mode_fail"));
				}
		}
		else if(cgiFormSubmitClicked("submit_stp") == cgiFormSuccess)
    	{
				
				stpmode1 = config_spanning_tree_mode("stp");//选择RSTP模式
				if(stpmode1 == 0 || stpmode1 ==1)
				{
					if(strcmp(rstp,"disable")==0)//关闭桥
					{
						if(config_spanning_tree(rstp,ccgi_dbus_connection)==0)
						{
							ShowAlert(search(lcon,"stop_succ"));
						}
						else
						{
							ShowAlert(search(lcon,"stop_fail"));
						}
					}
					else
					{
						if(config_spanning_tree(rstp,ccgi_dbus_connection)==0)//开启桥配置
						{
							if(ccgi_get_brg_g_state(&stpmode1)==1)
							{  
							    ccgi_get_br_info(&br_info);
								ret = config_bridge(lpublic,lcon,br_info);
								if(ret == 0)
								{
									ShowAlert(search(lpublic,"oper_succ"));
								}
								else if(ret == -1)
								{
									ShowAlert(search(lpublic,"oper_fail"));
								}
							}
							else
							{
								ShowAlert(search(lcon,"no_start_br"));
							}
						}
						else
						{
							ShowAlert(search(lcon,"start_fail"));
						}
					}
				}
				else if(stpmode1 == 2)
				{
					ShowAlert(search(lcon,"mstp_run"));
				}
				else
				{
					ShowAlert(search(lcon,"sel_mode_fail"));
				}
    	}

	  fprintf(cgiOut,"<form method=post >"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	  "<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
		"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>RSTP</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
			
				
			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
			  "<tr>"\
			  "<td width=62 align=center><input id=but type=submit name=submit_stp style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
			  if((cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)&&(cgiFormSubmitClicked("default") != cgiFormSuccess))
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
						if((cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)&&(cgiFormSubmitClicked("default") != cgiFormSuccess))
						{
							fprintf(cgiOut,"<tr height=25>"\
							 "<td align=left id=tdleft><a href=wp_stp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"br_info")); 
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"br_conf"));   //突出显示
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							  "<td align=left id=tdleft><a href=wp_show_port.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"port_info"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							  "<td align=left id=tdleft><a href=wp_config_stp_port.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"port_conf"));
							fprintf(cgiOut,"</tr>");
						}
						else
						{
							fprintf(cgiOut,"<tr height=25>"\
							 "<td align=left id=tdleft><a href=wp_stp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"br_info")); 
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr height=26>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"br_conf"));   //突出显示
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							  "<td align=left id=tdleft><a href=wp_show_port.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"port_info"));
							fprintf(cgiOut,"</tr>"\
							"<tr height=25>"\
							  "<td align=left id=tdleft><a href=wp_config_stp_port.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"port_conf"));
							fprintf(cgiOut,"</tr>");
						}



						  for(i=0;i<5;i++)
						  {
							fprintf(cgiOut,"<tr height=25>"\
							  "<td id=tdleft>&nbsp;</td>"\
							"</tr>");
						  }
	
					  fprintf(cgiOut,"</table>"\
				  "</td>"\
				  "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:20px\">"\
						  "<table border=0 cellspacing=0 cellpadding=0>");
					  fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td>%s:</td>",search(lcon,"br_conf"));
						fprintf(cgiOut,"<td width=70><select name=rstp style=width:138px><option value=enable>start<option value=disable>stop</select></td>"\
						"<td><font color=red>(%s)</font></td>",search(lcon,"start_br"));

                             // 优先级配置有问题，字符串也可以了
                             
					fprintf(cgiOut,"<tr height=30>");
					  fprintf(cgiOut,"<td>%s:</td>",search(lcon,"prior"));
					  fprintf(cgiOut,"<td width=140><input type=text name=prio size=21></td>"\
					  "<td><font color=red>(0--61440,%s)</font></td>",search(lcon,"times_4096"));
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=30>");
					  fprintf(cgiOut,"<td>%s:</td>",search(lcon,"max_age"));
					  fprintf(cgiOut,"<td width=140><input type=text name=max_age size=21></td>"\
					  "<td><font color=red>(6--40)</font></td>");
					fprintf(cgiOut,"</tr>");														  
					fprintf(cgiOut,"<tr height=30>");
					  fprintf(cgiOut,"<td>%s:</td>",search(lcon,"hello_t"));
					  fprintf(cgiOut,"<td width=140><input type=text name=hello size=21></td>"\
					  "<td><font color=red>(1--%d)</font></td>",10);
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=30>");
					  fprintf(cgiOut,"<td>%s:</td>",search(lcon,"delay_t"));
					  fprintf(cgiOut,"<td width=140><input type=text name=delay size=21></td>"\
					  "<td><font color=red>(4--%d)</font></td>",30);
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=30>");
					  fprintf(cgiOut,"<td>%s:</td>",search(lcon,"rstp_v"));
					  fprintf(cgiOut,"<td width=70><select name=rstp_ver style=width:138px><option value=2>2<option value=0>0</select></td>"\
					  );
					  fprintf(cgiOut,"<td><input type=submit name=default value=%s /></td>",search(lcon,"re_default"));
					fprintf(cgiOut,"</tr>");



					
					fprintf(cgiOut,"<tr>");
									  if((cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)&&(cgiFormSubmitClicked("default") != cgiFormSuccess))
									  {
										fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",encry);
									  }
									  else
									  { 			 
										fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",stp_encry);
									  }
					fprintf(cgiOut,"</tr>"\
	        			"</table>"\
					"<table width=750 border=0 cellspacing=0 cellpadding=0>"
						"<tr>"\
							"<td  id=sec1 style=\"padding-left:0px;width=750; border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
							fprintf(cgiOut,"</tr>");
							
							fprintf(cgiOut,"<tr height=25 style=\"padding-left:0px; padding-top:2px\">"\
							  "<td style=font-size:14px;color:#FF0000>%s</td>"\
						"</tr></table>",search(lpublic,"relation_des"));
							
							

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


int config_bridge(struct list *lpublic,struct list *lcon,bridge_info br_info)
{
	char prio[N],max_age[N],hello[N],delay[N],rstp_ver[N];	//**************char[N]
	int prio_arg, max_age_arg, hello_arg, delay_arg, rstp_ver_arg;
	int flag = 0;
	int stpmode;
	int result,ret;
	int prio_call = -1,max_age_call = -1,hello_call = -1,delay_call = -1,rstp_ver_call = -1;
	char arg_std[512];
	memset(prio,0,N); 
	memset(max_age,0,N); 
	memset(hello,0,N); 
	memset(delay,0,N); 
	memset(rstp_ver,0,N); 
	
	result = cgiFormStringNoNewlines("prio",prio,N);   //页面取得prio 数值
	if(result == cgiFormSuccess)
	{
       ret=check_abc(prio);
	   if(ret==0)
	   	{
	   prio_arg = strtoul (prio,0,10);//参数合法性检查，如果是字符串，就返回了零	不确切		
	   
		if(prio_arg < MIN_BR_PRIO || prio_arg > MAX_BR_PRIO){
			flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"prior_err"));
			//strcat(arg_std,search(lcon,"arg_std"));
			ShowAlert(arg_std);			
			//ShowAlert("prio_arg参数不符合标准");
		}
		else{ 
			    if(0 != (prio_arg%4096)){
				flag = -1;
				memset(arg_std,0,512);
				strcat(arg_std,search(lcon,"prior_err"));
				//strcat(arg_std,search(lcon,"arg_std"));
				ShowAlert(arg_std);

				//ShowAlert("prio_arg参数不符合标准");
			}
		}
		if(flag != -1)
		prio_call = 0;	
		
		}else{
		    flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"prior_err"));
			ShowAlert(arg_std);

			}
	}

	result = cgiFormStringNoNewlines("max_age",max_age,N);
	if(result == cgiFormSuccess)
	{
		max_age_arg = strtoul (max_age,0,10);
		if(max_age_arg < MIN_BR_MAXAGE || max_age_arg > MAX_BR_MAXAGE ){
			flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"max_age_err"));
			//strcat(arg_std,search(lcon,"arg_std"));
			ShowAlert(arg_std);			
			//ShowAlert("max_age参数不符合标准");
		}
		if(flag != -1)
		max_age_call = 0;	
	}else{	
	   max_age_arg=br_info.design_br_maxAge;	  
	   max_age_call=1;
		}

	result = cgiFormStringNoNewlines("hello",hello,N);
	if(result == cgiFormSuccess)
	{
		hello_arg = strtoul (hello,0,10);
		if(hello_arg < MIN_BR_HELLOT || hello_arg > MAX_BR_HELLOT )
		{
			flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"hello_t_err"));
			//strcat(arg_std,search(lcon,"arg_std"));
			ShowAlert(arg_std);			
			//ShowAlert("hello参数不符合标准");
		}
		if(flag != -1)
		hello_call = 0;	
	}else{	 
	  hello_arg=br_info.design_br_hTime;	  
	  hello_call=1;
		}

	result = 	cgiFormStringNoNewlines("delay",delay,N);
	if(result == cgiFormSuccess)
	{
		delay_arg = strtoul (delay,0,10);
		if(delay_arg < MIN_BR_FWDELAY || delay_arg > MAX_BR_FWDELAY )
		{
			flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"delay_t_err"));
			//strcat(arg_std,search(lcon,"arg_std"));
			ShowAlert(arg_std);			
			//ShowAlert("delay参数不符合标准");
		}	
		if(flag != -1)
		delay_call = 0;	
	}else{	 
	  delay_arg=br_info.design_br_fdelay;	
	  delay_call=1;
		}

	result = cgiFormStringNoNewlines("rstp_ver",rstp_ver,N);
	if(result == cgiFormSuccess)
	{
		rstp_ver_arg = strtoul (rstp_ver,0,10);
		if (!((rstp_ver_arg == 0)||(rstp_ver_arg == 2)) ) {
			flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"rstp_v_err"));
			//strcat(arg_std,search(lcon,"arg_std"));
			ShowAlert(arg_std);

			//ShowAlert("rstp_ver参数不符合标准");
		}
		if(flag != -1)
		rstp_ver_call = 0;	
	}

   //判断条件参数函数
	if((max_age_call==0) && (hello_call==0) && (delay_call==0))
	{

		if((2*(hello_arg + 1) > max_age_arg) || (max_age_arg > 2*(delay_arg - 1)))
		{
			flag = -1;
			ShowAlert(search(lcon,"arg_std"));
		}

	}
	//判断条件参数函数
	if((max_age_call==1) || (hello_call==1) || (delay_call==1))
	{
	    if((max_age_call==0) || (hello_call==0) ||(delay_call==0)){

		if((2*(hello_arg + 1) > max_age_arg) || (max_age_arg > 2*(delay_arg - 1)))
		{
			flag = -1;
			ShowAlert(search(lcon,"arg_std"));
		}
	    	}

	}
	if(flag==0)
	{

		if(ccgi_get_brg_g_state(&stpmode)==1)
		{
			if(prio_call==0)
				config_spanning_tree_pri(prio);
			if(max_age_call==0 ||max_age_call==1)
				config_spanning_tree_max_age(max_age);
			if(hello_call==0 || hello_call==1)
				config_spanning_tree_hello_time(hello);
			if(delay_call==0 || delay_call==1) 
				config_spanning_tree_forward_delay(delay);
			if(rstp_ver_call==0)
				config_spanning_tree_version(rstp_ver);
			return 0;
		}
		else
		{
			ShowAlert(search(lcon,"no_start_br"));
			return -1;
		}
	}
	if(flag == -1)
		return -2;
	return 0;



}




