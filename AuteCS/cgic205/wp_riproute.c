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
	* wp_riproute.c
	*
	*
	* CREATOR:
	* autelan.software.Network Dep. team
	* tangsq@autelan.com
	*
	* DESCRIPTION:
	* system contrl for route config 
	*
	*
	*******************************************************************************/
#include <stdio.h>
#include <sys/wait.h>

#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"


#define Info_Num               500
#define NEI_NUM                128
#define MAX_NEIGHBOR_NUM       128

#define RIP_VERSION_LEN         16
#define RIP_DEF_INFOR_LEN       16

#define RIP_UPDATE_TIME_LEN     16
#define RIP_TIMEOUT_TIME_LEN    16
#define RIP_GARBAGE_TIME_LEN    16


#define RIP_DEL_METRIC_LEN      16

#define RIP_REDIS_LEN           16


#define RIP_DISTANCE_LEN        16
#define RIP_DISTRIBUTE_LEN      16


	typedef struct {
		int 	RIP_enable;
		char  	RIP_version[RIP_VERSION_LEN];
		char  	RIP_default_infor[RIP_DEF_INFOR_LEN];
		
		struct {
		char	RIP_update_time[RIP_UPDATE_TIME_LEN];
		char	RIP_timeout_time[RIP_TIMEOUT_TIME_LEN];
		char	RIP_garbage_time[RIP_GARBAGE_TIME_LEN];
		}RIP_timer;
		
		char  	RIP_def_metric[RIP_DEL_METRIC_LEN];
		int  	RIP_resdis_flag[6];
		
		struct {
		char	RIP_REDIS_OSPF[RIP_REDIS_LEN];
		char	RIP_REDIS_BGP[RIP_REDIS_LEN];
		char	RIP_REDIS_CONNECTED[RIP_REDIS_LEN];
		char	RIP_REDIS_STATIC[RIP_REDIS_LEN];
		char	RIP_REDIS_KERNEL[RIP_REDIS_LEN];
		char	RIP_REDIS_ISIS[RIP_REDIS_LEN];
		}RIP_redis_metric;
		
		char  	RIP_distance[RIP_DISTANCE_LEN];
		char *	RIP_neigh[MAX_NEIGHBOR_NUM];
		char    RIP_distribute_in[RIP_DISTRIBUTE_LEN];
		char    RIP_distribute_out[RIP_DISTRIBUTE_LEN];
		char 	RIP_distribute_in_acl[RIP_DISTRIBUTE_LEN];
		char 	RIP_distribute_out_acl[RIP_DISTRIBUTE_LEN];
	}RIP_status;



	int ShowAddroutePage(); 
	int ReadConfig(char * ripInfo[],RIP_status * p_ripinfo,struct list * lpublic);
	int RIPconfig(int checkRip,char * checkversion,char * redis);
	int OPTNeighbor(char * addORdel,struct list *lpublic,struct list *lcontrol, char * NeiList);
	int check_str_par_valid(char * src, int min, long max);


	int cgiMain()
	{
		//ShowAlert("Operation Success");
	 ShowAddroutePage();
	 return 0;
	}




   /**************************************************************************

     name:          ShowAddroutePage
     fuction:       show page to admin or user to let them to know or configure RIProute
     parameter:     parameter is empty

     return value:  
                   0:       normal 
                   other:   error status
     
   **************************************************************************/

	int ShowAddroutePage()
	{
		struct list *lpublic;	/*解析public.txt文件的链表头*/
		struct list *lcontrol; 	/*解析help.txt文件的链表头*/
		lpublic=get_chain_head("../htdocs/text/public.txt");
		lcontrol=get_chain_head("../htdocs/text/control.txt"); 

		FILE *fp;
		char lan[3];
		char *encry=(char *)malloc(BUF_LEN);			  
		char *str=NULL;
		int i;
		char encry_riproute[BUF_LEN];
		char * RipItem[Info_Num]; 
		char * timer[3];
		char * neighbor[NEI_NUM];
		//char * redistribute[6];
		char * redistribute_metric[6];
		char * default_route=(char *)malloc(20);
		memset(default_route,0,20);
		char * default_metric=(char *)malloc(20);
		memset(default_metric,0,20);
		char * neighborlist=(char *)malloc(500);
		memset(neighborlist,0,500);

		char * neighborlistLater=(char *)malloc(500);
		memset(neighborlistLater,0,500);
		char * Vsion=(char *)malloc(10);
		memset(Vsion,0,10);
		
		int rip_enable=0; /*rip_enable=1为关闭，0为打开*/
		char * version=(char *)malloc(20);
		memset(version,0,20);
		char * metric=(char *)malloc(20);
		memset(metric,0,20);
		char * distance=(char *)malloc(20);
		memset(distance,0,20);
		char * PRIUsr=(char *)malloc(20);
		memset(PRIUsr,0,20);

		char * redis_param=(char *)malloc(40);
		memset (redis_param,0,40);

		
		int retu=0;     //retu 代表用户的组属性，0是管理员，1是普通用户，开始时，初值置0

		char distribute_in[20],distribute_out[20],acl_in[10],acl_out[10],dis_intf_in[20],dis_intf_out[20];
		memset( distribute_in, 0, 20 );
		memset( distribute_out, 0, 20 );
		memset( acl_in, 0, 10 );
		memset( acl_out, 0, 10 );
		memset( dis_intf_in, 0, 20 );
		memset( dis_intf_out, 0, 20 );

		int netghnorNum=0,exe_ret = 0;
		for(i=0;i<3;i++)
		{
			timer[i]=(char *)malloc(20);
			memset(timer[i],0,20);
		}
		for(i=0;i<NEI_NUM;i++)
		{
			neighbor[i]=(char *)malloc(20);
			memset(neighbor[i],0,20);
		}

		for(i=0;i<6;i++)
		{
			redistribute_metric[i]=(char *)malloc(5);
			memset(redistribute_metric[i],0,5);
		}
		
		for(i=0;i<Info_Num;i++)
		{
			RipItem[i]=(char *)malloc(60);
			memset(RipItem[i],0,60);
		}
		RIP_status status_info;
		memset(&status_info,0,sizeof(RIP_status));

		for(i=0;i<MAX_NEIGHBOR_NUM;i++)
		{
			status_info.RIP_neigh[i]=(char *)malloc(30);
			memset(status_info.RIP_neigh[i],0,30);
		}
		
		if((cgiFormSubmitClicked("submit_riproute") != cgiFormSuccess) && (cgiFormSubmitClicked("addneighbour") != cgiFormSuccess) && (cgiFormSubmitClicked("delneighbour") != cgiFormSuccess))
		{
		  memset(encry,0,BUF_LEN);
		  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		  
		  str=dcryption(encry);

		  
		  if(str==NULL)
		  {
			ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
			return 0;
		  }
		  memset(encry_riproute,0,BUF_LEN);					 /*清空临时变量*/
		}

	  cgiFormStringNoNewlines("encry_riproute",encry_riproute,BUF_LEN);
	  cgiFormStringNoNewlines("Version",Vsion,BUF_LEN);
	  cgiFormStringNoNewlines("NeiList",neighborlistLater,500);
	  cgiFormStringNoNewlines("PRIUsr",PRIUsr,20);
	  cgiFormStringNoNewlines("redis_param",redis_param,40);
	  
	  
	  if(strcmp(PRIUsr,"")!=0)
	  	retu=atoi(PRIUsr);

	  
	  //fprintf(stderr,"11neighborlistLater=%s111",neighborlistLater);

	  cgiHeaderContentType("text/html");
		fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
			
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"route_manage"));
	  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	  	  	"<style type=text/css>"\
		  ".a3{width:30;border:0; text-align:center}"\
		  "</style>"\
	  "</head>"\
	   "<script src=/ip.js>"\
	  "</script>"\
	  "<script language=javascript>"\
	  "function check_rip_enable(obj)"\
		"{"\
	  		"if(document.rip_route.Rip_Enable.value==\"closeRip\" || obj==\"abc\")"\
	  		"{"\
	  			"document.rip_route.route_update_time.disabled=true;"\
	  			"document.rip_route.Version.disabled=true;"\
	  			"document.rip_route.route_Invalid_time.disabled=true;"\
	  			"document.rip_route.default_route.disabled=true;"\
	  			"document.rip_route.route_Holddown_time.disabled=true;"\
	  			
	  			"document.rip_route.global_metric.disabled=true;"\
	  			"document.rip_route.ospf_metric.disabled=true;"\
	  			"document.rip_route.bgp_metric.disabled=true;"\
	  			"document.rip_route.connected_metric.disabled=true;"\
	  			"document.rip_route.static_metric.disabled=true;"\
	  			"document.rip_route.kernel_metric.disabled=true;"\
	  			"document.rip_route.isis_metric.disabled=true;"\
	  			
	  			"document.rip_route.distance.disabled=true;"\
	  			"document.rip_route.target_ip1.disabled=true;"\
	  			"document.rip_route.target_ip2.disabled=true;"\
	  			"document.rip_route.target_ip3.disabled=true;"\
	  			"document.rip_route.target_ip4.disabled=true;"\
	  			"document.rip_route.neighbourRouter.disabled=true;"\

					"document.rip_route.ospf.disabled=true;"\
					"document.rip_route.bgp.disabled=true;"\
					"document.rip_route.connected.disabled=true;"\
					"document.rip_route.static.disabled=true;"\
					"document.rip_route.kernel.disabled=true;"\
					"document.rip_route.isis.disabled=true;"\

				"document.rip_route.distribute_out.disabled=true;"\
				"document.rip_route.distribute_in.disabled=true;"\
				"document.rip_route.acl_index_out.disabled=true;"\
				"document.rip_route.acl_index_in.disabled=true;"\

	  		"}"\
	  		"else"\
	  		"{"\

	  			"document.rip_route.route_update_time.disabled=false;"\
	  			"document.rip_route.Version.disabled=false;"\
	  			"document.rip_route.route_Invalid_time.disabled=false;"\
	  			"document.rip_route.default_route.disabled=false;"\
	  			"document.rip_route.route_Holddown_time.disabled=false;"\
	  			
	  			"document.rip_route.global_metric.disabled=false;"\
	  			"document.rip_route.ospf_metric.disabled=false;"\
	  			"document.rip_route.bgp_metric.disabled=false;"\
	  			"document.rip_route.connected_metric.disabled=false;"\
	  			"document.rip_route.static_metric.disabled=false;"\
	  			"document.rip_route.kernel_metric.disabled=false;"\
	  			"document.rip_route.isis_metric.disabled=false;"\
	  			
	  			"document.rip_route.distance.disabled=false;"\
	  			"document.rip_route.target_ip1.disabled=false;"\
	  			"document.rip_route.target_ip2.disabled=false;"\
	  			"document.rip_route.target_ip3.disabled=false;"\
	  			"document.rip_route.target_ip4.disabled=false;"\
	  			"document.rip_route.neighbourRouter.disabled=false;"\

					"document.rip_route.ospf.disabled=false;"\
					"document.rip_route.bgp.disabled=false;"\
					"document.rip_route.connected.disabled=false;"\
					"document.rip_route.static.disabled=false;"\
					"document.rip_route.kernel.disabled=false;"\
					"document.rip_route.isis.disabled=false;"\

	  			"document.rip_route.distribute_out.disabled=false;"\
				"document.rip_route.distribute_in.disabled=false;"\
				"document.rip_route.acl_index_out.disabled=false;"\
				"document.rip_route.acl_index_in.disabled=false;"\

	  		"}"\
	  	"}"\
	  "</script>");
	  if((cgiFormSubmitClicked("submit_riproute") != cgiFormSuccess) && (cgiFormSubmitClicked("addneighbour") != cgiFormSuccess) && (cgiFormSubmitClicked("delneighbour") != cgiFormSuccess))
	  {
	  		retu=checkuser_group(str);
	  }
	  else if(cgiFormSubmitClicked("submit_riproute") == cgiFormSuccess)
	   {
	  	  if(retu==0)  /*管理员*/
	  	  {
	  		exe_ret = RIPconfig(rip_enable,Vsion,redis_param);
			if (exe_ret == -1)
				ShowAlert(search(lcontrol,"illegal_input"));
			else if (exe_ret == 0)
				ShowAlert(search(lcontrol,"exe_suc"));
			 
	  	  }
	  	  else
	  	  {
	  	  	 fprintf( cgiOut, "<script type='text/javascript'>\n" );
	       	 fprintf( cgiOut, "window.location.href='wp_srouter.cgi?UN=%s';\n", encry_riproute);
	       	 fprintf( cgiOut, "</script>\n" );
	  	  }
	   }
	   fprintf(stderr,"retu=%d",retu);





	//下面检查用户的组属性，即是属于管理员还是普通用户   
	  if(retu==0)  /*管理员*/
	  	{
	  	    fprintf(cgiOut,"<body onload=check_rip_enable(\"123\")>");
	  	}

	  else
	    {

	        fprintf(cgiOut,"<body onload=check_rip_enable(\"abc\")>");
	  	}



	  if(cgiFormSubmitClicked("delneighbour") == cgiFormSuccess)
	  {
	  		fprintf(stderr,"neighborlistLater=%s",neighborlistLater);
			OPTNeighbor("del",lpublic,lcontrol,neighborlistLater);
	  }


	  
	  if(cgiFormSubmitClicked("addneighbour") == cgiFormSuccess)
	  {
	  		fprintf(stderr,"neighborlistLater=%s",neighborlistLater);
			OPTNeighbor("add",lpublic,lcontrol,neighborlistLater);
	  }


	  
	  fprintf(cgiOut,"<form method=post name=rip_route>"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	  "<tr>"\
	    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcontrol,"route_manage"));
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
	          "<td width=62 align=center><input id=but type=submit name=submit_riproute style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));	  
			  if((cgiFormSubmitClicked("submit_riproute") != cgiFormSuccess) && (cgiFormSubmitClicked("addneighbour") != cgiFormSuccess) && (cgiFormSubmitClicked("delneighbour") != cgiFormSuccess))
	            fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
			  else                                         
	     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry_riproute,search(lpublic,"img_cancel"));
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

		

	                		if((cgiFormSubmitClicked("submit_riproute") != cgiFormSuccess) && (cgiFormSubmitClicked("addneighbour") != cgiFormSuccess) && (cgiFormSubmitClicked("delneighbour") != cgiFormSuccess))
	                		{
	                		  	if(retu==0)  /*管理员*/
	                		  	{
	    						  	fprintf(cgiOut,"<tr height=26>"\
	                    			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>RIP</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));   /*突出显示*/
	                    		  	fprintf(cgiOut,"</tr>");
	                       		 	fprintf(cgiOut,"<tr height=25>"\
	                				"<td align=left id=tdleft><a href=wp_riplist.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"riplist"));						
	                				fprintf(cgiOut,"</tr>");
	       						 	fprintf(cgiOut,"<tr height=25>"\
	                				"<td align=left id=tdleft><a href=wp_ripaddintf.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_rip_intf"));						
	                				fprintf(cgiOut,"</tr>");
	         					}
	         					else
	         					{
	         						fprintf(cgiOut,"<tr height=26>"\
	                    			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>RIP</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));   /*突出显示*/
	                    		  	fprintf(cgiOut,"</tr>");
	                       		 	fprintf(cgiOut,"<tr height=25>"\
	                				"<td align=left id=tdleft><a href=wp_riplist.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"riplist"));						
	                				fprintf(cgiOut,"</tr>");
	         					}
	                		}
	                		else		
	                		{

							  	if(retu==0)  /*管理员*/
	                		  	{
	     						  fprintf(cgiOut,"<tr height=26>"\
	                     			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>RIP</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));   /*突出显示*/
	                     		  fprintf(cgiOut,"</tr>");
	                     		  fprintf(cgiOut,"<tr height=25>"\
	                  				"<td align=left id=tdleft><a href=wp_riplist.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",encry_riproute,search(lpublic,"menu_san"),search(lcontrol,"riplist"));						
	                  				fprintf(cgiOut,"</tr>");
	         						  fprintf(cgiOut,"<tr height=25>"\
	                  				"<td align=left id=tdleft><a href=wp_ripaddintf.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",encry_riproute,search(lpublic,"menu_san"),search(lcontrol,"add_rip_intf"));						
	                  				fprintf(cgiOut,"</tr>");
	         					}
	            				else
	            				{
	            					fprintf(cgiOut,"<tr height=26>"\
	                     			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>RIP</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));   /*突出显示*/
	                     		  fprintf(cgiOut,"</tr>");
	                     		  fprintf(cgiOut,"<tr height=25>"\
	                  				"<td align=left id=tdleft><a href=wp_riplist.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",encry_riproute,search(lpublic,"menu_san"),search(lcontrol,"riplist"));						
	                  				fprintf(cgiOut,"</tr>");
	            				}
	                		}

						  for(i=0;i<21;i++)
						  {
							fprintf(cgiOut,"<tr height=25>"\
							  "<td id=tdleft>&nbsp;</td>"\
							"</tr>");
						  }

					  fprintf(cgiOut,"</table>"\
	              "</td>"\
	              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
						  "<table width=640 height=365 border=0 cellspacing=0 cellpadding=0>");
						  	fprintf(cgiOut,"<tr>"\
								"<td id=sec1 colspan=3 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"General_set"));
								fprintf(cgiOut,"</tr>"\
							"<tr>"\
							  "<td colspan=3 align=left valign=top style=padding-top:10px>"\
							  "<table width=600 border=0 cellspacing=0 cellpadding=0>");

								
					  			int flag[6];
					  			for(i=0;i<6;i++)
					  			{
					  				flag[i]=0;
					  			}
					  			
					  			
					  			ReadConfig(RipItem,	&status_info, lpublic);
								
								fprintf(cgiOut,"<tr height=35>"\
								"<td style=color:red>*</td>");
								fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"rip_enable"));
								

						
								if(status_info.RIP_enable==1)
								{
									fprintf(cgiOut,"<td colspan=2 align=left>");
									
									if(retu==0)  /*管理员*/
									{
										fprintf(cgiOut, "<select name=\"Rip_Enable\" onchange=check_rip_enable(this)>\n");
									}

									
									else
									{
	     								fprintf(cgiOut, "<select name=\"Rip_Enable\" disabled onchange=check_rip_enable(\"abc\")>\n");
									}

									
	     							fprintf(cgiOut, "<option value=closeRip>CLOSE\n");
	                              	fprintf(cgiOut, "<option value=openRip>OPEN\n");
	                              	fprintf(cgiOut, "</select>\n");
	     							fprintf(cgiOut,"</td>");
								}
								
								else if(status_info.RIP_enable==0)
								{
	     							fprintf(cgiOut,"<td colspan=2 align=left>");
									
	     							if(retu==0)  /*管理员*/
	     							{
										fprintf(cgiOut, "<select name=\"Rip_Enable\" onchange=check_rip_enable(this)>\n");
	     							}
									else
									{
	     								fprintf(cgiOut, "<select name=\"Rip_Enable\" disabled onchange=check_rip_enable(\"abc\")>\n");
									}
	                              	fprintf(cgiOut, "<option value=openRip>OPEN\n");
	                              	fprintf(cgiOut, "<option value=closeRip>CLOSE\n");
	                              	fprintf(cgiOut, "</select>\n");
	     							fprintf(cgiOut,"</td>");
								}
								
								
								fprintf(cgiOut,"<td align=right style=color:red>*</td>");
								fprintf(cgiOut,"<td colspan=2 align=left>%s:</td>",search(lcontrol,"route_update"));
								
								if(strcmp(status_info.RIP_timer.RIP_update_time,"")==0)
								{
									fprintf(cgiOut,"<td><input type=text name=route_update_time size=6 value=30></td>");
								}
								else
								{
									fprintf(cgiOut,"<td><input type=text name=route_update_time size=6 value=%s></td>",status_info.RIP_timer.RIP_update_time);
								}
								
								fprintf(cgiOut,"<td align=left>seconds</td>");
								fprintf(cgiOut,"</tr>"\

								"<tr align=left height=35>");
								fprintf(cgiOut,"<td style=color:red>*</td>");
								fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"version"));
								//fprintf(stderr,"status_info.RIP_version=%s",status_info.RIP_version);
								if( strcmp(status_info.RIP_version,"#2#") == 0 )
								{
	     							fprintf(cgiOut,"<td colspan=2 align=left>");
	     							if(retu==0)  /*管理员*/
	     								fprintf(cgiOut, "<select name=Version>\n");
	     							else
	     								fprintf(cgiOut, "<select name=Version disabled>\n");
	     							fprintf(cgiOut, "<option value=2>%s2\n",search(lcontrol,"version"));
	     							fprintf(cgiOut, "<option value=1>%s1\n",search(lcontrol,"version"));
									fprintf(cgiOut, "<option value=v2broadcast>%s2%s\n",search(lcontrol,"version"),search(lcontrol,"broadcast"));
	     							fprintf(cgiOut, "</select>\n");
	     							fprintf(cgiOut,"</td>");
								}
								else if( strcmp(status_info.RIP_version,"#2#broadcast") == 0 )
								{
									fprintf(cgiOut,"<td colspan=2 align=left>");
	     							if(retu==0)  /*管理员*/
	     								fprintf(cgiOut, "<select name=Version>\n");
	     							else
	     								fprintf(cgiOut, "<select name=Version disabled>\n");
									fprintf(cgiOut, "<option value=v2broadcast>%s2%s\n",search(lcontrol,"version"),search(lcontrol,"broadcast"));
	     							fprintf(cgiOut, "<option value=2>%s2\n",search(lcontrol,"version"));
	     							fprintf(cgiOut, "<option value=1>%s1\n",search(lcontrol,"version"));
									
	     							fprintf(cgiOut, "</select>\n");
	     							fprintf(cgiOut,"</td>");
								}
								else
								{
									fprintf(cgiOut,"<td colspan=2 align=left>");
									
									if(retu==0)  /*管理员*/
									{
										fprintf(cgiOut, "<select name=Version>\n");
									}
									else
									{
										fprintf(cgiOut, "<select name=Version disabled>\n");
									}
									
									fprintf(cgiOut, "<option value=1>%s1\n",search(lcontrol,"version"));
									fprintf(cgiOut, "<option value=2>%s2\n",search(lcontrol,"version"));
									fprintf(cgiOut, "<option value=v2broadcast>%s2%s\n",search(lcontrol,"version"),search(lcontrol,"broadcast"));
									fprintf(cgiOut, "</select>\n");
									fprintf(cgiOut,"</td>");
								}
								
								fprintf(cgiOut,"<td align=right style=color:red>*</td>");
								fprintf(cgiOut,"<td colspan=2 align=left>%s:</td>",search(lcontrol,"Invalid_Timer"));
								
								if(strcmp(status_info.RIP_timer.RIP_timeout_time,"")==0)
								{
									fprintf(cgiOut,"<td><input type=text name=route_Invalid_time size=6 value=180></td>");
								}
								else
								{
									fprintf(cgiOut,"<td><input type=text name=route_Invalid_time size=6 value=%s></td>",status_info.RIP_timer.RIP_timeout_time);
								}
								
								fprintf(cgiOut,"<td align=left>seconds</td>");
								fprintf(cgiOut,"</tr>"\

								"<tr align=left height=35>");
								fprintf(stderr, "RIP_default_infor=%s",status_info.RIP_default_infor);


								
								/////////////////////modify this at 11:36 A.M.,12/06/2010//////////////////////
								
							   /***************************modified content:**********
							   
							        *  add code to check user's group attributes

							        
							        **************************************************/
                                //////////////////注释的部分是原来的代码//////////////////////////////
                                //////////////原来的普通用户也可以选中复选框/////////////////////////
								#if 0
								
								if (strstr(status_info.RIP_default_infor,"originate") != 0)
								  {
									fprintf(cgiOut, "<td colspan=4><input type=checkbox name=default_route value=def_route checked>%s</td>",search(lcontrol,"accept_default"));
								  }
								else
								  {
									fprintf(cgiOut, "<td colspan=4><input type=checkbox name=default_route value=def_route>%s</td>",search(lcontrol,"accept_default"));
								  }

								#endif
								

	                            #if 1
								
	                            if(       ( 0  == retu) &&  (strstr(status_info.RIP_default_infor,"originate") != 0) )
								{
									  fprintf(cgiOut, "<td colspan=4><input type=checkbox name=default_route value=def_route checked>%s</td>",search(lcontrol,"accept_default"));
								}
	                            else if ( ( 0 == retu) &&   (strstr(status_info.RIP_default_infor,"originate") == 0) )
								{
									  fprintf(cgiOut, "<td colspan=4><input type=checkbox name=default_route value=def_route>%s</td>",search(lcontrol,"accept_default"));
								}
								else if ( 0 !=  retu )
							    {
									   fprintf(cgiOut, "<td colspan=4><input type=checkbox name=default_route   disabled  value=def_route  >%s</td>",search(lcontrol,"accept_default"));

							     }

								#endif

								
								fprintf(cgiOut,"<td align=right style=color:red>*</td>");
								fprintf(cgiOut,"<td  colspan=2 align=left>%s:</td>",search(lcontrol,"garbage_collect_Timer"));


                                ////////////////////////////////注释部分是原来的代码////////////////////////////////////////
                                ///////////////////原来的普通用户也在文本框中输入///////////////////////////////////////////

								#if 0
								
								if(strcmp(status_info.RIP_timer.RIP_garbage_time,"")==0)
									fprintf(cgiOut,"<td><input type=text name=route_Holddown_time size=6 value=120></td>");
								else
									fprintf(cgiOut,"<td><input type=text name=route_Holddown_time size=6 value=%s></td>",status_info.RIP_timer.RIP_garbage_time);

								#endif

								#if 1
								
								if (  ( 0 == retu)   &&    (strcmp(status_info.RIP_timer.RIP_garbage_time,"")==0) )
								{
									fprintf(cgiOut,"<td><input type=text name=route_Holddown_time size=6 value=120></td>");
								}
								
								else  if ( 0 == retu)
								{
									fprintf(cgiOut,"<td><input type=text name=route_Holddown_time size=6 value=%s></td>",status_info.RIP_timer.RIP_garbage_time);
								}

								else if( 0 
!= retu)
								{

                                    fprintf(cgiOut,"<td><input type=text name=route_Holddown_time size=6 value=%s  disabled></td>",status_info.RIP_timer.RIP_garbage_time);
								}

								#endif


								
								
								fprintf(cgiOut,"<td align=left>seconds</td>");
								fprintf(cgiOut,"</tr>"\

								"<tr align=left height=35>");
								fprintf(cgiOut,"<td width=10>&nbsp;</td>");
								fprintf(cgiOut,"<td align=left width=110>%s: </td>",search(lcontrol,"redistribute"));
								memset(redis_param,0,40);


					
	                            
								/////////////////////modify this at 11:36 A.M.,12/06/2010//////////////////////
															
								/***************************modified content:**********
														   
								* add code to check user's group attributes
								
																
								**************************************************/
								//////////////////注释的部分是原来的代码//////////////////////////////
                                //////////////原来的普通用户也可以选中复选框/////////////////////////

								#if 0
	     					    if (status_info.RIP_resdis_flag[0]==1)
	     						    fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=ospf value=ospf checked>OSPF</td>");
	     						else
	     						    fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=ospf value=ospf>OSPF</td>");
								fprintf(cgiOut,"<td align=left>OSPF Metric: </td>");
								if (strcmp(status_info.RIP_redis_metric.RIP_REDIS_OSPF,"")!=0)
									fprintf(cgiOut,"<td align=left><input type=text name=ospf_metric size=6 value=%s></td>",status_info.RIP_redis_metric.RIP_REDIS_OSPF );
								else
									fprintf(cgiOut,"<td align=left><input type=text name=ospf_metric size=6 value=1></td>");
								fprintf(cgiOut,"</tr>");

							    #endif


								#if 1
								
	     					    if (   (0 == retu)   &&   ( status_info.RIP_resdis_flag[0]    ==  1 )  )
	     					    	{
								
	     						       fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=ospf value=ospf checked>OSPF</td>");

	     					    	}  
								
	     						else if (  (0 == retu )  &&  (status_info.RIP_resdis_flag[0] !=1  )  )
	     							{
	     						        fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=ospf value=ospf>OSPF</td>");
	     							}

								else if ( 0 != retu )
									{

	                                    fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=ospf value=ospf  disabled>OSPF</td>");
									}

								#endif


								fprintf(cgiOut,"<td align=left>OSPF Metric: </td>");

	                           

								/////////////////////////////////////////////////////////////////////////////////////////

								

	                            ///////////////////////////////注释之间是原来的代码//////////////////////////
	                            ///////////////////原来不是管理员，也可以选中复选框///////////////////

								
	                            #if 0
								
								if (strcmp(status_info.RIP_redis_metric.RIP_REDIS_OSPF,"")!=0)
									fprintf(cgiOut,"<td align=left><input type=text name=ospf_metric size=6 value=%s></td>",status_info.RIP_redis_metric.RIP_REDIS_OSPF );

								
								else
									fprintf(cgiOut,"<td align=left><input type=text name=ospf_metric size=6 value=1></td>");

								
								fprintf(cgiOut,"</tr>");
								

							   

	     						fprintf(cgiOut,"<tr align=left height=35>");
								fprintf(cgiOut,"<td colspan=2>&nbsp;</td>");

								#endif 



	                            #if 1
								
								if (    (0 == retu ) && (strcmp(status_info.RIP_redis_metric.RIP_REDIS_OSPF,"")!=0) )
									{
									    fprintf(cgiOut,"<td align=left><input type=text name=ospf_metric size=6 value=%s></td>",status_info.RIP_redis_metric.RIP_REDIS_OSPF );

									}

								
								else  if (0 == retu ) 
									{
									    fprintf(cgiOut,"<td align=left><input type=text name=ospf_metric size=6 value=1></td>");
									}

								else if ( 0 != retu )
									{

									   fprintf(cgiOut,"<td align=left><input type=text name=ospf_metric size=6 value=1  disabled></td>");


									}

								
								fprintf(cgiOut,"</tr>");
			
	     						fprintf(cgiOut,"<tr align=left height=35>");
								fprintf(cgiOut,"<td colspan=2>&nbsp;</td>");

								#endif 



								//////////////////////////如果是管理员 ，那么允许选中复选框//////////////////////////////////
	                            #if 1
								 
	     						if(  (  0 == retu  ) &&  (status_info.RIP_resdis_flag[2]==1) )
	     						   {
								 	
	     							  fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=connected value=connected checked>Connected</td>");

	     						   }
								 
	     						 else  if( 0  == retu  ) 
	     							{
	     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=connected value=connected>Connected</td>");
	     							}

								 else   if( 0 != retu )
								 	{
								 	     fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=connected value=connected  disabled>Connected</td>");

								 	}

								#endif



						      //////////////////////////////////////注释之间是原来的代码//////////////////////////////////////////////
	                          /////////////////////////////////原来普通用户也可以在文本框中输入////////////////////////////////
								
							   #if 0

							   fprintf(cgiOut,"<td align=left>Connected Metric: </td>");
								 
							   if(    (  0 == retu )  &&   (strcmp(status_info.RIP_redis_metric.RIP_REDIS_CONNECTED,"")!=0) )
								  {
								    fprintf(cgiOut,"<td align=left><input type=text name=connected_metric size=6 value=%s></td>",status_info.RIP_redis_metric.RIP_REDIS_CONNECTED);
								  }
								
							   else  if ( (  0 == retu )  &&   (strcmp(status_info.RIP_redis_metric.RIP_REDIS_CONNECTED,"")  == 0) )
								  {
								      fprintf(cgiOut,"<td align=left><input type=text name=connected_metric size=6 value=1></td>");
								  }
								
							  else if ( 0 != retu )
							  	{
							  	     fprintf(cgiOut,"<td align=left><input type=text name=connected_metric size=6 value=1  disabled></td>");

							  	}

						      fprintf(cgiOut,"</tr>");
							  #endif


	                        ///////////////////////////////////如果是管理员，允许在文本框输入///////////////////////////////////////

							#if 1

							   fprintf(cgiOut,"<td align=left>Connected Metric: </td>");
								 
							if (   ( 0 == retu ) &&  (strcmp(status_info.RIP_redis_metric.RIP_REDIS_CONNECTED,"")!=0) )
								{
								   fprintf(cgiOut,"<td align=left><input type=text name=connected_metric size=6 value=%s></td>",status_info.RIP_redis_metric.RIP_REDIS_CONNECTED);
								}
								
						    else  if( 0  == retu)
								{
								     fprintf(cgiOut,"<td align=left><input type=text name=connected_metric size=6 value=1></td>");
								}

						    else if( 0 != retu)
								{

									  fprintf(cgiOut,"<td align=left><input type=text name=connected_metric size=6 value=1 disabled></td>");
								}
								 
						    fprintf(cgiOut,"</tr>");


						   #endif

							 						
	     				   fprintf(cgiOut,"<tr align=left height=35>");
						   fprintf(cgiOut,"<td colspan=2>&nbsp;</td>");



						   ////////////////////////////////////////////////////注释部分是原来的代码/////////////////////////////////////////
						   ////////////// ////// /  原来普通用户也可选中复选 框 ，可以在文本框输入//////////////////////////////


						   #if  0
						   
								 
	     				   if (status_info.RIP_resdis_flag[3]==1)
	     					  fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=static value=static checked>Static</td>");
	     				   else
	     						fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=static value=static>Static</td>");
						   fprintf(cgiOut,"<td align=left>Static Metric: </td>");

							   
								 
						   if (strcmp(status_info.RIP_redis_metric.RIP_REDIS_STATIC,"")!=0)
								fprintf(cgiOut,"<td align=left><input type=text name=static_metric size=6 value=%s></td>",status_info.RIP_redis_metric.RIP_REDIS_STATIC);
						   else
								fprintf(cgiOut,"<td align=left><input type=text name=static_metric size=6 value=1></td>");
					       fprintf(cgiOut,"</tr>");

									
	     				   fprintf(cgiOut,"<tr align=left height=35>");
						   fprintf(cgiOut,"<td colspan=2>&nbsp;</td>");
	     
	     				   if (status_info.RIP_resdis_flag[4]==1)
	     					  fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=kernel value=kernel checked>Kernel</td>");
	     				   else
	     					  fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=kernel value=kernel>Kernel</td>");
						   fprintf(cgiOut,"<td align=left>Kernel Metric: </td>");

								 
						   if (strcmp(status_info.RIP_redis_metric.RIP_REDIS_KERNEL,"")!=0)
							  fprintf(cgiOut,"<td align=left><input type=text name=kernel_metric size=6 value=%s></td>",status_info.RIP_redis_metric.RIP_REDIS_KERNEL);
						   else
							  fprintf(cgiOut,"<td align=left><input type=text name=kernel_metric size=6 value=1></td>");
						   fprintf(cgiOut,"</tr>");

							
		                  #endif 



						  #if  1
						   
								 
			     		  if (   ( 0 == retu )   &&  (status_info.RIP_resdis_flag[3]==1) )
			     		  	{
	     				   	 
	     				   	
	     					     fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=static value=static checked>Static</td>");
			     		  	}
	     				     
						   
	     				   else if (  0 == retu )
	     				   	 {
	     						fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=static value=static>Static</td>");
	     				   	 }

	                       else if ( 0 != retu )
	                       	{
	                       	  
	                               fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=static value=static  disabled >Static</td>");
	                       	}


						   fprintf(cgiOut,"<td align=left>Static Metric: </td>");

							 
						   if(  ( 0 == retu )  &&  (strcmp(status_info.RIP_redis_metric.RIP_REDIS_STATIC,"")!=0) )
						   	{
								fprintf(cgiOut,"<td align=left><input type=text name=static_metric size=6 value=%s></td>",status_info.RIP_redis_metric.RIP_REDIS_STATIC);
						   	}
						   else if( 0 == retu)
						   	{
								fprintf(cgiOut,"<td align=left><input type=text name=static_metric size=6 value=1></td>");

							}

						   else if( 0 != retu )
						   	{
						   	   fprintf(cgiOut,"<td align=left><input type=text name=static_metric size=6 value=1  disabled></td>");

						   	}

						   fprintf(cgiOut,"</tr>");

									
	     				   fprintf(cgiOut,"<tr align=left height=35>");
						   fprintf(cgiOut,"<td colspan=2>&nbsp;</td>");
	     
	     				   if(  ( 0 == retu ) &&  (status_info.RIP_resdis_flag[4]==1)  )
	     				   	{
	     					  fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=kernel value=kernel checked>Kernel</td>");
	     				   	}
						   
	     				   else if(  0 == retu)
	     				   	{
	     					  fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=kernel value=kernel>Kernel</td>");
	     				   	}

						   else if( 0 
!= retu)
						   	{
						      fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=kernel value=kernel disabled>Kernel</td>");
  
						   	}
						   fprintf(cgiOut,"<td align=left>Kernel Metric: </td>");

								 
						   if( (0  == retu)  &&  (strcmp(status_info.RIP_redis_metric.RIP_REDIS_KERNEL,"")!=0))
						   	{
							  fprintf(cgiOut,"<td align=left><input type=text name=kernel_metric size=6 value=%s></td>",status_info.RIP_redis_metric.RIP_REDIS_KERNEL);
						   	}
						   
						   else if ( 0 == retu )
						   	{
							  fprintf(cgiOut,"<td align=left><input type=text name=kernel_metric size=6 value=1></td>");
						   	}

						   else if( 0 != retu)
						   	{

							   fprintf(cgiOut,"<td align=left><input type=text name=kernel_metric size=6 value=1  disabled></td>");
						   	}

						   
						   fprintf(cgiOut,"</tr>");

							
	                      #endif 


		
					      fprintf(cgiOut,"<tr align=left height=35>");
						  fprintf(cgiOut,"<td>&nbsp;</td>");
						  fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"global_metric"));


						 ///////////////////////////////注释的是原来的内容//////////////////////////////////////
						 ///////////////////////////////////原来的非管理员也可在文本框输入///////////////

						  #if 0
						  
						  if(strcmp(status_info.RIP_def_metric,"")!=0)
						  	
								fprintf(cgiOut,"<td align=left><input type=text name=global_metric size=6 value=%s></td>", status_info.RIP_def_metric);
						  else
							  fprintf(cgiOut,"<td align=left><input type=text name=global_metric size=6 value=1></td>");


						  #endif 


						  #if 1
						  
						  if (   ( 0  == retu )  &&  (strcmp(status_info.RIP_def_metric,"")!=0) )
						  	{
						  	
								fprintf(cgiOut,"<td align=left><input type=text name=global_metric size=6 value=%s></td>", status_info.RIP_def_metric);
						  	}
						  
						  else  if ( 0 == retu)
						  	{
							  fprintf(cgiOut,"<td align=left><input type=text name=global_metric size=6 value=1></td>");
						  	}

						  else if ( 0 != retu)
						  	{
                               fprintf(cgiOut,"<td align=left><input type=text name=global_metric size=6 value=1  disabled></td>");

						  	}


						  #endif 

						  
						  fprintf(cgiOut,"<td colspan=6 style=color:red align=left>%s</td>",search(lcontrol,"metric_description"));

						  
						  fprintf(cgiOut,"</tr>"\
							///////////////////////////////////////////////////////////////////////end of metric////////////////////////////						
								"<tr align=left height=35>");
						  fprintf(cgiOut,"<td>&nbsp;</td>");
						  fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"distance"));


						  /////////////////////////////////////注释的是原来的内容/////////////////////////////////////
						  /////////////////////////////////////原来的普通用户也可在文本框中输入//////////////

						  #if 0
						  
						  if(strcmp(status_info.RIP_distance,"")!=0)
						  	{
							   fprintf(cgiOut,"<td align=left><input type=text name=distance size=6 value=%s></td>",status_info.RIP_distance);
						  	}
						  else
						  	{
						       fprintf(cgiOut,"<td align=left><input type=text name=distance size=6 value=120></td>");
						  	}

                          #endif



						  #if 1
						  
						  if(  (  0 == retu) &&  (strcmp(status_info.RIP_distance,"")!=0) )
						  	{
							   fprintf(cgiOut,"<td align=left><input type=text name=distance size=6 value=%s></td>",status_info.RIP_distance);
						  	}
						  else if  ( 0  == retu) 
						  	{
						       fprintf(cgiOut,"<td align=left><input type=text name=distance size=6 value=120></td>");
						  	}

						  else if ( 0 != retu)
						  	{

							   fprintf(cgiOut,"<td align=left><input type=text name=distance size=6 value=120  disabled ></td>");
						  	}

                          #endif

						 
						  
						 fprintf(cgiOut,"<td colspan=6 style=color:red align=left>%s</td>",search(lcontrol,"distance_description"));
					     fprintf(cgiOut,"</tr>"\
								"</table>"\
							  "</td>"\
							"</tr>"\


							"<tr>"\
								"<td id=sec1 colspan=3 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"Advanced_settings"));
								fprintf(cgiOut,"</tr>"\
							"<tr align=left style=padding-top:8px>"\
							"<td colspan=3 align=left>"\
							"<table width=420 border=0 cellspacing=0 cellpadding=0>"\
							"<tr>");


								
						fprintf(stderr,"distribute_in=%s-acl_in=%s",status_info.RIP_distribute_in,status_info.RIP_distribute_in_acl);


						//////////////////////////////注释的部分是原来的代码///////////////////////////////////////////////////////
						//////////////////////////////原来普通用户也可以选中复选框，并在文本框中输入/////////////////

						#if 0
						
						if( strcmp(status_info.RIP_distribute_in,"in") == 0 )
					     {
						  
							fprintf(cgiOut,"<td><input type=checkbox checked name=distribute_in>%s: </td>",search(lcontrol,"distribute_in"));
					     }
						else
						 {
							fprintf(cgiOut,"<td><input type=checkbox name=distribute_in>%s: </td>",search(lcontrol,"distribute_in"));
						 }
						
						fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"access_index"));

						
						if( strcmp(status_info.RIP_distribute_in_acl,"") != 0 )
							{
							fprintf(cgiOut,"<td align=left><input type=text name=acl_index_in size=6 value=%s></td>",status_info.RIP_distribute_in_acl);
							}
						else
							{
							fprintf(cgiOut,"<td align=left><input type=text name=acl_index_in size=6 ></td>");
							}


						#endif


						#if 1
						
						if( ( 0 == retu)  &&  (strcmp(status_info.RIP_distribute_in,"in") == 0 ) )
					     {
						  
							fprintf(cgiOut,"<td><input type=checkbox checked name=distribute_in>%s: </td>",search(lcontrol,"distribute_in"));
					     }
						else if( 0 == retu)
						 {
							fprintf(cgiOut,"<td><input type=checkbox name=distribute_in>%s: </td>",search(lcontrol,"distribute_in"));
						 }

						else if( 0 != retu )
						 {
						     fprintf(cgiOut,"<td><input type=checkbox name=distribute_in  disabled >%s: </td>",search(lcontrol,"distribute_in"));

						 }
						
						fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"access_index"));

						
						if(  ( 0 == retu)  && (strcmp(status_info.RIP_distribute_in_acl,"") != 0 ) )
						  {
							fprintf(cgiOut,"<td align=left><input type=text name=acl_index_in size=6 value=%s></td>",status_info.RIP_distribute_in_acl);
						  }
						
						else if ( 0 == retu)
						  {
							fprintf(cgiOut,"<td align=left><input type=text name=acl_index_in size=6 ></td>");
						  }

						else if ( 0 != retu )
						  {
						     fprintf(cgiOut,"<td align=left><input type=text name=acl_index_in size=6  disabled></td>");


						  }


						#endif
						
				
						fprintf(cgiOut,"</tr>"\
							
						"<tr>");
						fprintf(stderr,"distribute_out=%s-acl_out=%s",status_info.RIP_distribute_out,status_info.RIP_distribute_out_acl);

						//////////////////////////////////注释的部分是原来的代码//////////////////////////////////////////////
                        ////////////////////////////原来普通用户也可以选中复选框，并在文本框输入/////////////////


                       #if 0 

                       if(   strcmp(status_info.RIP_distribute_out,"out")  == 0  )	
							{
							   fprintf(cgiOut,"<td><input type=checkbox checked name=distribute_out>%s: </td>",search(lcontrol,"distribute_out"));
							}
						else ( 0 == retu)
							{
							  fprintf(cgiOut,"<td><input type=checkbox name=distribute_out>%s: </td>",search(lcontrol,"distribute_out"));
							}

						
						fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"access_index"));
						
												
				        if( strcmp(status_info.RIP_distribute_out_acl,"") != 0 )
				          {
							  fprintf(cgiOut,"<td align=left><input type=text name=acl_index_out size=6 value=%s></td>",status_info.RIP_distribute_out_acl);
				          }
						else
						  {
							  fprintf(cgiOut,"<td align=left><input type=text name=acl_index_out size=6 ></td>");
						  }
						
					   #endif
                       /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						

					   #if 1
					   if( ( 0 == retu )   &&  (  strcmp(status_info.RIP_distribute_out,"out") ) == 0 )	
						 {
							 fprintf(cgiOut,"<td><input type=checkbox checked name=distribute_out>%s: </td>",search(lcontrol,"distribute_out"));
						 }
					   else if ( 0 == retu)
						 {
							 fprintf(cgiOut,"<td><input type=checkbox name=distribute_out>%s: </td>",search(lcontrol,"distribute_out"));
						 }
					   else if ( 0 != retu)
						{
							 fprintf(cgiOut,"<td><input type=checkbox name=distribute_out  disabled >%s: </td>",search(lcontrol,"distribute_out"));
							
						}

						
				       fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"access_index"));

						
				       if( ( 0 == retu ) &&  (strcmp(status_info.RIP_distribute_out_acl,"") != 0 ))
				       	{
							fprintf(cgiOut,"<td align=left><input type=text name=acl_index_out size=6 value=%s></td>",status_info.RIP_distribute_out_acl);
				       	}
					   
				       else  if ( 0 == retu)
				       	{
 						  fprintf(cgiOut,"<td align=left><input type=text name=acl_index_out size=6 ></td>");
				       	}

					   else if  ( 0 != retu)
					   	{

						  fprintf(cgiOut,"<td align=left><input type=text name=acl_index_out size=6 disabled ></td>");

					   	}

					  #endif 	
							
					fprintf(cgiOut,"</tr>"\
					"</table></td></tr>"\
							
					"<tr align=left style=padding-top:8px>"\
					"<td colspan=3 align=left>"\
					"<table height=125 width=420 border=0 cellspacing=0 cellpadding=0>"\
					"<tr>"\
					"<td colspan=3 width=300>&nbsp;</td>"\
					"<td align=center style=font-size:14px;font-weight:bold valign=bottom>%s</td>",search(lcontrol,"neighbour_list"));
					fprintf(cgiOut,"</tr>"\
					"<tr>");
					fprintf(cgiOut,"<td align=left width=60>%s: </td>",search(lcontrol,"neighbor_ip"));
					fprintf(cgiOut,"<td align=left  style=padding-left:10px>"\
					"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");


					////////////////////
					#if 1

					if( 0 == retu)
					 {
					    fprintf(cgiOut,"<input type=text name=target_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					    fprintf(cgiOut,"<input type=text name=target_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					    fprintf(cgiOut,"<input type=text name=target_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					    fprintf(cgiOut,"<input type=text name=target_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));

					 }

					else if( 0 != retu)
					 {
					   
				       fprintf(cgiOut,"<input type=text name=target_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
				       fprintf(cgiOut,"<input type=text name=target_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
				       fprintf(cgiOut,"<input type=text name=target_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					   fprintf(cgiOut,"<input type=text name=target_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
					 }

				
                    #endif
					////////////////////////


				   #if 0
					 
				   fprintf(cgiOut,"<input type=text name=target_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
				   fprintf(cgiOut,"<input type=text name=target_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
				   fprintf(cgiOut,"<input type=text name=target_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
				   fprintf(cgiOut,"<input type=text name=target_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));

				   #endif





				   /////////////////////////////////////////////////////////////////////////////////
				   #if 0
					fprintf(cgiOut,"</div>");
					fprintf(cgiOut,"</td>"\
					"<td align=right style=padding-left:5px><input type=submit style=width:70px name=addneighbour value=\"%s\"></td>",search(lcontrol,"add"));

				   #endif 
				   ////////////////////////////////////////////////////////////////////////////////

				   #if 1 

				   if (  0 == retu)
				     {
				       fprintf(cgiOut,"</div>");
				       fprintf(cgiOut,"</td>"\
				       "<td align=right style=padding-left:5px><input type=submit style=width:70px name=addneighbour value=\"%s\"></td>",search(lcontrol,"add"));
				     }

				   else if ( 0 != retu)
				     {

						fprintf(cgiOut,"</div>");
				       fprintf(cgiOut,"</td>"\
				       "<td align=right style=padding-left:5px><input type=submit style=width:70px name=addneighbour value=\"%s\"  disabled></td>",search(lcontrol,"add"));
				     }

				   #endif

					
				  fprintf(cgiOut,"<td align=left rowspan=2 style=padding-left:5px>"\

					
				  "<select style=\"width:140\" style=\"height:100;background-color:#ebfff9\" name=\"neighbourRouter\" multiple>\n");
				  memset(neighborlist,0,500);

				  for(i=0;i<NEI_NUM;i++)
				   {
					  if(strcmp(status_info.RIP_neigh[i],"")!=0)
					    {
										
						   fprintf(cgiOut,"<option value=\"%s\">%s\n",status_info.RIP_neigh[i],status_info.RIP_neigh[i]);
						   netghnorNum++;
						   strcat(neighborlist,status_info.RIP_neigh[i]);
							strcat(neighborlist,"-");
						}
				   }

				  #if 0

				  fprintf(cgiOut,"</select>"\
						
				  "</td>"\
				  "</tr>"\
				  "<tr>"\
				  "<td width=200 colspan=2>&nbsp;</td>"\
				  "<td align=right><input type=submit  style=width:70px name=delneighbour value=\"%s\"></td>",search(lcontrol,"delete"));
				  fprintf(cgiOut,"</tr>"\
				  "</table>"\
				  "</td>"\
				  "</tr>");

				  #endif 


				  #if 0

				  fprintf(cgiOut,"</select>"\
						
				  "</td>"\
				  "</tr>"\
				  "<tr>"\
				  "<td width=200 colspan=2>&nbsp;</td>"\
				  "<td align=right><input type=submit  style=width:70px name=delneighbour value=\"%s\"></td>",search(lcontrol,"delete"));
				  fprintf(cgiOut,"</tr>"\
				  "</table>"\
				  "</td>"\
				  "</tr>");


				  #endif 

				  #if 1

				  if(0 == retu)
				    { 

				       fprintf(cgiOut,"</select>"\
						
				       "</td>"\
				       "</tr>"\
				       "<tr>"\
				       "<td width=200 colspan=2>&nbsp;</td>"\
				       "<td align=right><input type=submit  style=width:70px name=delneighbour value=\"%s\"></td>",search(lcontrol,"delete"));
				  
				       fprintf(cgiOut,"</tr>"\
				       "</table>"\
				       "</td>"\
				       "</tr>");

				  }

				  else if( 0 != retu)
				   {
				   	   fprintf(cgiOut,"</select>"\
						
				       "</td>"\
				       "</tr>"\
				       "<tr>"\
				       "<td width=200 colspan=2>&nbsp;</td>"\
				       "<td align=right><input type=submit  style=width:70px name=delneighbour value=\"%s\"  disabled></td>",search(lcontrol,"delete"));
				  
				       fprintf(cgiOut,"</tr>"\
				       "</table>"\
				       "</td>"\
				       "</tr>");
				   }


				 #endif 
							
							
							
	        	fprintf(cgiOut,"<tr>");
							 

						
	        	if((cgiFormSubmitClicked("submit_riproute") != cgiFormSuccess) && (cgiFormSubmitClicked("addneighbour") != cgiFormSuccess) && (cgiFormSubmitClicked("delneighbour") != cgiFormSuccess))
				  {
				     fprintf(cgiOut,"<td><input type=hidden name=encry_riproute value=%s></td>",encry);
					 fprintf(cgiOut,"<td><input type=hidden name=Version value=%s></td>",status_info.RIP_version);
					 fprintf(cgiOut,"<td><input type=hidden name=NeiList value=%s></td>",neighborlist);
					 fprintf(cgiOut,"<td><input type=hidden name=PRIUsr value=%d></td>",retu);
												
				  }
				else
				  {
					 fprintf(cgiOut,"<td><input type=hidden name=encry_riproute value=%s></td>",encry_riproute);
					 fprintf(cgiOut,"<td><input type=hidden name=Version value=%s></td>",status_info.RIP_version);
					 fprintf(cgiOut,"<td><input type=hidden name=NeiList value=%s></td>",neighborlist);
					 fprintf(cgiOut,"<td><input type=hidden name=PRIUsr value=%d></td>",retu);
							   
				  }

	        	  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"</table>"\

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
	              free(version);
	              free(default_metric);
	              free(default_route);
	              free(metric);
	              free(distance);
	              free(Vsion);
	              free(redis_param);
	              free(neighborlist);
	              free(neighborlistLater);


	              for(i=0;i<MAX_NEIGHBOR_NUM;i++)
	               {
		               free(status_info.RIP_neigh[i]);
	               }


	              for(i=0;i<3;i++)
	               {
		              free(timer[i]);
	               }
	              for(i=0;i<NEI_NUM;i++)
	               {
		               free(neighbor[i]);
	               }

	              for(i=0;i<6;i++)
	                {
		                free(redistribute_metric[i]);
	                }

	              for(i=0;i<Info_Num;i++)
	               {
		              free(RipItem[i]);	
	               }

	              release(lpublic);  
	              release(lcontrol);


	              return 0;
   }  // modified at 19:54  12/6/2010



	

#ifndef WP_RIPROUTE_PAGE_MACRO
#define WP_RIPROUTE_PAGE_MACRO
#define MAX_INFOR_LENTH 128
#define SED_RIP_STATUS_COMMAND "show_rip_status.sh | sed 's/:/*/g' | sed 's/ /#/g'"
#define PATH_INFOR "/var/run/apache2/RipInfo.txt"

#endif
																 
	int ReadConfig(char * ripInfo[],RIP_status * p_ripinfo,struct list * lpublic)
	{
		int i,j,q = 0;
		char * command=(char *)malloc(200);
		memset(command,0,200);
		sprintf(command,"%s  > %s ",SED_RIP_STATUS_COMMAND,PATH_INFOR);
		int status = system(command);
		int ret = WEXITSTATUS(status);				 
		if(0 != ret)
			ShowAlert("shell occur an error!");

		FILE *fd;
		char  temp[MAX_INFOR_LENTH];
		memset(temp,0,MAX_INFOR_LENTH);
		char * revRouteInfo[10];
		if((fd=fopen(PATH_INFOR,"r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			goto END_FREE;
		}
		i=0;
		q=0;
		p_ripinfo->RIP_enable = 1;
		while((fgets(temp,MAX_INFOR_LENTH,fd)) != NULL)
		{
			strcat(ripInfo[i],temp);
			//fprintf(stderr,"3333temp=%s--ripInfo[%d]=%s33333\n",temp,i,ripInfo[i]);
			
			
			revRouteInfo[0]=strtok(ripInfo[i],"*");
			fprintf(stderr,"revRouteInfo[0]=%s\n",revRouteInfo[0]);
			if(strcmp(revRouteInfo[0],"##Receive#version#")==0)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<1)
					{
						revRouteInfo[j+1]=strtok(NULL,"*");
						j++;
					}
				strncpy(p_ripinfo->RIP_version,revRouteInfo[1], strlen(revRouteInfo[1])-1 );
				fprintf(stderr,"p_ripinfo->RIP_version=%s\n",p_ripinfo->RIP_version);
			}
			if(strcmp(revRouteInfo[0],"RIP#Routing#Process#")==0)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<1)
					{
						revRouteInfo[j+1]=strtok(NULL,"#*");
						j++;
					}

				if (strstr(revRouteInfo[1],"enabled") != NULL)
						p_ripinfo->RIP_enable = 0;
				else
						p_ripinfo->RIP_enable = 1;
				fprintf(stderr,"p_ripinfo->RIP_enable=%d\n",p_ripinfo->RIP_enable );
			}
			if(strcmp(revRouteInfo[0],"##Route#update#time#")==0)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<1)
					{
						revRouteInfo[j+1]=strtok(NULL,"#*s");
						j++;
					}
				strncpy(p_ripinfo->RIP_timer.RIP_update_time,revRouteInfo[1], strlen(revRouteInfo[1]) );
				
			}
			if(strcmp(revRouteInfo[0],"##Timeout#time#")==0)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<1)
					{
						revRouteInfo[j+1]=strtok(NULL,"#*s");
						j++;
					}
				strncpy(p_ripinfo->RIP_timer.RIP_timeout_time,revRouteInfo[1], strlen(revRouteInfo[1]) );
				
			}
			if(strcmp(revRouteInfo[0],"##Garbage#collect#time#")==0)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<1)
					{
						revRouteInfo[j+1]=strtok(NULL,"#*s");
						j++;
					}
				strncpy(p_ripinfo->RIP_timer.RIP_garbage_time,revRouteInfo[1], strlen(revRouteInfo[1]) );
				
			}
			if(strstr(revRouteInfo[0],"##Default#redistribution#metric#") != NULL)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<1)
					{
						revRouteInfo[j+1]=strtok(NULL,"#*");
						j++;
					}
				strncpy(p_ripinfo->RIP_def_metric,revRouteInfo[1], strlen(revRouteInfo[1])-1 );
				//fprintf(stderr,"p_ripinfo->RIP_def_metric=%s",p_ripinfo->RIP_def_metric);
			}
			if(strcmp(revRouteInfo[0],"##Redistribute#type#")==0)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<1)  //j 值
				{                    									
					//strcat(revRouteInfo[j+1],strtok(NULL,"- "));
					revRouteInfo[j+1]=strtok(NULL,"#*");
					j++;
				}
				if( revRouteInfo[1] != NULL )
				{
					revRouteInfo[2]=strtok(NULL,"#*");
				}
				//fprintf(stderr,"revRouteInfo[2]=%s",revRouteInfo[2]);
				if( revRouteInfo[2] != NULL )
				{
					if( strcmp(revRouteInfo[2],"Metric") == 0 )
					{
						revRouteInfo[3]=strtok(NULL,"#*");
					}
				}
				//fprintf(stderr,"revRouteInfo[1]=%s",revRouteInfo[1]);
				if (strstr(revRouteInfo[1],"ospf"))
				{
					p_ripinfo->RIP_resdis_flag[0] = 1;
					if( revRouteInfo[2] != NULL )
						strcpy(p_ripinfo->RIP_redis_metric.RIP_REDIS_OSPF,revRouteInfo[3]);
				}
				if (strstr(revRouteInfo[1],"bgp"))
				{
					p_ripinfo->RIP_resdis_flag[1] = 1;
					if( revRouteInfo[2] != NULL )
						strcpy(p_ripinfo->RIP_redis_metric.RIP_REDIS_BGP,revRouteInfo[3]);
				}
				if (strstr(revRouteInfo[1],"connected"))
				{
					//fprintf(stderr,"22222222222222");
					p_ripinfo->RIP_resdis_flag[2] = 1;
					if( revRouteInfo[2] != NULL )
						strcpy(p_ripinfo->RIP_redis_metric.RIP_REDIS_CONNECTED,revRouteInfo[3]);
				}
				if (strstr(revRouteInfo[1],"static"))
				{
					//fprintf(stderr,"333333333333");
					p_ripinfo->RIP_resdis_flag[3] = 1;
					if( revRouteInfo[2] != NULL )
						strcpy(p_ripinfo->RIP_redis_metric.RIP_REDIS_STATIC,revRouteInfo[3]);
				}
				if (strstr(revRouteInfo[1],"kernel"))
				{
					p_ripinfo->RIP_resdis_flag[4] = 1;
					if( revRouteInfo[2] != NULL )
						strcpy(p_ripinfo->RIP_redis_metric.RIP_REDIS_KERNEL,revRouteInfo[3]);
				}
				if (strstr(revRouteInfo[1],"isis"))
				{
					p_ripinfo->RIP_resdis_flag[5] = 1;
					if( revRouteInfo[2] != NULL )
						strcpy(p_ripinfo->RIP_redis_metric.RIP_REDIS_ISIS,revRouteInfo[3]);
				}

			}
			if(strstr(revRouteInfo[0],"##Incoming#update#filter#list#for#all#interface#") != NULL)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<2) //j 值
				{                    									
					//strcat(revRouteInfo[j+1],strtok(NULL,"-,"));
					revRouteInfo[j+1]=strtok(NULL,"#*");
					j++;
				}
				
				fprintf(stderr,"revRouteInfo_in[1]=%s",revRouteInfo[1]);
				if ( !strcmp(revRouteInfo[1],"set") )
				{
					strcpy(p_ripinfo->RIP_distribute_in,"in");
					if( strcmp(revRouteInfo[2],"Filter") == 0)
					{
						revRouteInfo[3]=strtok(NULL,"#*");
						revRouteInfo[4]=strtok(NULL,"#*");
					}
					strcpy(p_ripinfo->RIP_distribute_in_acl,revRouteInfo[4]);
				}
				fprintf(stderr,"RIP_distribute_in=%s--in_acl=%s",p_ripinfo->RIP_distribute_in,p_ripinfo->RIP_distribute_in_acl);
			}
			if(strstr(revRouteInfo[0],"##Outgoing#update#filter#list#for#all#interface#") != NULL)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<2) //j 值
				{                    									
					//strcat(revRouteInfo[j+1],strtok(NULL,"-,"));
					revRouteInfo[j+1]=strtok(NULL,"#*");
					j++;
				}
				fprintf(stderr,"revRouteInfo_out[1]=%s",revRouteInfo[1]);
				if ( !strcmp(revRouteInfo[1],"set") )
				{
					strcpy(p_ripinfo->RIP_distribute_out,"out");
					if( strcmp(revRouteInfo[2],"Filter") == 0)
					{
						revRouteInfo[3]=strtok(NULL,"#*");
						revRouteInfo[4]=strtok(NULL,"#*");
					}
					fprintf(stderr,"revRouteInfo_in[1]=%s",revRouteInfo[1]);
					strcpy(p_ripinfo->RIP_distribute_out_acl,revRouteInfo[4]);
				}		
				fprintf(stderr,"RIP_distribute_out=%s--out_acl=%s",p_ripinfo->RIP_distribute_out,p_ripinfo->RIP_distribute_out_acl);
			}
			if(strcmp(revRouteInfo[0],"##Neighbor#")==0)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<1)
					{                    									
						//strcat(revRouteInfo[j+1],strtok(NULL,"-,"));
						revRouteInfo[j+1]=strtok(NULL,"#*");
						j++;
					}
				fprintf(stderr,"revRouteInfo_in[1]=%s",revRouteInfo[1]);
				strncpy( p_ripinfo->RIP_neigh[q], revRouteInfo[1], strlen(revRouteInfo[1])-1 );
				q++;
			}
			if(strcmp(revRouteInfo[0],"##Default-information#")==0)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<1)
					{
						revRouteInfo[j+1]=strtok(NULL,"#*");
						j++;
					}
				fprintf(stderr,"revRouteInfo_in[1]=%s",revRouteInfo[1]);
				strncpy(p_ripinfo->RIP_default_infor,revRouteInfo[1],strlen(revRouteInfo[1])-1 );
			}
			if(strcmp(revRouteInfo[0],"##Default#distance#")==0)
			{
				j=0;
				while(revRouteInfo[j]!=NULL && j<1)
					{                    									
						revRouteInfo[j+1]=strtok(NULL,"#*");
						j++;
					}
				fprintf(stderr,"revRouteInfo_in[1]=%s",revRouteInfo[1]);
				strncpy(p_ripinfo->RIP_distance,revRouteInfo[1], strlen(revRouteInfo[1])-1 );
			}
		i++;
		memset(temp,0,MAX_INFOR_LENTH);
		}
		fclose(fd);

		
	END_FREE:	
		free(command);
		return 1;
	}

	int RIPconfig(int checkRip,char * checkversion,char * redis)
	{
		int i=0;
		char * RIPsys=(char * )malloc(250);
		memset(RIPsys,0,250);
		int ret,status=0;
		char * RIPInfo[3];
		char * Ripdef=(char *)malloc(10);
		int enableflag=0; /*0  is opened ,1 is closed*/

		for(i=0;i<3;i++)
		{
			RIPInfo[i]=(char *)malloc(20);
			memset(RIPInfo[i],0,20);
		}
		if(checkRip==1)
		{
			strcat(RIPInfo[0],"closeRip");
			strcat(RIPInfo[1],"openRip");
		}
		else if(checkRip==0)
		{
			strcat(RIPInfo[0],"openRip");
			strcat(RIPInfo[1],"closeRip");

		}
		
		int Choice=0;
		cgiFormSelectSingle("Rip_Enable", RIPInfo, 2, &Choice, 0);
		if(strcmp(RIPInfo[Choice],"closeRip")==0)
		{
			enableflag=1;
			strcat(RIPsys,"router_rip.sh off >/dev/null");
			status = system(RIPsys);
		}
		else if(strcmp(RIPInfo[Choice],"openRip")==0)
		{
			strcat(RIPsys,"router_rip.sh on >/dev/null");
				status = system(RIPsys);
		}
		ret = WEXITSTATUS(status);

		if(ret==0)
		{}
		else
			ShowAlert("enbale_bash_fail");

		if(enableflag==1)
			return 1;
		
	/////////////////////////////////////////////////////////////////////////////
		for(i=0;i<2;i++)
		{
			memset(RIPInfo[i],0,20);
		}
		memset(RIPsys,0,250);
		if(strcmp(checkversion,"1"))
		{
			strcat(RIPInfo[0],"1");
			strcat(RIPInfo[1],"2");
			strcat(RIPInfo[2],"v2broadcast");
		}
		else if(strcmp(checkversion,"2"))
		{
			strcat(RIPInfo[0],"2");
			strcat(RIPInfo[1],"1");
			strcat(RIPInfo[2],"v2broadcast");
		}
		else
		{
			strcat(RIPInfo[0],"v2broadcast");
			strcat(RIPInfo[1],"2");
			strcat(RIPInfo[2],"1");
		}
		cgiFormSelectSingle("Version", RIPInfo, 3, &Choice, 0);
		fprintf(stderr,"Choice=%s",RIPInfo[Choice]);
		sprintf(RIPsys,"rip_version.sh on %s >/dev/null",RIPInfo[Choice]);
		if(enableflag==0)
		{
	     	status = system(RIPsys);
	     	ret = WEXITSTATUS(status);
	     
	     	if(ret==0)
	     	{}
	     	else
	     		ShowAlert("version_bash_fail");
		}
	//////////////////////////////////////////////////////////////////////////////
		memset(RIPsys,0,250);
		int valid_ret = 0;
		char * time1;
		char * time2;
		char * time3;
		time1=(char *)malloc(10);
		time2=(char *)malloc(10);
		time3=(char *)malloc(10);
		memset(time1,0,10);
		memset(time2,0,10);
		memset(time3,0,10);
		cgiFormStringNoNewlines("route_update_time",time1,10);
		cgiFormStringNoNewlines("route_Invalid_time",time2,10);
		cgiFormStringNoNewlines("route_Holddown_time",time3,10);
		valid_ret = check_str_par_valid(time1,5,2147483647);
		//fprintf(stderr,"valid_ret1=%d",valid_ret);
		if( valid_ret == -1 )
			return -1;

		valid_ret = check_str_par_valid(time2,5,2147483647);
		//fprintf(stderr,"valid_ret2=%d",valid_ret);
		if( valid_ret == -1 )
			return -1;
		valid_ret = check_str_par_valid(time3,5,2147483647);
		//fprintf(stderr,"valid_ret3=%d",valid_ret);
		if( valid_ret == -1 )
			return -1;
		
		if(0==strcmp(time1,"") && 0==strcmp(time2,"") && 0==strcmp(time3,""))
		{}
		else if(strcmp(time1,"")!=0 && strcmp(time2,"")!=0 && strcmp(time3,"")!=0)
		{
	      	sprintf(RIPsys,"rip_timer_base.sh on %s %s %s >/dev/null",time1,time2,time3);
	      	system(RIPsys);

		}
		else if(strcmp(time1,"")==0 && strcmp(time2,"")!=0 && strcmp(time3,"")!=0)
		{
			sprintf(RIPsys,"rip_timer_base.sh on %s %s %s >/dev/null","30",time2,time3);
	      	system(RIPsys);
		}
		else if(strcmp(time1,"")==0 && strcmp(time2,"")==0 && strcmp(time3,"")!=0)
		{
			sprintf(RIPsys,"rip_timer_base.sh on %s %s %s >/dev/null","30","180",time3);
	      	system(RIPsys);
		}
		else if(strcmp(time1,"")!=0 && strcmp(time2,"")==0 && strcmp(time3,"")!=0)
		{
			sprintf(RIPsys,"rip_timer_base.sh on %s %s %s >/dev/null",time1,"180",time3);
	      	system(RIPsys);
		}
		else if(strcmp(time1,"")!=0 && strcmp(time2,"")==0 && strcmp(time3,"")==0)
		{
			sprintf(RIPsys,"rip_timer_base.sh on %s %s %s >/dev/null",time1,"180","180");
	      	system(RIPsys);
		}
		else if(strcmp(time1,"")!=0 && strcmp(time2,"")!=0 && strcmp(time3,"")==0)
		{
			sprintf(RIPsys,"rip_timer_base.sh on %s %s %s >/dev/null",time1,time2,"180");
	      	system(RIPsys);
		}
		else if(strcmp(time1,"")==0 && strcmp(time2,"")!=0 && strcmp(time3,"")==0)
		{
			sprintf(RIPsys,"rip_timer_base.sh on %s %s %s >/dev/null","30",time2,"180");
	      	system(RIPsys);
		}	
	///////////////////////////////////////////////////////////////////////////////////

		memset(RIPsys,0,250);
		int result;
		char **responses;
			result = cgiFormStringMultiple("default_route", &responses);
		if (result == cgiFormNotFound) 
		{
			sprintf(RIPsys,"default_route.sh off >/dev/null");
	      	status = system(RIPsys);
	      	ret = WEXITSTATUS(status);
	      	if(ret==0)
	      	{}
	      	else
	      		ShowAlert("defaultRoute_bash_fail");
		}
		else
		{
	      	sprintf(RIPsys,"default_route.sh on >/dev/null");
	      	status = system(RIPsys);
	      	ret = WEXITSTATUS(status);
	      
	      	if(ret==0)
	      	{}
	      	else
	      		ShowAlert("defaultRoute_bash_fail");
		}

			cgiStringArrayFree(responses);
	////////////////////////////////////////////////////////////////////////////////
		i=0;
		int flag=0;
		result=0;
		char  tempName[20];
		
		char * distrubute_metric[6];
		for( i = 0 ;i < 6 ; i++ )
		{
			distrubute_metric[i] = (char *)malloc(10);
			memset( distrubute_metric[i], 0, 10 );
		}
		
	    cgiFormStringNoNewlines("ospf_metric",distrubute_metric[0],10);
		cgiFormStringNoNewlines("bgp_metric",distrubute_metric[1],10);
		cgiFormStringNoNewlines("connected_metric",distrubute_metric[2],10);
		cgiFormStringNoNewlines("static_metric",distrubute_metric[3],10);
		cgiFormStringNoNewlines("kernel_metric",distrubute_metric[4],10);
		cgiFormStringNoNewlines("isis_metric",distrubute_metric[5],10);
		
		for(i=0;i<6;i++)
		{
			flag=0,result=0;
			memset(RIPsys,0,250);
	    	char **responses1;
			memset(tempName,0,20);
			
	    	if(i==0)
	    		strcat(tempName,"ospf");
	    	else if(i==1)
	    		strcat(tempName,"bgp");
	    	else if(i==2)
	    		strcat(tempName,"connected");
	    	else if(i==3)
	    		strcat(tempName,"static");
	    	else if(i==4)
	    		strcat(tempName,"kernel");
	    	else if(i==5)
	    		strcat(tempName,"isis");
	    	
	     	result = cgiFormStringMultiple(tempName, &responses1);
			

			if ( result == cgiFormNotFound ) 
			{
				sprintf(RIPsys,"rip_redistribute.sh off %s >/dev/null",tempName);
	            status = system(RIPsys);
	         	ret = WEXITSTATUS(status);
			}
			else
			{

				sprintf(RIPsys,"rip_redistribute.sh on %s metric %s >/dev/null",tempName,distrubute_metric[i]);
				status = system(RIPsys);
				ret = WEXITSTATUS(status);
					
			}
				
	        	//cgiStringArrayFree(responses1);
		
		}
		
		memset(Ripdef,0,10);
		cgiFormStringNoNewlines("global_metric",Ripdef,10);
		memset(RIPsys,0,250);
		sprintf(RIPsys,"default_metric.sh on %s >/dev/null",Ripdef);
		system(RIPsys);
			
	//////////////////////////////////////////////////////////////////////////////

		memset(RIPsys,0,250);
		memset(Ripdef,0,10);
		cgiFormStringNoNewlines("distance",Ripdef,10);
		//fprintf(stderr,"Ripdef=%s",Ripdef);
		if(0==strcmp(Ripdef,"120") && 0==strcmp(Ripdef,""))
		{}
		else
		{
	      	sprintf(RIPsys,"distance.sh on %s >/dev/null",Ripdef);
	      	status = system(RIPsys);
	      	ret = WEXITSTATUS(status);
	      
	      	if(ret==0)
	      	{}
	      	else
	      		ShowAlert("distance_bash_fail");
		}
		////////////////////////////////////////////////////////////////////////
		char acl_index_in[10],acl_index_out[10],intf_name_in[20],intf_name_out[20];
		memset(RIPsys,0,250);
		memset(Ripdef,0,10);
		char **responses1;
		result = cgiFormStringMultiple("distribute_in", &responses1);
		char **responses2;
		result = cgiFormStringMultiple("distribute_out", &responses2);
		cgiFormStringNoNewlines("acl_index_in",acl_index_in,10);
		cgiFormStringNoNewlines("acl_index_out",acl_index_out,10);
		cgiFormStringNoNewlines("intf_name_in",intf_name_in,20);
		cgiFormStringNoNewlines("intf_name_out",intf_name_out,20);
		if( strcmp( acl_index_in, "" ) != 0 )
		{
			if(responses1[0])
			{
				sprintf( RIPsys, "rip_distribute_list.sh on %s in %s >/dev/null ", acl_index_in, intf_name_in);
			}
			else
			{
				sprintf( RIPsys, "rip_distribute_list.sh off %s in %s >/dev/null ", acl_index_in, intf_name_in);
			}
			system( RIPsys );
			status = system(RIPsys);
	      	ret = WEXITSTATUS(status);
	      	if(ret != 0)
	      		ShowAlert("distribute_in_bash_fail");
		}
		if( strcmp( acl_index_out, "" ) != 0 )
		{
			if(responses2[0])
			{
				sprintf( RIPsys, "rip_distribute_list.sh on %s out %s >/dev/null ", acl_index_out, intf_name_out);
			}
			else
			{
				sprintf( RIPsys, "rip_distribute_list.sh off %s out %s >/dev/null ", acl_index_out, intf_name_out);
			}
			system( RIPsys );
			status = system(RIPsys);
	      	ret = WEXITSTATUS(status);
	      
	      	if(ret != 0)
	      		ShowAlert("distribute_out_bash_fail");
		}
		

		////////////////////////////////////////////////////////////////////////
	//error_end:
		for( i = 0 ;i < 6 ; i++ )
		{
			free( distrubute_metric[i] );
		}
		free(RIPsys);
		free(Ripdef);
		free(time1);
		free(time2);
		free(time3);
		free(RIPInfo[0]);
		free(RIPInfo[1]);
		free(RIPInfo[2]);

		//ShowAlert("Operation Success");
		return 0;
	}

	int OPTNeighbor(char * addORdel,struct list *lpublic,struct list *lcontrol, char * NeiList)
	{
		int i=0;
		char target_ip1[4],target_ip2[4],target_ip3[4],target_ip4[4];
		char target_IP[20];
		memset(target_IP,0,20);
		char * command=(char *)malloc(250);
		memset(command,0,250);
		char * faver[NEI_NUM];
		int flavorChoices[NEI_NUM];
		for(i=0;i<NEI_NUM;i++)
		{
			flavorChoices[i]=0;
		}
		
		 /*还得抓多选框*/
		 int num=1;
		 int result,invalid;
		faver[0]=strtok(NeiList,"-");
		if(faver[0]==NULL)
		{
			num=0;
		}
		i=0;
		num=0;
		while(faver[i]!=NULL && i<199)
		 {
		 	 faver[i+1]=strtok(NULL,"-");
		 	 i++;
		 	 num++;
		 }
		if(strcmp(addORdel,"add")==0)
		{
	      	memset(target_ip1,0,4);
	      	cgiFormStringNoNewlines("target_ip1",target_ip1,4);
	      	strcat(target_IP,target_ip1);
	      	strcat(target_IP,"."); 	 
	      	memset(target_ip2,0,4);
	      	cgiFormStringNoNewlines("target_ip2",target_ip2,4);
	      	strcat(target_IP,target_ip2);
	      	strcat(target_IP,"."); 	 
	      	memset(target_ip3,0,4);
	      	cgiFormStringNoNewlines("target_ip3",target_ip3,4);
	      	strcat(target_IP,target_ip3);
	      	strcat(target_IP,".");
	      	memset(target_ip4,0,4);
	      	cgiFormStringNoNewlines("target_ip4",target_ip4,4);
	      	strcat(target_IP,target_ip4);
	      	
			if( !(strcmp(target_ip1,"")&&strcmp(target_ip2,"")&&strcmp(target_ip3,"")&&strcmp(target_ip4,"")) )
	    	 	{
	    			ShowAlert(search(lcontrol,"ip_null"));
	    			return 0;
	    	 	}
	    	 	
			sprintf(command,"rip_neighbor.sh on %s >/dev/null",target_IP);
			system(command);
		}
		else if(strcmp(addORdel,"del")==0)
		{
				result = cgiFormSelectMultiple("neighbourRouter", faver, num, flavorChoices, &invalid);
				if (result == cgiFormNotFound) 
				  {
				  	ShowAlert(search(lcontrol,"Not_Select"));
				  }
			  for (i=0; (i < num); i++) 
				  {
				  if (flavorChoices[i])
					  {
	    				  sprintf(command,"rip_neighbor.sh off %s >/dev/null",faver[i]);
	    				  //fprintf(stderr,"command=%s",command);
	    				  system(command);
					  }

				  }

		}
		
		free(command);
		ShowAlert(search(lcontrol,"Operation_Success"));
		 return 1;
	}

	/* 脚本实现剪切字符串
	<%   
	  a="bbbkdksdf"   
	  b="kdk"   
	  num=instr(a,b)   
	  num1=len(b)   
	  response.write   mid(a,1,num-1)&mid(a,num+num1,len(a)-num1+len(b))   
	  %>
	  */

	int check_str_par_valid(char * src, int min, long max)
	{
		if (src == NULL)
			return -2;
		int test = atoi(src);
		if (test < min || test > max)
			return -1;
		else
			return 0;
		
	}
