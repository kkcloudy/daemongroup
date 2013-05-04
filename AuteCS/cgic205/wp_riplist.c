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
* wp_riplist.c
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
#define intf_Num 100
#define Info_Num 20
#define MAX_NETWORK_NUM 8


#define DEF_INTF_INFOR_LENGTH 32
typedef struct{
	char intf_name[DEF_INTF_INFOR_LENGTH];
	char * intf_network[MAX_NETWORK_NUM];
	char send_version[DEF_INTF_INFOR_LENGTH];
	char rev_version[DEF_INTF_INFOR_LENGTH];
	char split[DEF_INTF_INFOR_LENGTH];
	char passmode[DEF_INTF_INFOR_LENGTH];
	char distance[DEF_INTF_INFOR_LENGTH];
	struct{
		char auth_mode[DEF_INTF_INFOR_LENGTH];
		char auth_key[DEF_INTF_INFOR_LENGTH];
	}auth;
}RIP_INTF_ST;


int ShowRipListPage();
char * blankConvertZero(char * str);

int CompareIPNetwork(char * src,char * dst);

int intf(char * IntfInfo[],int * Num,struct list *lpublic);

int  interfaceInfo(char *iname,char * intfname[],int * infoNum,struct list * lpublic);
//char * len_to_mask(int len);
int ReadConfig(int cl,int retu,char * encry,char * ripInfo[],RIP_INTF_ST * p_ripintf,struct list * lpublic,struct list * lcon );

int show_intf_network(char * intfname,char * revIntfNet[],int * Num ,struct list *lpublic);
int delete_hand();
char * trim(char * src);
int web_out_print(int cl,int k,int retu,char * encry, RIP_INTF_ST * p_ripintf_web, struct list * lcontrol);
char* get_token_rip(char *str_tok);



int cgiMain()
{
 ShowRipListPage();
 return 0;
}

int ShowRipListPage()
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
  	char Riplist_encry[BUF_LEN];       
 	int i;   
  	int cl=1;
  	int pageNum=0;
	char * pageNumCA=(char *)malloc(10);
	memset(pageNumCA,0,10);
	char * pageNumCD=(char *)malloc(10);
	memset(pageNumCD,0,10);

//////////////////////////////////
	char * intfName[intf_Num];
	for(i=0;i<intf_Num;i++)
	{
		intfName[i]=(char *)malloc(21);
		memset(intfName[i],0,21);
	}

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
	char * address[8];
	char * network=(char *)malloc(20);
	memset(network,0,20);
	
	char * passive=(char *)malloc(20);
	memset(passive,0,20);
	char * distance=(char *)malloc(20);
	memset(distance,0,20);
	
	char * RipItem[Info_Num]; 
	
	char * addressRoute[Info_Num]; /*用来接收分解的NETWORK的网段*/
	char * passiveRoute[Info_Num]; /*用来接收分解的PASSIVE的网段*/
	char * distance_intfname[Info_Num]; /*用来接收分解的DISTANCE的网段*/
	char * distance_value[Info_Num]; /*用来接收分解的DISTANCE的网段*/
	//int addTemp=0;
//////////////////////////////delete抓取的值/////////////////////////////////////////////
	char * deleteOP=(char *)malloc(10);
	memset(deleteOP,0,10);
	int retu=0;
	
	char * CheckUsr=(char *)malloc(10);
	memset(CheckUsr,0,10);

/************************/
	RIP_INTF_ST ST_RIP_INFOR;
	memset(&ST_RIP_INFOR, 0, sizeof(RIP_INTF_ST));


	for(i=0;i<MAX_NETWORK_NUM;i++)
	{
		ST_RIP_INFOR.intf_network[i]=(char *)malloc(100);
		memset(ST_RIP_INFOR.intf_network[i],0,100);
	}



