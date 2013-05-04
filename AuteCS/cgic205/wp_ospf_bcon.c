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
* wp_ospf_bcon.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* system contrl for ospf config 
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


#include <sys/wait.h>
#define Info_Num 500
#define NEI_NUM 50


int ShowOSPFBasicConPage(); 
int OSPF_ReadConfig(char * ospf_info[],int * infoNum,struct list * lpublic);
int OSPFconfig(int checkRip,char * router_id_param);
int OPTNeighbor(char * addORdel,struct list *lpublic,struct list *lcontrol, char * NeiList);
int ReadConfig_neibor(char * ospfInfo[],int * infoNum,struct list * lpublic);

int int_metric[6]; //0 kernel , 1 connected , 2 static, 3, rip , 4, bgp, 5 isis


int cgiMain()
{
	//ShowAlert("Operation Success");
 ShowOSPFBasicConPage();
 return 0;
}

int ShowOSPFBasicConPage()
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
	char encry_ospfroute[BUF_LEN];
	char * OSPFItem[Info_Num]; 
	char * neighbor[NEI_NUM];

	char * default_route=(char *)malloc(20);
	memset(default_route,0,20);
	char def_metric[20]={0};

	char * Route_ID=(char *)malloc(30);
	memset(Route_ID,0,30);
	char * neighborlist=(char *)malloc(1000);
	memset(neighborlist,0,1000);

	char * neighborlistLater=(char *)malloc(1000);
	memset(neighborlistLater,0,1000);
	char * endptr = NULL;


	//char * Vsion=(char *)malloc(10);
	//memset(Vsion,0,10);
	
	int OSPF_enable=0; /*OSPF_enable=1为关闭，0为打开*/

	char * metric=(char *)malloc(20);
	memset(metric,0,20);
	
	char * intra_dis=(char *)malloc(10);
	memset(intra_dis,0,10);
	char * inter_dis=(char *)malloc(10);
	memset(inter_dis,0,10);
	char * exter_dis=(char *)malloc(10);
	memset(exter_dis,0,10);
	char * RFC1583Compatibility=(char *)malloc(10);
	memset(RFC1583Compatibility,0,10);
	
	char * redistribute[6];
		
	char * red_metric[6];
	
		
	char * bandwidth=(char *)malloc(20);
	memset(bandwidth,0,20);
	
	char * distance=(char *)malloc(20);
	memset(distance,0,20);
	char * PRIUsr=(char *)malloc(20);
	memset(PRIUsr,0,20);
	
	int retu=0;

	int netghnorNum=0;

	for(i=0;i<NEI_NUM;i++)
	{
		neighbor[i]=(char *)malloc(20);
		memset(neighbor[i],0,20);
	}
	for(i=0;i<6;i++)
	{
		int_metric[i] = 0;
		redistribute[i]=(char *)malloc(10);
		memset(redistribute[i],0,10);
		red_metric[i]=(char *)malloc(10);
		memset(red_metric[i],0,10);
	}
	for(i=0;i<Info_Num;i++)
	{
		OSPFItem[i]=(char *)malloc(60);
		memset(OSPFItem[i],0,60);
	}
	
	
	if((cgiFormSubmitClicked("submit_ospfroute") != cgiFormSuccess) && (cgiFormSubmitClicked("addneighbour") != cgiFormSuccess) && (cgiFormSubmitClicked("delneighbour") != cgiFormSuccess))
	{
	  memset(encry,0,BUF_LEN);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(encry_ospfroute,0,BUF_LEN);					 /*清空临时变量*/
	}

  cgiFormStringNoNewlines("encry_ospfroute",encry_ospfroute,BUF_LEN);
  //cgiFormStringNoNewlines("Version",Vsion,BUF_LEN);
  cgiFormStringNoNewlines("ROUTE_ID",Route_ID,30);
  cgiFormStringNoNewlines("NeiList",neighborlistLater,1000);
  cgiFormStringNoNewlines("PRIUsr",PRIUsr,20);
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
  "function check_ospf_enable(obj)"\
	"{"\
  		"if(document.ospf_route.OSPF_Enable.value==\"closeOSPF\" || obj==\"abc\")"\
  		"{"\
  			"document.ospf_route.default_route.disabled=true;"\
  			"document.ospf_route.default_metric.disabled=true;"\
  			"document.ospf_route.default_metric.style.backgroundColor='ccc';"\
  			
  			"document.ospf_route.intra_area.disabled=true;"\
  			"document.ospf_route.intra_area.style.backgroundColor='ccc';"\
  			"document.ospf_route.inter_area.disabled=true;"\
  			"document.ospf_route.inter_area.style.backgroundColor='ccc';"\
  			"document.ospf_route.external_area.disabled=true;"\
  			"document.ospf_route.external_area.style.backgroundColor='ccc';"\
  			"document.ospf_route.compatible1583.disabled=true;"\
  			
  			"document.ospf_route.target_ip1.disabled=true;"\
  			"document.ospf_route.target_ip2.disabled=true;"\
  			"document.ospf_route.target_ip3.disabled=true;"\
  			"document.ospf_route.target_ip4.disabled=true;"\
  			"document.ospf_route.neighbourRouter.disabled=true;"\

  			"document.ospf_route.route_id.disabled=true;"\
  			"document.ospf_route.route_id.style.backgroundColor='ccc';"\
  			"document.ospf_route.reference_bandwidth.disabled=true;"\
  			"document.ospf_route.bandwidth_value.disabled=true;"\
  			"document.ospf_route.bandwidth_value.style.backgroundColor='ccc';"\
  			
  			"document.ospf_route.addneighbour.disabled=true;"\
  			"document.ospf_route.delneighbour.disabled=true;"\
  			
  			
  			

				"document.ospf_route.rip.disabled=true;"\
				"document.ospf_route.rip_metric.disabled=true;"\
				"document.ospf_route.rip_metric.style.backgroundColor='ccc';"\
				/*document.ospf_route.bgp.disabled=true;"\*/
				"document.ospf_route.connected.disabled=true;"\
				"document.ospf_route.connected_metric.disabled=true;"\
				"document.ospf_route.connected_metric.style.backgroundColor='ccc';"\
				"document.ospf_route.static.disabled=true;"\
				"document.ospf_route.static_metric.disabled=true;"\
				"document.ospf_route.static_metric.style.backgroundColor='ccc';"\
				"document.ospf_route.kernel.disabled=true;"\
				"document.ospf_route.kernel_metric.disabled=true;"\
				"document.ospf_route.kernel_metric.style.backgroundColor='ccc';"\
				"document.ospf_route.isis.disabled=true;"\


  		"}"\
  		"else"\
  		"{"\

  			"document.ospf_route.default_route.disabled=false;"\
  			"document.ospf_route.default_metric.disabled=false;"\
  			"document.ospf_route.default_metric.style.backgroundColor='fff';"\
  			
  			"document.ospf_route.intra_area.disabled=false;"\
  			"document.ospf_route.intra_area.style.backgroundColor='fff';"\
  			"document.ospf_route.inter_area.disabled=false;"\
  			"document.ospf_route.inter_area.style.backgroundColor='fff';"\
  			"document.ospf_route.external_area.disabled=false;"\
  			"document.ospf_route.external_area.style.backgroundColor='fff';"\
			"document.ospf_route.compatible1583.disabled=false;"\
			"document.ospf_route.addneighbour.disabled=false;"\
			"document.ospf_route.delneighbour.disabled=false;"\

  			"document.ospf_route.route_id.disabled=false;"\
  			"document.ospf_route.route_id.style.backgroundColor='fff';"\
						
            "document.ospf_route.reference_bandwidth.disabled=false;"\
            "if(document.ospf_route.reference_bandwidth.checked==true)"\
            "{"\
  				"document.ospf_route.bandwidth_value.disabled=false;"\
  				"document.ospf_route.bandwidth_value.style.backgroundColor='fff';"\
  			"}"\
  			"else"\
  			"{"\
  				"document.ospf_route.bandwidth_value.disabled=true;"\
  				"document.ospf_route.bandwidth_value.style.backgroundColor='ccc';"\
  			"}"\
  			
  			

  				"document.ospf_route.target_ip1.disabled=false;"\
     			"document.ospf_route.target_ip2.disabled=false;"\
     			"document.ospf_route.target_ip3.disabled=false;"\
     			"document.ospf_route.target_ip4.disabled=false;"\
     			"document.ospf_route.neighbourRouter.disabled=false;"\
				"document.ospf_route.rip.disabled=false;"\
				"document.ospf_route.rip_metric.disabled=false;"\
				"document.ospf_route.rip_metric.style.backgroundColor='fff';"\
				/*ocument.ospf_route.bgp.disabled=false;"\*/
				"document.ospf_route.connected.disabled=false;"\
				"document.ospf_route.connected_metric.disabled=false;"\
				"document.ospf_route.connected_metric.style.backgroundColor='fff';"\
				"document.ospf_route.static.disabled=false;"\
				"document.ospf_route.static_metric.disabled=false;"\
				"document.ospf_route.static_metric.style.backgroundColor='fff';"\
				"document.ospf_route.kernel.disabled=false;"\
				"document.ospf_route.kernel_metric.disabled=false;"\
				"document.ospf_route.kernel_metric.style.backgroundColor='fff';"\
				"document.ospf_route.isis.disabled=false;"\

  		"}"\
  	"}"\
  "</script>");
  if((cgiFormSubmitClicked("submit_ospfroute") != cgiFormSuccess) && (cgiFormSubmitClicked("addneighbour") != cgiFormSuccess) && (cgiFormSubmitClicked("delneighbour") != cgiFormSuccess))
  {
  		retu=checkuser_group(str);
  }
  else if(cgiFormSubmitClicked("submit_ospfroute") == cgiFormSuccess)
   {
   	  //fprintf(stderr,"OSPF_enable=%d-Route_ID=%s",OSPF_enable,Route_ID);
  	  if(retu==0)  /*管理员*/
  		  OSPFconfig(OSPF_enable,Route_ID);
  	  	
  	  else
  	  {
  	  	 fprintf( cgiOut, "<script type='text/javascript'>\n" );
       	 fprintf( cgiOut, "window.location.href='wp_srouter.cgi?UN=%s';\n", encry_ospfroute);
       	 fprintf( cgiOut, "</script>\n" );
  	  }
   }
   //fprintf(stderr,"retu=%d",retu);
  if(retu==0)  /*管理员*/
  	fprintf(cgiOut,"<body onload=check_ospf_enable(\"123\")>");
  else fprintf(cgiOut,"<body onload=check_ospf_enable(\"abc\")>");


  if(cgiFormSubmitClicked("delneighbour") == cgiFormSuccess)
  {
  		fprintf(stderr,"neighborlistLater=%s",neighborlistLater);
		OPTNeighbor("del",lpublic,lcontrol,neighborlistLater);
  }
  if(cgiFormSubmitClicked("addneighbour") == cgiFormSuccess)
  {
		OPTNeighbor("add",lpublic,lcontrol,neighborlistLater);
  }
  fprintf(cgiOut,"<form method=post name=ospf_route>"\
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
          "<td width=62 align=center><input id=but type=submit name=submit_ospfroute style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));	  
		  if((cgiFormSubmitClicked("submit_ospfroute") != cgiFormSuccess) && (cgiFormSubmitClicked("addneighbour") != cgiFormSuccess) && (cgiFormSubmitClicked("delneighbour") != cgiFormSuccess))
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry_ospfroute,search(lpublic,"img_cancel"));
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

                		if((cgiFormSubmitClicked("submit_ospfroute") != cgiFormSuccess) && (cgiFormSubmitClicked("addneighbour") != cgiFormSuccess) && (cgiFormSubmitClicked("delneighbour") != cgiFormSuccess))
                		{
                		  	if(retu==0)  /*管理员*/
                		  	{
    						  	fprintf(cgiOut,"<tr height=26>"\
                    			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>OSPF</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));   /*突出显示*/
                    		  	fprintf(cgiOut,"</tr>");
                       		 	fprintf(cgiOut,"<tr height=25>"\
                				"<td align=left id=tdleft><a href=wp_ospf_intf.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"riplist"));						
                				fprintf(cgiOut,"</tr>");
       						 	fprintf(cgiOut,"<tr height=25>"\
                				"<td align=left id=tdleft><a href=wp_ospf_addintf.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_ospf_intf"));						
                				fprintf(cgiOut,"</tr>");
         					}
         					else
         					{
         						fprintf(cgiOut,"<tr height=26>"\
                    			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>OSPF</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));   /*突出显示*/
                    		  	fprintf(cgiOut,"</tr>");
                       		 	fprintf(cgiOut,"<tr height=25>"\
                				"<td align=left id=tdleft><a href=wp_ospf_intf.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"riplist"));						
                				fprintf(cgiOut,"</tr>");
         					}
                		}
                		else		
                		{

						  	if(retu==0)  /*管理员*/
                		  	{
     						  fprintf(cgiOut,"<tr height=26>"\
                     			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>OSPF</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));   /*突出显示*/
                     		  fprintf(cgiOut,"</tr>");
                     		  fprintf(cgiOut,"<tr height=25>"\
                  				"<td align=left id=tdleft><a href=wp_ospf_intf.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",encry_ospfroute,search(lpublic,"menu_san"),search(lcontrol,"riplist"));						
                  				fprintf(cgiOut,"</tr>");
         						  fprintf(cgiOut,"<tr height=25>"\
                  				"<td align=left id=tdleft><a href=wp_ospf_addintf.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",encry_ospfroute,search(lpublic,"menu_san"),search(lcontrol,"add_ospf_intf"));						
                  				fprintf(cgiOut,"</tr>");
         					}
            				else
            				{
            					fprintf(cgiOut,"<tr height=26>"\
                     			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>OSPF</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));   /*突出显示*/
                     		  fprintf(cgiOut,"</tr>");
                     		  fprintf(cgiOut,"<tr height=25>"\
                  				"<td align=left id=tdleft><a href=wp_ospf_intf.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",encry_ospfroute,search(lpublic,"menu_san"),search(lcontrol,"riplist"));						
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
				  			int ospfconfNum=0,j=0,k=0,q=0,flag[6],OSPF_switch=0;
				  			for(i=0;i<6;i++)
				  			{
				  				flag[i]=0;
				  			}

							///////////////////////////////初始化//////////////////////////////////
								memset(Route_ID,0,30);
                            	memset(metric,0,20);
                            	memset(intra_dis,0,10);
                            	memset(inter_dis,0,10);
                            	memset(exter_dis,0,10);
                            	memset(RFC1583Compatibility,0,10);
                            	memset(bandwidth,0,20);
                            	memset(distance,0,20);
							
							/////////////////////////////////////////////////////////////////
				  			OSPF_ReadConfig(OSPFItem,&ospfconfNum,lpublic);
				  			char * revRouteInfo[20];
							//fprintf(stderr,"ospfconfNum=%d",ospfconfNum);
							//for(i=0;i<ospfconfNum;i++)
								//fprintf(stderr,"OSPFItem[%d]=%s",i,OSPFItem[i]);

			  				for(i=0;i<ospfconfNum;i++)
    			  				{
									
        							revRouteInfo[0]=strtok(OSPFItem[i],"*");
									//fprintf(stderr,"revRouteInfo[0]=%s",revRouteInfo[0]);
        							if(strcmp(revRouteInfo[0],"#OSPF#Routing#Process")==0)
            							{
            								j=0;
                     						while(revRouteInfo[j]!=NULL && j<1)
                 								{                    									
                 									revRouteInfo[j+1]=strtok(NULL,"*#");
                 									j++;
                 								}
											fprintf(stderr,"revRouteInfo[1]=%s",revRouteInfo[1]);
											if(strstr(revRouteInfo[1],"enable.")!=NULL)
												{
                         							OSPF_enable=0;
         											OSPF_switch=1;
												}
											else
												{
                         							OSPF_enable=1;
         											OSPF_switch=0;
												}
                							
            							}
        							else if(strcmp(revRouteInfo[0],"#Router#ID")==0 && OSPF_switch==1)
            							{
            								j=0;
                    						while(revRouteInfo[j]!=NULL && j<1)
                								{
                									revRouteInfo[j+1]=strtok(NULL,"*#");
                									j++;
                								}
                							strcpy(Route_ID,revRouteInfo[1]);
											//fprintf(stderr,"Route_ID=%s",Route_ID);
											//fprintf(stderr,"OSPF_switch=%d",OSPF_switch);
            							}
									else if(strcmp(revRouteInfo[0],"###default#metric")==0 && OSPF_switch==1)
            							{
            								j=0;
                    						while(revRouteInfo[j]!=NULL && j<1)
                								{
                									revRouteInfo[j+1]=strtok(NULL,"*#");
                									j++;
                								}
                							strcpy(def_metric,revRouteInfo[1]);
											fprintf(stderr,"def_metric=%s",def_metric);
											
            							}

            						else if(strcmp(revRouteInfo[0],"###Default-information#status")==0 && OSPF_switch==1)
            							{
            								j=0;
                    						while(revRouteInfo[j]!=NULL && j<1)
                								{
                									revRouteInfo[j+1]=strtok(NULL,"*#");
                									j++;
                								}
            								 strcpy(default_route,revRouteInfo[1]);
											 fprintf(stderr,"default_route=%s",default_route);
            							}

            						else if(strcmp(revRouteInfo[0],"#intra-area#")==0 && OSPF_switch==1)
            							{
            								j=0;
                    						while(revRouteInfo[j]!=NULL && j<1)
                								{
                									revRouteInfo[j+1]=strtok(NULL,"*#");
                									j++;
                								}
            								 strcpy(intra_dis,revRouteInfo[1]);
											 fprintf(stderr,"intra_dis=%s",intra_dis);
            							}
									else if(strcmp(revRouteInfo[0],"#inter-area#")==0 && OSPF_switch==1)
            							{
            								j=0;
                    						while(revRouteInfo[j]!=NULL && j<2)
                								{
                									revRouteInfo[j+1]=strtok(NULL,"*#");
                									j++;
                								}
            								 strcpy(inter_dis,revRouteInfo[1]);
											 fprintf(stderr,"inter_dis=%s",inter_dis);
            							}
									else if(strcmp(revRouteInfo[0],"#external#")==0 && OSPF_switch==1)
            							{
            								j=0;
                    						while(revRouteInfo[j]!=NULL && j<2)
                								{
                									revRouteInfo[j+1]=strtok(NULL,"*#");
                									j++;
                								}
            								 strcpy(exter_dis,revRouteInfo[1]);
											 fprintf(stderr,"exter_dis=%s",exter_dis);
            							}
									else if(strcmp(revRouteInfo[0],"#RFC1583Compatibility#flag#")==0 && OSPF_switch==1)
            							{
            								j=0;
                    						while(revRouteInfo[j]!=NULL && j<1)
                								{
                									revRouteInfo[j+1]=strtok(NULL,"*#");
                									j++;
                								}
            								 strcpy(RFC1583Compatibility,revRouteInfo[1]);
											 fprintf(stderr,"RFC1583Compatibility=%s",RFC1583Compatibility);
            							}
									else if(strcmp(revRouteInfo[0],"#reference#bandwidth#")==0 && OSPF_switch==1)
            							{
            								j=0;
                    						while(revRouteInfo[j]!=NULL && j<1)
                								{
                									revRouteInfo[j+1]=strtok(NULL,"*#");
                									j++;
                								}
            								 strcpy(bandwidth,revRouteInfo[1]);
											 int bandwidthTemp=0;
											 if(strcmp(bandwidth,"")!=0)
											 	{
											 		bandwidthTemp=atoi(bandwidth);
													bandwidthTemp=bandwidthTemp/1000;
													sprintf(bandwidth,"%d",bandwidthTemp);
											 	}
											 fprintf(stderr,"bandwidth=%s",bandwidth);
            							}
								///////////////////////////////////distrubute 0-rip,1-kernel,2-connected,3-static,4-bgp,5-isis//////////////////////////////	
									else if(strcmp(revRouteInfo[0],"#Redistribute#source")==0 && OSPF_switch==1)
            							{
            								j=0;
                    						while(revRouteInfo[j]!=NULL && j<1)
                								{
                									revRouteInfo[j+1]=strtok(NULL,"*#");
                									j++;
                								}
											 if( revRouteInfo[1] != NULL )
												{
													revRouteInfo[2]=strtok(NULL,"*#");
												}
											 //fprintf(stderr,"revRouteInfo[2]=%s",revRouteInfo[2]);
											 if( revRouteInfo[2] != NULL )
												{
													if( strcmp(revRouteInfo[2],"metric") == 0 )
													{
														revRouteInfo[3]=strtok(NULL,"*#");
													}
													if(strstr(revRouteInfo[2],"metric") != NULL)
												 		strcpy(red_metric[k],revRouteInfo[3]);
												}

            								 strcpy(redistribute[k],revRouteInfo[1]);
											 k++;
            							}
									
									
									///////////////////////////////////neighbor//////////////////////////////
									else if(strcmp(revRouteInfo[0],"neibor")==0 && OSPF_switch==1)
            							{
            								j=0;
                    						while(revRouteInfo[j]!=NULL && j<1)
                								{                    									
                									//strcat(revRouteInfo[j+1],strtok(NULL,"*"));
                									revRouteInfo[j+1]=strtok(NULL,"*#");
                									j++;
                								}
											strcat(neighbor[q],revRouteInfo[1]);
											q++;
            							}
            						
    			  				}

   							///////////////////////////////////////////////////////////////////////////////////////////////
   							for(i=0;i<Info_Num;i++)
								memset(OSPFItem[i],0,60);

							ospfconfNum=0;
							ReadConfig_neibor(OSPFItem,&ospfconfNum,lpublic);

							for(i=0;i<ospfconfNum;i++)
								{
        							revRouteInfo[0]=strtok(OSPFItem[i],"-,");
									fprintf(stderr,"revRouteInfo[0]=%s\n",revRouteInfo[0]);
                                  	if(strcmp(revRouteInfo[0],"neighbor")==0)
            							{
            								j=0;
                    						while(revRouteInfo[j]!=NULL && j<1)
                								{                    									
                									//strcat(revRouteInfo[j+1],strtok(NULL,"-,"));
                									revRouteInfo[j+1]=strtok(NULL,"-,");
                									j++;
                								}
        									strcat(neighbor[q],revRouteInfo[1]);
        									q++;
        									
            							}
							
								}
							fprintf(cgiOut,"<tr height=35>"\
							"<td style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"ospf_enable"));
							if(OSPF_enable==1)
							{
								fprintf(cgiOut,"<td colspan=2 align=left>");
								if(retu==0)  /*管理员*/
									fprintf(cgiOut, "<select name=\"OSPF_Enable\" onchange=check_ospf_enable(this)>\n");
								else
     								fprintf(cgiOut, "<select name=\"OSPF_Enable\" disabled onchange=check_ospf_enable(\"abc\")>\n");
     							fprintf(cgiOut, "<option value=closeOSPF>CLOSE\n");
                              	fprintf(cgiOut, "<option value=openOSPF>OPEN\n");
                              	fprintf(cgiOut, "</select>\n");
     							fprintf(cgiOut,"</td>");
							}
							else if(OSPF_enable==0)
							{
     							fprintf(cgiOut,"<td colspan=2 align=left>");
     							if(retu==0)  /*管理员*/
									fprintf(cgiOut, "<select name=\"OSPF_Enable\" onchange=check_ospf_enable(this)>\n");
								else
     								fprintf(cgiOut, "<select name=\"OSPF_Enable\" disabled onchange=check_ospf_enable(\"abc\")>\n");
                              	fprintf(cgiOut, "<option value=openOSPF>OPEN\n");
                              	fprintf(cgiOut, "<option value=closeOSPF>CLOSE\n");
                              	fprintf(cgiOut, "</select>\n");
     							fprintf(cgiOut,"</td>");
							}
							fprintf(cgiOut,"<td align=right>%s:</td>",search(lcontrol,"Router_ID"));
							if(strcmp(Route_ID,"")==0)
								fprintf(cgiOut,"<td align=left width=140 colspan=2><input type=text name=route_id></td>");
							else
								fprintf(cgiOut,"<td align=left width=140 colspan=2><input type=text name=route_id value=%s></td>",Route_ID);
							fprintf(cgiOut,"<td align=left colspan=2>(eg:3.3.3.3)</td>");
							fprintf(cgiOut,"</tr>"\


							"<tr align=left height=35>");
							if(strlen(default_route) > 0)
							{
								fprintf(cgiOut, "<td colspan=8><input type=checkbox name=default_route value=def_route checked>%s</td>",search(lcontrol,"accept_default"));
							}
							else
							{
								fprintf(cgiOut, "<td colspan=8><input type=checkbox name=default_route value=def_route>%s</td>",search(lcontrol,"accept_default"));
							}

	
							fprintf(cgiOut,"</tr>"\

							"<tr align=left height=35>");
							fprintf(cgiOut,"<td width=10>&nbsp;</td>");
							fprintf(cgiOut,"<td align=left width=110>%s: </td>",search(lcontrol,"redistribute"));
							//0 kernel , 1 connected , 2 static, 3, rip , 4, bgp, 5 isis
							
							for(i=0;i<6;i++)
							{
								
								if(strstr(redistribute[i],"rip")!=NULL)
								{
									flag[0]=1;
									int_metric[3] = strtoul(red_metric[i], &endptr, 0);
								}
								else if(strstr(redistribute[i],"bgp")!=NULL)
								{
									flag[1]=1;
									int_metric[4] = strtoul(red_metric[i], &endptr, 0);
								}
								else if(strstr(redistribute[i],"connected")!=NULL)
								{
									flag[2]=1;
									int_metric[1] = strtoul(red_metric[i], &endptr, 0);
								}
								else if(strstr(redistribute[i],"static")!=NULL)
								{
									flag[3]=1;
									int_metric[2] = strtoul(red_metric[i], &endptr, 0);
								}
								else if(strstr(redistribute[i],"kernel")!=NULL)
								{
									flag[4]=1;
									int_metric[0] = strtoul(red_metric[i], &endptr, 0);
								}
								else if(strstr(redistribute[i],"isis")!=NULL)
								{
									flag[5]=1;
									int_metric[5] = strtoul(red_metric[i], &endptr, 0);
								}

							}
							
							#if 0
     							if(flag[0]==1)
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=rip value=rip checked>RIP</td>");
     							else
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=rip value=rip>RIP</td>");
     							
         						if(flag[1]==1)
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=bgp value=bgp checked>BGP</td>");
     							else
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=bgp value=bgp>BGP</td>");
     
     							if(flag[2]==1)
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=connected value=connected checked>Connected</td>");
     							else
     								{
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=connected value=connected>Connected</td>");
     								}
     
     							if(flag[3]==1)
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=static value=static checked>Static</td>");
     							else
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=static value=static>Static</td>");
     
     							if(flag[4]==1)
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=kernel value=kernel checked>Kernel</td>");
     							else
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=kernel value=kernel>Kernel</td>");
     								
     							if(flag[5]==1)
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=isis value=isis checked>Isis</td>");
     							else
     								fprintf(cgiOut, "<td width=70 align=left><input type=checkbox name=isis value=isis>Isis</td>");

							fprintf(cgiOut,"<td width=60>&nbsp;</td>");
							fprintf(cgiOut,"</tr>");
							#endif
							//0 kernel , 1 connected , 2 static, 3, rip , 4, bgp, 5 isis
								if(flag[0]==1)
     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=rip value=ospf checked>RIP</td>");
     							else
     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=rip value=ospf>RIP</td>");
								fprintf(cgiOut,"<td align=left>OSPF Metric: </td>");
								if ( int_metric[3] != 0 )
									fprintf(cgiOut,"<td align=left><input type=text name=rip_metric size=6 value=%d></td>",int_metric[3] );
								else
									fprintf(cgiOut,"<td align=left><input type=text name=rip_metric size=6 value=1></td>");
								fprintf(cgiOut,"</tr>");
						

								#if 0
								fprintf(cgiOut,"<tr align=left height=35>");
								fprintf(cgiOut,"<td colspan=2>&nbsp;</td>");
         						if(flag[1]==1)
     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=bgp value=bgp checked>BGP</td>");
     							else
     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=bgp value=bgp>BGP</td>");

								fprintf(cgiOut,"<td align=left>BGP Metric: </td>");
								if ( int_metric[4] != 0 )
									fprintf(cgiOut,"<td align=left><input type=text name=bgp_metric size=6 value=%s></td>",int_metric[4]);
								else
									fprintf(cgiOut,"<td align=left><input type=text name=bgp_metric size=6 value=1></td>");
								fprintf(cgiOut,"</tr>");
								#endif
								
     							fprintf(cgiOut,"<tr align=left height=35>");
								fprintf(cgiOut,"<td colspan=2>&nbsp;</td>");
								
     							if(flag[2]==1)
     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=connected value=connected checked>Connected</td>");
     							else
     								{
     									fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=connected value=connected>Connected</td>");
     								}
								fprintf(cgiOut,"<td align=left>Connected Metric: </td>");
								if ( int_metric[1] != 0 )
									fprintf(cgiOut,"<td align=left><input type=text name=connected_metric size=6 value=%d></td>",int_metric[1]);
								else
									fprintf(cgiOut,"<td align=left><input type=text name=connected_metric size=6 value=1></td>");
								fprintf(cgiOut,"</tr>");

								
     							fprintf(cgiOut,"<tr align=left height=35>");
								fprintf(cgiOut,"<td colspan=2>&nbsp;</td>");
     							if(flag[3]==1)
     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=static value=static checked>Static</td>");
     							else
     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=static value=static>Static</td>");
								fprintf(cgiOut,"<td align=left>Static Metric: </td>");
								if ( int_metric[2] != 0 )
									fprintf(cgiOut,"<td align=left><input type=text name=static_metric size=6 value=%d></td>",int_metric[2]);
								else
									fprintf(cgiOut,"<td align=left><input type=text name=static_metric size=6 value=1></td>");
								fprintf(cgiOut,"</tr>");

								
     							fprintf(cgiOut,"<tr align=left height=35>");
								fprintf(cgiOut,"<td colspan=2>&nbsp;</td>");
     
     							if(flag[4]==1)
     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=kernel value=kernel checked>Kernel</td>");
     							else
     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=kernel value=kernel>Kernel</td>");
								fprintf(cgiOut,"<td align=left>Kernel Metric: </td>");
								if ( int_metric[0] != 0 )
									fprintf(cgiOut,"<td align=left><input type=text name=kernel_metric size=6 value=%d></td>",int_metric[0]);
								else
									fprintf(cgiOut,"<td align=left><input type=text name=kernel_metric size=6 value=1></td>");
								fprintf(cgiOut,"</tr>");

								#if 0
     							fprintf(cgiOut,"<tr align=left height=35>");
								fprintf(cgiOut,"<td colspan=2>&nbsp;</td>");
     								
     							if(flag[5]==1)
     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=isis value=isis checked>Isis</td>");
     							else
     								fprintf(cgiOut, "<td width=70 align=left colspan=2><input type=checkbox name=isis value=isis>Isis</td>");

								fprintf(cgiOut,"<td align=left>Isis Metric: </td>");
								if ( int_metric[5] != 0 )
									fprintf(cgiOut,"<td align=left><input type=text name=isis_metric size=6 value=%s></td>",int_metric[5]);
								else
									fprintf(cgiOut,"<td align=left><input type=text name=isis_metric size=6 value=1></td>");
								fprintf(cgiOut,"</tr>");
								#endif

								
							fprintf(cgiOut,"<tr align=left height=35>");
							fprintf(cgiOut,"<td>&nbsp;</td>");
							fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"default_metric"));
							if(strcmp(def_metric,"")!=0)
								fprintf(cgiOut,"<td align=left><input type=text name=default_metric size=6 value=%s></td>",def_metric);
							else
								fprintf(cgiOut,"<td align=left><input type=text name=default_metric size=6 value=1></td>");
							fprintf(cgiOut,"<td colspan=6 style=color:red align=left>%s</td>",search(lcontrol,"metric_description"));
							fprintf(cgiOut,"</tr>"\
								
							"<tr align=left height=35>");
							fprintf(cgiOut,"<td>&nbsp;</td>");
							fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"distance"));

							
							fprintf(cgiOut,"<td align=right>%s:</td>",search(lcontrol,"intra_area"));
							if(strcmp(intra_dis,"")!=0)
								fprintf(cgiOut,"<td align=left><input type=text name=intra_area size=6 value=%s></td>",intra_dis);
							else
								fprintf(cgiOut,"<td align=left><input type=text name=intra_area size=6 value=110></td>");
							fprintf(cgiOut,"<td align=right>%s:</td>",search(lcontrol,"inter_area"));
							if(strcmp(inter_dis,"")!=0)
								fprintf(cgiOut,"<td align=left><input type=text name=inter_area size=6 value=%s></td>",inter_dis);
							else
								fprintf(cgiOut,"<td align=left><input type=text name=inter_area size=6 value=110></td>");
							fprintf(cgiOut,"<td align=right>%s:</td>",search(lcontrol,"external_area"));
							if(strcmp(exter_dis,"")!=0)
								fprintf(cgiOut,"<td align=left><input type=text name=external_area size=6 value=%s></td>",exter_dis);
							else
								fprintf(cgiOut,"<td align=left><input type=text name=external_area size=6 value=110></td>");
							fprintf(cgiOut,"<td width=60>&nbsp;</td>");
							//fprintf(cgiOut,"<td style=color:red align=left>%s</td>",search(lcontrol,"distance_description"));
							fprintf(cgiOut,"</tr>"\
							"</table>"\
						  "</td>"\
						"</tr>"\


						"<tr>"\
							"<td id=sec1 colspan=3 style=\"border-bottom:2px solid #53868b;font-size:14px\">高级设置</td>");
							fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						"<td colspan=3 align=left>"\
						"<table height=125 width=420 border=0 cellspacing=0 cellpadding=0>"\
						"<tr align=left height=35>");
						//fprintf(stderr,"bandwidth=%s",bandwidth);
						if(strstr(RFC1583Compatibility,"enabled")!=NULL)
							fprintf(cgiOut,"<td colspan=3><input type=checkbox name=compatible1583 checked>%s</td>",search(lcontrol,"compatible_1583"));
						else
							fprintf(cgiOut,"<td colspan=3><input type=checkbox name=compatible1583>%s</td>",search(lcontrol,"compatible_1583"));
						fprintf(cgiOut,"</tr>"\
						"<tr>");
						if(strcmp(bandwidth,"100")!=0)
						 	fprintf(cgiOut,"<td><input type=checkbox name=reference_bandwidth checked onclick=check_ospf_enable(this)>%s: </td>",search(lcontrol,"reference_bandwidth"));
						else
							fprintf(cgiOut,"<td><input type=checkbox name=reference_bandwidth onclick=check_ospf_enable(this)>%s: </td>",search(lcontrol,"reference_bandwidth"));
						fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"bandwidth"));
						if(strcmp(bandwidth,"100")!=0)
							fprintf(cgiOut,"<td align=left><input type=text name=bandwidth_value size=6 value=%s></td>",bandwidth);
						else
							fprintf(cgiOut,"<td align=left><input type=text name=bandwidth_value size=6 value=100></td>");
						fprintf(cgiOut,"</tr>"\
						"</table>"\
						"</td>"\
						"</tr>");

						fprintf(cgiOut,"<tr>"\
							"<td id=sec1 colspan=3 style=\"border-bottom:2px solid #53868b;font-size:14px\">Neibour设置</td>");
							fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						"<td colspan=3 align=left>"\
						"<table height=125 width=420 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						"<td colspan=3 width=300>&nbsp;</td>"\
						"<td align=center style=font-size:14px;font-weight:bold valign=bottom>neighbour list</td>"\
						"</tr>"\
						"<tr>");
						fprintf(cgiOut,"<td align=left width=60>%s: </td>",search(lcontrol,"neighbor_ip"));
						fprintf(cgiOut,"<td align=left  style=padding-left:10px>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=target_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=target_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=target_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=target_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>"\
						"<td align=right style=padding-left:5px><input type=submit style=width:70px name=addneighbour value=\"add\"></td>"\
						"<td align=left rowspan=2 style=padding-left:5px>"\
						"<select style=\"width:140\" style=\"height:100;background-color:#ebfff9\" name=\"neighbourRouter\" multiple>\n");
						memset(neighborlist,0,1000);
						for(i=0;i<NEI_NUM;i++)
						{
							if(strcmp(neighbor[i],"")!=0)
								{
									fprintf(cgiOut,"<option value=\"%s\">%s\n",neighbor[i],neighbor[i]);
									netghnorNum++;
									strcat(neighborlist,neighbor[i]);
									strcat(neighborlist,"-");
								}
						}
						fprintf(cgiOut,"</select>"\
						"</td>"\
						"</tr>"\
						"<tr>"\
						"<td width=200 colspan=2>&nbsp;</td>"\
						"<td align=right><input type=submit  style=width:70px name=delneighbour value=\"delete\"></td>"\
						"</tr>"\
						"</table>"\
						"</td>"\
						"</tr>");

        				fprintf(cgiOut,"<tr>");
        				if((cgiFormSubmitClicked("submit_ospfroute") != cgiFormSuccess) && (cgiFormSubmitClicked("addneighbour") != cgiFormSuccess) && (cgiFormSubmitClicked("delneighbour") != cgiFormSuccess))
						 {
						 	fprintf(cgiOut,"<td><input type=hidden name=encry_ospfroute value=%s></td>",encry);
							fprintf(cgiOut,"<td><input type=hidden name=ROUTE_ID value=%s></td>",Route_ID);
							fprintf(cgiOut,"<td><input type=hidden name=NeiList value=%s></td>",neighborlist);
						   	fprintf(cgiOut,"<td><input type=hidden name=PRIUsr value=%d></td>",retu);
						 }
						 else
							 {
							 	fprintf(cgiOut,"<td><input type=hidden name=encry_ospfroute value=%s></td>",encry_ospfroute);
								fprintf(cgiOut,"<td><input type=hidden name=ROUTE_ID value=%s></td>",Route_ID);
								fprintf(cgiOut,"<td><input type=hidden name=NeiList value=%s></td>",neighborlist);
							   fprintf(cgiOut,"<td><input type=hidden name=NeiList value=%d></td>",retu);
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
free(default_route);
free(metric);
free(distance);
free(red_metric);
free(neighborlist);
free(neighborlistLater);
free(Route_ID);
free(exter_dis);
free(intra_dis);
free(inter_dis);
free(RFC1583Compatibility);
free(bandwidth);


for(i=0;i<NEI_NUM;i++)
{
	free(neighbor[i]);
}

for(i=0;i<6;i++)
{
	free(redistribute[i]);
	free(red_metric[i]);
}

for(i=0;i<Info_Num;i++)
{
	free(OSPFItem[i]);	
}

release(lpublic);  
release(lcontrol);


return 0;

}
															 
