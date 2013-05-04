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
* wp_config_stp_port.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for stp port config 
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

static unsigned int productid = 1;//产品	ID
//static int config_mode = 0;

int ShowSTPbridge(struct list *lpublic,struct list *lcon);
int config_port(char *port,struct list *lpublic,struct list *lcon);

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
	  char port[N],rstp[N];
	  int i,ret = -1;
	  int stpmode1;
	  int port_state;
	  //int cl=1; 				//cl标识表格的底色，1为#f9fafe，0为#ffffff

     ETH_SLOT_LIST  head,*p;
     ETH_PORT_LIST *pp;
     int result,p_num,num;
     char *slot_port=(char *)malloc(10);
////////////// add new

      char port_num[N];  //接收传来的值
	  memset(port_num,0,N);
      //char *device = readproductID();
	  cgiFormStringNoNewlines("port_num",port_num,N);
	  if(strlen(port_num) == 0)	  
	  {
		ret=show_ethport_list(&head,&num);
		///////

       p=head.next;
		if(p!=NULL)
		{
			while(p!=NULL)
			{
				pp=p->port.next;
				while(pp!=NULL)
				{
					if(p->slot_no==0)
					{
						memset(port_num,0,N);
						sprintf(port_num,"%d-%d",1,pp->port_no);
					}
					else
					{
						memset(port_num,0,N);
						sprintf(port_num,"%d-%d",p->slot_no,pp->port_no);
					}	
					pp=pp->next;
					break;
				}
				p=p->next;
		       }
		}	
		//////

		 if((ret==0)&&(num>0))
          {
        	Free_ethslot_head(&head);
          }   //释放
	  }

	unsigned char slot_no,port_no;
	unsigned int port_index ;
	port_info port_information;
	memset(&port_information,0,sizeof(port_info));
	  
