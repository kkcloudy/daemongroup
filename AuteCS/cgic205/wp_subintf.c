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
* wp_subintf.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for port sub interface
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
#include "ws_dcli_interface.h"
#include "ws_dcli_portconf.h"
#include "ws_public.h"

#define TABLE_HEAD	"<table width=300 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>\n"\
						"<tr height=30 bgcolor=#eaeff9 id=td1 align=left>\n"\
							"<th width=120>%s</th>\n"\
							"<th width=120>%s</th>\n"\
							"<th width=20></th></tr>\n"

#define POP_MENU	"<script type=text/javascript>\n"\
					"	var %s=new popMenu( '%s','%d');\n"\
					"	%s.addItem( new popMenuItem( '%s', '%s' ) );\n"\
					"	%s.addItem( new popMenuItem( '%s', '%s' ) );\n"\
					"	%s.show();\n"\
					"</script>"

#define POP_MENU_IP		"<script type=text/javascript>\n"\
						"	var %s=new popMenu( '%s','%d');\n"\
						"	%s.addItem( new popMenuItem( '%s', '%s' ) );\n"\
						"	%s.show();\n"\
						"</script>"

#define TABLE_LINE	"<tr height=25 bgcolor=%s>\n"\
						"<td>%s</td>\n"\
						"<td>%s</td>\n"\
						"<td>%s</td>\n"\
					"</tr>\n"


typedef struct inputInfo
{
	char process_type[12];
	char port_num[30];
	char tag[12];
	char ipaddr[24];
	char mask[24];	
	char sintf[50];
	int iMask;
} STUserInputInfo;

int showPortSelect( char *selected_portno, char *name, char *onchange );
int getUserInput( STUserInputInfo *pstUserInputInfo );
int deleteSubinterface( STUserInputInfo *pstUserInputInfo ,char * encry);
int createSubinterface( STUserInputInfo *pstUserInputInfo ,char * encry );

struct list *lcontrol;
struct list *lpublic;