/***********************/

	for(i=0;i<8;i++)
	{
		address[i] = (char * )malloc(60);
		memset(address[i],0,60);
	}
	
	for(i=0;i<Info_Num;i++)
	{
		RipItem[i]=(char *)malloc(60);
		memset(RipItem[i],0,60);
	}
	
	for(i=0;i<Info_Num;i++)
	{
		intfItem[i]=(char *)malloc(60);
		memset(intfItem[i],0,60);
	}
	for(i=0;i<Info_Num;i++)
	{
		addressRoute[i]=(char *)malloc(30);
		memset(addressRoute[i],0,30);
		
		passiveRoute[i]=(char *)malloc(30);
		memset(passiveRoute[i],0,30);
		
		distance_intfname[i]=(char *)malloc(30);
		memset(distance_intfname[i],0,30);
		
		distance_value[i]=(char *)malloc(10);
		memset(distance_value[i],0,10);
	}
	
	
	
  if(cgiFormSubmitClicked("submit_Riplist") != cgiFormSuccess)
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
      return 0;
	}
	memset(Riplist_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
    memset(PNtemp,0,10);
	cgiFormStringNoNewlines("PN",PNtemp,10);
	pageNum=atoi(PNtemp);
	memset(SNtemp,0,10);
	cgiFormStringNoNewlines("SN",SNtemp,10);
  cgiFormStringNoNewlines("encry_Riplist",Riplist_encry,BUF_LEN);
  cgiFormStringNoNewlines("DELETE",deleteOP,10);
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
  if(strcmp(deleteOP,"delete")==0)
  {
  	 delete_hand();
  }
  if(cgiFormSubmitClicked("submit_Riplist") != cgiFormSuccess)
  {
  	retu=checkuser_group(str);
  }
  if(cgiFormSubmitClicked("submit_Riplist") == cgiFormSuccess)
  {
  	//	ss();
  	 fprintf( cgiOut, "<script type='text/javascript'>\n" );
   	 fprintf( cgiOut, "window.location.href='wp_srouter.cgi?UN=%s';\n", Riplist_encry);
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
          "<td width=62 align=center><input id=but type=submit name=submit_Riplist style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));	  
		  if(cgiFormSubmitClicked("submit_Riplist") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_riproute.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_riproute.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",Riplist_encry,search(lpublic,"img_cancel"));
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
         		if(cgiFormSubmitClicked("submit_Riplist") != cgiFormSuccess)
         		{
         			if(retu==0)
         			{
            			fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_riproute.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));					   
            			fprintf(cgiOut,"</tr>");
            			fprintf(cgiOut,"<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>RIP</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"riplist"));   /*突出显示*/
            			fprintf(cgiOut,"</tr>"\
            			"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_ripaddintf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_rip_intf"));
            			fprintf(cgiOut,"</tr>");
         			}
         			else
         			{
         				fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_riproute.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));					   
            			fprintf(cgiOut,"</tr>");
            			fprintf(cgiOut,"<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>RIP</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"riplist"));   /*突出显示*/
            			fprintf(cgiOut,"</tr>");
         			}
         		}
         		else if(cgiFormSubmitClicked("submit_Riplist") == cgiFormSuccess) 			  
         		{
         			if(retu==0)
         			{
            			fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_riproute.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",Riplist_encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));						
            			fprintf(cgiOut,"</tr>");
            			fprintf(cgiOut,"<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>RIP</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"riplist"));   /*突出显示*/
            			fprintf(cgiOut,"</tr>"\
            			"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_ripaddintf.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",Riplist_encry,search(lpublic,"menu_san"),search(lcontrol,"add_rip_intf"));
            			fprintf(cgiOut,"</tr>");
         			}
         			else
         			{
         				fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_riproute.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",Riplist_encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));						
            			fprintf(cgiOut,"</tr>");
            			fprintf(cgiOut,"<tr height=26>"\
            			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>RIP</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"riplist"));   /*突出显示*/
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
						"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"rip_intf_info"));
						fprintf(cgiOut,"</tr>");
						 fprintf(cgiOut,"<tr>"\
						   "<td align=left valign=top style=padding-top:18px>"\
						   "<div class=ShowRoute><table width=750 border=1 frame=below  rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
							 fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 id=td1 align=left>"\
							 "<th width=62 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"interface"));
							 fprintf(cgiOut,"<th width=134 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"network"));
							 fprintf(cgiOut,"<th width=164 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"open_Horizon"));
							 //fprintf(cgiOut,"<th width=70 style=font-size:12px>DIS</th>");
							 fprintf(cgiOut,"<th width=85 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"Rev_version"));
							 fprintf(cgiOut,"<th width=85 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"send_version"));
							 fprintf(cgiOut,"<th width=110 style=font-size:12px><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"Authentication"));
							 fprintf(cgiOut,"<th width=35 style=font-size:12px>&nbsp;</th>");
							 fprintf(cgiOut,"<th width=35 style=font-size:12px>&nbsp;</th>");	
							 fprintf(cgiOut,"</tr>");
							 //int intfNum = 0,retCom = 0, net_num_for_one_intf = 0 ;
				  			 //char * revIntfInfo[6];
				  			//for(i=0;i<6;i++)
				  			//{
				  			//	revIntfInfo[i]=(char *)malloc(30);
				  			//}

							 //intf(intfName,&intfNum,lpublic);

							//int ReadConfig(int cl,int retu,char * encry,char * ripInfo[],RIP_INTF_ST * p_ripintf,struct list * lpublic,struct list * lcon );
	
							 if(cgiFormSubmitClicked("submit_Riplist") != cgiFormSuccess)
								 ReadConfig(cl,retu,encry,RipItem,&ST_RIP_INFOR,lpublic,lcontrol);
							 else
							 	 ReadConfig(cl,retu,Riplist_encry,RipItem,&ST_RIP_INFOR,lpublic,lcontrol);
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
						if(cgiFormSubmitClicked("submit_Riplist") != cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_riplist.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_riplist.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
							}
						else if(cgiFormSubmitClicked("submit_Riplist") == cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_riplist.cgi?UN=%s&PN=%s&SN=%s>%s</td>",Riplist_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_riplist.cgi?UN=%s&PN=%s&SN=%s>%s</td>",Riplist_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));  
							}
						fprintf(cgiOut,"</tr></table></td>"\
						"</tr>"\
						 "<tr>");
						 if(cgiFormSubmitClicked("submit_Riplist") != cgiFormSuccess)
						 {
						   fprintf(cgiOut,"<td><input type=hidden name=encry_Riplist value=%s></td>",encry);
						   fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
						 }
						 else if(cgiFormSubmitClicked("submit_Riplist") == cgiFormSuccess)
							 {
							   fprintf(cgiOut,"<td><input type=hidden name=encry_Riplist value=%s></td>",Riplist_encry);
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
for(i=0;i<intf_Num;i++)
{
	free(intfName[i]); 
}


for(i=0;i<Info_Num;i++)
{
  free(intfItem[i]);   
}

for(i=0;i<Info_Num;i++)
{
	free(RipItem[i]);	
}

for(i=0;i<Info_Num;i++)
{
	free(addressRoute[i]);	
	free(passiveRoute[i]);
	free(distance_intfname[i]);
	free(distance_value[i]);
}
for(i=0;i<8;i++)
	free(address[i]);

for(i=0;i<MAX_NETWORK_NUM;i++)
{
	free(ST_RIP_INFOR.intf_network[i]);
}


free(horizon);
free(recversion);
free(sendversion);
free(authString);
free(mode);

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

int intf(char * IntfInfo[],int * Num,struct list * lpublic) //取所有接口名
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
     		strncpy( IntfInfo[i], temp, strlen(temp)-1 );
			//fprintf(stderr,"IntfInfo[%d]=%s",i,IntfInfo[i]);
     		i++;
     		memset(temp,0,30);
		}

        fprintf(stderr,"num=%d",i);
	*Num=i;
	free(syscommand);
	fclose(ft);
	return 1;
}

/////////////////////////////////////////////////////////eth0不能屏蔽掉，空文件里不为空						 
int  interfaceInfo(char * iname,char * intfname[],int * infoNum,struct list * lpublic) //读取錽how running 中RIP接口的信息
{
 FILE * ft;
 char * syscommand=(char *)malloc(500);
 memset(syscommand,0,500);
 int i;
 sprintf(syscommand,"show_run_conf.sh | awk 'BEGIN{FS=\"\\n\";RS=\"!\";ORS=\"#\\n\";OFS=\"|\"}/interface/{$1=$1;print}' | awk '{RS=\"#\";FS=\"|\";ORS=\"\\n\";OFS=\"#\"}/rip/{$1=$1;print}' | awk 'BEGIN{FS=\"#\";OFS=\"#\\n\"}/%s/{$1=$1;gsub(\" \",\"#\",$0);print}' >/var/run/apache2/RIPIntf.txt",iname);
 int status = system(syscommand);
 int ret = WEXITSTATUS(status);  
 if(0==ret)
	 {}
 else ShowAlert("bash_intf_fail");
 if((ft=fopen("/var/run/apache2/RIPIntf.txt","r"))==NULL)
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
		 strcpy( intfname[i], temp);
		 i++;
		 memset(temp,0,60);
	 }
 fclose(ft);
 *infoNum=i;
 free(syscommand);
 return 1;
}

#ifndef RIP_INTF_INFOR
#define RIP_INTF_INFOR
#define INFOR_LENGTH 128
#define AWK_INTF_COMMAND "show_rip_interface.sh | awk 'BEGIN{FS=\"\\n\";RS=\"\\n\\n\"}{print}' "
#define RIP_INTF_FILE_PATH "/var/run/apache2/RipInfo.txt"
#endif


int ReadConfig(int cl,int retu,char * encry,char * ripInfo[],RIP_INTF_ST * p_ripintf,struct list * lpublic ,struct list * lcon) //读取錽how running 中rip router的信息
{
	int i,q,j = 0;
	char * command=(char *)malloc(250);
	memset(command,0,250);
	sprintf(command,"%s > %s",AWK_INTF_COMMAND,RIP_INTF_FILE_PATH);
	int status = system(command);
	int ret = WEXITSTATUS(status);

	char * revRIPINTFInfo[3]={NULL};
	
	
	if(0==ret)
		{}
	else ShowAlert(search(lpublic,"bash_fail"));

	FILE *fd;
	char  temp[INFOR_LENGTH];
	memset(temp,0,60);
	if((fd=fopen(RIP_INTF_FILE_PATH,"r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i = 0;
	q = 0;
	while((fgets(temp,INFOR_LENGTH,fd)) != NULL)
		{

			///////////////init/////////////////////////////
			if(strcmp(p_ripintf->passmode,"")!=0)
			{
				memset( p_ripintf, 0, sizeof(RIP_INTF_ST ));

				for(i=0;i<MAX_NETWORK_NUM;i++)
				{
					p_ripintf->intf_network[i]=(char *)malloc(100);
					memset(p_ripintf->intf_network[i],0,100);
				}
				q = 0;
			}
			//fprintf(stderr,"temp=%s",temp);
			strcpy(ripInfo[i], temp);
			revRIPINTFInfo[0]=strtok(ripInfo[i],":");
			//fprintf(stderr,"revRIPINTFInfo[0]=%s",revRIPINTFInfo[0]);
			if(strstr(revRIPINTFInfo[0],"IfName")!=NULL)
			{
				j=0;
					while(revRIPINTFInfo[j]!=NULL && j<1)
						{                    							
							revRIPINTFInfo[j+1]=strtok(NULL,":\n");
							j++;
						}
					strcpy(p_ripintf->intf_name,revRIPINTFInfo[1]);
					//p_ripintf->intf_name = trim(p_ripintf->intf_name);
					//fprintf(stderr,"intfName=%s",p_ripintf->intf_name);
			}
			if(strstr(revRIPINTFInfo[0],"Network")!=NULL)
			{
				j=0;
					while(revRIPINTFInfo[j]!=NULL && j<1)
						{                    							
							revRIPINTFInfo[j+1]=strtok(NULL,":\n");
							j++;
						}
					//fprintf(stderr,"p_ripintf->intf_network[%d]=%s",q,p_ripintf->intf_network[q]);
					strcpy(p_ripintf->intf_network[q],revRIPINTFInfo[1]);
					//p_ripintf->intf_network = trim(p_ripintf->intf_network);
					//fprintf(stderr,"Network=%s",p_ripintf->intf_network[q]);
					q++;
			}
			if(strstr(revRIPINTFInfo[0],"Distance")!=NULL)
			{
				j=0;
					while(revRIPINTFInfo[j]!=NULL && j<1)
						{                    							
							revRIPINTFInfo[j+1]=strtok(NULL,":\n");
							j++;
						}
					strcpy(p_ripintf->distance,revRIPINTFInfo[1]);
					//p_ripintf->distance = trim(p_ripintf->distance);
					//fprintf(stderr,"Distance=%s",p_ripintf->distance);
			}
			if(strstr(revRIPINTFInfo[0],"Send version")!=NULL)
			{
				j=0;
					while(revRIPINTFInfo[j]!=NULL && j<1)
						{                    							
							revRIPINTFInfo[j+1]=strtok(NULL,":\n");
							j++;
						}
					strcpy(p_ripintf->send_version,revRIPINTFInfo[1]);
					if (strstr(p_ripintf->send_version,"1 2") != NULL)
						{
							strcpy(p_ripintf->send_version, "1,2");
						}
					//p_ripintf->send_version = trim(p_ripintf->send_version);
					//fprintf(stderr,"Send=%s",p_ripintf->send_version);
			}
			if(strstr(revRIPINTFInfo[0],"Recv version")!=NULL)
			{
				j=0;
					while(revRIPINTFInfo[j]!=NULL && j<1)
						{                    							
							revRIPINTFInfo[j+1]=strtok(NULL,":\n");
							j++;
						}
					strcpy(p_ripintf->rev_version,revRIPINTFInfo[1]);
					if (strstr(p_ripintf->rev_version,"1 2") != NULL)
						{
							strcpy(p_ripintf->rev_version, "1,2");
						}
					//p_ripintf->rev_version = trim(p_ripintf->rev_version);
					//fprintf(stderr,"Recv=%s",p_ripintf->rev_version);
			}
			if(strstr(revRIPINTFInfo[0],"Auth mode")!=NULL)
			{
				j=0;
					while(revRIPINTFInfo[j]!=NULL && j<1)
						{                    							
							revRIPINTFInfo[j+1]=strtok(NULL,":\n");
							j++;
						}
					strcpy(p_ripintf->auth.auth_mode,revRIPINTFInfo[1]);
					//p_ripintf->auth.auth_key = trim(p_ripintf->auth.auth_key);
					//fprintf(stderr,"mode=%s",p_ripintf->auth.auth_mode);
			}
			if(strstr(revRIPINTFInfo[0],"Auth str")!=NULL)
			{
				j=0;
					while(revRIPINTFInfo[j]!=NULL && j<1)
						{                    							
							revRIPINTFInfo[j+1]=strtok(NULL,":\n");
							j++;
						}
					strcpy(p_ripintf->auth.auth_key,revRIPINTFInfo[1]);
					//p_ripintf->auth.auth_key = trim(p_ripintf->auth.auth_key);
					//fprintf(stderr,"str=%s",p_ripintf->auth.auth_key);
			}
			if(strstr(revRIPINTFInfo[0],"Split-horizon status")!=NULL)
			{
				j=0;
					while(revRIPINTFInfo[j]!=NULL && j<1)
						{                    							
							revRIPINTFInfo[j+1]=strtok(NULL,":\n");
							j++;
						}
					strcpy(p_ripintf->split,revRIPINTFInfo[1]);
					//p_ripintf->split = trim(p_ripintf->split);
					//fprintf(stderr,"Split=%s",p_ripintf->split );
			}
			if(strstr(revRIPINTFInfo[0],"Passive")!=NULL)
			{
				j=0;
					while(revRIPINTFInfo[j]!=NULL && j<1)
						{                    							
							revRIPINTFInfo[j+1]=strtok(NULL,":\n");
							j++;
						}
					strcpy(p_ripintf->passmode,revRIPINTFInfo[1]);
					//p_ripintf->split = trim(p_ripintf->split);
					//fprintf(stderr,"passmode=%s",p_ripintf->passmode );
			}
			
			if(strcmp(p_ripintf->passmode,"")!=0)
			{
				while(q > 0)
				{
					fprintf(stderr,"enter in!");
					q--;
					web_out_print(cl,q,retu,encry,p_ripintf,lcon);
					
				}

				for(i=0;i<MAX_NETWORK_NUM;i++)
				{
					free(p_ripintf->intf_network[i]);
				}
			}	

			//////////////
			i++;
			memset(temp,0,INFOR_LENGTH);
		}
	fclose(fd);


	free(command);
	return 1;
}


int show_intf_network(char * intfname,char * revIntfNet[],int * Num ,struct list *lpublic)
{
	FILE * ft;
	char * command=(char * )malloc(250);
	memset(command,0,250);
	char temp[30];
	strcat(command,"show_intf_ip.sh");
	strcat(command," ");
	strcat(command,intfname);
	strcat(command," ");
	strcat(command,"2>/dev/null | awk '{if($1==\"inet\") {print $2}}' >/var/run/apache2/vlan_intf_ip.txt");
	system(command);
	if((ft=fopen("/var/run/apache2/vlan_intf_ip.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	memset(temp , 0, 30);
	int i = 0;
	while(fgets(temp,28,ft))
		{
			fprintf(stderr,"temp=%s",temp);
			memset( revIntfNet[i] ,0 ,30);
			strncpy(revIntfNet[i],temp,strlen(temp)-1);
			i++;
			memset(temp,0,30);
		}
	fclose(ft);
	*Num = i;
	free(command);
	return 0;
}


int delete_hand()
{
	char * modeLater=(char *)malloc(10);
	memset(modeLater,0,10);
	char * authStringLater=(char *)malloc(50);
	memset(authStringLater,0,50);
	char * sendversionLater=(char *)malloc(10);
	memset(sendversionLater,0,10);
	char * recversionLater=(char *)malloc(10);
	memset(recversionLater,0,10);
	char * horizonLater=(char *)malloc(80);
	memset(horizonLater,0,80);
	char * networkLater=(char *)malloc(20);
	memset(networkLater,0,20);
	char * syscommand=(char *)malloc(200);
	memset(syscommand,0,200);
	char * IFname=(char *)malloc(30);
	memset(IFname,0,30);
	char * PASSIVE=(char *)malloc(30);
	memset(PASSIVE,0,30);
	
	cgiFormStringNoNewlines("MODE",modeLater,10);
    cgiFormStringNoNewlines("AUTHSTRING",authStringLater,50);
    cgiFormStringNoNewlines("REVVER",recversionLater,10);
    cgiFormStringNoNewlines("SENDVER",sendversionLater,10);
    cgiFormStringNoNewlines("HORIZON",horizonLater,80);
    cgiFormStringNoNewlines("NETWORK",networkLater,20);
	cgiFormStringNoNewlines("IFNAME",IFname,30);
	cgiFormStringNoNewlines("PASSIVE",PASSIVE,30);
	fprintf(stderr,"modeLater=%s-authStringLater=%s-recversionLater=%s-sendversionLater=%s-horizonLater=%s-networkLater=%s-IFname=%s",modeLater,authStringLater,recversionLater,sendversionLater,horizonLater,networkLater,IFname);
	//fprintf(stderr,"modeLater=%s-authStringLater=%s-recversionLater=%s-sendversionLater=%s-horizonLater=%s-networkLater=%s-IFname=%s",modeLater,authStringLater,recversionLater,sendversionLater,horizonLater,networkLater,IFname);
	sprintf(syscommand,"rip_network.sh off %s >/dev/null",networkLater);
	system(syscommand);
	
	memset(syscommand,0,200);
    sprintf(syscommand,"rip_auth_mode.sh off %s %s >/dev/null",IFname,modeLater);
	system(syscommand);
	
	memset(syscommand,0,200);
	sprintf(syscommand,"rip_auth_string.sh off %s %s >/dev/null",IFname,authStringLater);
	system(syscommand);

	memset(syscommand,0,200);
	sprintf(syscommand,"rip_receive_version.sh off %s >/dev/null",IFname);
	system(syscommand);

	memset(syscommand,0,200);
	sprintf(syscommand,"rip_send_version.sh off %s >/dev/null",IFname);
	system(syscommand);

	memset(syscommand,0,200);
	sprintf(syscommand,"rip_passive_int.sh off %s >/dev/null",IFname);
	system(syscommand);

	if(strcmp(horizonLater,"split-horizon-poisoned-reverse")==0)
	{
		memset(horizonLater,0,80);
		strcpy(horizonLater,"poisoned");
	}
	else if(strcmp(horizonLater,"split-horizon")==0)
	{
		memset(horizonLater,0,80);
		strcpy(horizonLater,"normal");
	}
	memset(syscommand,0,200);
	sprintf(syscommand,"rip_split.sh off %s %s >/dev/null",IFname,horizonLater);
	system(syscommand);


	
	free(PASSIVE);
	free(horizonLater);
    free(recversionLater);
    free(sendversionLater);
    free(authStringLater);
    free(modeLater);
    free(networkLater);
    free(syscommand);
    free(IFname);
	return 0;
}

/***********************************************/
/*
    功能：比较2个IP网段是不是在同一网段
    输入：2个待比较的IP带掩码位数的网段地址
    返回：
              0：成功 在同一网段
            -1：失败 不在同一网段
*/
/***********************************************/

int CompareIPNetwork(char * src,char * dst)
{
	if( src == NULL || dst == NULL )
		return -1;
	if( strcmp(src,"") == 0 || strcmp(dst,"") == 0 )
		return -2;
	
	unsigned int m0,m1,m2,m3,int_mask=0;
	unsigned int iIPbegin,iIPend;
	//unsigned int iMask;
	char chIP[32] = "";
	char chMask[32] = "";
	char *temp;
/////////////////////////////////src/////////////
	temp = strchr( src, '/' );
	strncpy( chIP, src, temp - src );
	strcpy( chMask, temp+1 );

	if( strcmp(chMask, "")!=0 )
		int_mask=atoi(chMask);
	
	sscanf( chIP, "%u.%u.%u.%u", &m3,&m2,&m1,&m0 );
	iIPbegin = m3*256*256*256 + m2*256*256 + m1*256 + m0;
	iIPbegin = iIPbegin >> (32-int_mask);

/////////////////////////dst///////////////////
	memset(chMask, 0, 32);
	int_mask = 0 ;
	m0 = 0;
	m1 = 0;
	m2 = 0;
	m3 = 0;
	temp = NULL;
	temp = strchr( dst, '/' );
	strncpy( chIP, dst, temp - dst );
	strcpy( chMask, temp+1 );
	if( strcmp(chMask, "")!=0 )
		int_mask=atoi(chMask);
	//fprintf(stderr,"chIP=%s-chMask=%s",chIP,chMask);
	sscanf( chIP, "%u.%u.%u.%u", &m3,&m2,&m1,&m0 );
	iIPend = m3*256*256*256 + m2*256*256 + m1*256 + m0;
	iIPend = iIPend >> (32-int_mask);

	//fprintf(stderr,"iIPbegin=%u--iIPend=%u",iIPbegin,iIPend);

	if(iIPbegin == iIPend)
		return 0;
	else
		return -1;
	
	
}


char * blankConvertZero(char * str)
{
	char * temp=(char *)malloc(30);
	memset(temp,0,30);
	strcpy(temp,str);
	int i=0;
	for(i=0;i<strlen(temp);i++)
	{
		if(temp[i]==' ')
			temp[i]='0';
	}
	//fprintf(stderr,"temp=%s",temp);
	return temp;
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

int web_out_print(int cl,int k,int retu,char * encry, RIP_INTF_ST * p_ripintf_web, struct list * lcontrol)
{
	char * intf_name = (char *)malloc(100);
	char * auth_mode = (char *)malloc(100);
	char * auth_key = (char *)malloc(100);
	char * rev_version = (char *)malloc(100);
	char * send_version = (char *)malloc(100);
	char * split = (char *)malloc(100);
	char * passmode = (char *)malloc(100);
	char * distance = (char *)malloc(100);
	char * intf_network = (char *)malloc(100);
	memset(intf_name,0, 100);
	memset(auth_mode,0, 100);
	memset(auth_key,0, 100);
	memset(rev_version,0, 100);
	memset(send_version,0, 100);
	memset(split,0, 100);
	memset(passmode,0, 100);
	memset(distance,0, 100);
	memset(intf_network,0, 100);
	char * 	L_intf_name = NULL;
	char * 	L_intf_network = NULL;
	char * 	L_split = NULL;
	char * 	L_rev_version = NULL;
	char * 	L_send_version = NULL;
	char * 	L_passmode = NULL;
	char * 	L_distance = NULL;
	char * 	L_auth_mode = NULL;
	char * 	L_auth_key = NULL;
	
	
	
	memcpy(intf_name , p_ripintf_web->intf_name,strlen(p_ripintf_web->intf_name));
	//fprintf(stderr,"intf_name=%s\n",intf_name);
	memcpy (intf_network , p_ripintf_web->intf_network[k],strlen(p_ripintf_web->intf_network[k]));
	memcpy (split , p_ripintf_web->split,strlen(p_ripintf_web->split));
	memcpy (rev_version , p_ripintf_web->rev_version,strlen(p_ripintf_web->rev_version));
	memcpy (send_version , p_ripintf_web->send_version,strlen(p_ripintf_web->send_version));
	memcpy (passmode , p_ripintf_web->passmode,strlen(p_ripintf_web->passmode));
	memcpy (distance , p_ripintf_web->distance,strlen(p_ripintf_web->distance));
	memcpy (auth_mode , p_ripintf_web->auth.auth_mode,strlen(p_ripintf_web->auth.auth_mode));
	memcpy (auth_key , p_ripintf_web->auth.auth_key,strlen(p_ripintf_web->auth.auth_key));

	#if 1
	L_intf_name = get_token_rip(intf_name);
	L_intf_network = get_token_rip(intf_network);
	L_split = get_token_rip(split);
	L_rev_version = get_token_rip(rev_version);
	L_send_version = get_token_rip(send_version);
	L_passmode = get_token_rip(passmode);
	L_distance = get_token_rip(distance);
	L_auth_mode = get_token_rip(auth_mode);
	L_auth_key = get_token_rip(auth_key);

	if(!strcmp(L_split,"split-horizon poisoned-reverse"))
		{
			fprintf(stderr,"ta\n");
			memset(L_split, 0 ,100);
			strcpy(L_split,"split-horizon-poisoned-reverse");
		}
	
	fprintf(stderr,"intf_name=%s-intf_network=%s--split=%s--rev_version=%s--send_version=%s--passmode=%s--distance=%s--auth_mode=%s--auth_key=%s\n",L_intf_name,L_intf_network,L_split,L_rev_version,L_send_version,L_passmode,L_distance,L_auth_mode,L_auth_key);
	#endif

	
 	//fprintf(stderr,"enter!!");
  	fprintf(cgiOut,"<tr height=25 bgcolor=%s align=left>",setclour(cl));
	fprintf(cgiOut,"<td>%s</td>",p_ripintf_web->intf_name);
	fprintf(cgiOut,"<td>%s</td>",p_ripintf_web->intf_network[k]);

	fprintf(cgiOut,"<td>%s</td>",p_ripintf_web->split);
	fprintf(cgiOut,"<td>%s</td>",p_ripintf_web->rev_version);
	fprintf(cgiOut,"<td>%s</td>",p_ripintf_web->send_version);

	//fprintf(stderr,"%s--%s--%s--%s--%s--%s--%s--%s--%s",p_ripintf_web->intf_name,p_ripintf_web->auth.auth_mode,p_ripintf_web->auth.auth_key,p_ripintf_web->rev_version,p_ripintf_web->send_version,p_ripintf_web->split,p_ripintf_web->passmode,p_ripintf_web->distance,p_ripintf_web->intf_network[k]);
	if(retu==0)
	 {
		 if(strcmp(p_ripintf_web->auth.auth_mode,"")==0)
		 	fprintf(cgiOut,"<td>%s</td>","None");
		 else
		 	fprintf(cgiOut,"<td>%s</td>",p_ripintf_web->auth.auth_mode);
	 }
	 else
	 {
	 	if(strcmp(p_ripintf_web->auth.auth_mode,"")==0)
		 	fprintf(cgiOut,"<td colspan=3>%s</td>","None");
		 else
		 	fprintf(cgiOut,"<td colspan=3>%s</td>",p_ripintf_web->auth.auth_mode);
	 }

	 if(retu==0)
	 {
		 //fprintf(cgiOut,"<td><a href=wp_ripeditintf.cgi?UN=%s&INTFNAME=%s&MODE=%s&AUTHSTRING=%s&REVVER=%s&SENDVER=%s&HORIZON=%s&PASSIVE=%s&DISTANCE=%s&NETWORK=%s target=mainFrame style=text-decoration:underline>%s</td>",encry,p_ripintf_web->intf_name,p_ripintf_web->auth.auth_mode,p_ripintf_web->auth.auth_key,p_ripintf_web->rev_version,p_ripintf_web->send_version,p_ripintf_web->split,p_ripintf_web->passmode,p_ripintf_web->distance,p_ripintf_web->intf_network[k],search(lcontrol,"edit"));	 
		 //fprintf(cgiOut,"<td><a href=wp_riplist.cgi?UN=%s&DELETE=%s&MODE=%s&AUTHSTRING=%s&REVVER=%s&SENDVER=%s&HORIZON=%s&IFNAME=%s&NETWORK=%s target=mainFrame onclick=\"return confirm('%s')\" style=text-decoration:underline>%s</td>",encry,"delete",p_ripintf_web->auth.auth_mode,p_ripintf_web->auth.auth_key,p_ripintf_web->rev_version,p_ripintf_web->send_version,p_ripintf_web->split,p_ripintf_web->intf_name,p_ripintf_web->intf_network[k],search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
		 
		 fprintf(cgiOut,"<td><a href=wp_ripeditintf.cgi?UN=%s&INTFNAME=%s&MODE=%s&AUTHSTRING=%s&REVVER=%s&SENDVER=%s&HORIZON=%s&PASSIVE=%s&DISTANCE=%s&NETWORK=%s target=mainFrame style=text-decoration:underline>%s</td>",encry,L_intf_name,L_auth_mode,L_auth_key,L_rev_version,L_send_version,L_split,L_passmode,L_distance,L_intf_network,search(lcontrol,"edit"));  
		 fprintf(cgiOut,"<td><a href=wp_riplist.cgi?UN=%s&DELETE=%s&MODE=%s&AUTHSTRING=%s&REVVER=%s&SENDVER=%s&HORIZON=%s&IFNAME=%s&NETWORK=%s target=mainFrame onclick=\"return confirm('%s')\" style=text-decoration:underline>%s</td>",encry,"delete",L_auth_mode,L_auth_key,L_rev_version,L_send_version,L_split,L_intf_name,L_intf_network,search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
	 }
 fprintf(cgiOut,"</tr>");
cl=!cl;


free(intf_name);

free(intf_network);
free(split);
free(rev_version);
free(send_version);
free(passmode);
free(distance);
free(auth_mode);
free(auth_key);
return 0;
}

///////////////去掉前面的空格/////////////
//char* get_token(char *str_tok, unsigned int *pos)
char* get_token_rip(char *str_tok)
{
    //unsigned int temp = *pos;
    unsigned int temp = 0;
    while(isspace(*(str_tok+temp)) && (*(str_tok+temp) != '\0'))
    {
        temp++;
    }
   // *pos = temp;
    return (char *)(str_tok+temp);
}


