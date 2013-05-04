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
* wp_ospf_intf.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* system infos 
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
#define PageNum  10
#define intf_Num 100


#define Info_Num 1000


//#define    showrouteCom   "show_route.sh | awk '{OFS="-"}NR==4,NR==0{if($1=="S"){print $1,$2,$5,$6} else if($1=="K>*"){print $1,$2,$4,$5} else if($1=="C>*"){print $1,$2,$6}}' >/var/run/apache2/ip_route.txt"


int ShowOSPFListPage();
char * trim(char * src);


//int intf(char * IntfInfo[],int * Num,struct list *lpublic);

//int  interfaceInfo(char *iname,char * intfname[],int * infoNum,struct list * lpublic);

int ReadConfig_INTF(char * ospfInfo[],int * infoNum,struct list * lpublic);
char * nameToIP(char * intfname,struct list * lpublic);
int delete_hand();


int cgiMain()
{
 ShowOSPFListPage();
 return 0;
}

int ShowOSPFListPage()
{ 
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 
  char *encry=(char *)malloc(BUF_LEN);
  char *PNtemp=(char *)malloc(10);
  char *SNtemp=(char *)malloc(10);
  char *str=NULL;
  FILE *fp;
  char lan[3];
  char OSPFlist_encry[BUF_LEN];       
  int i;   
  int cl=1;
  	int pageNum=0;
	char * pageNumCA=(char *)malloc(10);
	memset(pageNumCA,0,10);
	char * pageNumCD=(char *)malloc(10);
	memset(pageNumCD,0,10);

//////////////////////////////////
	/*char * intfName[intf_Num];
	for(i=0;i<intf_Num;i++)
	{
		intfName[i]=(char *)malloc(21);
		memset(intfName[i],0,21);
	}*/

////////////////////////////////////  		
  	char * intfItem[Info_Num];
	char * mode=(char *)malloc(10);
	memset(mode,0,10);
	char * authString=(char *)malloc(50);
	memset(authString,0,50);
	char * sendversion=(char *)malloc(10);
	memset(sendversion,0,10);
	char * recversion=(char *)malloc(10);
	memset(recversion,0,10);
	char * horizon=(char *)malloc(80);
	memset(horizon,0,80);
	
	char * address=(char *)malloc(60);
	memset(address,0,60);
	
	char * network=(char *)malloc(20);
	memset(network,0,20);
	
	char * passive=(char *)malloc(20);
	memset(passive,0,20);
	char * distance=(char *)malloc(20);
	memset(distance,0,20);
	
	char * OSPF_intf_info[Info_Num]; 

	char * Area=(char *)malloc(50);
	memset(Area,0,50);
	char * AreaID=(char *)malloc(50);
	memset(AreaID,0,50);
	char * Area_type=(char *)malloc(50);
	memset(Area_type,0,50);
	char * Router_ID=(char *)malloc(50);
	memset(Router_ID,0,50);

	char * Transmit=(char *)malloc(10);
	memset(Transmit,0,10);
	char * Hello=(char *)malloc(10);
	memset(Hello,0,10);
	char * Retransmit=(char *)malloc(10);
	memset(Retransmit,0,10);
	char * Dead=(char *)malloc(10);
	memset(Dead,0,10);
	char * Priority=(char *)malloc(10);
	memset(Priority,0,10);
	char * Cost=(char *)malloc(10);
	memset(Cost,0,10);
	char * auth_mode=(char *)malloc(10);
	memset(auth_mode,0,10);
	char * auth_key=(char *)malloc(30);
	memset(auth_key,0,30);
	char * MD5key_id=(char *)malloc(30);
	memset(MD5key_id,0,30);
	
	
	char * Network_type=(char *)malloc(20);
	memset(Network_type,0,20);
	char * Adjacent_neighbor_count=(char *)malloc(10);
	memset(Adjacent_neighbor_count,0,10);
	char * MTU_check=(char *)malloc(10);
	memset(MTU_check,0,10);
	
	char * intfName=(char *)malloc(30);
	memset(intfName,0,30);
	//int addTemp=0;
//////////////////////////////delete抓取的值/////////////////////////////////////////////
	char * deleteOP=(char *)malloc(15);
	memset(deleteOP,0,15);
	int retu=0;
	
	char * CheckUsr=(char *)malloc(10);
	memset(CheckUsr,0,10);
	
	for(i=0;i<Info_Num;i++)
	{
		OSPF_intf_info[i]=(char *)malloc(60);
		memset(OSPF_intf_info[i],0,60);
	}
	
	for(i=0;i<Info_Num;i++)
	{
		intfItem[i]=(char *)malloc(60);
		memset(intfItem[i],0,60);
	}

  if(cgiFormSubmitClicked("submit_OSPFlist") != cgiFormSuccess)
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
      return 0;
	}
	memset(OSPFlist_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
    memset(PNtemp,0,10);
	cgiFormStringNoNewlines("PN",PNtemp,10);
	pageNum=atoi(PNtemp);
	memset(SNtemp,0,10);
	cgiFormStringNoNewlines("SN",SNtemp,10);
  cgiFormStringNoNewlines("encry_Riplist",OSPFlist_encry,BUF_LEN);
  cgiFormStringNoNewlines("DELRULE",deleteOP,15);
  fprintf(stderr,"deleteOP=%s",deleteOP);
  cgiFormStringNoNewlines("CheckUsr",CheckUsr,10);
  if(strcmp(CheckUsr,"")!=0)
	retu=atoi(CheckUsr);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"route_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".ShowRoute {overflow-x:hidden;  overflow:auto; width: 760px; height: 286px; clip: rect( ); padding-top: 2px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
  	"</style>"\
  "</head>"\
  "<body>");
  if(strcmp(deleteOP,"delete_intf")==0)
  {
  	 //fprintf(stderr,"11111111111");
  	 delete_hand();
  }
  if(cgiFormSubmitClicked("submit_OSPFlist") != cgiFormSuccess)
  {
  	retu=checkuser_group(str);
  }
  if(cgiFormSubmitClicked("submit_OSPFlist") == cgiFormSuccess)
  {
  	 fprintf( cgiOut, "<script type='text/javascript'>\n" );
   	 fprintf( cgiOut, "window.location.href='wp_srouter.cgi?UN=%s';\n", OSPFlist_encry);
   	 fprintf( cgiOut, "</script>\n" );
  		
  }

  fprintf(cgiOut,"<form method=post>"\
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
          "<td width=62 align=center><input id=but type=submit name=submit_OSPFlist style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));	  
		  if(cgiFormSubmitClicked("submit_OSPFlist") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",OSPFlist_encry,search(lpublic,"img_cancel"));
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
         		if(cgiFormSubmitClicked("submit_OSPFlist") != cgiFormSuccess)
         		{
         			if(retu==0)
         			{
            			fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_ospf_bcon.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));					   
            			fprintf(cgiOut,"</tr>");
            			fprintf(cgiOut,"<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>OSPF</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"riplist"));   /*突出显示*/
            			fprintf(cgiOut,"</tr>"\
            			"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_ospf_addintf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_ospf_intf"));
            			fprintf(cgiOut,"</tr>");
         			}
         			else
         			{
         				fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_ospf_bcon.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));					   
            			fprintf(cgiOut,"</tr>");
            			fprintf(cgiOut,"<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>OSPF</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"riplist"));   /*突出显示*/
            			fprintf(cgiOut,"</tr>");
						
         			}
         		}
         		else if(cgiFormSubmitClicked("submit_OSPFlist") == cgiFormSuccess) 			  
         		{
         			if(retu==0)
         			{
            			fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_ospf_bcon.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",OSPFlist_encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));						
            			fprintf(cgiOut,"</tr>");
            			fprintf(cgiOut,"<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>OSPF</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"riplist"));   /*突出显示*/
            			fprintf(cgiOut,"</tr>"\
            			"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_ospf_addintf.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",OSPFlist_encry,search(lpublic,"menu_san"),search(lcontrol,"add_ospf_intf"));
            			fprintf(cgiOut,"</tr>");
         			}
         			else
         			{
         				fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_ospf_bcon.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",OSPFlist_encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));						
            			fprintf(cgiOut,"</tr>");
            			fprintf(cgiOut,"<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>OSPF</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"riplist"));   /*突出显示*/
            			fprintf(cgiOut,"</tr>");
         			}
         		}
				for(i=0;i<13;i++)
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}
				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
						"<table width=770 height=360 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"OSPF_intf_info"));
						fprintf(cgiOut,"</tr>");
						 fprintf(cgiOut,"<tr>"\
						   "<td align=left valign=top style=padding-top:18px>"\
						   "<div class=ShowRoute><table width=750 border=1 frame=below  rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
							 fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 id=td1 align=left>"\
							 "<th width=82 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"interface"));
							 fprintf(cgiOut,"<th width=134 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"network"));
							 fprintf(cgiOut,"<th width=134 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"area_ID"));
							 fprintf(cgiOut,"<th width=85 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"network_type"));
							 fprintf(cgiOut,"<th width=85 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"prior"));
							 fprintf(cgiOut,"<th width=120 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"Authentication"));
							 fprintf(cgiOut,"<th width=35 style=font-size:12px>&nbsp;</th>");
							 fprintf(cgiOut,"<th width=35 style=font-size:12px>&nbsp;</th>");	
							 fprintf(cgiOut,"</tr>");
							 int Ospf_item_num=0,j=0;

							 ReadConfig_INTF(OSPF_intf_info,&Ospf_item_num,lpublic);
							// for(i=0;i<Ospf_item_num;i++)
							 	//fprintf(stderr,"OSPF_intf_info[%d]=%s",i,OSPF_intf_info[i]);

							 char * revOSPFRouteInfo[5]={NULL};

							 
							 for(i=0;i<Ospf_item_num;i++)
							 	{

									if(strcmp(Adjacent_neighbor_count,"")!=0)
										{
											//////////临时变量初始化/////////////////////////////
        							 		memset(Area,0,50);
                                         	memset(Router_ID,0,50);
                                         	memset(Transmit,0,10);
                                         	memset(Hello,0,10);
                                         	memset(Retransmit,0,10);
                                         	memset(Dead,0,10);
                                         	memset(Priority,0,10);
                                         	memset(Network_type,0,20);
											memset(Adjacent_neighbor_count,0,10);
											memset(auth_mode,0,10);
                                         	memset(auth_key,0,10);
            								/////////////////////////////////////////////////////////
										}
							 		revOSPFRouteInfo[0]=strtok(OSPF_intf_info[i],"#");
									//fprintf(stderr,"revOSPFRouteInfo[0]=%s",revOSPFRouteInfo[0]);
									//revOSPFRouteInfo[0]=trim(revOSPFRouteInfo[0]);
									if(strstr(revOSPFRouteInfo[0],"Ifname")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(intfName,revOSPFRouteInfo[1]);
												fprintf(stderr,"intfName=%s",intfName);
										}
									if(strstr(revOSPFRouteInfo[0],"Internet")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(address,revOSPFRouteInfo[1]);
												fprintf(stderr,"address=%s",revOSPFRouteInfo[1]);
										}
									if(strstr(revOSPFRouteInfo[0],"Area")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(Area,revOSPFRouteInfo[1]);
												//fprintf(stderr,"Area=%s",revOSPFRouteInfo[1]);
										}
									if(strstr(revOSPFRouteInfo[0],"Router ID")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(Router_ID,revOSPFRouteInfo[1]);
										
												//fprintf(stderr,"Router ID=%s",revOSPFRouteInfo[1]);
										}
									if(strstr(revOSPFRouteInfo[0],"Transmit")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#s\n");
                    									j++;
                    								}
												strcpy(Transmit,revOSPFRouteInfo[1]);
												//fprintf(stderr,"Transmit=%s",revOSPFRouteInfo[1]);
										}
									if(strstr(revOSPFRouteInfo[0],"Retransmit")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#s\n");
                    									j++;
                    								}
												strcpy(Retransmit,revOSPFRouteInfo[1]);
												//fprintf(stderr,"Retransmit=%s",revOSPFRouteInfo[1]);
										}
									if(strcmp(revOSPFRouteInfo[0],"  Hello ")==0)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#s\n");
                    									j++;
                    								}
												strcpy(Hello,revOSPFRouteInfo[1]);
												fprintf(stderr,"Hello=%s",revOSPFRouteInfo[1]);
										}
									if(strstr(revOSPFRouteInfo[0],"Dead")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#s\n");
                    									j++;
                    								}
												strcpy(Dead,revOSPFRouteInfo[1]);
												//fprintf(stderr,"Dead=%s",revOSPFRouteInfo[1]);
										}
									if(strstr(revOSPFRouteInfo[0],"Priority")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(Priority,revOSPFRouteInfo[1]);
												//fprintf(stderr,"Priority=%s",revOSPFRouteInfo[1]);
										}
									if(strstr(revOSPFRouteInfo[0],"Network")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(Network_type,revOSPFRouteInfo[1]);
												//fprintf(stderr,"Network_type=%s",revOSPFRouteInfo[1]);
										}

									///////////////////////////////////////////////////////////////////
									if(strstr(revOSPFRouteInfo[0],"MTU mismatch detection")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(MTU_check,revOSPFRouteInfo[1]);
												//fprintf(stderr,"MTU_check=%s",MTU_check);
										}
									if(strstr(revOSPFRouteInfo[0],"Cost")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(Cost,revOSPFRouteInfo[1]);
												//fprintf(stderr,"Cost=%s",Cost);
										}
									/////////////////////////认证///////////////////////////////////////////
									if(strstr(revOSPFRouteInfo[0],"Simple key")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(auth_key,revOSPFRouteInfo[1]);
												strcpy(auth_mode,"Simple");
												//fprintf(stderr,"auth_key=%s",auth_key);
										}
									if(strcmp(revOSPFRouteInfo[0],"  MD5key")==0)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(auth_key,revOSPFRouteInfo[1]);
												strcpy(auth_mode,"MD5");
												//fprintf(stderr,"auth_key=%s",auth_key);
										}
									if(strstr(revOSPFRouteInfo[0],"  MD5keyid") != NULL )
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{                    							
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(MD5key_id,revOSPFRouteInfo[1]);
												strcpy(auth_mode,"MD5");
												//fprintf(stderr,"MD5key_id=%s",MD5key_id);
										}
									//////////////////////////////////////////////////////////////////
									if(strstr(revOSPFRouteInfo[0],"Adjacent neighbor")!=NULL)
										{
											j=0;
                        						while(revOSPFRouteInfo[j]!=NULL && j<1)
                    								{
                    									revOSPFRouteInfo[j+1]=strtok(NULL,"#\n");
                    									j++;
                    								}
												strcpy(Adjacent_neighbor_count,revOSPFRouteInfo[1]);
												//fprintf(stderr,"Adjacent_neighbor_count=%s",revOSPFRouteInfo[1]);
										}
									intfName=trim(intfName);
									Area=trim(Area);
									Hello=trim(Hello);
									Dead=trim(Dead);
									Transmit=trim(Transmit);
									Retransmit=trim(Retransmit);
									Priority=trim(Priority);
									Network_type=trim(Network_type);
									MTU_check=trim(MTU_check);
									address=trim(address);
									Cost=trim(Cost);
									auth_mode=trim(auth_mode);
									auth_key=trim(auth_key);
									
									/*处理AREA里的STUB和NSSA*/
									if(strstr(Area,"[")!=NULL)
										{
         									revOSPFRouteInfo[2]=strtok(Area,"[]");
         									revOSPFRouteInfo[3]=strtok(NULL,"[]");
         									//fprintf(stderr,"areaId=%s-area_type=%s",revOSPFRouteInfo[2],revOSPFRouteInfo[3]);
         									strcpy(AreaID,revOSPFRouteInfo[2]);
         									strcpy(Area_type,revOSPFRouteInfo[3]);
         									AreaID=trim(AreaID);
         									Area_type=trim(Area_type);
											fprintf(stderr,"@@@AreaID=%s-Area_type=%s",AreaID,Area_type);
										}
									else
										{
											strcpy(AreaID,Area);
											
										}
        							if(strcmp(Adjacent_neighbor_count,"")!=0)
                					   {
        								  	fprintf(cgiOut,"<tr height=25 bgcolor=%s align=left>",setclour(cl));
        			  						fprintf(cgiOut,"<td>%s</td>",intfName);
                							fprintf(cgiOut,"<td>%s</td>",address);
                							fprintf(cgiOut,"<td>%s</td>",AreaID);
											if(strcmp(Network_type,"BROADCAST")==0)
												fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"broadcast"));
											else if(strcmp(Network_type,"POINTOMULTIPOINT")==0)
												fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"point_to_mulpoint"));
											else if(strcmp(Network_type,"POINTOPOINT")==0)
												fprintf(cgiOut,"<td>%s</td>",search(lcontrol,"point_to_point"));
											else
												fprintf(cgiOut,"<td>%s</td>",Network_type);
                							
                							fprintf(cgiOut,"<td>%s</td>",Priority);
											//fprintf(stderr,"auth_mode=%s",auth_mode);
											if(retu==0)
											{
												if(strcmp(auth_mode,"")!=0)
	                								fprintf(cgiOut,"<td>%s</td>",auth_mode);
												else
													fprintf(cgiOut,"<td>%s</td>","None");
											}
											else
											{			
												if(strcmp(auth_mode,"")!=0)
													fprintf(cgiOut,"<td colspan=3>%s</td>",auth_mode);
												else
													fprintf(cgiOut,"<td colspan=3>%s</td>","None");
											}
											//fprintf(stderr,"areaId=%s-area_type=%s",AreaID,Area_type);
											if(retu==0)
											{
	        									if(cgiFormSubmitClicked("submit_OSPFlist") != cgiFormSuccess)
        										{
        											//fprintf(stderr,"xxintfName=%s-Area=%s-Hello=%s-Dead=%s-Transmit=%s-Retransmit=%s-Priority=%s-Network_type=%s-MTU_check=%s-address=%s",intfName,Area,Hello,Dead,Transmit,Retransmit,Priority,Network_type,MTU_check,address);
                 									fprintf(cgiOut,"<td><a href=wp_ospf_editIntf.cgi?UN=%s&IFNAME=%s&AREA=%s&AREA_TYPE=%s&ADDRESS=%s&NET_TYPE=%s&PRI=%s&HELLO=%s&DEAD=%s&TRANSMIT=%s&RESTRANS=%s&MTU=%s&COST=%s&auth_mode=%s&auth_key=%s&MD5key_id=%s>%s</a></td>",encry,intfName,AreaID,Area_type,address,Network_type,Priority,Hello,Dead,Transmit,Retransmit,MTU_check,Cost,auth_mode,auth_key,MD5key_id,search(lcontrol,"edit"));	 
                 									fprintf(cgiOut,"<td><a href=wp_ospf_intf.cgi?UN=%s&DELRULE=%s&IFNAME=%s&AREA=%s&AREA_TYPE=%s&ADDRESS=%s&NET_TYPE=%s&PRI=%s&HELLO=%s&DEAD=%s&TRANSMIT=%s&RESTRANS=%s&MTU=%s&COST=%s&auth_mode=%s&auth_key=%s&MD5key_id=%s target=mainFrame onclick=\"return confirm('%s')\" style=text-decoration:underline>%s</a></td>",encry,"delete_intf",intfName,AreaID,Area_type,address,Network_type,Priority,Hello,Dead,Transmit,Retransmit,MTU_check,Cost,auth_mode,auth_key,MD5key_id,search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
        										}
												else
												{
													fprintf(cgiOut,"<td><a href=wp_ospf_editIntf.cgi?UN=%s&IFNAME=%s&AREA=%s&AREA_TYPE=%s&ADDRESS=%s&NET_TYPE=%s&PRI=%s&HELLO=%s&DEAD=%s&TRANSMIT=%s&RESTRANS=%s&MTU=%s&COST=%s&auth_mode=%s&auth_key=%s&MD5key_id=%s>%s</a></td>",OSPFlist_encry,intfName,AreaID,Area_type,address,Network_type,Priority,Hello,Dead,Transmit,Retransmit,MTU_check,Cost,auth_mode,auth_key,MD5key_id,search(lcontrol,"edit"));	 
        											fprintf(cgiOut,"<td><a href=wp_ospf_intf.cgi?UN=%s&DELRULE=%s&IFNAME=%s&AREA=%s&AREA_TYPE=%s&ADDRESS=%s&NET_TYPE=%s&PRI=%s&HELLO=%s&DEAD=%s&TRANSMIT=%s&RESTRANS=%s&MTU=%s&COST=%s&auth_mode=%s&auth_key=%s&MD5key_id=%s  target=mainFrame onclick=\"return confirm('%s')\" style=text-decoration:underline>%s</a></td>",OSPFlist_encry,"delete_intf",intfName,AreaID,Area_type,address,Network_type,Priority,Hello,Dead,Transmit,Retransmit,MTU_check,Cost,auth_mode,auth_key,MD5key_id,search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
												}
											}
        								 fprintf(cgiOut,"</tr>");
        								cl=!cl;
        							   }
									 }	
						   fprintf(cgiOut,"</table></div>"\
							 "</td>"\
						   "</tr>"\
						 
						"<tr>"\
						"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\"></td>");
						fprintf(cgiOut,"</tr>"\
						 "<tr height=25 style=padding-top:2px>"\
			   			"<td style=font-size:14px;color:#FF0000></td>"\
			   			"</tr>"\
			   			
						 "<tr>"\
						"<td>"\
						"<table width=430 style=padding-top:2px>"\
						"<tr>");
						sprintf(pageNumCA,"%d",pageNum+1);
						sprintf(pageNumCD,"%d",pageNum-1);
						if(cgiFormSubmitClicked("submit_OSPFlist") != cgiFormSuccess)
						{
							fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_ospf_intf.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
							fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_ospf_intf.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
						}
						else if(cgiFormSubmitClicked("submit_OSPFlist") == cgiFormSuccess)
						{
							fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_ospf_intf.cgi?UN=%s&PN=%s&SN=%s>%s</td>",OSPFlist_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
							fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_ospf_intf.cgi?UN=%s&PN=%s&SN=%s>%s</td>",OSPFlist_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));  
						}
						fprintf(cgiOut,"</tr></table></td>"\
						"</tr>"\
						 "<tr>");
						 if(cgiFormSubmitClicked("submit_OSPFlist") != cgiFormSuccess)
						 {
						   fprintf(cgiOut,"<td><input type=hidden name=encry_Riplist value=%s></td>",encry);
						   fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
						 }
						 else if(cgiFormSubmitClicked("submit_OSPFlist") == cgiFormSuccess)
							 {
							   fprintf(cgiOut,"<td><input type=hidden name=encry_Riplist value=%s></td>",OSPFlist_encry);
							   fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
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

for(i=0;i<Info_Num;i++)
{
  free(intfItem[i]);   
}

for(i=0;i<Info_Num;i++)
{
	free(OSPF_intf_info[i]);	
}

free(MD5key_id);
free(AreaID);
free(Area_type);
free(auth_mode);
free(auth_key);
free(Cost);
free(MTU_check);
free(intfName);
free(Adjacent_neighbor_count);
free(Network_type);
free(Priority);	
free(Dead);
free(Transmit);
free(Hello);
free(Retransmit);
free(Router_ID);
free(Area);
free(horizon);
free(recversion);
free(sendversion);
free(authString);
free(mode);
free(address);
free(network);
free(passive);
free(distance);
free(deleteOP);
free(PNtemp);
free(SNtemp);
free(pageNumCA);
free(pageNumCD);
release(lpublic);  
release(lcontrol);

return 0;
}


