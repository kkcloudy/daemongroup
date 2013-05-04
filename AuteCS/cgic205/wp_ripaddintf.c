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
* wp_ripaddintf.c
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
#include "ws_init_dbus.h"


#include <sys/wait.h>
#define Info_Num 500
#define INTF_NAME_LENTH 20
#define Network_Num 8


int ShowAddIntfPage();
int ReadConfig(char * ripInfo[],int * infoNum,struct list * lpublic);

int interfaceInfo(char * IntfInfo[],int * Num,struct list * lpublic);
int executeconfig(char * IntfInfo[],int num,struct list *lcontrol,struct list * lpublic);
char * nameToIP(char * intfname,struct list * lpublic);
int CompareIPNetwork(char * src,char * dst);
int show_intf_network(char * intfname,char * revIntfNet[],int * Num ,struct list *lpublic);
int check_str_par_valid(char * src, int min, long max);


int cgiMain()
{
 ShowAddIntfPage();
 return 0;
}

int ShowAddIntfPage()
{ 
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	FILE *fp;
	char lan[3];
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	int i;
	//char encry_addintf[BUF_LEN];
	char * intfName[Info_Num];
	int IntfNum=0;

	char * select_opt = (char *)malloc(INTF_NAME_LENTH);
	memset( select_opt, 0, INTF_NAME_LENTH );
	char * Network[Network_Num];
	for( i = 0; i< Network_Num ; i++ )
		{
			Network[i]=(char *)malloc(30);
			memset(Network[i],0,30);
		}
	for(i=0;i<Info_Num;i++)
		{
			intfName[i]=(char *)malloc(20);
			memset(intfName[i],0,20);
		}
	memset(encry,0,BUF_LEN);
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN) != cgiFormNotFound)
	{
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	}
	else
	{
		cgiFormStringNoNewlines("encry_addintf",encry,BUF_LEN);
	}
   cgiFormStringNoNewlines("Rip_intf_select",select_opt,INTF_NAME_LENTH);
   fprintf(stderr,"select_opt=%s",select_opt);
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
  "function changestate(obj)"\
  "{"\
      "if(obj.value==\"NoAuth\" || obj==\"abc\")"\
      "{"\
      		"document.formadd.simplepwd.disabled=true;"\
      		"document.formadd.MD5pwd.disabled=true;"\
      		"document.formadd.simplepwd.style.backgroundColor = \"#ccc\";"\
      		"document.formadd.MD5pwd.style.backgroundColor = \"#ccc\";"\
      "}"\
      "else if(obj.value==\"text\")"\
      "{"\
      		"document.formadd.simplepwd.disabled=false;"\
      		"document.formadd.MD5pwd.disabled=true;"\
      		"document.formadd.simplepwd.style.backgroundColor = \"#fff\";"\
      		"document.formadd.MD5pwd.style.backgroundColor = \"#ccc\";"\

      "}"\
      "else if(obj.value==\"MD5\")"\
      "{"\
      		"document.formadd.simplepwd.disabled=true;"\
      		"document.formadd.MD5pwd.disabled=false;"\
      		"document.formadd.MD5pwd.style.backgroundColor = \"#fff\";"\
      		"document.formadd.simplepwd.style.backgroundColor = \"#ccc\";"\
      "}"\
  "}"\
  "</script>"\
  "<body onload=changestate(\"abc\")>");
  ccgi_dbus_init();
  if(cgiFormSubmitClicked("submit_addintf") == cgiFormSuccess)
  {
  		interfaceInfo(intfName,&IntfNum,lpublic);
    	executeconfig(intfName,IntfNum,lcontrol,lpublic);
  }

  fprintf(cgiOut,"<form method=post name=formadd>"\
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
          "<td width=62 align=center><input id=but type=submit name=submit_addintf style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
          fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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

					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_riproute.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));						
					fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_riplist.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>RIP</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"riplist"));						
					fprintf(cgiOut,"</tr>");
				  	fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_rip_intf"));   /*突出显示*/
				  	fprintf(cgiOut,"</tr>");
    		

					  for(i=0;i<15;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }
					// unsigned int tem; 
				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=340 border=0 cellspacing=0 cellpadding=0>");
				       //fprintf(cgiOut,"%d",sizeof(tem));
					  	fprintf(cgiOut,"<tr>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"interface_info"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						  "<td align=left valign=top style=padding-top:10px>"\
						  "<table width=500 border=0 cellspacing=0 cellpadding=0>");
				  			//int routeUpdate=30;
				  			interfaceInfo(intfName,&IntfNum,lpublic);
							fprintf(cgiOut,"<tr align=left height=35>"\
							"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left width=40>%s: </td>",search(lcontrol,"interface"));
							fprintf(cgiOut,"<td align=left width=80>");
							fprintf(cgiOut, "<select name=\"Rip_intf_select\" onChange=\"javacript:this.form.submit();\">");
							for(i=0;i<IntfNum;i++)
							{
								fprintf(stderr,"intfName[%d]=%s",i,intfName[i]);
								if(strcmp(intfName[i],"lo")!=0)
									{
										if(strcmp(intfName[i],select_opt) == 0)
											fprintf(cgiOut, "<option  selected=selected value=%s>%s",intfName[i],intfName[i]);
										else
											fprintf(cgiOut, "<option value=%s>%s",intfName[i],intfName[i]);
									}
							}
                         	fprintf(cgiOut, "</select>\n");
							fprintf(cgiOut,"</td>");
							/////add at 2009-3-3/////

							int Net_num = 0;
							if( strcmp(select_opt , "") != 0 )
								show_intf_network( select_opt ,Network ,&Net_num ,lpublic ) ;
							else if( IntfNum > 0 )
								show_intf_network( intfName[0] ,Network ,&Net_num ,lpublic );
							
							//fprintf(stderr,"intfName[0]=%s-Net_num=%d",intfName[0],Net_num);
							fprintf(cgiOut,"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left width=40>%s: </td>",search(lcontrol,"network"));
							fprintf(cgiOut,"<td align=left width=130>");
							fprintf(cgiOut, "<select name=\"intf_network\">");
							for(i=0;i<Net_num;i++)
							{
                     				fprintf(cgiOut, "<option value=%s>%s",Network[i],Network[i]);
							}
                         	fprintf(cgiOut, "</select>\n");
							fprintf(cgiOut,"</td>");
							fprintf(cgiOut,"<td align=left>%s</td>",search(lcontrol,"interface_description"));
							fprintf(cgiOut,"</tr>"\

							
							"</table>"\
						  "</td>"\
						"</tr>"\
						"<tr>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"General_set"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td>"\
						"<table width=500 border=0 cellspacing=0 cellpadding=0>"\
						"<tr align=left height=35>");
							fprintf(cgiOut,"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left width=140>%s: </td>",search(lcontrol,"send_version"));
							fprintf(cgiOut, "<td width=70 align=left><input type=radio name=send_select value=1>%s1</td>",search(lcontrol,"version"));
							fprintf(cgiOut, "<td width=70 align=left><input type=radio name=send_select  checked value=2>%s2</td>",search(lcontrol,"version"));
							fprintf(cgiOut, "<td width=150 align=left><input type=radio name=send_select value=3>%s1 %s %s2</td>",search(lcontrol,"version"),search(lcontrol,"and"),search(lcontrol,"version"));
							fprintf(cgiOut,"<td width=10>&nbsp;</td>");
							fprintf(cgiOut,"</tr>");

						fprintf(cgiOut,"<tr align=left height=35>");
							fprintf(cgiOut,"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left width=140>%s: </td>",search(lcontrol,"Rev_version"));
							fprintf(cgiOut, "<td width=70 align=left><input type=radio name=receive_select value=1>%s1</td>",search(lcontrol,"version"));
							fprintf(cgiOut, "<td width=70 align=left><input type=radio name=receive_select value=2>%s2</td>",search(lcontrol,"version"));
							fprintf(cgiOut, "<td width=150 align=left><input type=radio name=receive_select checked value=3>%s1 %s %s2</td>",search(lcontrol,"version"),search(lcontrol,"and"),search(lcontrol,"version"));
							fprintf(cgiOut,"<td width=10>&nbsp;</td>");
							fprintf(cgiOut,"</tr>");
							 
						fprintf(cgiOut,"<tr align=left height=35>"\
							"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left width=120>%s: </td>","Horizon");
							fprintf(cgiOut,"<td align=left width=120 colspan=2>");
							fprintf(cgiOut, "<select name=Rip_loop>");
                         	fprintf(cgiOut, "<option value=horizon_poison>%s",search(lcontrol,"open_Horizon_poison_reverse"));
							fprintf(cgiOut, "<option value=open_Horizon>%s",search(lcontrol,"open_Horizon"));
							fprintf(cgiOut, "<option value=None>None");
                         	fprintf(cgiOut, "</select>");
							fprintf(cgiOut,"</td>");
							fprintf(cgiOut,"<td colspan=2 width=270>&nbsp;</td>");
							fprintf(cgiOut,"</tr>"\
						"<tr align=left height=35>");	
						fprintf(cgiOut, "<td colspan=2><input type=checkbox name=passive value=passive>%s</td>",search(lcontrol,"passive_mode"));
						fprintf(cgiOut, "<td align=right>%s:</td>",search(lcontrol,"distance"));
						fprintf(cgiOut, "<td colspan=3><input type=text name=intf_distance size=8 value=></td>");
						fprintf(cgiOut,"</tr>"\
						/*"<tr align=left height=35>");
						fprintf(cgiOut, "<td>&nbsp;</td>");

						fprintf(cgiOut,"</tr>"\*/
						"</table>"\
						"</td>"\
						"</tr>"\
						"<tr height=35>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"Authentication"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td>"\
						"<table>"\
						"<tr align=left height=35>"\
						"<td><input type=radio name=radiobutton value=NoAuth onclick=\"changestate(this);\" checked></td>"\
						"<td>None</td>"\
						"</tr>"\
						"<tr height=35 align=left>");
						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=text onclick=\"changestate(this);\"></td>");
						fprintf(cgiOut,"<td>%s: </td>",search(lcontrol,"simple_passwd"));
						fprintf(cgiOut,"<td><input type=text name=simplepwd></td>"\
						"</tr>"\
						
						"<tr align=left height=35>");
						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=MD5 onclick=\"changestate(this);\"></td>");
						fprintf(cgiOut,"<td>%s: </td>",search(lcontrol,"MD5_passwd"));
						fprintf(cgiOut,"<td><input type=text name=MD5pwd></td>"\
						"</tr>"\
						"</table>"\
						"</td>"\
						"</tr>"\
						"<tr>");

    					fprintf(cgiOut,"<td><input type=hidden name=encry_addintf value=%s></td>",encry);
 
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
	free( intfName[i] );
}
for( i = 0; i< Network_Num ; i++ )
{
	free( Network[i] );
}


