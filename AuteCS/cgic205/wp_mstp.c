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
* wp_mstp.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for multistp  config 
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



int ShowMstpPage(struct list *lpublic,struct list *lcon);

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcon;	   /*解析control.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcon=get_chain_head("../htdocs/text/control.txt");
	ccgi_dbus_init();
    ShowMstpPage(lpublic,lcon); 
	release(lpublic);  
	release(lcon);

 	return 0;
}

int ShowMstpPage(struct list *lpublic,struct list *lcon)
{
	  char *str=NULL;
	//  char encry[128];  /*存储从wp_usrmag.cgi带入的加密字符串*/
	// memset(encry,0,128);
	 // char stp_encry[BUF_LEN]; 
	char encry[BUF_LEN] = {0};

      //int ret;
	  unsigned short *pvid = NULL;
	  unsigned int num = 0;
	  int mstid=0;
	  
	  br_self_info aa;
	  memset(&aa,0,sizeof(aa));
	  msti_info bb;
	  memset(&bb,0,sizeof(bb));
	  cist_info cc;
	  memset(&cc,0,sizeof(cc));
		  

	  int i;
	  
	  char instance[N];
	  memset(instance,0,N); 	  
	 // cgiFormStringNoNewlines("instance",instance,N); 
		cgiFormStringNoNewlines("INS",instance,N); 
		int instance_num = 0;
    //  if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
     // {
 			 memset(encry,0,BUF_LEN);
 			 cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
 			 str=dcryption(encry);
 			 if(str==NULL)
 			 {
	 			   ShowErrorPage(search(lpublic,"ill_user"));	 /*用户非法*/
	 			   return 0;
 			 }
 			// memset(stp_encry,0,BUF_LEN); 				  /*清空临时变量*/
	 // }
		//memset(stp_encry,0,BUF_LEN); 				  /*清空临时变量*/
		//cgiFormStringNoNewlines("stp_encry",stp_encry,BUF_LEN);
		//cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		
			instance_num = atoi(instance);
		cgiHeaderContentType("text/html");
		fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
		fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
		fprintf(cgiOut,"<title>%s</title>",search(lcon,"stp_man"));
		fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
		  "<style type=text/css>"\
		  ".a3{width:30;border:0; text-align:center}"\
		  "</style>"\
		"</head>"\
		/*"<script src=/ip.js>"\
		"</script>"\*/
		"<body>");

	
	
	  fprintf(cgiOut,"<form method=post >"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	  "<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
		"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>MSTP</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

		//fprintf(cgiOut,"<input type=hidden name=UN  value=%s />",encry);
			
				
			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
			  "<tr>"\
			/*"<td width=62 align=center><input id=but type=submit name=submit_stp style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));*/	
				"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
			//  if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
				fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
			 // else										   
				//fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",stp_encry,search(lpublic,"img_cancel"));
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
						int retu=checkuser_group(str);
						//if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
						//{
							if(retu==0)  /*管理员*/
							{
     							fprintf(cgiOut,"<tr height=26>"\
     							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"br_info"));   /*突出显示*/
     							fprintf(cgiOut,"</tr>");
     							fprintf(cgiOut,"<tr height=25>"\
     							  "<td align=left id=tdleft><a href=wp_config_mstp_bridge.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"br_conf"));
     							fprintf(cgiOut,"</tr>"\
     							"<tr height=25>"\
     							  "<td align=left id=tdleft><a href=wp_show_port_mstp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"port_info"));
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
								fprintf(cgiOut,"<tr height=26>"\
     							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"br_info"));   /*突出显示*/
     							fprintf(cgiOut,"</tr>"\
     							"<tr height=25>"\
     							  "<td align=left id=tdleft><a href=wp_show_port_mstp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"port_info"));
     							fprintf(cgiOut,"</tr>");
							}

						//}
						#if 0
						else if(cgiFormSubmitClicked("submit_stp") == cgiFormSuccess)
						{
							if(retu==0)  /*管理员*/
			{
					fprintf(cgiOut,"<tr height=26>"\
					  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"br_info"));   /*突出显示*/
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_config_mstp_bridge.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"br_conf"));
					fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_show_port_mstp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"port_info"));
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
								fprintf(cgiOut,"<tr height=26>"\
     							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"br_info"));   /*突出显示*/
     							fprintf(cgiOut,"</tr>"\
     							"<tr height=25>"\
     							  "<td align=left id=tdleft><a href=wp_show_port_mstp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",stp_encry,search(lpublic,"menu_san"),search(lcon,"port_info"));
     							fprintf(cgiOut,"</tr>");
							}

						}
						#endif
						  for(i=0;i<14;i++)
						  {
							fprintf(cgiOut,"<tr height=25>"\
							  "<td id=tdleft>&nbsp;</td>"\
							"</tr>");
						  }
						
					  fprintf(cgiOut,"</table>"\
				  "</td>"\
				  "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
						  "<table border=0 cellspacing=0 cellpadding=0>");
											fprintf(cgiOut,"<tr height=30>");
												fprintf(cgiOut,"<td>%s:</td>",search(lcon,"instance"));
													fprintf(cgiOut,"<td width=70>");
				/*								fprintf(cgiOut,"<td width=70><input type=text name=instance size=10 value=%s></td>"\*/
				
												/*"<td><font color=red>(0--63)</font></td>",instance);*/
										fprintf(cgiOut,"<select name=insno onchange=ins_sel_change(this)>");
										for (i = 0; i < MAX_MST_ID; i++)
										{
											if(i == instance_num)
											{
												fprintf(cgiOut,"<option value=\"%d\" selected=selected>%d</option>",i,i);
											}
											else
											{
												fprintf(cgiOut,"<option value=\"%d\">%d</option>",i,i);
											}
										}
										fprintf(cgiOut,"</select>");
										fprintf(cgiOut,"</td>"\
										"<td><font color=red>(0--63)</font></td>");
											fprintf(cgiOut,"</tr></table>");

		//if(strcmp(instance,"")!=0&&strtoul(instance,0,10)<=MAX_MST_ID&&strtoul(instance,0,10)>=MIN_MST_ID)
		//{
					  fprintf(cgiOut,"<script type=text/javascript>\n");
		   	fprintf(cgiOut,"function ins_sel_change( obj )\n"\
		   	"{\n"\
		   	"var ins_num = obj.options[obj.selectedIndex].text;\n"\
		   	"var url = 'wp_mstp.cgi?UN=%s&INS='+ins_num;\n"\
		   	"window.location.href = url;\n"\
		   	"}\n", encry);	
		    fprintf(cgiOut,"</script>\n" );
			fprintf(cgiOut,	"<table width=700 border=0 cellspacing=0 cellpadding=0>");				
			fprintf(cgiOut,"<tr height=30>");
					
					fprintf(cgiOut,"<td align=center id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s%s%s</td>",search(lpublic,"stp1_des"),instance,search(lpublic,"stp2_des"));
				
						  fprintf(cgiOut,"</tr>"\
						"<tr>"\
						  "<td align=left valign=top  style=\"padding-top:18px\">");
					fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>");
		//ccgi_dbus_init();
	  		//int stpmode;
			//fprintf(stderr,"-----------------\n");
			//if(ccgi_get_brg_g_state(&stpmode)==1)
				int stpmode = 0;
				int result1 = 0;
				result1 = ccgi_get_brg_g_state(&stpmode);
				if(1 == result1)
         		{
         		    
         			if(stpmode==1)
         			{	
         				int ret = 0;
         				int op_ret=0;
         
         			
					mstid=atoi(instance);  //转化成整型
					
					
         			if(mstid < MIN_MST_ID || mstid > MAX_MST_ID) {
                             //vty_out(vty,"input param out range.\n");
                                 return CMD_FAILURE;
                        }
         			/*get bridge info*/
               ret = ccgi_get_br_self_info(mstid, &pvid, &num, 1,&aa);   //进入到此
         	   if(CMD_SUCCESS == ret) {
         		  if(0 == mstid) 
         		  {
         			op_ret=ccgi_get_cist_info_new(mstid,&cc);	
         		  }
         		    op_ret=ccgi_get_msti_info_new(mstid,&bb);
         		  }
         		  if(ret == 0)					
         	  	  {
         
         				fprintf(cgiOut,"<tr>"\
         				   "<td id=td1>Region name\t\t:</td>"\
         				   "<td id=td2>%s</td>"\
         			   "</tr>",aa.pname);   
         		fprintf(cgiOut,"<tr>"\
         			   "<td id=td1>Bridge revision\t\t:</td>"\
         			   "<td id=td2>%d</td>"\
         		   "</tr>",aa.revision);
         
              //  fprintf(cgiOut,"<tr>"\
         		//					   "<td id=td1>Vlan map\t\t:</td>");
         	   // fprintf(cgiOut,"<td id=td2>%s%s</td>",aa.t1,aa.map);   //有判断，注意一下

				#if 0
				///////////////////

                  if(aa.count==0||mstid==0){
                  fprintf(cgiOut,"<tr>"\
         							   "<td id=td1>Vlan map\t\t:</td>");
         	      fprintf(cgiOut,"<td id=td2>%s</td>",aa.map);  //有判断，注意一下
                 	}else{
                  fprintf(cgiOut,"<tr>"\
         							   "<td id=td1>Vlan map\t\t:</td>");
         	      fprintf(cgiOut,"<td id=td2>%s%s</td>",aa.t1,aa.map);   //有判断，注意一下
                 		}     

				//////////////////
				#endif
					fprintf(cgiOut,"<tr>"\
				"<td id=td1>Vlan map\t\t:</td>");
				fprintf(cgiOut,"<td id=td2>%s%s</td>",aa.t1,aa.map);   //óD?D??￡?×￠òaò???
         	    fprintf(cgiOut,"<tr>"\
         							   "<td id=td1>Bridge ID Mac Address\t:</td>"\
         							   "<td id=td2>%02x:%02x:%02x:%02x:%02x:%02x</td>"\
         						   "</tr>",aa.mac[0],aa.mac[1],aa.mac[2],aa.mac[3],aa.mac[4],aa.mac[5]);	
         	    fprintf(cgiOut,"<tr>"\
         							   "<td id=td1>Bridge Priority\t\t:</td>"\
         							   "<td id=td2>%d</td>"\
         						   "</tr>",aa.br_prio);
         	    fprintf(cgiOut,"<tr>"\
         							   "<td id=td1>Bridge Force Version\t:</td>"\
         							   "<td id=td2>%d</td>"\
         						   "</tr>",aa.br_version);
         	    fprintf(cgiOut,"<tr>"\
         							   "<td id=td1>Root Max Age:</td>"\
         							   "<td id=td2>%6d</td>"\
         						   "</tr>",aa.br_maxAge);
         	    fprintf(cgiOut,"<tr>"\
         							   "<td id=td1>Hello Time:</td>"\
         							   "<td id=td2>%5d</td>"\
         						   "</tr>",aa.br_hTime);
         	    fprintf(cgiOut,"<tr>"\
         							   "<td id=td1>Forward Delay:</td>"\
         							   "<td id=td2>%5d</td>"\
         						   "</tr>",aa.br_fdelay);
         	    fprintf(cgiOut,"<tr>"\
         								   "<td id=td1>MaxHops:</td>"\
         								   "<td id=td2>%5d</td>"\
         							   "</tr>",aa.br_hops);
         
         
         //   free(aa.pname);  //ws_stp.c中分配get_br_self_info 函数中

			fprintf(cgiOut,"<tr height=15>"\
         					   "<td> </td></tr>"); //中间的空格

			if ( mstid == 0)
			{            
            
         	fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>Root Bridge\t\t:</td>"\
         						   "<td id=td2>%02x:%02x:%02x:%02x:%02x:%02x</td>"\
         					   "</tr>",cc.mac[0],cc.mac[1],cc.mac[2],cc.mac[3],cc.mac[4],cc.mac[5]);		
         		
         	fprintf(cgiOut,"<tr>"\
         							   "<td id=td1>Root Priority\t\t:</td>"\
         							   "<td id=td2>%d</td>"\
         						   "</tr>",cc.br_prio);
         			
         		fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>Root Path Cost\t\t:</td>"\
         						   "<td id=td2>%d</td>"\
         					   "</tr>",cc.path_cost);
         		if(cc.root_portId==0)
         			{
         			fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>Root Port\t\t:</td>"\
         						   "<td id=td2>%s</td>"\
         					   "</tr>","none");
         			}else{
         		fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>Root Port\t\t:</td>"\
         						   "<td id=td2>%d</td>"\
         					   "</tr>",cc.root_portId);
         				}
         			fprintf(cgiOut,"<tr height=15>"\
         					   "<td> </td></tr>"); //中间的空格

         		  	}
         
         	fprintf(cgiOut,"<tr>"\
         							   "<td id=td1>Region Root\t\t:</td>"\
         							   "<td id=td2>%02x:%02x:%02x:%02x:%02x:%02x</td>"\
         						   "</tr>",bb.mac[0],bb.mac[1],bb.mac[2],bb.mac[3],bb.mac[4],bb.mac[5]);
         	fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>Region Root Priority\t:</td>"\
         						   "<td id=td2>%d</td>"\
         					   "</tr>",bb.br_prio);
         	fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>Region Root Path Cost\t:</td>"\
         						   "<td id=td2>%d</td>"\
         					   "</tr>",bb.path_cost);
         	if(bb.root_portId==0)
         		{
         		fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>Region Root Port\t:</td>"\
         						   "<td id=td2>%s</td>"\
         					   "</tr>","none");
         		}else{
         	fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>Region Root Port\t:</td>"\
         						   "<td id=td2>%d</td>"\
         					   "</tr>",bb.root_portId);
         			}
         		fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>Root Max Age:</td>"\
         						   "<td id=td2>%6d</td>"\
         					   "</tr>",bb.br_maxAge);
         		fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>Hello Time:</td>"\
         						   "<td id=td2>%5d</td>"\
         					   "</tr>",bb.br_hTime);
         		fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>Forward Delay:</td>"\
         						   "<td id=td2>%5d</td>"\
         					   "</tr>",bb.br_fdelay);
         		fprintf(cgiOut,"<tr>"\
         						   "<td id=td1>MaxHops:</td>"\
         						   "<td id=td2>%5d</td>"\
         					   "</tr>",bb.br_hops);
         
         //以上是ret 返回成功的情况
         //注意ret返回值的不同
         					
         	  			}
         				//else if(ret == -4)
         				else if(ret==2)
         				{
         					//ShowAlert("no the instance");
         					fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcon,"instance_not_exist"));
         				}
         				else
         				{
         						fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));
         				}
         			}
         			else
         			{
         				fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcon,"stp_run"));
         			}
           		}
         								else
         								{
         									fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcon,"no_start_br"));
         								}
		fprintf(cgiOut,"</table>");


				fprintf(cgiOut,"</td>"\
						/*  "</tr>"\*/
							"</tr>");
							//  if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
							 // {
							//	fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",encry);
							//  }
							//  else if(cgiFormSubmitClicked("submit_stp") == cgiFormSuccess)
							//  { 			 
							//	fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",stp_encry);
							//  }
			//fprintf(cgiOut,"</tr>"\
			//			"</table>");			
				fprintf(cgiOut,"</table>");	
		//}
		//else if(strcmp(instance,"")==0)
		//{
			//ShowAlert(search(lcon,"instance_not_null"));
		//	fprintf(cgiOut,"<table><tr><td><font color=red>%s</font></td></tr></table>",search(lcon,"instance_not_null"));
	//	}
		//else
		//{
		//	ShowAlert(search(lcon,"instance_std"));
		//}	


		//if(cgiFormSubmitClicked("submit_stp") != cgiFormSuccess)
		//{
		//  fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",encry);
		
		fprintf(cgiOut,"<input type=hidden name=UN value=%s>",encry);
		//}
		//else if(cgiFormSubmitClicked("submit_stp") == cgiFormSuccess)
		//{			   
		//  fprintf(cgiOut,"<td><input type=hidden name=stp_encry value=%s></td>",stp_encry);
		//}	


	
	
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

																 
	return 0;

}
#if 0