int OSPF_ReadConfig(char * ospf_info[],int * infoNum,struct list * lpublic)
{
	int i;
	char * command=(char *)malloc(200);
	memset(command,0,200);
	strcat(command,"ospf_show_ip_ospf.sh | sed 's/:/*/g' | sed 's/ /#/g'  >/var/run/apache2/OSPF_info.txt");
	int status = system(command);
	int ret = WEXITSTATUS(status);
	if(0==ret)
		{}
	else ShowAlert("occur an error!");

	FILE *fd;
	char  temp[60];
	memset(temp,0,60);
	if((fd=fopen("/var/run/apache2/OSPF_info.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i=0;
	while((fgets(temp,60,fd)) != NULL)
		{
			strcat(ospf_info[i],temp);
			i++;
			memset(temp,0,60);
		}
	fclose(fd);
	*infoNum=i;
	
	free(command);
	return 1;
}

int OSPFconfig(int checkRip,char * router_id_param)
{
	int i=0;
	char * OSPFsys=(char * )malloc(250);
	memset(OSPFsys,0,250);
	int ret,status=0;
	char * OSPFInfo[2];
	char * OSPFdef=(char *)malloc(10);
	int enableflag=0; /*0  is opened ,1 is closed*/

	for(i=0;i<2;i++)
	{
		OSPFInfo[i]=(char *)malloc(20);
		memset(OSPFInfo[i],0,20);
	}
	if(checkRip==1)
	{
		strcat(OSPFInfo[0],"closeOSPF");
		strcat(OSPFInfo[1],"openOSPF");
	}
	else if(checkRip==0)
	{
		strcat(OSPFInfo[0],"openOSPF");
		strcat(OSPFInfo[1],"closeOSPF");

	}
	
	int Choice=0;
	cgiFormSelectSingle("OSPF_Enable", OSPFInfo, 2, &Choice, 0);
	fprintf(stderr,"OSPFInfo[Choice]=%s",OSPFInfo[Choice]);
	if(strcmp(OSPFInfo[Choice],"closeOSPF")==0)
	{
		enableflag=1;
		strcat(OSPFsys,"router_ospf.sh off >/dev/null");
		status = system(OSPFsys);
	}
	else if(strcmp(OSPFInfo[Choice],"openOSPF")==0)
	{
		strcat(OSPFsys,"router_ospf.sh on >/dev/null");
			status = system(OSPFsys);
	}
	ret = WEXITSTATUS(status);
	fprintf(stderr,"ret1=%d",ret);
	if(ret==-1 || ret==-2 || ret==-3)
		ShowAlert("enbale_bash_fail");

	if(enableflag==1)
		return 1;
/////////////////////////////////////////////////////////////////////////////
	memset(OSPFsys,0,250);
	char * router_id=(char *)malloc(30);
	memset(router_id,0,30);
	cgiFormStringNoNewlines("route_id",router_id,30);
	//fprintf(stderr,"router_id_param=%s-router_id=%s",router_id_param,router_id);
	if( strcmp(router_id,router_id_param)==0)
		{
			//fprintf(stderr,"router_id_param=%s-router_id=%s",router_id_param,router_id);
		}
	else if(strcmp(router_id,"")==0)
		{
			sprintf(OSPFsys,"ospf_rou.sh on %s >/dev/null",router_id);
			system(OSPFsys);
		}
	else if(strcmp(router_id,"")!=0)
		{
			sprintf(OSPFsys,"ospf_rou.sh off %s >/dev/null",router_id_param);
			system(OSPFsys);
			memset(OSPFsys,0,250);
			sprintf(OSPFsys,"ospf_rou.sh on %s >/dev/null",router_id);
			system(OSPFsys);
		}
	



//////////////////////////////////////////////////////////////////////////////
	memset(OSPFsys,0,250);
	int result=0,result2=0,result3=0,result4=0;
	char **responses;
		result = cgiFormStringMultiple("default_route", &responses);
	if (result == cgiFormNotFound) 
	{
		sprintf(OSPFsys,"ospf_default_information.sh off >/dev/null");
      	status = system(OSPFsys);
      	ret = WEXITSTATUS(status);
      	if(ret==0)
      	{}
      	else
      		ShowAlert("defaultRoute_bash_fail");
	}
	else
	{
      	sprintf(OSPFsys,"ospf_default_information.sh on >/dev/null");
      	status = system(OSPFsys);
      	ret = WEXITSTATUS(status);
      
      	if(ret == -3)
      		ShowAlert("defaultRoute_bash_fail");
	}

		//cgiStringArrayFree(responses);
		
		


	
///////////////////////////////////////////////////////////////////////////////////

	memset(OSPFsys,0,250);
	char **responses2;
		result2 = cgiFormStringMultiple("compatible1583", &responses2);
	if (result2 == cgiFormNotFound) 
	{
		
		sprintf(OSPFsys,"ospf_compatible_rfc1583.sh off >/dev/null");
      	status = system(OSPFsys);
      	ret = WEXITSTATUS(status);
      	if(ret==0)
      	{}
      	else
      		ShowAlert("compatible_bash_fail");
	}
	else
	{
      	sprintf(OSPFsys,"ospf_compatible_rfc1583.sh on >/dev/null");
      	status = system(OSPFsys);
      	ret = WEXITSTATUS(status);
      
      	if(ret==-1 ||ret == -3)
      		ShowAlert("compatible_bash_fail");
	}

		//cgiStringArrayFree(responses2);

////////////////////////////////////////////////////////////////////////////////
	
	memset(OSPFsys,0,250);
	char * bandwidth_value=(char *)malloc(10);
	memset(bandwidth_value,0,10);
	char **responses3;
		result3 = cgiFormStringMultiple("reference_bandwidth", &responses3);

	cgiFormStringNoNewlines("bandwidth_value",bandwidth_value,10);
	if (result3 == cgiFormNotFound)
	{
	
		sprintf(OSPFsys,"ospf_auto_cost.sh off 11 >/dev/null");
      	status = system(OSPFsys);
      	ret = WEXITSTATUS(status);
      	if(ret==-1 || ret == -2 || ret == -3)
      		ShowAlert("auto_cost_bash_fail");
	}
	else
	{
		if(strcmp(bandwidth_value,"") !=0 )
			sprintf(OSPFsys,"ospf_auto_cost.sh on %s >/dev/null",bandwidth_value);
		else
      		sprintf(OSPFsys,"ospf_auto_cost.sh on >/dev/null");
      	status = system(OSPFsys);
      	ret = WEXITSTATUS(status);
      
      	if(ret==-1 || ret == -2 || ret == -3)
      		ShowAlert("auto_cost_bash_fail");
	}

		//cgiStringArrayFree(responses3);
///////////////////////////////////////////////////////////////////////////////
	i=0;
	result4=0;
	char  tempName[20];
	char  redMetric[20]={0};
	for(i=0;i<6;i++)
	{
		memset(OSPFsys,0,250);
    	char **responses4;
    	memset(OSPFdef,0,10);
		memset(tempName,0,20);
		memset(redMetric, 0, 20);
    	if(i==0)
    		strcat(tempName,"rip");
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

		sprintf(redMetric, "%s_metric", tempName);
		//fprintf(stderr,"!!tempName=%s\n",tempName);	
     	result4 = cgiFormStringMultiple(tempName, &responses4);
		
     	cgiFormStringNoNewlines(redMetric, OSPFdef, 10);
		fprintf(stderr, "OSPFdef=%s\n", OSPFdef);
     	if(strcmp(OSPFdef,"")==0 || strcmp(OSPFdef,"1")==0)
     	{
     		fprintf(stderr, "333333\n");
     		if (result4 == cgiFormNotFound) 
				{
					fprintf(stderr,"tempName=%s",tempName);
            		sprintf(OSPFsys,"ospf_red.sh off %s -1 -1 non >/dev/null",tempName);
                  	status = system(OSPFsys);
      				ret = WEXITSTATUS(status);
      				if(ret== -3)
      					ShowAlert("bash_fail");
      	        }
      	   else
      	   		{
      	   			//fprintf(stderr,"xxtempName=%s",tempName);
    				sprintf(OSPFsys,"ospf_red.sh on %s -1 -1 non >/dev/null",tempName);
    				status = system(OSPFsys);
      				ret = WEXITSTATUS(status);
      				if(ret== -3)
      					ShowAlert("bash_fail");
         		}
     	}
     	else
     	{
     
     		if (result4 == cgiFormNotFound) 
				{
					//fprintf(stderr, "111111\n");
            		sprintf(OSPFsys,"ospf_red.sh off %s -1 -1 non >/dev/null",tempName);
                  	system(OSPFsys);
      	        }
      	   else
      	   		{
      	   			//fprintf(stderr, "22222222\n");
    			   sprintf(OSPFsys,"ospf_red.sh on %s %s -1 non >/dev/null",tempName, OSPFdef);
    			   system(OSPFsys);
         		}       		
     	}
     	//cgiStringArrayFree(responses4);
	}
	cgiFormStringNoNewlines("default_metric",OSPFdef,10);
	fprintf(stderr, "default_metric=%s\n", OSPFdef);

	memset(OSPFsys,0,250);
	sprintf(OSPFsys,"ospf_default_metric.sh on %s >/dev/null",OSPFdef);
	system(OSPFsys);
//////////////////////////////////////////////////////////////////////////////

	memset(OSPFsys,0,250);
	char * intra_dis=(char *)malloc(10);
	memset(intra_dis,0,10);
	char * inter_dis=(char *)malloc(10);
	memset(inter_dis,0,10);
	char * exteral_dis=(char *)malloc(20);
	memset(exteral_dis,0,20);
	cgiFormStringNoNewlines("intra_area",intra_dis,10);
	cgiFormStringNoNewlines("inter_area",inter_dis,10);
	cgiFormStringNoNewlines("external_area",exteral_dis,20);
	//fprintf(stderr,"OSPFdef=%s",OSPFdef);
	if((0==strcmp(intra_dis,"110") || 0==strcmp(intra_dis,"")) && (0==strcmp(inter_dis,"110") || 0==strcmp(inter_dis,"")) && (0==strcmp(exteral_dis,"110") || 0==strcmp(exteral_dis,"")))
	{}
	else
	{
      	sprintf(OSPFsys,"ospf_distance_ospf.sh on intra-area %s inter-area %s external %s >/dev/null",intra_dis,inter_dis,exteral_dis);
      	status = system(OSPFsys);
      	ret = WEXITSTATUS(status);
      
      	if(ret== -1 || ret == -2 || ret == -3 )
      		ShowAlert("ospf_distance_bash_fail");
	}
	
	////////////////////////////////////////////////////////////////////////
	free(OSPFsys);
	free(OSPFdef);
	free(router_id);
	free(intra_dis);
	free(inter_dis);
	
	free(exteral_dis);
	free(bandwidth_value);
	free(OSPFInfo[0]);
	free(OSPFInfo[1]);

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
    	 	
		sprintf(command,"ospf_neighbor.sh on %s non %s %s %s >/dev/null",target_IP,"param2","param4","param5");
		fprintf(stderr,"command=%s",command);
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
    				  sprintf(command,"ospf_neighbor.sh off %s non %s %s %s >/dev/null",faver[i],"param2","param4","param5");
    				  fprintf(stderr,"command=%s",command);
    				  system(command);
				  }

			  }

	}
	free(command);

	ShowAlert(search(lcontrol,"Operation_Success"));
	 return 1;
}

int ReadConfig_neibor(char * ospfInfo[],int * infoNum,struct list * lpublic)
{
	int i;
	char * command=(char *)malloc(200);
	memset(command,0,200);
	strcat(command,"show_run_conf.sh | awk 'BEGIN{FS=\"\\n\";RS=\"!\"}/router ospf/{print}'| awk '{OFS=\"-\";ORS=\"-\\n\"}{$1=$1;print}'  >/var/run/apache2/OSPF_neibor.txt");
	int status = system(command);
	int ret = WEXITSTATUS(status);				 
	if(0==ret)
		{}
	else ShowAlert(search(lpublic,"bash_fail"));

	FILE *fd;
	char  temp[60];
	memset(temp,0,60);
	if((fd=fopen("/var/run/apache2/OSPF_neibor.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i=0;
	while((fgets(temp,60,fd)) != NULL)
		{
			strcat(ospfInfo[i],temp);
			i++;
			memset(temp,0,60);
		}
	fclose(fd);
	*infoNum=i;
	
	free(command);
	return 1;
}

