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
* wp_show_access.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* system contrl for show access
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


#define MAX_ACCESS_LIST 128
#define PageNum  10
#define MAX_OPER_LENTG  10
#define MAX_ACCESS_ID_LENTH  16
#define MAX_IP_LENTG  	32

struct AccessInfo_st {
	unsigned int access_number;
	char  oper[MAX_OPER_LENTG];
	char  ipaddr[MAX_IP_LENTG];
};


int ShowAccessListPage();
int ShowRouteAccess(struct AccessInfo_st stAccess[],int * access_num,struct list *lpublic,struct list *lcontrol);
int del_access(char * accessID,char *operate,char* ip_address,struct list *lcontrol);


int cgiMain()
{
 ShowAccessListPage();
 return 0;
}

int ShowAccessListPage()
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
  char routelist_encry[BUF_LEN]; 
  char addn[N];         
  char * IpRouteItem[MAX_ACCESS_LIST];
  char * IpRouteItem_for_num[MAX_ACCESS_LIST]; 
  int i;   
  int cl=1;
    int retu=0;
	char accID[MAX_ACCESS_ID_LENTH];
	memset(accID, 0, MAX_ACCESS_ID_LENTH);
	char oper[10];
	memset(oper,0,10);
	char ip_addr[20];
	memset(ip_addr,0,10);	
	char  i_char[MAX_ACCESS_ID_LENTH];
	memset(i_char, 0, MAX_ACCESS_ID_LENTH);
	char  menu[MAX_ACCESS_ID_LENTH+10];
	memset(menu, 0, MAX_ACCESS_ID_LENTH+10);
  	int pageNum=0;
	char del_rule[10];
	memset(del_rule, 0, 10);
	
	char * pageNumCA=(char *)malloc(10);
	memset(pageNumCA,0,10);
	char * pageNumCD=(char *)malloc(10);
	memset(pageNumCD,0,10);
	char * pageNumCF=(char *)malloc(10);//first page
	memset(pageNumCF,0,10);
	char * pageNumCL=(char *)malloc(10);//last page
	memset(pageNumCL,0,10);
  	for(i=0;i<MAX_ACCESS_LIST;i++)
  		{
  			IpRouteItem[i]=(char *)malloc(60);
			memset(IpRouteItem[i],0,60);
			IpRouteItem_for_num[i]=(char *)malloc(60);
			memset(IpRouteItem_for_num[i],0,60);
  		}