//显示指定实例的状态信息  0:succ , -1:fail ,-2: MSTP not enable -4:hasn't the instance

int show_spanning_tree_one_instance(char *mstID)
{	
	unsigned int ret;
	unsigned short *pvid = NULL;
	unsigned int mstid = 0,num = 0;


	
	mstid =  strtoul (mstID,0,10);
	if(mstid < MIN_MST_ID || mstid > MAX_MST_ID) {
		//vty_out(vty,"input param out range.\n");
		return CMD_FAILURE;
	}

	/*get bridge info*/
	ret = ccgi_get_br_self_info(mstid, &pvid, &num, 1);
	fprintf(cgiOut,"<tr height=15>"\
					"<td> </td></tr>");	
	//DCLI_DEBUG(("dcli_stp 1774:: show instance mstid %d,num %d\n",mstid,num));
	if(CMD_SUCCESS == ret) {
		if(0 == mstid) {
			ccgi_get_cist_info(mstid);
			fprintf(cgiOut,"<tr height=15>"\
							"<td> </td></tr>"); 

		}
		//DCLI_DEBUG(("dcli_stp 1853:: dcli_get_msti_info\n"));
		ccgi_get_msti_info(mstid);
			
		//get port info
		//DCLI_DEBUG(("dcli_stp 1857:: port info \n"));
		//vty_out(vty,"\n----------------------All ports information of MSTP domain %d-----------------------------------\n",mstid);		
		//vty_out(vty,"%-12s%-4s%-4s%-9s%-5s%-12s%-3s%-4s%-5s%-20s%-10s%-10s\n","Name","vid","pri","cost","role","span-state","lk","p2p","edge","Desi-bridge-id","Dcost","D-port");
/*		if(0 != mstid) {
			tmp = pvid;
			for(j = 0; j < num; j++) {
			 	ret = ccgi_get_one_vlan_portmap(*tmp,&ports_bmp);
				//DCLI_DEBUG(("dcli_stp 1804:: show instance vid %d\n",*tmp));
				if(CMD_SUCCESS == ret) {
					portsmap = (ports_bmp.tagbmp | ports_bmp.untagbmp);
					//DCLI_DEBUG(("dcli_stp 1804:: show instance vid %d,portsmap %02x,\n",*tmp,portsmap));
					show_slot_port_by_productid(productid,portsmap,*tmp,mstid);
#if 0
					for (i=0;i<32;i++) {
						slot = i/8 + 1;
						port = i%8;
						tmpVal = (1<<i);
						if(portsmap & tmpVal) {				
							vty_out(vty,"%-d/%-10d",slot,port);
							vty_out(vty,"%-4d",*tmp);
							if(dcli_get_one_port_index(vty,slot,port,&port_index) < 0)	{
								DCLI_DEBUG(("execute command failed\n"));
								continue;
							}
							dcli_get_mstp_one_port_info(vty,mstid,port_index);
						}
					}
#endif
					tmp++;
					memset(&ports_bmp,0,sizeof(VLAN_PORTS_BMP));
				}
			}
		}
		else {
			ccgi_change_all_ports_to_bmp(&ports_bmp,&count);
			portsmap = (ports_bmp.tagbmp | ports_bmp.untagbmp);
			//DCLI_DEBUG(("dcli_stp 1804:: show instance vid %d,portsmap %02x,\n",*tmp,portsmap));
			for (i=0;i<32;i++) {
				slot = i/8 + 1;
				port = i%8;
				tmpVal = (1<<i);
				if(portsmap & tmpVal) {				
					//vty_out(vty,"%-d/%-10d",slot,port);
					//vty_out(vty,"%-4s"," ");
					if(ccgi_get_one_port_index(slot,port,&port_index) < 0)	{
						//DCLI_DEBUG(("execute command failed\n"));
						continue;
					}
					ccgi_get_mstp_one_port_info(mstid,port_index);
				}
			}
		}*/
	}
	if(0 != num)
		free(pvid);
				
	else if(STP_DISABLE == ret)
		//vty_out(vty,"MSTP hasn't enabled\n");
		return -2;
	else if(DCLI_STP_NO_SUCH_MSTID == ret)
		//vty_out(vty," hasn't the instance\n");
		return -4;//hasn't the instance
	
	return CMD_SUCCESS;
}




