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
* wp_interface_bindip.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for port sub interface config ip
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
#include "stdlib.h"

#define PATH_LENG 512  
#define SHOW_AMOUNT 64 
#define ADD_MOUNT 5

int InterfaceBindIp(struct list *lpublic, struct list *lcontrol);   /*n代表加密后的字符串*/
int maskstr2int( char *mask );

int cgiMain()
{
 	struct list *lpublic;
 	struct list *lcontrol;

 	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");
 	InterfaceBindIp(lpublic, lcontrol);
 	return 0;
}

int InterfaceBindIp(struct list *lpublic, struct list *lcontrol)
{ 
	ccgi_dbus_init();
	int i;
	char *encry=malloc(BUF_LEN);                /*存储从wp_usrmag.cgi带入的加密字符串*/
	char *str = NULL;        
	char addn[N];
	char * turn_intf=(char *)malloc(20);
	memset(turn_intf ,0, 20);
	char IsSubmit[5] = { 0 };
	
//	FILE *fp;
	memset(encry,0,BUF_LEN);
	cgiFormStringNoNewlines("UN", encry, BUF_LEN);
	cgiFormStringNoNewlines("INTF",turn_intf,20);
	fprintf(stderr,"turn_intf=%s",turn_intf);
	str=dcryption(encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic, "ill_user")); 	       /*用户非法*/
		return 0;
	}
	
	strcpy(addn,str);
	cgiHeaderContentType("text/html");
	
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"\
				"<head>\n");
		fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>\n");
		fprintf(cgiOut,"<title>%s</title>\n",search(lpublic,"config_interface"));
		fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
					"<style type=text/css>\n"\
	  					".a3{width:30;border:0; text-align:center}\n"\
	  				"</style>\n"\
	  				"<script type=\"text/javascript\">\n");
		fprintf(cgiOut,"</script>\n");
fprintf(cgiOut,"<script language=javascript src=/ip.js>"\
			"</script>"\
			"<script language=javascript src=/fw.js>"\
			"</script>");
		fprintf(cgiOut,"</head>\n");

//*****************get input********************
	int ret,flag=1;
	char * port_num =(char *)malloc(PATH_LENG);
	char tempChar[25];
	
	char * ip1=(char *)malloc(ADD_MOUNT);
	char * ip2=(char *)malloc(ADD_MOUNT);
	char * ip3=(char *)malloc(ADD_MOUNT);
	char * ip4=(char *)malloc(ADD_MOUNT);
	
	char * mask1=(char *)malloc(ADD_MOUNT);
	char * mask2=(char *)malloc(ADD_MOUNT);
	char * mask3=(char *)malloc(ADD_MOUNT);
	char * mask4=(char *)malloc(ADD_MOUNT);
	
	char * get_port = (char *)malloc(SHOW_AMOUNT);

	char * ip=(char *)malloc(PATH_LENG);
	char * input_mask=(char *)malloc(PATH_LENG);

	char * command=(char *)malloc(PATH_LENG);

	char * mode = (char *)malloc(PATH_LENG);

	int mask=0;
	int flag1=-1,flag2=-1;
	int status = 0,retu = 0;

	memset(port_num,0,PATH_LENG);

	memset(ip1,0,ADD_MOUNT);
	memset(ip2,0,ADD_MOUNT);
	memset(ip3,0,ADD_MOUNT);
	memset(ip4,0,ADD_MOUNT);
	
	memset(mask1,0,ADD_MOUNT);
	memset(mask2,0,ADD_MOUNT);
	memset(mask3,0,ADD_MOUNT);
	memset(mask4,0,ADD_MOUNT);

	memset(get_port,0,SHOW_AMOUNT);

	memset(ip,0,PATH_LENG);
	memset(input_mask,0,PATH_LENG);

	memset(command,0,PATH_LENG);
	memset(mode,0,PATH_LENG);
	//**********************************************
	cgiFormStringNoNewlines("PORT",port_num,PATH_LENG);

	cgiFormStringNoNewlines("MODE",mode,PATH_LENG);
