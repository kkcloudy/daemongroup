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
* wp_staticroute.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* system contrl for static route config 
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
#define Route_Num 4096

void DeleteRoutePage();
int   delete_Route_Operation(struct list *lcontrol,char * param1,char * param2,char * param3);
int ShowIPRoute(char * routeInfo[],int * route_num,struct list * lpublic,struct list * lcontrol);


int cgiMain()
{
	DeleteRoutePage();
	return 0;
}


void DeleteRoutePage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	char *encry=(char *)malloc(BUF_LEN);			   /*存储从wp_usrmag.cgi带入的加密字符串*/
	char *PNtemp=(char *)malloc(10);
  	char *SNtemp=(char *)malloc(10);
	char *str=NULL;
	FILE *fp1;
	char lan[3];
	char delroute_encry[BUF_LEN]; 
	char * IpRouteItem[Route_Num]; 
	int i;   
	int cl=1;
	int pageNum=0;
	char * pageNumCA=(char *)malloc(10);
	memset(pageNumCA,0,10);
	char * pageNumCD=(char *)malloc(10);
	memset(pageNumCD,0,10);
	char * pageNumCF=(char *)malloc(10);
	memset(pageNumCF,0,10);
	char * pageNumCL=(char *)malloc(10);
	memset(pageNumCL,0,10);
	char * deleteOP=(char *)malloc(10);
	memset(deleteOP,0,10);
	char * RevParamLATER=(char *)malloc(50);
	memset(RevParamLATER,0,50);
	char * Param1=(char * )malloc(30);
	memset(Param1,0,30);
	char * Param2=(char * )malloc(30);
	memset(Param2,0,30);
	char * Param3=(char * )malloc(30);
	memset(Param3,0,30);

	char * CheckUsr=(char * )malloc(10);
	memset(CheckUsr,0,10);
	int retu=0;
	 for(i=0;i<Route_Num;i++)
		 {
			 IpRouteItem[i]=(char *)malloc(60);
			 memset(IpRouteItem[i],0,60);
		 }
	 if(cgiFormSubmitClicked("submit_delroute") != cgiFormSuccess)
	 {
	   memset(encry,0,BUF_LEN);
	   cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	   str=dcryption(encry);
	   if(str==NULL)
	   {
		 ShowErrorPage(search(lpublic,"ill_user"));	/*用户非法*/
		 return;
	   }
	   memset(delroute_encry,0,BUF_LEN); 				  /*清空临时变量*/
	 }
	 memset(PNtemp,0,10);
	 cgiFormStringNoNewlines("PN",PNtemp,10);
	 pageNum=atoi(PNtemp);
	 memset(SNtemp,0,10);
	 cgiFormStringNoNewlines("SN",SNtemp,10);
	 //fprintf(stderr,"PNtemp=%s-SNtemp=%s",PNtemp,SNtemp);
	 cgiFormStringNoNewlines("DELETE",deleteOP,10);
	 cgiFormStringNoNewlines("DELPARAM",RevParamLATER,50); 
	 cgiFormStringNoNewlines("CheckUsr",CheckUsr,50);
	 if(strcmp(CheckUsr,"")!=0)
	 	retu=atoi(CheckUsr);
	 
  	 cgiFormStringNoNewlines("encry_del",delroute_encry,BUF_LEN);
     cgiHeaderContentType("text/html");
     fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
     fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
     fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"route_manage"));
     fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	 "<style type=text/css>"\
		  "#div1{ width:82px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
		  "#div2{ width:80px; height:15px; padding-left:5px; padding-top:3px}"\
		  "#link{ text-decoration:none; font-size: 12px}"\
		  ".delroute {overflow-x:hidden;	overflow:auto; width: 750px; height: 286px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	"</style>"\

  "</head>"\
  "<body>");
  if(strcmp(deleteOP,"delete")==0)
  {
  	 Param1=strtok(RevParamLATER,"!");
	 Param2=strtok(NULL,"!");
	 if( Param2 != NULL )
	 	Param3=strtok(NULL,"!");
	 
	 //fprintf(stderr,"Param1=%s-Param2=%s--Param3=%s",Param1,Param2,Param3);
  	 delete_Route_Operation(lcontrol,Param1,Param2,Param3);
  }
  if(cgiFormSubmitClicked("submit_delroute") != cgiFormSuccess)
  {
  	 retu=checkuser_group(str);
  }
  else if(cgiFormSubmitClicked("submit_delroute") == cgiFormSuccess)
	{
		 fprintf( cgiOut, "<script type='text/javascript'>\n" );
     	 fprintf( cgiOut, "window.location.href='wp_srouter.cgi?UN=%s';\n", delroute_encry);
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
	    if((fp1=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
	    {
			ShowAlert(search(lpublic,"error_open"));
	    }
		else
		{
			fseek(fp1,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
			fgets(lan,3,fp1);	   
			fclose(fp1);
		}

    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_delroute style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_delroute") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",delroute_encry,search(lpublic,"img_cancel"));
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
            		if(cgiFormSubmitClicked("submit_delroute") != cgiFormSuccess)
            		{
            		  if(retu==0)
            		  {
                 		  fprintf(cgiOut,"<tr height=26>"\
                 			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"static_route_list"));	 /*突出显示*/
                 		  fprintf(cgiOut,"</tr>");
                 		  fprintf(cgiOut,"<tr height=25>"\
                 			"<td align=left id=tdleft><a href=wp_addroute.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_route"));
                 		  fprintf(cgiOut,"</tr>");
            		  }
            		  else
            		  {
            		  	fprintf(cgiOut,"<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"static_route_list"));	 /*突出显示*/
            		  	fprintf(cgiOut,"</tr>");
            		  }
            		}
            		else if(cgiFormSubmitClicked("submit_delroute") == cgiFormSuccess)
            		{ 
            		  if(retu==0)
            		  {
                		  fprintf(cgiOut,"<tr height=26>"\
                			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"static_route_list"));	 /*突出显示*/
                		  fprintf(cgiOut,"</tr>");
                		  fprintf(cgiOut,"<tr height=25>"\
                			"<td align=left id=tdleft><a href=wp_addroute.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",delroute_encry,search(lpublic,"menu_san"),search(lcontrol,"add_route"));
                		  fprintf(cgiOut,"</tr>");
            		  }
            		  else
            		  {
            		  	fprintf(cgiOut,"<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"static_route_list"));	 /*突出显示*/
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
						"<table width=640 height=360 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"static_route_info"));
						fprintf(cgiOut,"</tr>"\
						 "<tr>"\
						   "<td align=left valign=top style=\"padding-top:18px\">"\
						   "<div class=delroute><table width=670 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>"\
								 "<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
								  
								  fprintf(cgiOut,"<th width=100 style=font-size:14px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"route_type"));
								  fprintf(cgiOut,"<th width=100 style=font-size:14px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"target_IP"));
								  fprintf(cgiOut,"<th width=100  style=font-size:14px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"next_hop"));
								  fprintf(cgiOut,"<th width=124 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"PRI_HOPS"));
								  fprintf(cgiOut,"<th width=100 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"operation"));
							 	  fprintf(cgiOut,"<th width=70 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"relative"));
								  fprintf(cgiOut,"<th width=60 >&nbsp;</th>");
								  fprintf(cgiOut,"</tr>");
								  
								   int routeNum=0,FirstPage=0,LastPage=0;
								   i=0;
								   int k=ShowIPRoute(IpRouteItem,&routeNum,lpublic,lcontrol);
								   if((routeNum%PageNum)==0)
    								   LastPage=(routeNum/PageNum); //计算出最大页数
    							   else	
    								   LastPage=(routeNum/PageNum)+1; //计算出最大页数
								   //fprintf(stderr,"routeNum=%d",routeNum);
								   int j;
							 		int xnt=0,head=0,tail=0;
								   char * revRouteInfo[7];

							 if(1==k)
								 {
								 	//int s=0;  /*控制显示    当前没有可以删除的静态路由     这句话*/
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
										//fprintf(stderr,"head=%d-tail=%d",head,tail);
										sprintf(pageNumCA,"%d",pageNum);
									 for(i=head;i<tail;i++)
										{
                    							revRouteInfo[0]=strtok(IpRouteItem[i],"#,");							
                    							j=0;
                    								while(revRouteInfo[j]!=NULL && j<5)
                    								{                    									
                    									revRouteInfo[j+1]=strtok(NULL,"#,");
                    									j++;
                    								}
                    							if(strstr(revRouteInfo[0],"S")!=NULL)
                    								{
                    									if(strcmp(revRouteInfo[2],"directly")==0)
                    										{
                    											char * RevParam=(char *)malloc(50);
            													memset(RevParam,0,50);
            													strcat(RevParam,revRouteInfo[1]);
            													strcat(RevParam,"!");
            													strcat(RevParam,revRouteInfo[5]);
																revRouteInfo[6]=strtok(NULL,"#,");
            													//fprintf(stderr,"revRouteInfo[5]=%s",revRouteInfo[5]);
                                								fprintf(cgiOut,"<tr height=25 bgcolor=%s  align=left>",setclour(cl));
                                								//fprintf(cgiOut,"<td><input type=radio name=radiobutton value=\"%s\"></td>",RevParam);
                                								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[0]);
                                								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[1]);
                                								fprintf(cgiOut,"<td style=font-size:12px align=left>%s %s</td>",revRouteInfo[2],revRouteInfo[3]);
                                								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[4]);
                                								if(strstr(revRouteInfo[6],"rej")!=NULL)
                                									fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","reject");
                                								else if(strstr(revRouteInfo[6],"bh")!=NULL)
                                									fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","blackhole");
                                								else
                                									fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","none");
                                								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[5]);
                                								if(retu==0)
                                									fprintf(cgiOut,"<td><a href=wp_staticroute.cgi?UN=%s&DELETE=%s&DELPARAM=%s&PN=%s target=mainFrame onclick=\"return confirm('%s')\" style=text-decoration:underline>%s</td>",encry,"delete",RevParam,pageNumCA,search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
                                								else
                                									fprintf(cgiOut,"<td>&nbsp;</td>");
                                								fprintf(cgiOut,"</tr>");
        														//s=1;
            													free(RevParam);
                    										}
														else
															{
                                								char * RevParam=(char *)malloc(50);
            													memset(RevParam,0,50);
            													strcat(RevParam,revRouteInfo[1]);
            													strcat(RevParam,"!");
            													strcat(RevParam,revRouteInfo[2]);
																if(strstr(revRouteInfo[5],"equalize")!=NULL)
																{
																	strcat(RevParam,"!");
        															strcat(RevParam,revRouteInfo[5]);
																}
            													//fprintf(stderr,"revRouteInfo[5]=%s",revRouteInfo[5]);
                                								fprintf(cgiOut,"<tr height=25 bgcolor=%s  align=left>",setclour(cl));
                                								//fprintf(cgiOut,"<td><input type=radio name=radiobutton value=\"%s\"></td>",RevParam);
                                								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[0]);
                                								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[1]);
                                								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[2]);
                                								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[4]);
                                								if(strstr(revRouteInfo[5],"rej")!=NULL)
                                									fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","reject");
                                								else if(strstr(revRouteInfo[5],"bh")!=NULL)
                                									fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","blackhole");
                                								else if(strstr(revRouteInfo[5],"equalize")!=NULL)
																	fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","equalize");
																else
                                									fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>","none");
                                								fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",revRouteInfo[3]);
                                								if(retu==0)
                                									fprintf(cgiOut,"<td><a href=wp_staticroute.cgi?UN=%s&DELETE=%s&DELPARAM=%s&PN=%s target=mainFrame onclick=\"return confirm('%s')\" style=text-decoration:underline>%s</td>",encry,"delete",RevParam,pageNumCA,search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
                                								else
                                									fprintf(cgiOut,"<td>&nbsp;</td>");
                                								fprintf(cgiOut,"</tr>");
        														//s=1;
            													free(RevParam);
															}
                    								}
                    							else{
                    							}
                        							cl=!cl;
                    						}
													 
								 }
							 else
							 	{
							 			ShowAlert(search(lpublic,"contact_adm"));
							 	}
							 
						   fprintf(cgiOut,"</table></div>"\
							 "</td>"\
						   "</tr>"\
						
						 "<tr>"\
						"<td>"\
						"<table width=430 style=padding-top:2px>"\
						"<tr height=30>");
						memset(pageNumCA,0,10);
						sprintf(pageNumCF,"%d",FirstPage);
						sprintf(pageNumCA,"%d",pageNum+1);
						sprintf(pageNumCD,"%d",pageNum-1);
						sprintf(pageNumCL,"%d",LastPage);
						if(cgiFormSubmitClicked("submit_delroute") != cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_staticroute.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCF,"PageFirst",search(lcontrol,"Page_First"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_staticroute.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_staticroute.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_staticroute.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCL,"PageLast",search(lcontrol,"Page_Last"));
							}
						else if(cgiFormSubmitClicked("submit_delroute") == cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_staticroute.cgi?UN=%s&PN=%s&SN=%s>%s</td>",delroute_encry,pageNumCF,"PageFirst",search(lcontrol,"Page_First"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_staticroute.cgi?UN=%s&PN=%s&SN=%s>%s</td>",delroute_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_staticroute.cgi?UN=%s&PN=%s&SN=%s>%s</td>",delroute_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_staticroute.cgi?UN=%s&PN=%s&SN=%s>%s</td>",delroute_encry,pageNumCL,"PageLast",search(lcontrol,"Page_Last"));
							}
						fprintf(cgiOut,"</tr>"\
						"<tr height=30  align=center valign=bottom>"\
						"<td colspan=4>%s%d%s(%s%d%s)</td>",search(lpublic,"current_sort"),pageNum+1,search(lpublic,"page"),search(lpublic,"total"),LastPage,search(lpublic,"page"));
						fprintf(cgiOut,"</tr>"\
						"</table></td>"\
						"</tr>"\
						 "<tr>");
						 if(cgiFormSubmitClicked("submit_delroute") != cgiFormSuccess)
						 {
						   fprintf(cgiOut,"<td><input type=hidden name=encry_del value=%s></td>",encry);
						   fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
						 }
						 else if(cgiFormSubmitClicked("submit_delroute") == cgiFormSuccess)
							 {
							   fprintf(cgiOut,"<td><input type=hidden name=encry_del value=%s></td>",delroute_encry);
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
if(strcmp(SNtemp,"")!=0 && strcmp(PNtemp,"")!=0)
{
	if(cgiFormSubmitClicked("submit_delroute") != cgiFormSuccess)
	{
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_staticroute.cgi?UN=%s&PN=%d';\n", encry,pageNum);
		fprintf( cgiOut, "</script>\n" );
	}
	else
	{
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_staticroute.cgi?UN=%s&PN=%d';\n", delroute_encry,pageNum);
		fprintf( cgiOut, "</script>\n" );
	}
}
free(encry);
free(PNtemp);
free(CheckUsr);
free(SNtemp);
free(pageNumCA);
free(pageNumCD);
free(pageNumCF);
free(pageNumCL);
free(deleteOP);
free(RevParamLATER);
free(Param1);
free(Param2);
free(Param3);

for(i=0;i<Route_Num;i++)
{
	free(IpRouteItem[i]);
}
release(lpublic);  
release(lcontrol);

}
															 
int   delete_Route_Operation(struct list *lcontrol,char * param1,char * param2,char * param3)
{

	char command[100];
	memset(command,0,100);
	int ret,state;

	strcat(command,"del_route.sh");
	strcat(command," ");
	strcat(command,param1);
	strcat(command," ");
	strcat(command,param2);
	if( param3 != NULL )
	{
		strcat(command," ");
		strcat(command,param3);
	}
	strcat(command," ");
	strcat(command,">/dev/null");
	state=system(command);
	ret = WEXITSTATUS(state);
	//fprintf(stderr,"command=%s",command);
	if(0==ret)
		{
			ShowAlert(search(lcontrol,"del_route_suc"));
															 
		}
	else
		{
			ShowAlert(search(lcontrol,"del_route_fail"));
		}


	return 1;
}
															 
int ShowIPRoute(char * routeInfo[],int * route_num,struct list * lpublic,struct list * lcontrol)
{
	int i;
	char * showRoute=(char *)malloc(350);
	memset(showRoute,0,350);
	char	routePath[128];
	memset(routePath,0,128);
																  
	strcat(showRoute,"show_route.sh 2>/dev/null | awk '{OFS=\"#\"}NR==4,NR==0{if($1~/S/){print $1,$2,$5,$6,$3,$7,$8} }'>/var/run/apache2/ip_route.txt");
	system(showRoute);
	FILE *fd;
	char	temp[60];
	memset(temp,0,60);
																  
	if((fd=fopen("/var/run/apache2/ip_route.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i=0;
 	while((fgets(temp,60,fd)) != NULL)
		{
			strcat(routeInfo[i],temp);
			i++;
			memset(temp,0,60);
			//if(i>=255)
			//{
			//	ShowAlert(search(lcontrol,"static_route_num_error"));
			//	*route_num=i-1;
			//	free(showRoute);
			//	return 1;
			//}
				
		}
	fclose(fd);
	*route_num=i;
	//fprintf(stderr,"route_num=%d",*route_num);
	free(showRoute);
															 
	return 1;
}