int ccgi_get_cist_info()
{
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter	iter,iter_struct;


	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int		path_cost;
	unsigned short root_portId;
	unsigned short  br_prio;

	int i,ret;

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_SHOW_CIST_INFO);
	
	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);

	//vty_out(vty,"STP>> slot_count = %d\n",slot_count);
	if(DCLI_STP_OK == ret) {

	dbus_message_iter_recurse(&iter,&iter_struct);

	//vty_out(vty,"\nRoot Bridge\t\t:  ");
	for(i = 0; i< 5; i++)
	{
		dbus_message_iter_get_basic(&iter_struct,&mac[i]);
		//vty_out(vty,"%02x:",mac[i]);
		dbus_message_iter_next(&iter_struct);
	}

		dbus_message_iter_get_basic(&iter_struct,&mac[i]);
		//vty_out(vty,"%02x",mac[5]);
		dbus_message_iter_next(&iter_struct);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Root Bridge\t\t:</td>"\
						   "<td id=td2>%02x:%02x:%02x:%02x:%02x:%02x</td>"\
					   "</tr>",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);	

		//vty_out(vty,"\nRoot Priority\t\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&br_prio);
		//vty_out(vty,"%d\n",br_prio);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Root Priority\t\t:</td>"\
						   "<td id=td2>%d</td>"\
					   "</tr>",br_prio);		
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Root Path Cost\t\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&path_cost);
		//vty_out(vty,"%d\n",path_cost);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Root Path Cost\t\t:</td>"\
						   "<td id=td2>%d</td>"\
					   "</tr>",path_cost);

		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Root Port\t\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&root_portId);
		if(root_portId){
			//vty_out(vty,"%d\n",root_portId);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Root Port\t\t:</td>"\
						   "<td id=td2>%d</td>"\
					   "</tr>",root_portId);			
		}
		else{
			//vty_out(vty,"%s\n","none");
			fprintf(cgiOut,"<tr>"\
							   "<td id=td1>Root Port\t\t:</td>"\
							   "<td id=td2>%s</td>"\
						   "</tr>","none");

		}
		dbus_message_iter_next(&iter_struct);

		//DCLI_DEBUG(("dcli_common872 :: END suc dcli_get_cist_info\n"));
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	else if(DCLI_STP_NO_SUCH_MSTID == ret)
	{
		//DCLI_DEBUG(("dcli_common879 :: END no ins dcli_get_cist_info\n"));
		dbus_message_unref(reply);
		return DCLI_STP_NO_SUCH_MSTID;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(reply);
		return ret;
	}
	else
	{
		dbus_message_unref(reply);
		return ret;
	}
	
}