///////////add 2009-5-21////////////////////////
	struct AccessInfo_st pShowAccess[MAX_ACCESS_LIST];

	
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
 
  	retu=checkuser_group(str);
    memset(PNtemp,0,10);
	cgiFormStringNoNewlines("PN",PNtemp,10);
	pageNum=atoi(PNtemp);
	memset(SNtemp,0,10);
	cgiFormStringNoNewlines("SN",SNtemp,10);
  cgiFormStringNoNewlines("encry_routelist",routelist_encry,BUF_LEN);
  if(cgiFormSubmitClicked("submit_routelist") == cgiFormSuccess)
  {
	fprintf( cgiOut, "<script type='text/javascript'>\n" );
	fprintf( cgiOut, "window.location.href='wp_srouter.cgi?UN=%s';\n", routelist_encry);
   	fprintf( cgiOut, "</script>\n" );

  }
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"route_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	"#div1{ width:42px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:40px; height:15px; padding-left:5px; padding-top:3px}"\
    "#link{ text-decoration:none; font-size: 12px}"\
  	".ShowAccess {overflow-x:hidden;  overflow:auto; width: 396px; height: 286px; clip: rect( ); padding-top: 2px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
  	"</style>"\
  "</head>"\
  "<script type=\"text/javascript\">"\
      "function popMenu(objId)"\
      "{"\
         "var obj = document.getElementById(objId);"\
         "if (obj.style.display == 'none')"\
    	 "{"\
           "obj.style.display = 'block';"\
    	 "}"\
    	 "else"\
    	 "{"\
           "obj.style.display = 'none';"\
    	 "}"\
	 "}"\
	 "</script>"\
  "<body>");
  cgiFormStringNoNewlines("DELRULE",del_rule,10);
  cgiFormStringNoNewlines("ACCESSID",accID,MAX_ACCESS_ID_LENTH);
  cgiFormStringNoNewlines("OPER",oper,10);
  cgiFormStringNoNewlines("IPADDR",ip_addr,20);
	if(!strcmp(del_rule, "delete"))
		{
			del_access(accID,oper,ip_addr,lcontrol);
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
      "<tr>");
      //"<td width=62 align=center><input id=but type=submit name=submit_routelist style=background-image:url(/images/ok-ch.jpg) value=""></td>"); 
	  if(cgiFormSubmitClicked("submit_routelist") != cgiFormSuccess)
	  	{
        	fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
       		fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
        }
	  else
	  	{
 			fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",routelist_encry,search(lpublic,"img_ok"));
 			fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",routelist_encry,search(lpublic,"img_cancel"));
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
					if(cgiFormSubmitClicked("submit_routelist") != cgiFormSuccess)
					{
						fprintf(cgiOut,"<tr height=26>"\
         				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"show_access"));   /*突出显示*/
         				fprintf(cgiOut,"</tr>");
						if(retu == 0)
						{
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_add_access.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_access"));
							fprintf(cgiOut,"</tr>");
						}
						
					}
					else if(cgiFormSubmitClicked("submit_routelist") == cgiFormSuccess) 			  
					{
						fprintf(cgiOut,"<tr height=26>"\
         				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"show_access"));   /*突出显示*/
         				fprintf(cgiOut,"</tr>");
						if(retu == 0)
						{
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_add_access.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",routelist_encry,search(lpublic,"menu_san"),search(lcontrol,"add_access"));
							fprintf(cgiOut,"</tr>");
						}
						
					}

				for(i=0;i<15;i++)
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
						   "<div class=ShowAccess><table width=354 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
							 fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 style=font-size:14px align=left>");
							 fprintf(cgiOut,"<th width=124 style=font-size:14px><font id=%s>ACCESS LIST</font></th>", search(lpublic,"menu_thead"));
							 fprintf(cgiOut,"<th width=124 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"operation"));	
							 fprintf(cgiOut,"<th width=124 style=font-size:14px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"ip_addr"));
							 fprintf(cgiOut,"<th width=13>&nbsp;</th>");
							 fprintf(cgiOut,"</tr>");
							 int accessNum=0;
							 int FirstPage=0,LastPage=0;
							 
							 int k=ShowRouteAccess(pShowAccess,&accessNum,lpublic,lcontrol);

							 if((accessNum%PageNum)==0)
							 	LastPage=(accessNum/PageNum); //计算出最大页数
							 else	
							 	LastPage=(accessNum/PageNum)+1; //计算出最大页数
							 	
							 //ShowIPRoute_For_Number(IpRouteItem_for_num,&accessNum,&kroute_num,&sroute_num,&oroute_num,&rroute_num,&croute_num);
							 
							 int xnt=0,head=0,tail=0;
							 //char * revRouteInfo[4];
							 /*for(q=0;q<4;q++)
								 {
									 revRouteInfo[q]=(char *)malloc(30);
								 }*/
							 //fprintf(stderr,"SNtemp=%s-pageNum=%d",SNtemp,pageNum);
							 if( 0 == k )
								 {
								 	 if(0==strcmp(SNtemp,"PageFirst"))
								 	 	{
								 	 		if(accessNum-pageNum*PageNum<PageNum)
													xnt=accessNum;
											else	  xnt=(pageNum+1)*PageNum;

											head=0;
											tail=xnt;
								 	 	}
									 else if(0==strcmp(SNtemp,"PageDown") || 0==strcmp(SNtemp,""))
										{
											if(0==strcmp(SNtemp,"PageDown"))
												{
													if(accessNum-pageNum*PageNum<=0)
														{
															pageNum=pageNum-1;
															ShowAlert(search(lcontrol,"Page_end")); 
														}
												}
											if(accessNum-pageNum*PageNum<PageNum)
													xnt=accessNum;
											else	  xnt=(pageNum+1)*PageNum;
												
					 						if(accessNum!=0)
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
 											if(accessNum-pageNum*PageNum<PageNum)
 													xnt=accessNum;
 											else	  xnt=(pageNum+1)*PageNum;
 											
 											//head=pageNum*PageNum;
 											//tail=xnt;
 											if(accessNum!=0)
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
											tail = accessNum;
											pageNum--;
										}
									 for(i=head;i<tail;i++)
									 	{
									 		memset(menu,0,21);   
	                                        strcpy(menu,"menulist");
	                                        sprintf(i_char,"%d",i+1);
	                                    	strcat(menu,i_char);
									 		fprintf(cgiOut,"<tr height=25 bgcolor=%s align=left>",setclour(cl));
											fprintf(cgiOut,"<td>%d</td>",pShowAccess[i].access_number);
											fprintf(cgiOut,"<td>%s</td>",pShowAccess[i].oper);
											fprintf(cgiOut,"<td>%s</td>",pShowAccess[i].ipaddr);
											#if 1
											fprintf(cgiOut,"<td>");
											if(retu==0)  /*管理员*/
            						   		{
												fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(tail-i),menu,menu);
	                                            fprintf(cgiOut,"<img src=/images/detail.gif>"\
	                                            "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
	 											fprintf(cgiOut,"<div id=div1>");
	                                	   		
	                                            fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_show_access.cgi?UN=%s&ACCESSID=%u&OPER=%s&IPADDR=%s&DELRULE=%s&PN=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,pShowAccess[i].access_number,pShowAccess[i].oper,pShowAccess[i].ipaddr,"delete",pageNumCA,search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
	            						   		
	                                              
	                                    	    fprintf(cgiOut,"</div>"\
	                    					    "</div>"\
	                    					    "</div>");
											}
											fprintf(cgiOut,"</td>");
											#endif
											fprintf(cgiOut,"</tr>");
									 		cl = !cl;
									 	}
													 
								 }
							 
						   fprintf(cgiOut,"</table></div>"\
							 "</td>"\
							 
							
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
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_access.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCF,"PageFirst",search(lcontrol,"Page_First"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_access.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_access.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_access.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCL,"PageLast",search(lcontrol,"Page_Last"));
							}
						else if(cgiFormSubmitClicked("submit_routelist") == cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_access.cgi?UN=%s&PN=%s&SN=%s>%s</td>",routelist_encry,pageNumCF,"PageFirst",search(lcontrol,"Page_First"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_access.cgi?UN=%s&PN=%s&SN=%s>%s</td>",routelist_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));  
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_access.cgi?UN=%s&PN=%s&SN=%s>%s</td>",routelist_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_access.cgi?UN=%s&PN=%s&SN=%s>%s</td>",routelist_encry,pageNumCL,"PageLast",search(lcontrol,"Page_Last"));
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
free(encry);
for(i=0;i<MAX_ACCESS_LIST;i++)
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
						 