release(lpublic);  
release(lcontrol);

return 0;

}

int interfaceInfo(char * IntfInfo[],int * Num,struct list * lpublic)
{
	FILE * ft;
	char * syscommand=(char *)malloc(200);
	memset(syscommand,0,200);
	strcat(syscommand,"ip_addr.sh 2>/dev/null;");
	strcat(syscommand,"awk '{print $1}' /var/run/apache2/ip_addr.file> /var/run/apache2/InterfaceName.txt");
	int status =system(syscommand);
	int ret = WEXITSTATUS(status);
						 
	if(0 != ret)
		ShowAlert(search(lpublic,"bash_fail"));
	

	char temp[20];
	memset(temp,0,20);
	if((ft=fopen("/var/run/apache2/InterfaceName.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	int i=0;
	while((fgets(temp,18,ft)) != NULL)
		{
			strncpy(IntfInfo[i],temp,strlen(temp)-1);
			i++;
			memset(temp,0,20);
			//fprintf(stderr,"IntfInfo[%d]=%s",i,IntfInfo[i]);
		}
	fclose(ft);
	*Num=i;
	free(syscommand);
	return 1;

}

int executeconfig(char * IntfInfo[],int num,struct list * lcontrol,struct list * lpublic)
{
	char * intfNamesys=(char * )malloc(300);
	memset(intfNamesys,0,300);
	char * networktemp=(char *)malloc(30);
	memset(networktemp,0,30);
	char * Intfname=(char *)malloc(20);
	memset(Intfname,0,20);
	char * Rip_loop=(char *)malloc(20);
	memset(Rip_loop,0,20);
	char * intf_distance=(char *)malloc(20);
	memset(intf_distance,0,20);
	char * radiobutton=(char *)malloc(10);
	memset(radiobutton,0,10);
	char * simplepwd=(char *)malloc(100);
	memset(simplepwd,0,100);
	char * MD5pwd[20];
	memset(MD5pwd,0,20);
	cgiFormString("simplepwd", simplepwd, 100);
	cgiFormString("MD5pwd", MD5pwd, 20);

	int net_ret ;
    if(strlen(simplepwd) >= 16)
    {
    	ShowAlert(search(lcontrol,"text_key_can_not_too_long"));
    	return -1;
    }
	if(strlen(MD5pwd) >= 16)
    {
    	ShowAlert(search(lcontrol,"text_key_can_not_too_long"));
    	return -1;
    }
	
	char * RipItem[Info_Num];
	int itemNum=0,i=0,j,flag=0;
	for(i=0;i<Info_Num;i++)
	{
		RipItem[i]=(char *)malloc(60);
		memset(RipItem[i],0,60);
	}
	char * revRouteInfo[5];
	for(j=0;j<5;j++)
		{
			revRouteInfo[j]=(char *)malloc(20);
		}
	ReadConfig(RipItem,&itemNum,lpublic);
	//fprintf(stderr,"itemNum=%d",itemNum);
	/*int Choice=0;
	cgiFormSelectSingle("Rip_intf", IntfInfo, num, &Choice, 0);
	fprintf(stderr,"IntfInfo[%d]=%s",Choice,IntfInfo[Choice]);
	strcat(Intfname,IntfInfo[Choice]);*/
	cgiFormStringNoNewlines("Rip_intf_select",Intfname,20);
	cgiFormStringNoNewlines("intf_network",networktemp,30);
	fprintf(stderr,"networktemp=%s",networktemp);
	for(i=0;i<itemNum;i++)
	{
		for(j=0;j<5;j++)
		{
			memset(revRouteInfo[j],0,20);
		}
		revRouteInfo[0]=strtok(RipItem[i],"-");
		//strcat(revRouteInfo[0],strtok(RipItem[i],"-,"));
		if(revRouteInfo[0]!=NULL)
		{
     		if(strcmp(revRouteInfo[0],"network")==0)
             {
             	//fprintf(stderr,"3333333");
             	j=0;
         		while(revRouteInfo[j]!=NULL && j<1)
         			{
         				strcat(revRouteInfo[j+1],strtok(NULL,"-,"));
         				j++;
         				
         			}
				fprintf(stderr,"revRouteInfo[1]=%s",revRouteInfo[1]);
				#if 0
         		if(strstr(networktemp,revRouteInfo[1])!=NULL)
         		{
             		flag=1;
             	}
				#endif
			    net_ret = CompareIPNetwork(networktemp,revRouteInfo[1]);
				if( net_ret == 0)
					flag=1;
             }
         


        }
        	
        
	}
	if(flag!=1)
	{
    	strcat(intfNamesys,"rip_network.sh");
    	strcat(intfNamesys," ");
    	strcat(intfNamesys,"on");
    	strcat(intfNamesys," ");
    	strcat(intfNamesys,networktemp);
		strcat(intfNamesys," ");
		strcat(intfNamesys,"1>/dev/null 2>/dev/null");
    	int status = system(intfNamesys);
    	int ret = WEXITSTATUS(status);
    					 
    	if( ret != 0)
    	{
    		ShowAlert(search(lcontrol,"rip_network_fail"));
			goto bash_return;
    	}
		
    		//ShowAlert(search(lpublic,"bash_fail"));

/////////////////////////////////////////////////////////////////////////////////////
     	memset(intfNamesys,0,300);	
     	int result;
     	//char **responses1;
     	//result = cgiFormStringMultiple("send_select", &responses1);
		char sendversion[10];
		cgiFormString("send_select", sendversion, 10);
		fprintf(stderr,"sendversion=%s",sendversion);

		strcat(intfNamesys,"rip_send_version.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"on");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,sendversion);
		strcat(intfNamesys," ");
		strcat(intfNamesys,"1>/dev/null 2>/dev/null");
		system(intfNamesys);

     		
     	
     //////////////////////////////////////////////////////////////////////////////////////////
     
     	memset(intfNamesys,0,300);	
     	result=0;
		char revversion[10];
     	//char **responses2;
     	//result = cgiFormStringMultiple("receive_select", &responses2);
		cgiFormString("receive_select", revversion, 10);
		fprintf(stderr,"revversion=%s",revversion);

		strcat(intfNamesys,"rip_receive_version.sh");
		strcat(intfNamesys," ");
		strcat(intfNamesys,"on");
		strcat(intfNamesys," ");
		strcat(intfNamesys,Intfname);
		strcat(intfNamesys," ");
		strcat(intfNamesys,revversion);
		strcat(intfNamesys," ");
		strcat(intfNamesys,"1>/dev/null 2>/dev/null");
		system(intfNamesys);

     //////////////////////////////////////////////////////////////////////////////////////////////
     	memset(intfNamesys,0,300);
     	cgiFormStringNoNewlines("Rip_loop",Rip_loop,20);
     	if(strcmp(Rip_loop,"horizon_poison")==0)
     	{
     		strcat(intfNamesys,"rip_split.sh");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"on");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,Intfname);
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"poisoned");
			strcat(intfNamesys," ");
			strcat(intfNamesys,"1>/dev/null 2>/dev/null");
     		system(intfNamesys);
     	}
     	else if(strcmp(Rip_loop,"open_Horizon")==0)
     	{
     		strcat(intfNamesys,"rip_split.sh");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"on");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,Intfname);
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"normal");
			strcat(intfNamesys," ");
			strcat(intfNamesys,"1>/dev/null 2>/dev/null");
     		system(intfNamesys);
     	}
     	else if(strcmp(Rip_loop,"None")==0)
     	{
     		strcat(intfNamesys,"rip_split.sh");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"off");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,Intfname);
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"normal");
			strcat(intfNamesys," ");
			strcat(intfNamesys,"1>/dev/null 2>/dev/null");
     		system(intfNamesys);
     		
     	}
     ///////////////////////////////////////////////////////////////////////////////////////////////
     	memset(intfNamesys,0,300);
     	result=0;
     	char **responses3;
     	result = cgiFormStringMultiple("passive", &responses3);
     	if(responses3[0])
     	{
     		strcat(intfNamesys,"rip_passive_int.sh");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"on");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,Intfname);
			strcat(intfNamesys," ");
			strcat(intfNamesys,"1>/dev/null 2>/dev/null");
     		system(intfNamesys);
     	}
     	memset(intfNamesys,0,300);
     	cgiFormStringNoNewlines("intf_distance",intf_distance,20);
     
     	if(strcmp(intf_distance,"")!=0)
     	{
     		strcat(intfNamesys,"distance.sh");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"on");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,intf_distance);
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,networktemp);
			strcat(intfNamesys," ");
			strcat(intfNamesys,"1>/dev/null 2>/dev/null");
     		system(intfNamesys);
     	}	
     ////////////////////////////////////////////////////////////////////////////////
     	cgiFormString("radiobutton", radiobutton, 10);
     	memset(intfNamesys,0,300);
     	if(strcmp(radiobutton,"NoAuth")==0)
     	{}
     	else if(strcmp(radiobutton,"text")==0)
     	{
     		strcat(intfNamesys,"rip_auth_mode.sh");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"on");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,Intfname);
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"text");
			strcat(intfNamesys," ");
			strcat(intfNamesys,"1>/dev/null 2>/dev/null");
     		system(intfNamesys);
     		

     		memset(intfNamesys,0,300);
     		strcat(intfNamesys,"rip_auth_string.sh");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"on");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,Intfname);
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,simplepwd);
			strcat(intfNamesys," ");
			strcat(intfNamesys,"1>/dev/null 2>/dev/null");
     
     		system(intfNamesys);
     	}
     	else if(strcmp(radiobutton,"MD5")==0)
     	{
     		strcat(intfNamesys,"rip_auth_mode.sh");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"on");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,Intfname);
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"md5");
			strcat(intfNamesys," ");
			strcat(intfNamesys,"1>/dev/null 2>/dev/null");
     		system(intfNamesys);
     

     		memset(intfNamesys,0,300);
     		strcat(intfNamesys,"rip_auth_string.sh");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,"on");
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,Intfname);
     		strcat(intfNamesys," ");
     		strcat(intfNamesys,MD5pwd);
			strcat(intfNamesys," ");
			strcat(intfNamesys,"1>/dev/null 2>/dev/null");
     		system(intfNamesys);
     	}
     	ShowAlert(search(lcontrol,"rip_add_suc"));
	}
	else ShowAlert(search(lcontrol,"rip_intf_exist"));

