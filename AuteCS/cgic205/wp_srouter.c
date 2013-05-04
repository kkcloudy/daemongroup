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
* wp_srouter.c
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
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include <sys/wait.h>
#define PageNum  10
#define Route_Num 40960
//#define    showrouteCom   "show_route.sh | awk '{OFS="-"}NR==4,NR==0{if($1=="S"){print $1,$2,$5,$6} else if($1=="K>*"){print $1,$2,$4,$5} else if($1=="C>*"){print $1,$2,$6}}' >/var/run/apache2/ip_route.txt"


int ShowRouteListPage();
int ShowIPRoute(char * routeInfo[],int * route_num,struct list *lpublic,struct list *lcontrol);
int ShowIPRoute_For_Number(char * routeInfo[],int * route_num,int * Kroute_num,int * Sroute_num,int * OSPF_num,int * RIP_num,int * Croute_num);


int cgiMain()
{
 ShowRouteListPage();
 return 0;
}

int ShowRouteListPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 	
  char *encry=(char *)malloc(BUF_LEN);
  char *PNtemp=(char *)malloc(10);
  char *SNtemp=(char *)malloc(10);
  char *str;
  FILE *fp;
  char lan[3];
  char routelist_encry[BUF_LEN]; 
  char addn[N];         
  char * IpRouteItem[Route_Num];
  char * IpRouteItem_for_num[Route_Num]; 
  int i;   
  int cl=1;
  	int pageNum=0;
	char * pageNumCA=(char *)malloc(10);
	memset(pageNumCA,0,10);
	char * pageNumCD=(char *)malloc(10);
	memset(pageNumCD,0,10);
	char * pageNumCF=(char *)malloc(10);//first page
	memset(pageNumCF,0,10);
	char * pageNumCL=(char *)malloc(10);//last page
	memset(pageNumCL,0,10);
  	for(i=0;i<Route_Num;i++)
  		{
  			IpRouteItem[i]=(char *)malloc(60);
			memset(IpRouteItem[i],0,60);
			IpRouteItem_for_num[i]=(char *)malloc(60);
			memset(IpRouteItem_for_num[i],0,60);
  		}
  if(cgiFormSubmitClicked("submit_routelist") != cgiFormSuccess)
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
      return 0;
	}
	strcpy(addn,str);
	memset(routelist_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
  
    memset(PNtemp,0,10);
	cgiFormStringNoNewlines("PN",PNtemp,10);
	pageNum=atoi(PNtemp);
	memset(SNtemp,0,10);
	cgiFormStringNoNewlines("SN",SNtemp,10);
  cgiFormStringNoNewlines("encry_routelist",routelist_encry,BUF_LEN);
  if(cgiFormSubmitClicked("submit_routelist") == cgiFormSuccess)
  	cgiFormStringNoNewlines("log_name", addn, N);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"route_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".ShowRoute {overflow-x:hidden;  overflow:auto; width: 566px; height: 286px; clip: rect( ); padding-top: 2px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
  	"</style>"\
  "</head>"\
  "<body>");


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
      "<tr>");
      //"<td width=62 align=center><input id=but type=submit name=submit_routelist style=background-image:url(/images/ok-ch.jpg) value=""></td>"); 
	  if(cgiFormSubmitClicked("submit_routelist") != cgiFormSuccess)
	  	{
        	fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
       		fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
        }
	  else
	  	{
 			fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",routelist_encry,search(lpublic,"img_ok"));
 			fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",routelist_encry,search(lpublic,"img_cancel"));
 		}
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
         			fprintf(cgiOut,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"route_list"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
					if(cgiFormSubmitClicked("submit_routelist") != cgiFormSuccess)
					{
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_staticroute.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"static_route"));					   
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_riproute.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_ospf_bcon.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));
						fprintf(cgiOut,"</tr>");
						#if 1
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_show_access.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"access_list"));
						fprintf(cgiOut,"</tr>");
						#endif
					}
					else if(cgiFormSubmitClicked("submit_routelist") == cgiFormSuccess) 			  
					{
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_staticroute.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",routelist_encry,search(lpublic,"menu_san"),search(lcontrol,"static_route"));						
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_riproute.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",routelist_encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_ospf_bcon.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",routelist_encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));
						fprintf(cgiOut,"</tr>");
						#if 1
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_show_access.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",routelist_encry,search(lpublic,"menu_san"),search(lcontrol,"access_list"));
						fprintf(cgiOut,"</tr>");
						#endif
					}

				for(i=0;i<12;i++)
				{
					fprintf(cgiOut,"<tr height=25>"\
					  "<td id=tdleft>&nbsp;</td>"\
					"</tr>");
				}
				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
						"<table width=740 height=360 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						"<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"route_info"));
						//fprintf(stderr,"444444");
						fprintf(cgiOut,"</tr>"\
						 "<tr>"\
						   "<td align=left valign=top style=padding-top:18px>"\
						   "<div class=ShowRoute><table width=554 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
							 fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 style=font-size:14px align=left>"\
							 "<th width=90 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"route_type"));
							 fprintf(cgiOut,"<th width=124 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"target_IP"));
							 fprintf(cgiOut,"<th width=124 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"next_hop"));	
							 fprintf(cgiOut,"<th width=124 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"PRI_HOPS"));
							 fprintf(cgiOut,"<th width=92 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"relative"));
							 fprintf(cgiOut,"</tr>");
							 int routeNum=0,kroute_num=0,croute_num=0,sroute_num=0,oroute_num=0,rroute_num=0;
							 int FirstPage=0,LastPage=0;
							
							 int k=ShowIPRoute(IpRouteItem,&routeNum,lpublic,lcontrol);

							 if((routeNum%PageNum)==0)
							 	LastPage=(routeNum/PageNum); //计算出最大页数
							 else	
							 	LastPage=(routeNum/PageNum)+1; //计算出最大页数
							 //fprintf(stderr,"LastPage=%d",LastPage);
							 ShowIPRoute_For_Number(IpRouteItem_for_num,&routeNum,&kroute_num,&sroute_num,&oroute_num,&rroute_num,&croute_num);
							 //fprintf(stderr,"kroute_num=%d-croute_num=%d-sroute_num=%d-oroute_num=%d-rroute_num=%d",kroute_num,croute_num,sroute_num,oroute_num,rroute_num);
							 int j;
							 int xnt=0,head=0,tail=0;
							 char * revRouteInfo[4];
							 /*for(q=0;q<4;q++)
								 {
									 revRouteInfo[q]=(char *)malloc(30);
								 }*/
							 //fprintf(stderr,"SNtemp=%s-pageNum=%d",SNtemp,pageNum);
							 if(1==k)
								 {
								 	 if(0==strcmp(SNtemp,"PageFirst"))
								 	 	{
								 	 		if(routeNum-pageNum*PageNum<PageNum)
													xnt=routeNum;
											else	  xnt=(pageNum+1)*PageNum;

											head=0;
											tail=xnt;
								 	 	}
									 else if(0==strcmp(SNtemp,"PageDown") || 0==strcmp(SNtemp,""))
										{
											if(0==strcmp(SNtemp,"PageDown"))
												{
													if(routeNum-pageNum*PageNum<=0)
														{
															pageNum=pageNum-1;
															ShowAlert(search(lcontrol,"Page_end")); 
														}
												}
											if(routeNum-pageNum*PageNum<PageNum)
													xnt=routeNum;
											else	  xnt=(pageNum+1)*PageNum;
												
					 						if(routeNum!=0)
					 							{
													head=pageNum*PageNum;
													tail=xnt;
					 							}
											else
												{
													head=0;
													tail=0;
												}
										}
									else if(0==strcmp(SNtemp,"PageUp"))
										{
											if(pageNum<0)
											{
												pageNum=pageNum+1;
												ShowAlert(search(lcontrol,"Page_Begin"));
											}
 											if(routeNum-pageNum*PageNum<PageNum)
 													xnt=routeNum;
 											else	  xnt=(pageNum+1)*PageNum;
 											
 											//head=pageNum*PageNum;
 											//tail=xnt;
 											if(routeNum!=0)
 					 							{
 													head=pageNum*PageNum;
 													tail=xnt;
 					 							}
 											else
 												{
 													head=0;
 													tail=0;
 												}
										}
									else if(0==strcmp(SNtemp,"PageLast"))
										{
											head = (pageNum-1)*PageNum;
											tail = routeNum;
											pageNum--;
										}
									 for(i=head;i<tail;i++)
										 {
					  
											 revRouteInfo[0]=strtok(IpRouteItem[i],"#,");							 
											 j=0;
											 
											 while(revRouteInfo[j]!=NULL && j<4)
												 {
													 revRouteInfo[j+1]=strtok(NULL,"#,");
													 j++;
												 }
											 
											 if(0==strcmp(revRouteInfo[0],"C>*") || 0==strcmp(revRouteInfo[0],"C") ||  0==strcmp(revRouteInfo[0],"C>") ||  0==strcmp(revRouteInfo[0],"C*"))
												 {
    												 fprintf(cgiOut,"<tr height=25 bgcolor=%s align=left>",setclour(cl));
    												 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[0]);
    												 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[1]);
    												 fprintf(cgiOut,"<td style=font-size:12px align=left>directly connect</td>");
    												 fprintf(cgiOut,"<td style=font-size:12px align=left>None</td>");
    												 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[2]);
    												 fprintf(cgiOut,"</tr>");
												 }
											 else{
    												 fprintf(cgiOut,"<tr height=25 bgcolor=%s align=left>",setclour(cl));
     												 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[0]);
     												 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[1]);
     												 if((0==strcmp(revRouteInfo[0],"S>*") || 0==strcmp(revRouteInfo[0],"S") || 0==strcmp(revRouteInfo[0],"S>")) && 0==strcmp(revRouteInfo[2],"directly"))
														fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[2]);
     												 else
     												 	fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[2]);
     												 if(0==strcmp(revRouteInfo[0],"K>*") || 0==strcmp(revRouteInfo[0],"K") || 0==strcmp(revRouteInfo[0],"K>"))
														fprintf(cgiOut,"<td style=font-size:12px align=left>None</td>");
													 else 
     												 	fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[4]);

     												
     												 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[3]);
     												 fprintf(cgiOut,"</tr>");
												 }
												 cl=!cl;
										 }
													 
								 }
							 
						   fprintf(cgiOut,"</table></div>"\
							 "</td>"\
							 
							 "<td valign=top style=padding-left:15px;padding-top:18px>"\
							 "<table width=160 border=1  bordercolor=#cccccc cellspacing=0 cellpadding=0>"\
							 "<tr bgcolor=%s height=40 align=center>",setclour(cl));
							 fprintf(cgiOut,"<th width=80 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"route_type"));
							 fprintf(cgiOut,"<th width=80 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"route_num"));
							 fprintf(cgiOut,"</tr>");
							 cl=!cl;
							 fprintf(cgiOut,"<tr bgcolor=%s  height=35 align=center>",setclour(cl));
							 fprintf(cgiOut,"<td>%s:</td>",search(lcontrol,"kernel"));
							 fprintf(cgiOut,"<td>%d</td>",kroute_num);
							 fprintf(cgiOut,"</tr>");
							 cl=!cl;
							 fprintf(cgiOut,"<tr bgcolor=%s  height=35 align=center>",setclour(cl));
							 fprintf(cgiOut,"<td>%s:</td>",search(lcontrol,"connected"));
							 fprintf(cgiOut,"<td>%d</td>",croute_num);
							 fprintf(cgiOut,"</tr>");
							 cl=!cl;
							 fprintf(cgiOut,"<tr bgcolor=%s  height=35 align=center>",setclour(cl));
							 fprintf(cgiOut,"<td>%s:</td>","OSPF");
							 fprintf(cgiOut,"<td>%d</td>",oroute_num);
							 fprintf(cgiOut,"</tr>");
							 cl=!cl;
							 fprintf(cgiOut,"<tr bgcolor=%s height=35 align=center>",setclour(cl));
							 fprintf(cgiOut,"<td>%s:</td>","RIP");
							 fprintf(cgiOut,"<td>%d</td>",rroute_num);
							 fprintf(cgiOut,"</tr>");
							 cl=!cl;
							 fprintf(cgiOut,"<tr bgcolor=%s height=35 align=center>",setclour(cl));
							 fprintf(cgiOut,"<td>%s:</td>",search(lcontrol,"static"));
							 fprintf(cgiOut,"<td>%d</td>",sroute_num);
							 fprintf(cgiOut,"</tr>");
							 cl=!cl;

							 fprintf(cgiOut,"<tr bgcolor=%s height=35 align=center>",setclour(cl));
							 fprintf(cgiOut,"<td>%s:</td>",search(lcontrol,"total"));
							 fprintf(cgiOut,"<td>%d</td>",routeNum);
							 fprintf(cgiOut,"</tr>"\
							 
							 "</table>"\
							 "</td>"\
						   "</tr>"\
						 
						"<tr>"\
						"<td id=sec1 colspan=2 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
						fprintf(cgiOut,"</tr>"\
						 "<tr height=25 style=padding-top:2px>"\
			   			"<td colspan=2 style=font-size:14px;color:#FF0000> K - kernel route, C - connected, S - static, R - RIP, O - OSPF,I - ISIS, B - BGP, > - selected route, * - FIB route</td>"\
			   			"</tr>"\
			   			
						 "<tr>"\
						"<td colspan=2>"\
						"<table width=430 style=padding-top:2px>"\
						"<tr  height=30>");
						sprintf(pageNumCF,"%d",FirstPage);
						sprintf(pageNumCA,"%d",pageNum+1);
						sprintf(pageNumCD,"%d",pageNum-1);
						sprintf(pageNumCL,"%d",LastPage);
						if(cgiFormSubmitClicked("submit_routelist") != cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCF,"PageFirst",search(lcontrol,"Page_First"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCL,"PageLast",search(lcontrol,"Page_Last"));
							}
						else if(cgiFormSubmitClicked("submit_routelist") == cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",routelist_encry,pageNumCF,"PageFirst",search(lcontrol,"Page_First"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",routelist_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));  
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",routelist_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",routelist_encry,pageNumCL,"PageLast",search(lcontrol,"Page_Last"));
							}
						fprintf(cgiOut,"</tr>"\
						"<tr height=30  align=center valign=bottom>"\
						"<td colspan=4>%s%d%s(%s%d%s)</td>",search(lpublic,"current_sort"),pageNum+1,search(lpublic,"page"),search(lpublic,"total"),LastPage,search(lpublic,"page"));
						fprintf(cgiOut,"</tr>"\
						"</table></td>"\
						"</tr>"\
						 "<tr>");
						 if(cgiFormSubmitClicked("submit_routelist") != cgiFormSuccess)
							 {
							   fprintf(cgiOut,"<td><input type=hidden name=encry_routelist value=%s></td>",encry);
							 }
						 else if(cgiFormSubmitClicked("submit_routelist") == cgiFormSuccess)
							 {
							   fprintf(cgiOut,"<td><input type=hidden name=encry_routelist value=%s></td>",routelist_encry);
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

#if 0
if(strcmp(SNtemp,"")!=0 && strcmp(PNtemp,"")!=0)
{
	if(cgiFormSubmitClicked("submit_delroute") != cgiFormSuccess)
	{
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_srouter.cgi?UN=%s&PN=%d';\n", encry,pageNum);
		fprintf( cgiOut, "</script>\n" );
	}
	else
	{
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_srouter.cgi?UN=%s&PN=%d';\n", routelist_encry,pageNum);
		fprintf( cgiOut, "</script>\n" );
	}
}
#endif
free(encry);
for(i=0;i<Route_Num;i++)
{
	free(IpRouteItem[i]);
	free(IpRouteItem_for_num[i]);
}

free(PNtemp);
free(SNtemp);
free(pageNumCA);
free(pageNumCD);
free(pageNumCF);
free(pageNumCL);
release(lpublic);  
release(lcontrol);

return 0;
}
						 