//*****************************do action while choose different mode*********************************

	memset(IsSubmit,0,sizeof(IsSubmit));  
	cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
	if((cgiFormSubmitClicked("config_interface")==cgiFormSuccess)&&(strcmp(IsSubmit,"")))
	{
		cgiFormStringNoNewlines("port_ip1",ip1,ADD_MOUNT);
		cgiFormStringNoNewlines("port_ip2",ip2,ADD_MOUNT);
		cgiFormStringNoNewlines("port_ip3",ip3,ADD_MOUNT);
		cgiFormStringNoNewlines("port_ip4",ip4,ADD_MOUNT);
		
		cgiFormStringNoNewlines("port_mask1",mask1,ADD_MOUNT);
		cgiFormStringNoNewlines("port_mask2",mask2,ADD_MOUNT);
		cgiFormStringNoNewlines("port_mask3",mask3,ADD_MOUNT);
		cgiFormStringNoNewlines("port_mask4",mask4,ADD_MOUNT);
		
		cgiFormStringNoNewlines("in_port",get_port,SHOW_AMOUNT);

		if((strlen(ip1)==0)||(strlen(ip2)==0)||(strlen(ip3)==0)||(strlen(ip4)==0))
		{
			//ShowAlert(search(lpublic,"ip_not_null"));
			flag1=0;
		}
		else
		{
			sprintf(ip,"%s.%s.%s.%s",ip1,ip2,ip3,ip4);
			flag1=1;
		}
		
		if((strlen(mask1)==0)||(strlen(mask2)==0)||(strlen(mask3)==0)||(strlen(mask4)==0))
		{
			//ShowAlert( search(lpublic,"mask_not_null"));
			flag2=0;
		}
		else
		{
			sprintf(input_mask,"%s.%s.%s.%s",mask1,mask2,mask3,mask4);
			mask = maskstr2int(input_mask);
			if((mask<0)||(mask>32))
			{
				ShowAlert(search(lcontrol,"subif_mask_illegal"));
				flag2=0;
			}
			else
			{
				flag2=1;
			}
		}
		if(strcmp(mode,"")==0)
		{
		//if not set port under mode,config sub-interface 没有设置端口模式的情况下

			if((flag1==1)&&(flag2==1))
			{
				sprintf(command,"sudo /usr/bin/set_intf_ip.sh %s %s/%d > /dev/null 2>&1",get_port,ip,mask);
				int ret_alert = system(command);
				if(ret_alert==0)
				{
					//ShowAlert(search(lpublic,"oper_succ"));
				}
				/*
				else if(ret_alert==256)
				{
					//ShowAlert(search(lcontrol,"config_eight_ip"));
					ShowAlert(search(lcontrol,"execute_fail"));
				}

				*/
				else if(ret_alert!=0)
				{
					flag=0;
					ShowAlert(search(lcontrol,"con_ip_fail"));
				}
			}
		}
		else if( get_port[0]=='0' && strcmp(mode,"promiscuous")!=0 )//主接口只能是promiscuous，不能配置
		{
			flag=0;
			ShowAlert(search(lcontrol,"port_mode_main_suport_p"));
		}
		else if(strcmp(mode,"")!=0)   
		{
		//if set mode,config sub-interface 设置了端口模式，配置子接口
			if((flag1==1)&&(flag2==1))
			{
				ret = 1;
				if( get_port[0] !='0' )//非主控口才调用下面的命令，主空口跳过，直接执行后面的设置ip的动作。
				 ret=ccgi_port_mode_conf(get_port,mode);   /*返回0表示失败，返回1表示成功，返回-1表示no such port，返回-2表示it is already this mode*/
											  /*返回-3表示unsupport this command，返回-4表示execute command failed*/
				  switch(ret)
				  {
				   	case 0:
					{
						ShowAlert(search(lcontrol,"con_port_mode_fail"));
						flag=0;
				             break;
				    	}
					case 1:break;
				    case -1:
				   	{
						ShowAlert(search(lcontrol,"no_port"));
						flag=0;
				              break;
				    	}
				    case -2:
					{
						ShowAlert(search(lcontrol,"already_mode"));
						flag=0;
				              break;
				    	}
				    case -3:
					{
						ShowAlert(search(lcontrol,"not_support"));
						flag=0;
				              break;
				    	}
					case -4:
					{
						ShowAlert(search(lcontrol,"execute_fail"));
						flag=0;
				              break;
				    	}
				  }   
					if(strcmp(mode,"switch")!=0)
					{
						strcat(command,"set_intf_ip.sh");
						strcat(command," ");
						strcat(command,"eth");
						strcat(command,get_port);
						strcat(command," ");
						strcat(command,ip);
						strcat(command,"/");\
						sprintf(tempChar,"%d > /dev/null 2>&1",mask);
						strcat(command,tempChar);
						status = system(command); 	 
						retu = WEXITSTATUS(status);
						if(retu!=0)    /*command fail*/
						{
		  					flag=0;
		  					ShowAlert(search(lcontrol,"con_ip_fail"));
						}
						
	  				}
			}
		}

		/*********************set_interface_mode***************************/
		char in_port[20] = { 0 };
		char interface_mode[10] = { 0 };

		memset(in_port,0,sizeof(in_port));
  	    cgiFormStringNoNewlines("in_port",in_port,20);
		memset(interface_mode,0,sizeof(interface_mode));
  	    cgiFormStringNoNewlines("interface_mode",interface_mode,10);
  	    if((strcmp(in_port,"")!=0) && (strcmp(interface_mode,"")!=0))
  	    {
  	    	status = 1;
  	    	memset(command,0,PATH_LENG);
		    snprintf(command, PATH_LENG-1, "set_interface_mode.sh %s %s > /dev/null 2>&1", in_port, interface_mode);

		    status = system(command); 	 
		    if(0 != status)    /*command fail*/
		    {
		    	flag=0;
				ShowAlert(search(lcontrol,"set_interface_mode_fail"));
		    }
  	    }

		/*********************add_if_description_fail***************************/
		char if_description[255] = { 0 };

		memset(if_description,0,sizeof(if_description));
  	    cgiFormStringNoNewlines("if_description",if_description,255);	
  	    if((strcmp(in_port,"")!=0) && (strcmp(if_description,"")!=0))
  	    {
  	    	status = 1;
  	    	memset(command,0,PATH_LENG);
		    snprintf(command, PATH_LENG-1, "interface_desc.sh %s %s", in_port, if_description);

		    status = system(command); 	 
		    if(0 != status)    /*command fail*/
		    {
		    	flag=0;
				ShowAlert(search(lcontrol,"add_if_description_fail"));
		    }
  	    }
		
		if(flag)
		{
			ShowAlert(search(lpublic,"oper_succ"));
		}
	}

	if((cgiFormSubmitClicked("no_if_description")==cgiFormSuccess)&&(strcmp(IsSubmit,"")))
	{
		char in_port[20] = { 0 };
		
		memset(in_port,0,sizeof(in_port));
  	    cgiFormStringNoNewlines("in_port",in_port,20);
		if(strcmp(in_port,"")!=0)
  	    {
  	    	status = 1;
  	    	memset(command,0,PATH_LENG);
		    snprintf(command, PATH_LENG-1, "no_interface_desc.sh %s", in_port);

		    status = system(command); 	 
		    if(0 == status)    /*command succ*/
			{
				ShowAlert(search(lcontrol,"del_if_description_succ"));
		    }	
			else
		    {
				ShowAlert(search(lcontrol,"del_if_description_fail"));
		    }
  	    }
	}	
	//*************************************************************************************************************
	fprintf(cgiOut,"<body>\n"\
				"<form id=Form1>\n"\
	  			"<div align=center>\n"\
	  				"<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
	  					"<tr>\n"\
	    						"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>\n"\
	    						"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>\n"\
	    						"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>\n",search(lpublic,"title_style"),search(lpublic,"config_interface"));
	    			fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>\n");
					fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>\n"\
	          							"<tr>\n"\
	          								"<td width=62 align=center><input id=but type=submit name=config_interface style=background-image:url(/images/%s) value=""></td>\n",search(lpublic,"img_ok"));	
	          					fprintf(cgiOut,"<td width=62 align=center><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>\n",encry,search(lpublic,"img_cancel"));
			  			fprintf(cgiOut,"</tr>\n"\
	          						"</table>\n");
	      			fprintf(cgiOut,"</td>\n"\
	    						"<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>\n"\
	  					"</tr>\n"\
	  					"<tr>\n"\
	    						"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>\n"\
	      					"<tr>\n"\
	        					"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>\n"\
	        					"<td width=948>\n"\
	        						"<table width=947 border=0 cellspacing=0 cellpadding=0>\n"\
	            							"<tr height=4 valign=bottom>\n"\
	              							"<td width=120>&nbsp;</td>\n"\
	              							"<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>\n"\
	            							"</tr>\n"\
	            							"<tr>\n"\
	              							"<td>\n"\
	              								"<table width=120 border=0 cellspacing=0 cellpadding=0>\n"\
	                   									"<tr height=25>\n"\
	                    										"<td id=tdleft>&nbsp;</td>\n"\
	                  									"</tr>\n");
						//left function table
										fprintf(cgiOut,"<tr height=25>"\
												  		"<td align=left id=tdleft><a href=wp_prtsur.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"prt_sur"));
							                     fprintf(cgiOut,"</tr>"\
													"<tr height=25>"\
							  					    		"<td align=left id=tdleft><a href=wp_prtarp.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>ARP<font><font id=%s> %s<font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"survey"));                       
							                    	fprintf(cgiOut,"</tr>");
									if(checkuser_group(addn)==0)  //administrator
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
									fprintf(cgiOut,"<tr height=25>"\
  					    								"<td align=left id=tdleft><a href=wp_subintf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",encry,search(lpublic,"menu_san"),search(lcontrol,"title_subintf"));  	                    
									fprintf(cgiOut,"<tr height=26>\n"\
						  							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>\n",search(lpublic,"menu_san"),search(lpublic,"config_interface"));
									fprintf(cgiOut,"<tr height=25>"\
  					    							"<td align=left id=tdleft><a href=wp_all_interface.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",encry,search(lpublic,"menu_san"),search(lcontrol,"interface")); 
						//set page length
									for(i=0;i<2;i++) 
		              					{
	  				    				fprintf(cgiOut,"<tr height=25>\n"\
	                      									"<td id=tdleft>&nbsp;</td>\n"\
	                    									"</tr>\n");
		              					}
	                					fprintf(cgiOut,"</table>\n"\
										"</td>\n"\
										"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">\n"\
											"<table  border=0 cellspacing=0 cellpadding=0>\n"\
												"<tr>\n"\
													"<td align='left'>\n");
											fprintf(cgiOut,"<input type='hidden' name=UN value=%s>",encry);//right
											fprintf(cgiOut,"<input type='hidden' name=MODE value=%s>",mode);
											fprintf(cgiOut,"<input type='hidden' name=PORT value=%s>",port_num);
											fprintf(cgiOut,"<input type='hidden' name=SubmitFlag value=%d>",1);

		 //MY CODE********************************************************************************************************	
			 								fprintf(cgiOut,"<table align='left'>\n");
		 									if(strcmp(mode,"")==0)
		 									{
		 										for(i=0;i<2;i++)
		 										{
		 										fprintf(cgiOut,"<tr><td height='25'>&nbsp;</td></tr>\n");
		 										}
		 									}
											else
											{
												for(i=0;i<2;i++)
		 										{
		 										fprintf(cgiOut,"<tr><td height='25'>&nbsp;</td></tr>\n");
		 										}
												fprintf(cgiOut,"<tr>\n"\
																"<td style='font-size:14px'><font color='red'><b>%s:</b></font></td>\n"\
																"<td colspan='2' style='font-size:14px'><font color='red'><b>%s</b></font></td>\n"\
															"</tr>\n",search(lcontrol,"mode"),mode);
												fprintf(cgiOut,"<tr><td height='25'>&nbsp;</td></tr>\n");
											}
												if(strcmp(port_num,"")==0 && strcmp(turn_intf,"")==0)
												{
				 									fprintf(cgiOut,"<tr>\n");
					 								fprintf(cgiOut,"<td height='25'>%s:</td>\n",search(lcontrol,"interface"));
													fprintf(cgiOut,"<td height='25'><input type='text' name='in_port' style='width:100%%;height:auto'></td>\n"\
				 												"</tr>\n");
												}
												else
												{
													fprintf(cgiOut,"<tr>\n"\
					 												"<td height='25'>%s:</td>\n",search(lcontrol,"interface"));
													if(strcmp(port_num,"")!=0)
					 									fprintf(cgiOut,"<td height='25'><input type='text' name='in_port' value='%s' readonly style='width:100%%;height:auto'></td>\n</tr>\n",port_num);
													else if(strcmp(turn_intf,"")!=0)
														fprintf(cgiOut,"<td height='25'><input type='text' name='in_port' value='%s' readonly style='width:100%%;height:auto'></td>\n</tr>\n",turn_intf);
												}
												fprintf(cgiOut,"<tr><td height='25'>&nbsp;</td></tr>\n");
		 										fprintf(cgiOut,"<tr>\n");
													fprintf(cgiOut,"<td>IP/%s</td>\n",search(lcontrol,"mask"));
													fprintf(cgiOut,"<td>\n");
														fprintf(cgiOut,"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">" );
															fprintf(cgiOut,"<input type=text name='port_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
															fprintf(cgiOut,"<input type=text name='port_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
															fprintf(cgiOut,"<input type=text name='port_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
															fprintf(cgiOut,"<input type=text name='port_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
														fprintf(cgiOut,"</div>\n"\
																"</td>\n"\
																"<td>/</td>\n");
													fprintf(cgiOut,"<td>\n"\
																	"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">" );
															fprintf(cgiOut,"<input type=text name='port_mask1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
															fprintf(cgiOut,"<input type=text name='port_mask2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
															fprintf(cgiOut,"<input type=text name='port_mask3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
															fprintf(cgiOut,"<input type=text name='port_mask4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
														fprintf(cgiOut,"</div>"\
																"</td>");
												fprintf(cgiOut,"</tr>\n");

												fprintf(cgiOut,"<tr><td height='25'>&nbsp;</td></tr>\n");
												
												fprintf(cgiOut,"<tr>\n");
													fprintf(cgiOut,"<td height='25'>%s:</td>\n",search(lcontrol,"interface_mode"));
													fprintf(cgiOut,"<td height='25'>"\
														"<select name=interface_mode id=interface_mode style=width:150px>"\
															"<option value=></option>"\
															"<option value=global>global</option>"\
															"<option value=local>local</option>"\
														"</select>"\
													"</td>\n"\
												"</tr>\n");

												fprintf(cgiOut,"<tr><td height='25'>&nbsp;</td></tr>\n");
												
												fprintf(cgiOut,"<tr>\n");
					 								fprintf(cgiOut,"<td height='25'>%s:</td>\n",search(lcontrol,"add_if_description"));
													fprintf(cgiOut,"<td height='25'><input type='text' name='if_description' style='width:100%%;height:auto'></td>\n"\
													"<td align=left style=padding-left:10px><input type=submit style=width:80px; height:36px  border=0 name=no_if_description style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"del_if_description"));
				 												fprintf(cgiOut,"</tr>\n");
												
			 								fprintf(cgiOut,"</table>\n");
	//*********************************************************END
										fprintf(cgiOut,"</td>\n"\
			  	  								"</tr>\n"\
	      										"</table>\n"\
	              							"</td>\n"\
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
	"</body>\n"\
	"</html>\n");

	release(lpublic);
	release(lcontrol);
	free(turn_intf);
	free(encry);
	free(ip1);
	free(ip2);
	free(ip3);
	free(ip4);
	free(mask1);
	free(mask2);
	free(mask3);
	free(mask4);
	free(get_port);
	free(ip);
	free(input_mask);
	free(port_num);
	free(command);
	free(mode);
	
	return 0;
}

int maskstr2int( char *mask )
{
	unsigned int iMask, m0, m1, m2, m3;
	char binarystr[64]="";
	int i,iRet;
	
	sscanf(mask,"%u.%u.%u.%u", &m3,&m2,&m1,&m0);
	iMask = m3*256*256*256 + m2*256*256 + m1*256 + m0;
	
	iRet = 0;
	for( i=0; i < 32 ;i++ )
	{
		if((iMask&1) == 1 )
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