bash_return:

	for(i=0;i<Info_Num;i++)
    {
    	free(RipItem[i]);	
    }
	free(Intfname);
	free(Rip_loop);
	free(simplepwd);
	free(radiobutton);
	free(intf_distance);
	free(networktemp);
	free(intfNamesys);
	return 0;
}


char * nameToIP(char * intfname,struct list * lpublic)
{
	FILE * fd;
	char * command=(char *)malloc(100);
	memset(command,0,100);
	strcat(command,"NameToIP.sh");
	strcat(command," ");
	strcat(command,intfname);
	strcat(command," ");
	strcat(command,">/var/run/apache2/NameToIp.txt");
	system(command);
	fprintf(stderr,"command=%s",command);
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
	temp1=strtok(temp,"-\n");
	temp2=strtok(NULL,"-\n");
	
	sprintf(templater,"%s/%s",temp1,temp2);

	free(temp);
	free(command);
	return templater;
}

int ReadConfig(char * ripInfo[],int * infoNum,struct list * lpublic)
{
	int i;
	char * command=(char *)malloc(200);
	memset(command,0,200);
	strcat(command,"show_run_conf.sh | awk 'BEGIN{FS=\"\\n\";RS=\"!\"}/router rip/{print}'| awk '{OFS=\"-\";ORS=\"-\\n\"}{$1=$1;print}'  >/var/run/apache2/RipInfo.txt");
	int status = system(command);
	int ret = WEXITSTATUS(status);				 
	if(0==ret)
		{}
	else ShowAlert(search(lpublic,"bash_fail"));

	FILE *fd;
	char  temp[60];
	memset(temp,0,60);
	if((fd=fopen("/var/run/apache2/RipInfo.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i=0;
	while((fgets(temp,60,fd)) != NULL)
		{
			strcat(ripInfo[i],temp);
			i++;
			memset(temp,0,60);
		}
	fclose(fd);
	*infoNum=i;
	
	free(command);
	return 1;
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
			strncpy(revIntfNet[i],temp,strlen(temp)-1);
			i++;
			memset(temp,0,30);
		}
	fclose(ft);
	*Num = i;
	free(command);
	return 0;
}

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