int cgiMain()
{
	ccgi_dbus_init();       
	char encry[BUF_LEN] = {0};
	char *str;
	char addn[N];
	char port_num[30];
	int retu;
	STUserInputInfo stUserInputInfo;
	
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");

	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
		return 0;
	}
	strcpy(addn,str);
	//***********************************
	char command[128];
	char line[256];
	FILE *fp;
	char popMenuStr[1024];
	char sintf[50] = {0};
	
	int i=0,j=0,m = 0;
	char *temp_ip;
	char *temp_tag;
	char popMenuName[32];
	char cur_url1[256];
	char cur_url2[256];
	char cur_url_ip[256];
	char * ip[8];
	char transfer_ip[512] = {0};
	char delete_com[32] = {0};
	char del_ip[32] = {0};
	char sub_intf[32] = {0};
	
	for(i=0;i<8;i++)
	{
		ip[i]=(char *)malloc(256);
		memset(ip[i],0,256);
		strcpy(ip[i],"");
	}
	
	
	//***********************************
	cgiHeaderContentType("text/html");
		
	memset(port_num,0,30);
	cgiFormStringNoNewlines("port_num", port_num, sizeof(port_num));
	if(strlen(port_num) == 0)
	{
		ccgi_dbus_init();
		
		struct eth_portlist c_head,*p;
		int cnum,cflag=-1;
		cflag=ccgi_intf_show_advanced_routing_list(0, 1, &c_head, &cnum);
		p=c_head.next;
		if(p!=NULL)
		{
			while(p!=NULL)
			{
				memset(port_num,0,30);
				strcpy(port_num,p->ethport);
				p=p->next;
				break;
		    }
		}	

		if((cflag==0 )&& (cnum > 0))
		Free_ethp_info(&c_head);
	    }
	    memset( &stUserInputInfo, 0, sizeof(STUserInputInfo) );
	 
	    if(0 == getUserInput(&stUserInputInfo))
	    {
	    	createSubinterface( &stUserInputInfo,encry);
	    }
	
	    else
	    {
	    	deleteSubinterface( &stUserInputInfo,encry);
	    }

		cgiFormStringNoNewlines("DELETE",delete_com,32);
		cgiFormStringNoNewlines("IP",del_ip,32);
		cgiFormStringNoNewlines("SUBINTF",sub_intf,32);
		
		
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"\
	  			"<head>\n");
		fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
		 		//下面三句话用于禁止页面缓存
		fprintf(cgiOut,"<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
		fprintf(cgiOut,"<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
		fprintf(cgiOut,"<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");	  
		fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
		  			"<style type=text/css>\n"\
		  				".a3{width:30;border:0; text-align:center}\n"\
		  			"</style>\n"\
	  			"</head>\n"\
	"<script language=javascript src=/ip.js>"\
	"</script>"\
	"<script language=javascript src=/fw.js>"\
	"</script>"\
	"<body>");
  	if(strcmp(delete_com,"delete")==0)
  	{
  		sprintf(command,"sudo /usr/bin/del_intf_ip.sh %s %s",sub_intf,del_ip);
		if (NULL == strstr(command,";"))
		{
			fp = popen(command,"r");		
		}
		if(NULL == fp)
		{
			ShowAlert("do command error!");
			return -1;
		}
		pclose(fp);
		ShowAlert(search(lcontrol,"del_succ"));
		
  	}
	fprintf(cgiOut,"<form method=post action='wp_subintf.cgi' >\n" );
	fprintf(cgiOut,"<input type=hidden name=UN value=%s />\n",encry);

	fprintf(cgiOut, "<div align=center>"\
				"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	  				"<tr>"\
	    					"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	    					"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	    					"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcontrol, "title_subintf" ));
	    		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");	
				fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
			  					"<tr>");
			 			if(checkuser_group(addn)==0)  /*管理员*/
			 			{
						fprintf(cgiOut,"<td width=62 align=center><input id='but' type='submit' name='submit_create' style='background-image:url(/images/%s)' value=''></td>\n",search(lpublic,"img_ok") );		  		
			 			}
			 			else
			 			{
						fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s&port_num=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,port_num,search(lpublic,"img_ok"));
			 			}
			    			fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s&port_num=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,port_num,search(lpublic,"img_cancel"));
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
	              							"<td>\n"\
	              								"<table width=120 border=0 cellspacing=0 cellpadding=0>"\
	                   									"<tr height=25>"\
	                    										"<td id=tdleft>&nbsp;</td>"\
	                  									"</tr>");
									fprintf(cgiOut,"<tr height=25>"\
						  							"<td align=left id=tdleft><a href=wp_prtsur.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"prt_sur"));
	                     					fprintf(cgiOut,"</tr>"\
												"<tr height=25>"\
	  					    							"<td align=left id=tdleft><a href=wp_prtarp.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>ARP<font><font id=%s> %s<font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"survey"));                       
	                    						fprintf(cgiOut,"</tr>");
							retu=checkuser_group(addn);
									if(retu==0)  /*管理员*/
									{
									fprintf(cgiOut,"<tr height=25>"\
													"<td align=left id=tdleft><a href=wp_static_arp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",encry,search(lpublic,"menu_san"),search(lcontrol,"prt_static_arp"));						 
						  			fprintf(cgiOut,"<tr height=25>"\
	  					    							"<td align=left id=tdleft><a href=wp_prtcfg.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"prt_cfg"));                       
	                      					fprintf(cgiOut,"</tr>"\
						 	 					"<tr height=25>"\
	  					    							"<td align=left id=tdleft><a href=wp_prtfuncfg.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"func_cfg"));                       
	                      					fprintf(cgiOut,"</tr>");                    
									}                  
	         							fprintf(cgiOut,"<tr height=26>"\
	         											"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"title_subintf"));   /*突出显示*/
	         							fprintf(cgiOut,"</tr>");
									if(retu==0)  /*管理员*/
									{
									fprintf(cgiOut,"<tr height=25>"\
						  							"<td align=left id=tdleft><a href=wp_interface_bindip.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",encry,search(lpublic,"menu_san"),search(lpublic,"config_interface"));	
									}
									fprintf(cgiOut,"<tr height=25>"\
  					    				"<td align=left id=tdleft><a href=wp_all_interface.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",encry,search(lpublic,"menu_san"),search(lcontrol,"interface"));		

									//for(i=0;i<2;i++)
									//{
									fprintf(cgiOut,"<tr id=tr_auto_check_height>"\
							  						"<td id=tdleft>&nbsp;</td>"\
												"</tr>");
									//}
	 							fprintf(cgiOut,"</table>"\
	              	 						"</td>"\
	              							"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");
								fprintf(cgiOut,"<div id=div_auto_check_height style='width:768px;overflow:auto'>"\
												"<div>");
								if(retu==0)//管理员
								{
									fprintf(cgiOut,"<table border=0 width=450>"\
										"<tr height=50px>"\
										"</tr>");
									fprintf(cgiOut,"<tr><td height='25' colspan=3>&nbsp;</td></tr>\n");
									fprintf(cgiOut,"<tr>"\
										"<td>%s:</td>"\
										"<td>", search(lcontrol,"port_no"));
									showPortSelect( port_num,"port_num", "port_sel_change(this)" );

									fprintf(cgiOut,"</td>"\
										"<td></td>"\
										"</tr>\n");
									fprintf(cgiOut,"<tr><td height=6 colspan=3></td></tr>\n");

									fprintf(cgiOut,"<tr>"\
										"<td>%s</td>"\
										"<td colspan=2><input type=text name=subinterface_tag maxlenghth=4 /><font color=#ff0000>(1-4094)</font></td>"\
										"</tr>\n", "tag:");
									///////////////////////////
									fprintf(cgiOut,"<tr><td height=6 colspan=3></td></tr>\n");

									fprintf(cgiOut,"<tr>\n");
									fprintf(cgiOut,"<td>%s qinq:</td>\n",search(lcontrol,"config"));
									fprintf(cgiOut,"<td>Yes:<input type=\"radio\" name=\"showtype\" value=\"1\" ></td>\n");
									fprintf(cgiOut,"<td>No:<input type=\"radio\" name=\"showtype\" value=\"2\" checked></td>\n");
									fprintf(cgiOut,"</tr>\n");
									fprintf(cgiOut,"<tr><td height=6 colspan=3></td></tr>\n");
									fprintf(cgiOut,"<tr>\n");
									fprintf(cgiOut,"<td>%s:</td>\n","qinq-type");
									fprintf(cgiOut,"<td colspan=2><input type=text name=ssubvalue value=\"\"></td>\n");
									fprintf(cgiOut,"</tr>\n");
									///////////////////////////
									fprintf(cgiOut,"</table>\n" );
								}
								else
								{
									fprintf(cgiOut,"<table>"\
										"<tr height=50px>"\
										"</tr>");

									fprintf(cgiOut,"<tr>"\
										"<td width=50%%>%s:</td>"\
										"<td>", search(lcontrol,"port_no"));
									showPortSelect( port_num,"port_num", "port_sel_change(this)" );
									fprintf(cgiOut,"</td>"\
										"</tr>\n");													
									fprintf(cgiOut,"</table>\n" );
								}
			//*********************************show sub-interface and configed ip**********************************************
				//***********************static frame*********************************************
			fprintf(cgiOut,"<br /><span id=sec1 style='font-size=14px;'>%s</span><hr width=90%% color=#53868b />", search(lcontrol,"subif_info") );
			fprintf(cgiOut,"<table width='90%%'>\n"\
							"<tr height='30' bgcolor='#eaeff9' id='td1' align='left'>");
				    	fprintf(cgiOut,"<th colspan='2'>%s</th>",search(lcontrol,"subif_interface"));
				    	fprintf(cgiOut,"<th colspan='8'>IP</th>");
				fprintf(cgiOut,"</tr>");
				//**********************************************************************************
				sprintf(command,"/usr/bin/if_show_subif.sh %s",port_num);
				if (NULL == strstr(command,";"))
				{
					fp = popen(command,"r");		
				}
				memset(line,0,sizeof(line));
				if(fp != NULL)
				{
					fgets(line,sizeof(line),fp);
					while(strlen(line)>0)
					{
						for( ;strlen(line)>0;memset(line,0,sizeof(line)), fgets(line,sizeof(line),fp))
						{
							m++;
							temp_ip = strchr(line,'#');
							if(NULL == temp_ip)
							{
								continue;
							}
							*temp_ip = 0;
							temp_ip++;
							
							temp_tag = strchr(temp_ip,'#');
							if(NULL == temp_tag)
							{
								continue;
							}
							*temp_tag = 0;
							temp_tag++;
							//************************get ip***********************
							ip[0] = strtok(temp_ip,",");
							for(i = 1;i<8;i++)
							{
								ip[i] = strtok(NULL,",");
							}
							//***************************get ip*****************************
							for(j=0;temp_tag[j]!=0;j++,(temp_tag[j]==0x0d||temp_tag[j]==0x0a)?temp_tag[j]=0:1);
							sprintf(popMenuName,"eth%s_%s",port_num,temp_tag);
					memset(sintf,0,sizeof(sintf));
					snprintf(sintf,sizeof(sintf),"eth%s.%s",port_num,temp_tag);
					for(j=0;popMenuName[j]!=0;j++,(popMenuName[j]=='-')?popMenuName[j]='_':1 );
					
					sprintf( cur_url1, "wp_subintf.cgi?UN=%s&process_type=del&port_num=%s&subinterface_tag=%s&SINTF=%s", encry, port_num, temp_tag,sintf );
					sprintf( cur_url2, "wp_interface_bindip.cgi?UN=%s&PORT=%s",encry,line);
					memset(popMenuStr,0,sizeof(popMenuStr));
					snprintf(popMenuStr, sizeof(popMenuStr),POP_MENU, 
					popMenuName, popMenuName, 4096-m, 
							popMenuName,search(lcontrol,"bind_ip"),cur_url2,
							popMenuName,search(lcontrol,"del"), cur_url1, 
							popMenuName );
							//****************画表格*************
											fprintf(cgiOut,"<tr height='25' bgcolor='%s'>",setclour(m%2));
											    	fprintf(cgiOut,"<td width='93' rowspan='2' width='15%%'>%s</td>\n",line);
											    	fprintf(cgiOut,"<td width='25' rowspan='2' width='5%%'>%s</td>\n",popMenuStr);
													for(j=0;j<4;j++)
													{
												    		if((ip[j]==NULL)||(strcmp(ip[j],"")==0))
														{
													fprintf(cgiOut,"<td width='15%%'>%s</td>\n",search(lcontrol,"no_config_ip"));
												    	fprintf(cgiOut,"<td width='5%%'>&nbsp;</td>\n");
														}
														else
														{
													sprintf(cur_url_ip,"wp_subintf.cgi?UN=%s&DELETE=%s&IP=%s&SUBINTF=%s",encry,"delete",ip[j],line);
													sprintf(transfer_ip,"%s_%d",popMenuName,j);
													fprintf(cgiOut,"<td width='15%%'>%s</td>\n",ip[j]);
													fprintf(cgiOut,"<td width='5%%'>\n"\
																	"<script type=text/javascript>\n"\
																		"var %s=new popMenu( '%s','%d');\n"\
																		"%s.addItem( new popMenuItem( '%s', '%s' ) );\n"\
																		"%s.show();\n"\
																	"</script>"\
																"</td>\n",transfer_ip,transfer_ip,4-j,transfer_ip,search(lcontrol,"del"),cur_url_ip,transfer_ip);	
														}
													}
											fprintf(cgiOut,"</tr>\n");
											
											fprintf(cgiOut,"<tr height='25' bgcolor='%s'>\n",setclour(m%2));
													for(j=4;j<8;j++)
													{
														if((ip[j]==NULL)||(strcmp(ip[j],"")==0))
														{
													fprintf(cgiOut,"<td width='15%%'>%s</td>\n",search(lcontrol,"no_config_ip"));
												    	fprintf(cgiOut,"<td width='5%%'>&nbsp;</td>\n");
														}
														else
														{
													sprintf(cur_url_ip,"wp_subintf.cgi?UN=%s&DELETE=%s&IP=%s&SUBINTF=%s",encry,"delete",ip[j],line);
													sprintf(transfer_ip,"%s_%d",popMenuName,j);
													fprintf(cgiOut,"<td width='15%%'>%s</td>\n",ip[j]);
													fprintf(cgiOut,"<td width='5%%'>\n"\
																	"<script type=text/javascript>\n"\
																		"var %s=new popMenu( '%s','%d');\n"\
																		"%s.addItem( new popMenuItem( '%s', '%s' ) );\n"\
																		"%s.show();\n"\
																	"</script>"\
																"</td>\n",transfer_ip,transfer_ip,8-j,transfer_ip,search(lcontrol,"del"),cur_url_ip,transfer_ip);	
														}
													}
											fprintf(cgiOut,"</tr>\n");
											
						}
					}
					pclose(fp);
					}

				//****************画表格*************************
					
									fprintf(cgiOut,"</table>\n");
									//添加一个空的table，否则最后一个接口的popmenu出来后不能看全，不能进行删除操作。
									fprintf( cgiOut, "<table><tr height='30'><td>&nbsp</td></tr></table>\n" );
				
			//************************************************************************************************************
									fprintf(cgiOut,"</div>\n"\
											"</div>\n");
				fprintf(cgiOut,"<script type=text/javascript>\n");
					fprintf(cgiOut,"function port_sel_change( obj )\n"\
								"{\n"\
									"var port_num = obj.options[obj.selectedIndex].text;\n"\
									"var url = 'wp_subintf.cgi?UN=%s&port_num='+port_num;\n"\
									"window.location.href = url;\n"\
								"}\n", encry);
				fprintf(cgiOut,"</script>\n" );
							fprintf(cgiOut,"</td>\n"\
	            							"</tr>\n"\
	            							"<tr height=4 valign=top>\n"\
	              							"<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>\n"\
	              							"<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>\n"\
	            							"</tr>\n"\
	          						"</table>\n"\
	        					"</td>\n"\
	        					"<td width=15 background=/images/di999.jpg>&nbsp;</td>\n"\
	      					"</tr>\n"\
	    				"</table>\n"\
	    			"</td>\n"\
	  		"</tr>\n"\
	  		"<tr>\n"\
	    			"<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>\n"\
	    			"<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>\n"\
	    			"<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>\n"\
	  		"</tr>\n"\
		"</table>\n"\
	"</div>\n"\
	"</form>\n"\
	"</body>\n" );
		
	fprintf(cgiOut,"<script type=text/javascript>\n");
	fprintf(cgiOut,"document.getElementById('tr_auto_check_height').style.height = document.getElementById('div_auto_check_height').offsetHeight-100;\n");
	fprintf(cgiOut,"</script>\n" );
	fprintf(cgiOut,"</html>\n");  


	release(lcontrol);
	release(lpublic); 
	for(i=0;i<8;i++)
	{
		free(ip[i]);
	}
	return 0;
}

int showPortSelect( char *selected_portno, char *name, char *onchange )
{
	#if 0 
	struct slot sr;
  	
	sr.module_status=0;     
	sr.modname=(char *)malloc(20);     //为结构体成员申请空间，假设该字段的最大长度为20
	sr.sn=(char *)malloc(20);          //为结构体成员申请空间，假设该字段的最大长度为20
	sr.hw_ver=0;
	sr.ext_slot_num=0;  
  	#endif
  	

	fprintf(cgiOut,"<select name='%s' onchange='%s' style='width:100%%;height:auto'>\n",name,onchange);

	    ccgi_dbus_init();
	    struct eth_portlist c_head,*p;
		int cnum,cflag=-1;
		cflag=ccgi_intf_show_advanced_routing_list(0, 1, &c_head, &cnum);
		p=c_head.next;
		
		if((NULL == selected_portno || strlen(selected_portno) == 0)&&(p!=NULL))
		{
			strcpy(selected_portno,p->ethport);
		}		
		if(p!=NULL)
		{
			while(p!=NULL)
			{
			    if(strcmp(p->ethport,selected_portno)==0)
				{
					fprintf(cgiOut,"<option value=%s selected=selected>%s",p->ethport,p->ethport);
				}
				else
				{
					fprintf(cgiOut,"<option value=%s>%s",p->ethport,p->ethport);
				}				
				p=p->next;
		      }
		}	

		if((cflag==0 )&& (cnum > 0))
		Free_ethp_info(&c_head);

	fprintf(cgiOut,"</select>" );
	fprintf(cgiOut,"<input type=hidden name=port_num value=%s>", selected_portno );
	return 0;
}

int isTagLegal( char *tag )
{
	int i=0;
	int iTag;
	
	for( i=0; i<strlen(tag);i++ )
	{
		if(tag[i] < '0' || tag[i] > '9') 
		{
			return 0;	
		}
	}
	
	sscanf( tag, "%d", &iTag );
	
	if( iTag < 1 || iTag > 4094 )
	{
		return 0;	
	}
	
	return 1;
}


int maskstr2int( char *mask )
{
	unsigned int iMask, m0, m1, m2, m3;
	char binarystr[64]="";
	int i, iRet;
	
	sscanf( mask, "%u.%u.%u.%u", &m3,&m2,&m1,&m0 );
	iMask = m3*256*256*256 + m2*256*256 + m1*256 + m0;
	
	iRet = 0;
	for( i=0; i < 32 ;i++ )
	{
		if( ( iMask & 1 ) == 1 )
		{
			binarystr[31-i] = '1';
			iRet ++;
		}
		else
		{
			binarystr[31-i] = '0';	
		}
		iMask = iMask >> 1;
	}
	
	if( strstr( binarystr, "01" ) )
	{
		return -1;	
	}
	
	return iRet;
}

int getUserInput( STUserInputInfo *pstUserInputInfo )
{
	int add_subif,retu;
	char qtype[50];
	memset(qtype,0,50);
	char rtypez[10];
	memset(rtypez,0,10);
	char subinfz[128];
	if(NULL == pstUserInputInfo)
	{
		return -1;
	}
	
	cgiFormStringNoNewlines( "process_type", pstUserInputInfo->process_type, sizeof(pstUserInputInfo->process_type) );
	cgiFormStringNoNewlines( "port_num", pstUserInputInfo->port_num, sizeof(pstUserInputInfo->port_num) );
	
	cgiFormStringNoNewlines( "subinterface_tag", pstUserInputInfo->tag, sizeof(pstUserInputInfo->tag) );
	cgiFormStringNoNewlines( "ssubvalue", qtype, 50 );
	cgiFormStringNoNewlines( "showtype", rtypez, 10 );
	cgiFormStringNoNewlines( "SINTF", pstUserInputInfo->sintf, sizeof(pstUserInputInfo->sintf) );
	add_subif = cgiFormSubmitClicked("submit_create");
	if( add_subif == cgiFormSuccess )
	{
		if( strlen( pstUserInputInfo->tag ) == 0 )
		{
			ShowAlert( search(lcontrol, "subif_not_tag") );
			return -1;
		}
		if( isTagLegal(pstUserInputInfo->tag) == 0 )
		{
			ShowAlert( search(lcontrol, "subif_illegal_tag") );
			return -1;
		}
		if(strcmp(rtypez,"1")==0)
		{
			if(strcmp(qtype,"")==0)
			{
				memset(subinfz,0,128);
				sprintf(subinfz,"%s %s","qinq-type",search(lpublic,"param_null"));
				ShowAlert( subinfz );
			}
			else
			{
				memset(subinfz,0,128);
				sprintf(subinfz,"eth%s.%s",pstUserInputInfo->port_num,pstUserInputInfo->tag);
				retu=set_intf_qinq_type(subinfz, qtype);
				if(retu==0)
					ShowAlert(search(lpublic,"oper_succ"));
				else
					ShowAlert(search(lpublic,"oper_fail"));
			}
		}
		return 0;
	}
	
	return -1;
}


int createSubinterface( STUserInputInfo *pstUserInputInfo ,char * encry)
{
	int ret=0;
	char url_temp[512];
	if( NULL == pstUserInputInfo )
		return -1;
	if(strcmp(pstUserInputInfo->process_type,"del")!=0)
	{
		ret = interface_eth_port(pstUserInputInfo->port_num,pstUserInputInfo->tag);
		switch(ret)
		{
			case 1:
			{
				ShowAlert(search(lcontrol,"appendsucc"));				
			}
			break;
			
			case -3:
			{
				ShowAlert(search(lcontrol,"no_such_port"));
			}
			break;

			default:
			{
				ShowAlert(search(lcontrol,"create_intf_error"));
			}
			break;

       
		}
		sprintf( url_temp, "wp_subintf.cgi?UN=%s&port_num=%s",encry,pstUserInputInfo->port_num);
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
		fprintf( cgiOut, "</script>\n" );

		
		return 0;
	}
	return -1;
}


int deleteSubinterface( STUserInputInfo *pstUserInputInfo,char * encry)
{
	char url_temp[512];
	int ret = 0;
	if( NULL == pstUserInputInfo )
		return -1;
	if( strcmp(pstUserInputInfo->process_type,"del") == 0 )
	{
		//int ret = no_interface_eth_port(pstUserInputInfo->port_num,pstUserInputInfo->tag);
		ret = ccgiconfig_no_interface_ifname_eth_port(pstUserInputInfo->sintf);
		switch(ret)
		{
			case 0:
			{
				ShowAlert(search(lcontrol,"del_succ"));
			}
			break;
			#if 0 
			case NPD_DBUS_ERROR_NO_SUCH_PORT:
			{
				ShowAlert(search(lcontrol,"no_such_port"));
			}
			break;

			case NPD_DBUS_ERROR:
			{
				ShowAlert(search(lcontrol,"create_intf_error"));
			}
			break;

			case DCLI_VLAN_NOTEXISTS:
			{
				ShowAlert(search(lcontrol,"s_arp_vlan_notexist"));
			}
			break;

			case NPD_VLAN_BADPARAM:
			{
				ShowAlert(search(lcontrol,"input_tag_error"));
			}
			break;

			case DCLI_NOT_CREATE_ROUTE_PORT_SUB_INTF:
			{
				ShowAlert(search(lcontrol,"no_route_sub"));
			}
			break;

			case DCLI_NOT_CREATE_VLAN_INTF_SUB_INTF:
			{
				ShowAlert(search(lcontrol,"no_vlan_subintf"));
			}
			break;

			case NPD_VLAN_PORT_NOTEXISTS:
			{
				ShowAlert(search(lcontrol,"port_not_tag"));
			}
			break;

			case DCLI_PARENT_INTF_NOT_EXSIT:
			{
				 ShowAlert(search(lcontrol,"parent_interface_not_exist"));
			}
			break;

			case DCLI_PROMI_SUBIF_EXIST:
			{	
				ShowAlert(search(lcontrol,"sub_interface_exist"));
			}
			break;
			#endif
			default:
			{
				ShowAlert(search(lpublic,"oper_fail"));
			}
			break;
		}
		sprintf( url_temp, "wp_subintf.cgi?UN=%s&port_num=%s",encry,pstUserInputInfo->port_num);
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
		fprintf( cgiOut, "</script>\n" );


	}
	return -1;	
}