//////////////


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
		cgiFormStringNoNewlines("UN",encry,BUF_LEN);
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
		memset(port,0,N); 
		cgiFormStringNoNewlines("portno",port,N);		
		 
		
		memset(rstp,0,N); 
		cgiFormStringNoNewlines("rstp",rstp,N);

		if(cgiFormSubmitClicked("default") == cgiFormSuccess)
		{
				config_spanning_tree_mode("stp");//选择RSTP模式
				port_state = config_spanning_tree_ethport(port,rstp);//开启或关闭端口配置			
				if(port_state == 0)
				{
					if(ccgi_get_brg_g_state(&stpmode1)==1)
					{
						ret = config_spanning_tree_port_default(port);
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
				else if(port_state == -2)
				{
					ShowAlert(search(lcon,"port_no_lk"));
				}
				else
					ShowAlert(search(lpublic,"oper_fail"));
		}
		else if(cgiFormSubmitClicked("submit_stp") == cgiFormSuccess)
    	{
				config_spanning_tree_mode("stp");//选择RSTP模式
				port_state = config_spanning_tree_ethport(port,rstp);//开启或关闭端口配置				
				if(strcmp(rstp,"disable")==0)//关闭端口配置
				{
					if(port_state == 0)
					{
						ShowAlert(search(lcon,"stop_succ"));
					}
					else if(port_state == -2)
					{
						ShowAlert(search(lcon,"port_no_lk"));
					}
					else
						ShowAlert(search(lcon,"stop_fail"));
				}                                                 //开启端口
				else
				{
				
				
				
					if(port_state == 0)
					{
						ret = config_port(port,lpublic,lcon);						
						if(ret == 0)
						{
							ShowAlert(search(lpublic,"oper_succ"));
						}
						else if(ret == -1)
						{
							ShowAlert(search(lpublic,"oper_fail"));
						}
						else if(ret==4)
						{
                         ShowAlert(search(lpublic,"oper_fail"));

						}
					}
					else if(port_state == -2)
					{
						ShowAlert(search(lcon,"port_no_lk"));
					}
					else
						ShowAlert(search(lcon,"start_fail"));
				}

    	}

      if(strcmp(encry,"")==0)
	  	strcpy(encry,stp_encry);
	    strcpy(stp_encry,encry);
	  
	 // fprintf(cgiOut,"<form method=post action='wp_config_stp_port.cgi?UN=%s'>",encry);
	 
	  fprintf(cgiOut,"<form method=post>");
	  fprintf(cgiOut,"<input type=hidden name=UN value=%s />\n",encry);
	  fprintf(cgiOut,"<input type=hidden name=stp_encry value=%s />\n",stp_encry);

	  
	  fprintf(cgiOut,"<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	  "<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
		"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>RSTP</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
			
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

							if((cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)&&(cgiFormSubmitClicked("default") != cgiFormSuccess))
							{
								fprintf(cgiOut,"<tr height=25>"\
								 "<td align=left id=tdleft><a href=wp_stp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"br_info")); 
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr height=25>"\
								"<td align=left id=tdleft><a href=wp_config_stp_bridge.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"br_conf"));   
								fprintf(cgiOut,"</tr>"\
								"<tr height=25>"\
								  "<td align=left id=tdleft><a href=wp_show_port.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"port_info"));
								fprintf(cgiOut,"</tr>"\
								"<tr height=26>"\
								  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"port_conf"));//突出显示非连接
								fprintf(cgiOut,"</tr>");
							}
							else
							{
								fprintf(cgiOut,"<tr height=25>"\
								 "<td align=left id=tdleft><a href=wp_stp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"br_info")); 
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr height=25>"\
								"<td align=left id=tdleft><a href=wp_config_stp_bridge.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"br_conf"));   
								fprintf(cgiOut,"</tr>"\
								"<tr height=25>"\
								  "<td align=left id=tdleft><a href=wp_show_port.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"port_info"));
								fprintf(cgiOut,"</tr>"\
								"<tr height=26>"\
								  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"port_conf"));//突出显示非连接
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
				  "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:20px\">"\
						  "<table  border=0 cellspacing=0 cellpadding=0>");
					  fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td>%s:</td>",search(lcon,"port_conf"));
						fprintf(cgiOut,"<td width=70><select name=rstp style=width:138px>");

						///////////////////		
					  int port_flag=-1;
					  
					  get_port_admin_state(port_num, &port_flag);
					  
                        if(port_flag==0)
                        {
							fprintf(cgiOut,"<option value=disable>stop</option>");
							fprintf(cgiOut,"<option value=enable>start</option>");

                        }
						else if(port_flag)
						{
							fprintf(cgiOut,"<option value=enable>start</option>");
							fprintf(cgiOut,"<option value=disable>stop</option>");
						}
						
						fprintf(cgiOut,"</select></td>"\
						"<td><font color=red>(%s)</font></td>",search(lcon,"start_port"));
						fprintf(cgiOut,"<tr height=30>");


						  fprintf(cgiOut,"<td>%s:</td>",search(lcon,"port_no")); 
						  fprintf(cgiOut,"<td width=70><select name=portno style=width:138px onchange=port_sel_change(this)>");
						 // fprintf(cgiOut,"<option value=%s selected=selected>%s</option>",port_num,port_num);
                         /////////////////////////

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
            							memset(slot_port,0,10);							
            							sprintf(slot_port,"%d/%d",p->slot_no,pp->port_no);		 /*int转成char*/
            							if(strcmp(slot_port,port_num)==0)
            			 				  fprintf(cgiOut,"<option value=%s selected=selected>%s",slot_port,slot_port);
            							else
            							  fprintf(cgiOut,"<option value=%s>%s",slot_port,slot_port);
            							}
            							pp=pp->next;
            						}
            						p=p->next;
            					}
            			}
                         						  
						 ////////////////////////
						  	fprintf(cgiOut,"</select></td>"\
						  "<td><font color=red>(%s)</font></td>",search(lcon,"slot_port_no"));//********此处结束

                            
                           ///////////////////////                            
							
                            fprintf(cgiOut,"<script type=text/javascript>\n");
                           	fprintf(cgiOut,"function port_sel_change( obj )\n"\
                           	"{\n"\
                           	"var port_num = obj.options[obj.selectedIndex].text;\n"\
                           	"var url = 'wp_config_stp_port.cgi?UN=%s&port_num='+port_num;\n"\
                           	"window.location.href = url;\n"\
                           	"}\n", encry);
                            fprintf(cgiOut,"</script>\n" );

                            //获得并分析端口
							parse_slotport_no((unsigned char *)port_num,&slot_no,&port_no);
                            //得出要的参数
							ccgi_get_one_port_index(slot_no,port_no,&port_index) ;	
							//存储信息到结构体中去
	                        ret=ccgi_get_one_port_info(port_index,productid,&port_information);

							///////////////////应该是获得某个端口的全部状态


	  
				  	fprintf(cgiOut,"<tr height=30>");
					fprintf(cgiOut,"<td>%s:</td>",search(lcon,"path_cost"));
					fprintf(cgiOut,"<td width=140><input type=text name=path_cost size=21 maxLength=9 value=\"%d\"></td>",port_information.port_cost);
					fprintf(cgiOut,"<td><font color=red>(1--200000000,%s||%s)</font></td>",search(lcon,"times_20"),"auto");
					fprintf(cgiOut,"</tr>");

					
					fprintf(cgiOut,"<tr height=30>");
					fprintf(cgiOut,"<td>%s:</td>",search(lcon,"prior"));
					fprintf(cgiOut,"<td width=140><input type=text name=prio size=21 maxLength=3 value=\"%d\"></td>",port_information.port_prio);
					fprintf(cgiOut,"<td><font color=red>(0--240,%s)</font></td>",search(lcon,"times_16"));
					fprintf(cgiOut,"</tr>");


				    
                    
					if(strcmp(stp_port_role[port_information.port_role],"NStp")==0)
					{


						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td>%s:</td>",search(lcon,"calcul"));
						fprintf(cgiOut,"<td width=140><select name=count style=width:138px><option value=yes selected=selected>yes<option value=no>no</select></td>"\
						);					  
						fprintf(cgiOut,"</tr>");
					}
					else
					{

						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td>%s:</td>",search(lcon,"calcul"));
						fprintf(cgiOut,"<td width=140><select name=count style=width:138px><option value=yes>yes<option value=no  selected=selected>no</select></td>"\
						);					  
						fprintf(cgiOut,"</tr>");
					}
				
					if(port_information.port_p2p == 0)
					{

						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td>%s:</td>",search(lcon,"link_type"));
						fprintf(cgiOut,"<td width=140><select name=link_style style=width:138px><option value=auto>auto<option value=yes >yes<option value=no selected=selected>no</select></td>"\
						"<td><font color=red>(%s)</font></td>",search(lcon,"real"));
						fprintf(cgiOut,"</tr>");
					}
				    else if(port_information.port_p2p ==1 )
					{

						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td>%s:</td>",search(lcon,"link_type"));
						fprintf(cgiOut,"<td width=140><select name=link_style style=width:138px><option value=auto>auto<option value=yes selected=selected>yes<option value=no >no</select></td>"\
						"<td><font color=red>(%s)</font></td>",search(lcon,"real"));
						fprintf(cgiOut,"</tr>");
					}
					else if(port_information.port_p2p ==2 )
					{

						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td>%s:</td>",search(lcon,"link_type"));
						fprintf(cgiOut,"<td width=140><select name=link_style style=width:138px><option value=auto selected=selected>auto<option value=yes>yes<option value=no>no</select></td>"\
						"<td><font color=red>(%s)</font></td>",search(lcon,"real"));
						fprintf(cgiOut,"</tr>");
					}

					if(port_information.port_edge == 0)
					{

						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td>%s:</td>",search(lcon,"edge_port"));
						fprintf(cgiOut,"<td width=140><select name=edge style=width:138px><option value=yes>yes<option value=no selected=selected>no</select></td>");
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
					}
					else if(port_information.port_edge == 1)
					{

						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td>%s:</td>",search(lcon,"edge_port"));
						fprintf(cgiOut,"<td width=140><select name=edge style=width:138px><option value=yes selected=selected>yes<option value=no>no</select></td>");
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=30>");
					}
					

					//*************************协商类型
					
					  fprintf(cgiOut,"<td>%s:</td>",search(lcon,"prtl_type"));
					  fprintf(cgiOut,"<td width=70><select name=mcheck style=width:138px><option value=yes>yes</select></td>");
					  fprintf(cgiOut,"<td><input type=submit width=60 name=default value=%s /></td>",search(lcon,"re_default"));
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr>");

					//**********************默认设置
					
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
	
    if((result==0)&&(p_num>0))
    {
    	Free_ethslot_head(&head);
    }	
	
	return 0;


}