#if 0
int intf(char * IntfInfo[],int * Num,struct list * lpublic)
{
	FILE * ft;
	int status,ret,i;
	char * syscommand=(char *)malloc(200);
	memset(syscommand,0,200);
	
	strcpy(syscommand,"ip_addr.sh");
	status = system(syscommand);
	 if(status == 0)
	     {
	     	}
	 else 
	 {
	  ShowAlert(search(lpublic,"bash_fail"));
	}
	//ret = WEXITSTATUS(status);
						 
//	if(0==ret)
//		{}
//	else 
	memset(syscommand,0,200);
	strcat(syscommand,"cat /var/run/apache2/ip_addr.file | awk '{print $1}' > /var/run/apache2/InterfaceName.txt");
	status = system(syscommand);
	ret = WEXITSTATUS(status);
	if(0==ret)
		{}
	else ShowAlert(search(lpublic,"bash_fail"));

	/*char * temp[intf_Num];
	for(i=0;i<intf_Num;i++)
	{
		temp[i]=(char *)malloc(21);
		memset(temp[i],0,21);
	}*/
	char temp[30];
	memset(temp,0,30);
	if((ft=fopen("/var/run/apache2/InterfaceName.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i=0;
	while((fgets(temp,28,ft)) != NULL)
		{
     		strcat(IntfInfo[i],temp);
     		IntfInfo[i]=strtok(IntfInfo[i],"\n");
     		i++;
     		memset(temp,0,30);
		}
	fclose(ft);

        fprintf(stderr,"num=%d",i);
	*Num=i;
	free(syscommand);
	return 1;
}




int  interfaceInfo(char * iname,char * intfname[],int * infoNum,struct list * lpublic)
{
 FILE * ft;
 char * syscommand=(char *)malloc(500);
 memset(syscommand,0,500);
 int i;
 sprintf(syscommand,"ospf_show_ip_ospf_if.sh NON | awk 'BEGIN{FS=\"\\n\";RS=\"\\n\\n\"}!/OSPF status/{print}' | awk 'NR>1{print}' >/var/run/apache2/OSPFIntf.txt");
 int status = system(syscommand);
 int ret = WEXITSTATUS(status);  
 if(0==ret)
	 {}
 else ShowAlert("bash_intf_fail");
 if((ft=fopen("/var/run/apache2/OSPFIntf.txt","r"))==NULL)
	 {
		 ShowAlert(search(lpublic,"error_open"));
		 return 0;
	 }
 char  temp[60];
 memset(temp,0,60);
 i=0;
 while((fgets(temp,60,ft)) != NULL)
	 {
	 	if(strcmp(temp,"")==0)
	 		return -1;
		 strcat(intfname[i],temp);
		 i++;
		 memset(temp,0,60);
	 }
 fclose(ft);
 *infoNum=i;
 free(syscommand);
 return 1;
}

#endif

int ReadConfig_INTF(char * ospfInfo[],int * infoNum,struct list * lpublic)
{
	int i;
	char * command=(char *)malloc(200);
	memset(command,0,200);
	strcat(command,"ospf_show_ip_ospf_if.sh NON 2>/dev/null | awk 'BEGIN{FS=\"\\n\";RS=\"\\n\\n\"}!/OSPF status/{print}' | awk 'BEGIN{FS=\":\";OFS=\"#\"}{$1=$1;print}' >/var/run/apache2/OSPFIntf.txt");
	int status = system(command);
	int ret = WEXITSTATUS(status);				 
	if(0==ret)
		{}
	else ShowAlert(search(lpublic,"bash_fail"));

	FILE *fd;
	char  temp[60];
	memset(temp,0,60);
	if((fd=fopen("/var/run/apache2/OSPFIntf.txt","r"))==NULL)
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

char * nameToIP(char * intfname,struct list * lpublic)
{
	FILE * fd;
	char * command=(char *)malloc(100);
	memset(command,0,100);
	strcat(command,"NameToIP.sh");
	strcat(command," ");
	strcat(command,intfname);
	system(command);
	//fprintf(stderr,"command=%s",command);
	if((fd=fopen("/var/run/apache2/NameToIp.txt","r"))==NULL)
			{
				ShowAlert(search(lpublic,"error_open"));
				return 0;
			}
	char * temp=(char *)malloc(30);
	memset(temp,0,30);
	char * temp1=(char *)malloc(30);
	memset(temp1,0,30);
	char * temp2=(char *)malloc(30);
	memset(temp2,0,30);	
	char * templater=(char *)malloc(30);
	memset(templater,0,30);	
	fgets(temp,28,fd);
	fclose(fd);
	temp1=strtok(temp,"-");
	temp2=strtok(NULL,"-\n");

	sprintf(templater,"%s/%s",temp1,temp2);

	free(temp);
	free(command);
	return templater;
}

int delete_hand()
{
	char * syscommand=(char *)malloc(250);
	memset(syscommand,0,250);
	char * Area=(char *)malloc(50);
	memset(Area,0,50);
	char * Area_type=(char *)malloc(10);
	memset(Area_type,0,10);

	char * Transmit=(char *)malloc(10);
	memset(Transmit,0,10);
	char * Hello=(char *)malloc(10);
	memset(Hello,0,10);
	char * Retransmit=(char *)malloc(10);
	memset(Retransmit,0,10);
	char * Dead=(char *)malloc(10);
	memset(Dead,0,10);
	char * Priority=(char *)malloc(10);
	memset(Priority,0,10);
	char * Cost=(char *)malloc(10);
	memset(Cost,0,10);
	
	char * Network_type=(char *)malloc(20);
	memset(Network_type,0,20);
	char * Network_type_forcommand=(char *)malloc(30);
	memset(Network_type_forcommand,0,30);

	char * MTU_check=(char *)malloc(10);
	memset(MTU_check,0,10);
	
	char * intfName=(char *)malloc(30);
	memset(intfName,0,30);
	char * address=(char *)malloc(60);
	memset(address,0,60);
	char * auth_mode=(char *)malloc(10);
	memset(auth_mode,0,10);
	char * auth_key=(char *)malloc(30);
	memset(auth_key,0,30);
	char * MD5key_id=(char *)malloc(30);
	memset(MD5key_id,0,30);
	


	int flag=0;
		
	cgiFormStringNoNewlines("IFNAME", intfName, 30);
    cgiFormStringNoNewlines("AREA",Area,50);
	cgiFormStringNoNewlines("AREA_TYPE",Area_type,10);
    cgiFormStringNoNewlines("ADDRESS",address,60);
    cgiFormStringNoNewlines("HELLO",Hello,10);
    cgiFormStringNoNewlines("DEAD",Dead,10);
    cgiFormStringNoNewlines("TRANSMIT",Transmit,10);
    cgiFormStringNoNewlines("RESTRANS",Retransmit,10);
    cgiFormStringNoNewlines("PRI",Priority,10);
    cgiFormStringNoNewlines("NET_TYPE",Network_type,20);
    cgiFormStringNoNewlines("MTU",MTU_check,10);
    cgiFormStringNoNewlines("COST",Cost,10);
	cgiFormStringNoNewlines("auth_mode",auth_mode,10);
	cgiFormStringNoNewlines("auth_key",auth_key,30);
	cgiFormStringNoNewlines("MD5key_id",MD5key_id,30);
	
	fprintf(stderr,"@@intfName=%s-Area=%s-Hello=%s-Dead=%s-Transmit=%s-Retransmit=%s-Priority=%s-Network_type=%s-MTU_check=%s-address=%s",intfName,Area,Hello,Dead,Transmit,Retransmit,Priority,Network_type,MTU_check,address);
	//IFNAME=%s&AREA=%s&AREA_TYPE=%s&ADDRESS=%s&NET_TYPE=%s&PRI=%s&HELLO=%s&DEAD=%s&TRANSMIT=%s&RESTRANS=%s&MTU=%s&COST=%s&auth_mode=%s&auth_key=%s
	//fprintf(stderr,"modeLater=%s-authStringLater=%s-recversionLater=%s-sendversionLater=%s-horizonLater=%s-networkLater=%s-IFname=%s",modeLater,authStringLater,recversionLater,sendversionLater,horizonLater,networkLater,IFname);
	//fprintf(stderr,"modeLater=%s-authStringLater=%s-recversionLater=%s-sendversionLater=%s-horizonLater=%s-networkLater=%s-IFname=%s",modeLater,authStringLater,recversionLater,sendversionLater,horizonLater,networkLater,IFname);
	sprintf(syscommand,"ospf_if_hellointervla.sh %s off -2 non>/dev/null",intfName);
	system(syscommand);
	
	memset(syscommand,0,250);
    sprintf(syscommand,"ospf_if_deadinterval.sh %s off mini -1 non >/dev/null",intfName);
	system(syscommand);
	
	memset(syscommand,0,250);
	sprintf(syscommand,"ospf_if_mtu.sh %s off  non >/dev/null",intfName);
	system(syscommand);

	memset(syscommand,0,250);
	sprintf(syscommand,"ospf_if_cost.sh %s off -1 non >/dev/null",intfName);
	system(syscommand);

	if(strcmp(auth_mode,"Simple")==0)
		{
        	memset(syscommand,0,250);
        	sprintf(syscommand,"ospf_if_authentication_key.sh %s OFF 123 NON >/dev/null",intfName);
        	system(syscommand);
        	
        	memset(syscommand,0,250);
        	sprintf(syscommand,"ospf_if_authentication.sh %s off 123 non >/dev/null",intfName);
        	system(syscommand);
		}
	else if(strcmp(auth_mode,"MD5")==0)
		{
			memset(syscommand,0,250);
        	sprintf(syscommand,"ospf_if_md5_key.sh %s off %s non non >/dev/null",intfName,MD5key_id);
        	system(syscommand);
        	
        	memset(syscommand,0,250);
        	sprintf(syscommand,"ospf_if_authentication.sh %s off 123 non >/dev/null",intfName);
        	system(syscommand);
		}

	if(strcmp(Network_type,"NBMA")==0)
		strcpy(Network_type_forcommand,"non-broadcast");
	else if(strcmp(Network_type,"POINTOMULTIPOINT")==0)
		strcpy(Network_type_forcommand,"point-to-multipoint");
	else if(strcmp(Network_type,"POINTOPOINT")==0)
		strcpy(Network_type_forcommand,"point-to-point");
	else flag=1;
	
	if(flag==0)
		{
    		memset(syscommand,0,250);
    		sprintf(syscommand,"ospf_if_network.sh %s off %s>/dev/null",intfName,Network_type_forcommand);
    		system(syscommand);
		}

	memset(syscommand,0,250);
	sprintf(syscommand,"ospf_network.sh off %s %s>/dev/null",address,Area);
	system(syscommand);

	

	free(MD5key_id);
	free(Network_type_forcommand);
	free(Area_type);
	free(syscommand);
	free(Cost);
	free(MTU_check);
	free(intfName);
	free(Network_type);
	free(Priority); 
	free(Dead);
	free(Transmit);
	free(Hello);
	free(Retransmit);
	free(Area);
	free(address);

	return 0;
}


char * trim(char * src)
{
         int i = 0;
         char *begin = src;
         while(src[i] != '\0'){
                if(src[i] != ' '){
                      break;
                }else{
                      begin++;
                }
               i++;
         }
          for(i = strlen(src)-1; i >= 0;  i--){
                        if(src[i] != ' '){
                      break;
                }else{
                      src[i] = '\0';
                }
         }
         return begin;
}