int ccgi_get_msti_info
(
	int mstid
)
{
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter	iter,iter_struct;//,iter_sub_array,iter_sub_struct;

	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int		path_cost;
	unsigned short root_portId;
	unsigned short  br_prio;
	unsigned short  br_maxAge;
	unsigned short  br_hTime;
	unsigned short  br_fdelay;
	unsigned char 	 br_hops;
	int i,ret;

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_SHOW_MSTI_INFO);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&mstid,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);
	//vty_out(vty,"STP>> slot_count = %d\n",slot_count);
	if(DCLI_STP_OK == ret) {

		dbus_message_iter_recurse(&iter,&iter_struct);

		//vty_out(vty,"\nRegion Root\t\t:  ");
		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&iter_struct,&mac[i]);
			//vty_out(vty,"%02x:",mac[i]);
			dbus_message_iter_next(&iter_struct);
		}

		dbus_message_iter_get_basic(&iter_struct,&mac[i]);
		//vty_out(vty,"%02x",mac[5]);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Region Root\t\t:</td>"\
						   "<td id=td2>%02x:%02x:%02x:%02x:%02x:%02x</td>"\
					   "</tr>",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);	

		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"\nRegion Root Priority\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&br_prio);
		//vty_out(vty,"%d\n",br_prio);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Region Root Priority\t:</td>"\
						   "<td id=td2>%d</td>"\
					   "</tr>",br_prio);

		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Region Root Path Cost\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&path_cost);
		//vty_out(vty,"%d\n",path_cost);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Region Root Path Cost\t:</td>"\
						   "<td id=td2>%d</td>"\
					   "</tr>",path_cost);		
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Region Root Port\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&root_portId);
		if(root_portId){
			//vty_out(vty,"%d\n",root_portId);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Region Root Port\t:</td>"\
						   "<td id=td2>%d</td>"\
					   "</tr>",root_portId);				
		}
		else{
			//vty_out(vty,"%s\n","none");
			fprintf(cgiOut,"<tr>"\
							   "<td id=td1>Region Root Port\t:</td>"\
							   "<td id=td2>%s</td>"\
						   "</tr>","none");			
		}
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Root Max Age");
		dbus_message_iter_get_basic(&iter_struct,&br_maxAge);
		//vty_out(vty,"%6d\t",br_maxAge);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Root Max Age:</td>"\
						   "<td id=td2>%6d</td>"\
					   "</tr>",br_maxAge);

		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Hello Time");
		dbus_message_iter_get_basic(&iter_struct,&br_hTime);
		//vty_out(vty,"%5d\t\t",br_hTime);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Hello Time:</td>"\
						   "<td id=td2>%5d</td>"\
					   "</tr>",br_hTime);

		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Forward Delay");
		dbus_message_iter_get_basic(&iter_struct,&br_fdelay);
		//vty_out(vty,"%5d\t",br_fdelay);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Forward Delay:</td>"\
						   "<td id=td2>%5d</td>"\
					   "</tr>",br_fdelay);

		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"MaxHops");
		dbus_message_iter_get_basic(&iter_struct,&br_hops);
		//vty_out(vty,"%5d",br_hops);
		fprintf(cgiOut,"<tr>"\
						   "<td id=td1>MaxHops:</td>"\
						   "<td id=td2>%5d</td>"\
					   "</tr>",br_hops);

		dbus_message_iter_next(&iter_struct);
		
		//DCLI_DEBUG(("dcli_common995:: END suc dcli_get_msti_info\n"));
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	else if(DCLI_STP_NO_SUCH_MSTID == ret)
	{
		///DCLI_DEBUG(("dcli_common879 :: END no ins dcli_get_cist_info\n"));
		dbus_message_unref(reply);
		return DCLI_STP_NO_SUCH_MSTID;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(reply);
		return ret;
	}
	else
	{
		dbus_message_unref(reply);
		return ret;
	}
}
#endif