int config_port(char *port,struct list *lpublic,struct list *lcon)
{
	char path_cost[N],prio[N],count[N],link_style[N],edge[N],mcheck[N];	
	int path_cost_arg, prio_arg;
	int flag = 0;
	int stpmode;
	int result;
	int path_cost_call = -1,prio_call = -1,count_call = -1,link_style_call = -1,edge_call = -1,mcheck_call = -1;//配置标志
    int rest;
	
	char arg_std[512];
	memset(path_cost,0,N); 
	memset(prio,0,N); 
	memset(count,0,N); 
	memset(link_style,0,N); 
	memset(edge,0,N); 
	memset(mcheck,0,N);
	
	result = cgiFormStringNoNewlines("path_cost",path_cost,N);
	if(result == cgiFormSuccess)
	{
		if(strcmp(path_cost, "auto")== 0)
		{
		
		}
		else {
			path_cost_arg = strtoul (path_cost,0,10);
			if (!((path_cost_arg > 0)&&(path_cost_arg % 20 ==0)&&(path_cost_arg<=200000000)))
			{
				flag = -1;
				memset(arg_std,0,512);
				strcat(arg_std,search(lcon,"path_cost_err"));
				//strcat(arg_std,search(lcon,"arg_std"));
				ShowAlert(arg_std);
			}
		}
		if(flag != -1)
			path_cost_call = 0;
	}
	
	result = cgiFormStringNoNewlines("prio",prio,N);
	if(result == cgiFormSuccess)
	{	
		prio_arg = strtoul (prio,0,10);
		if(prio_arg < MIN_PORT_PRIO || prio_arg > MAX_PORT_PRIO ){
			flag = -1;
			memset(arg_std,0,512);
			strcat(arg_std,search(lcon,"prior_err")); 
			//strcat(arg_std,search(lcon,"arg_std"));
			ShowAlert(arg_std);

			//ShowAlert("path_cost参数不符合标准");
		}
		else{
			if(0 != (prio_arg%16)){
				flag = -1;
				memset(arg_std,0,512);
				strcat(arg_std,search(lcon,"prior_err"));
				//strcat(arg_std,search(lcon,"arg_std"));
				ShowAlert(arg_std);

				//ShowAlert("path_cost参数不符合标准");
			}
		}
		if(flag != -1)
			prio_call = 0;
	}

	result = cgiFormStringNoNewlines("count",count,N);
	if(result == cgiFormSuccess)
	{
		count_call = 0;
	}

	result = cgiFormStringNoNewlines("link_style",link_style,N);
	if(result == cgiFormSuccess)
	{
		link_style_call = 0;
	}

	result = cgiFormStringNoNewlines("edge",edge,N);
	if(result == cgiFormSuccess)
	{
		edge_call = 0;
	}

	result = cgiFormStringNoNewlines("mcheck",mcheck,N);
	if(result == cgiFormSuccess)
	{	
		if(strcmp(mcheck,"yes")==0)
			mcheck_call = 0;
	}
	
	if(flag == 0)
	{
		if(ccgi_get_brg_g_state(&stpmode)==1)
		{
			if(path_cost_call==0)
				{
				rest=config_spanning_tree_port_cost(port, path_cost);//配置端口路径开销
				if(rest==4)
					return 4;
				}
			if(prio_call==0)
				config_spanning_tree_port_pri(port, prio);//配置端口优先级
				
			if(count_call==0)
				config_spanning_tree_port_nonstp(port, count);//配置端口是否参与RSTP计算
				
			if(link_style_call==0)
				config_spanning_tree_port_p2p(port, link_style);//配置端口链路类型
				
			if(edge_call==0)
				config_spanning_tree_port_edge(port, edge);//配置端口是否为边缘端口
				
			if(mcheck_call==0)
			{
				result = config_spanning_tree_port_mcheck(port, mcheck);//配置端口进行协议类型协商
			}
			return 0;
		}
		else
		{
			ShowAlert(search(lcon,"no_start_br"));
		}
	}
	if(flag == -1)
		return -2;
	return 0;
}