int ShowRouteAccess(struct AccessInfo_st stAccess[],int * access_num,struct list *lpublic,struct list *lcontrol)
{
	int i;
	char * showAccess=(char *)malloc(350);
	memset(showAccess,0,350);
						 					 
	strcat(showAccess,"flag=0;listid=0;show_access_list.sh | sed -n '/ZEBRA/,/RIP/p' | sed \"1,1d\" | sed \"/RIP/d\" | while read line ;do if [ $(echo $line | grep -c \"IP access list\") -eq 1 ]; then listid=`echo $line | awk '{printf $5}'`; if [ $listid -gt 99 ];then break;fi;else printf ${listid}-; echo $line | awk 'BEGIN{OFS=\"-\"}{print $1,$2}'; fi; done");

	FILE * fd = NULL;
	//char temp[128];
	char * AccessInfo[MAX_ACCESS_LIST];
	for( i = 0; i < MAX_ACCESS_LIST; i++ )
		{
			AccessInfo[i] = (char * )malloc(128);
			memset(AccessInfo[i], 0 ,128);
		}
	char * pieceData[3];
	char * endptr = NULL;

	i = 0;
	if( NULL != (fd = popen(showAccess,"r")) )
	{
		while((fgets(AccessInfo[i],128,fd)) != NULL)
		{
			//fprintf(stderr,"temp=%s",temp);
			//strcpy(accessInfo[i],temp);
			pieceData[0] = strtok(AccessInfo[i],"-\n");
			pieceData[1] = strtok(NULL,"-\n");
			pieceData[2] = strtok(NULL,"-\n");
			stAccess[i].access_number = strtoul(pieceData[0], &endptr, 10);
			strncpy(stAccess[i].oper, pieceData[1], MAX_OPER_LENTG); 
			strncpy(stAccess[i].ipaddr, pieceData[2], MAX_IP_LENTG);
			i++;
			//memset(temp,0,60);
		}
		pclose(fd);
		*access_num=i;
	}
	else
		return -1;

	for( i = 0; i < MAX_ACCESS_LIST; i++ )
		{
			free( AccessInfo[i] );
		}
							 
	free(showAccess);
	return 0;
}

int del_access(char * accessID,char *operate,char* ip_address, struct list *lcontrol)
{
	int sh_ret = 0;
	int state = 0;
	char command[250];
	memset(command, 0, 250);
	sprintf(command, "del_access_list.sh %s %s %s", accessID, operate, ip_address);
	state = system(command);
	sh_ret = WEXITSTATUS(state);
	if( sh_ret == 0)
		ShowAlert(search(lcontrol,"no_access_suc"));
	
	return 0;
}