int ShowIPRoute(char * routeInfo[],int * route_num,struct list *lpublic,struct list *lcontrol)
{
	int i;
	char * showRoute=(char *)malloc(350);
	memset(showRoute,0,350);
						 
	char  routePath[128];
	memset(routePath,0,128);					 
	strcat(showRoute,"show_route.sh 2>/dev/null | awk '{OFS=\"#\"}NR==4,NR==0{if($1~/S/){print $1,$2,$5,$6,$3} else if($1~/K/){print $1,$2,$4,$5,$3} else if($1~/C/){print $1,$2,$6,$3} else if($1~/R/){print $1,$2,$5,$6,$3} else if($1~/O/){print $1,$2,$5,$7,$3}}' ");
	//int status = system(showRoute);
	//int ret = WEXITSTATUS(status);
	//fprintf(stderr,"showRoute=%s",showRoute);
	//if(0 != ret)
	//	ShowAlert(search(lcontrol,"show_route_fail"));
	FILE *fd;
	char  temp[60];
	memset(temp,0,60);
							 
	if((fd=popen(showRoute,"r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i=0;
	while((fgets(temp,60,fd)) != NULL)
		{
			//fprintf(stderr,"temp=%s",temp);
			strcpy(routeInfo[i],temp);
			i++;
			memset(temp,0,60);
		}
	*route_num=i;

	pclose(fd);
	free(showRoute);
return 1;
}

int ShowIPRoute_For_Number(char * routeInfo[],int * route_num,int * Kroute_num,int * Sroute_num,int * OSPF_num,int * RIP_num,int * Croute_num)
{
	int i=0,c=0,k=0,s=0,o=0,r=0,j=0;
	char * showRoute=(char *)malloc(350);
	memset(showRoute,0,350);

	char  routePath[128];
	memset(routePath,0,128);
	char * revRouteInfo[4];

	strcat(showRoute,"show_route.sh 2>/dev/null | awk '{OFS=\"-\"}NR==4,NR==0{if($1~/S/){print $1,$2,$5,$6,$3,$7} else if($1~/K/){print $1,$2,$4,$5,$3} else if($1~/C/){print $1,$2,$6,$3} else if($1~/R/){print $1,$2,$5,$6,$3} else if($1~/O/){print $1,$2,$5,$6,$3}}'>/var/run/apache2/ip_route.txt");
	system(showRoute);

	FILE *fd;
	char  temp[60];
	memset(temp,0,60);
	if((fd=fopen("/var/run/apache2/ip_route.txt","r"))==NULL)
		{
			//ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i=0;

	while((fgets(temp,60,fd)) != NULL)
		{
			strcat(routeInfo[i],temp);
			i++;
			memset(temp,0,60);
		}
	fclose(fd);
	*route_num=i;


	for(i=0;i<*route_num;i++)
		{

			revRouteInfo[0]=strtok(routeInfo[i],"-,");
			//fprintf(stderr,"555revRouteInfo[0]=%s44444",revRouteInfo[0]);
			j=0;
			
			while(revRouteInfo[j]!=NULL && j<3)
				{
					revRouteInfo[j+1]=strtok(NULL,"-,");
					j++;
				}
			if(strstr(revRouteInfo[0],"C")!=NULL)
			c++;
			else if(strstr(revRouteInfo[0],"K")!=NULL)
				k++;
			else if(strstr(revRouteInfo[0],"S")!=NULL)
				s++;
			else if(strstr(revRouteInfo[0],"O")!=NULL)
				o++;
			else if(strstr(revRouteInfo[0],"R")!=NULL)
				r++;
		}

	*Sroute_num=s;
	*Croute_num=c;
	*Kroute_num=k;
	*OSPF_num=o;
	*RIP_num=r;

	free(showRoute);

	
	return 1;
}


