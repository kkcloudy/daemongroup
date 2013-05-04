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
* wp_addroute.c
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
#define Route_Num 4096
#define Info_Num 500

char * nexthop[] =
{
	"network",
	"interface",
	"null0"
};

int ShowAddroutePage();
int interfaceInfo(char * IntfInfo[],int * Num,struct list * lpublic);

int addroute_hand(struct list *lpublic,struct list *lcontrol); 
int checkMark(unsigned long mark);
int ShowIPRoute(char * routeInfo[],int * route_num,int * Kroute_num,int * Sroute_num,int * Croute_num);
char * trim_zero(char * src);

int cgiMain()
{
 
 ShowAddroutePage();
 return 0;
}

int ShowAddroutePage()
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
	//char encry[BUF_LEN];
	char * select_a=(char *)malloc(20);
	memset(select_a,0,20);
	char * IpRouteItem[Route_Num]; 
	 for(i=0;i<Route_Num;i++)
		 {
			 IpRouteItem[i]=(char *)malloc(60);
			 memset(IpRouteItem[i],0,60);
		 }
	char * intfName[Info_Num];
	for(i=0;i<Info_Num;i++)
		{
			intfName[i]=(char *)malloc(20);
			memset(intfName[i],0,20);
		}
	int IntfNum=0;
	memset(encry,0,BUF_LEN);
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
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
       cgiFormStringNoNewlines("encry_add",encry,BUF_LEN);
       cgiFormStringNoNewlines("nethop_type",select_a,20);
	   fprintf(stderr,"encry=%s",encry);
    }
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
   "function net_hop_change(obj)"\
  	"{"\
  		"if(obj == 'first_load' || obj.value == 'network' )"\
  		"{"\
  			"document.getElementById('network_type').style.display = ''; "\
  			"document.getElementById('intf_type').style.display = 'none'; "\
  			"document.getElementById('operation').style.display = ''; "\
  		"}"\
  		"else if( obj.value == 'interface')"\
  		"{"\
  			"document.getElementById('network_type').style.display = 'none'; "\
  			"document.getElementById('intf_type').style.display = ''; "\
  			"document.getElementById('operation').style.display = ''; "\
  		"}"\
  		"else if( obj.value == 'null0')"\
  		"{"\
  			"document.getElementById('network_type').style.display = 'none'; "\
  			"document.getElementById('intf_type').style.display = 'none'; "\
  		"}"\
  	"}"\
  "</script>"\
  "<body onload=net_hop_change('first_load')>");
  if(cgiFormSubmitClicked("submit_addroute") == cgiFormSuccess)
  {
  	int routeNum=0,SrouteNum=0,KrouteNum=0,CrouteNum=0;
  	ShowIPRoute(IpRouteItem,&routeNum,&KrouteNum,&SrouteNum,&CrouteNum);
  	//fprintf(stderr,"SrouteNum=%d",SrouteNum);
  	 if(SrouteNum<=255)
  	  {
    	addroute_hand(lpublic,lcontrol);
      }
    else
      {
      	ShowAlert(search(lcontrol,"static_route_num_error"));
      }
  }
  fprintf(cgiOut,"<form method=post name=addroute>"\
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
          "<td width=62 align=center><input id=but type=submit name=submit_addroute style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_addroute") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_srouter.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
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
                		if(cgiFormSubmitClicked("submit_addroute") != cgiFormSuccess)
                		{
                		  
                		  fprintf(cgiOut,"<tr height=25>"\
                			"<td align=left id=tdleft><a href=wp_staticroute.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"static_route_list"));
                		  fprintf(cgiOut,"</tr>");
                		  fprintf(cgiOut,"<tr height=26>"\
                			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_route"));   /*突出显示*/
                		  fprintf(cgiOut,"</tr>");
                		}
                		else if(cgiFormSubmitClicked("submit_addroute") == cgiFormSuccess)					
                		{
                		  
                		  fprintf(cgiOut,"<tr height=25>"\
                			"<td align=left id=tdleft><a href=wp_staticroute.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"static_route_list"));
                		  fprintf(cgiOut,"</tr>");
                		  fprintf(cgiOut,"<tr height=26>"\
                			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"add_route"));   /*突出显示*/
                		  fprintf(cgiOut,"</tr>");
                		}

					  for(i=0;i<7;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=185 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top style=padding-top:10px>"\
						  "<table width=400 border=0 cellspacing=0 cellpadding=0>"\
							 "<tr height=35>"\
							  "<td align=right id=tdprompt>%s: </td>",search(lcontrol,"target_IP"));
							  fprintf(cgiOut,"<td align=left  style=padding-left:10px colspan=2>"\
							  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
							  fprintf(cgiOut,"<input type=text name=target_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=target_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=target_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"<input type=text name=target_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
							  fprintf(cgiOut,"</div>");
							  fprintf(cgiOut,"</td>"\
							  "</tr>"\
							  "<tr height=35>"\
								"<td align=right id=tdprompt>%s: </td>",search(lcontrol,"mask"));
									  fprintf(cgiOut,"<td align=left style=padding-left:10px colspan=2>"\
									  "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
										  fprintf(cgiOut,"<input type=text name=mask1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
										  fprintf(cgiOut,"<input type=text name=mask2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
										  fprintf(cgiOut,"<input type=text name=mask3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
										  fprintf(cgiOut,"<input type=text name=mask4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
										  fprintf(cgiOut,"</div>");
										  fprintf(cgiOut,"</td>"\
							  "</tr>"\
							  "<tr height=35>"\
								"<td align=right id=tdprompt>%s: </td>",search(lcontrol,"next_hop"));
        							fprintf(cgiOut,"<td align=left style=padding-left:10px colspan=2>");
        							fprintf(cgiOut, "<select name=nethop_type onchange=\"net_hop_change(this)\">");
        							for(i=0;i<3;i++)
        							//if(strcmp(nexthop[i],select_a)==0)				/*显示上次选中的*/
        								//fprintf(cgiOut,"<option value=%s selected=selected>%s",nexthop[i],nexthop[i]);
        							//else				
        								fprintf(cgiOut,"<option value=%s>%s",nexthop[i],nexthop[i]);
        
                                 	fprintf(cgiOut, "</select>\n"\
                                 	"</td>"\
                                 	"</tr>");
									//int NexthopTypechoice=0;
									//cgiFormSelectSingle("nethop_type", nexthop, 3, &NexthopTypechoice, 0);
									interfaceInfo(intfName,&IntfNum,lpublic);
									
										fprintf(cgiOut,"<tr height=35 id=network_type>"\
										"<td align=right id=tdprompt>%s: </td>",search(lcontrol,"network"));
										fprintf(cgiOut,"<td align=left style=padding-left:10px colspan=2>"\
							  			"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
								  		fprintf(cgiOut,"<input type=text name=next_hop1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
								  		fprintf(cgiOut,"<input type=text name=next_hop2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
								  		fprintf(cgiOut,"<input type=text name=next_hop3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
								 	 	fprintf(cgiOut,"<input type=text name=next_hop4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
								  		fprintf(cgiOut,"</div>");
								  		fprintf(cgiOut,"</td>"\
										"</tr>");
										
									
										fprintf(cgiOut,"<tr height=35 id=intf_type>"\
										"<td align=right id=tdprompt>%s: </td>",search(lcontrol,"interface"));
										fprintf(cgiOut,"<td align=left width=40 style=padding-left:10px colspan=2>");
										fprintf(cgiOut, "<select name=\"route_intf\">");
										for(i=0;i<IntfNum;i++)
         								{
         									if(strcmp(intfName[i],"lo")!=0)
                                  				fprintf(cgiOut, "<option value=%s>%s",intfName[i],intfName[i]);
         								}
										fprintf(cgiOut,"</select></td>"\
										"</tr>");
											
									  	      
									  fprintf(cgiOut,"<tr align=right height=35 id=operation>"\
									  "<td align=right id=tdprompt width=150>%s: </td>",search(lcontrol,"operation"));
									  fprintf(cgiOut,"<td align=left style=padding-left:10px colspan=2><select name=staticRoute_mode>"\
									  "<option value=none>none"\
									  "<option value=reject>reject"\
									  "<option value=blackhole>blackhole"\
									  "<option value=equalize>equalize"\
									  "</select>"\
									  "</td>"\
									  "</tr>");
								
							  fprintf(cgiOut,"<tr align=right height=35>"\
							  "<td align=right id=tdprompt width=100>%s: </td>",search(lcontrol,"distance"));
							  fprintf(cgiOut,"<td align=left style=padding-left:10px width=120><input type=text name=distance_SR value=1 size=21></td>"\
							  "<td align=left style=color:red;padding-left:5px width=180><1-255></td>"\
							  "</tr>"\
							  
							  "<tr>");
							  if(cgiFormSubmitClicked("submit_addroute") != cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_add value=%s></td>",encry);
							  }
							  else if(cgiFormSubmitClicked("submit_addroute") == cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_add value=%s></td>",encry);
							  }
							  fprintf(cgiOut,"</tr>"\
							"</table>"\
						  "</td>"\
						"</tr>"\
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
for(i=0;i<Route_Num;i++)
{
	free(IpRouteItem[i]);
}
for(i=0;i<Info_Num;i++)
{
	free(intfName[i]);
}

free(select_a);
release(lpublic);  
release(lcontrol);
return 0;

}

int interfaceInfo(char * IntfInfo[],int * Num,struct list * lpublic)
	{
		FILE * ft;
		char * syscommand1=(char *)malloc(200);
		memset(syscommand1,0,200);
		char * syscommand2=(char *)malloc(200);
		memset(syscommand2,0,200);
		strcat(syscommand1,"ip_addr.sh 2>/dev/null");
		strcat(syscommand2,"awk '{print $1}' /var/run/apache2/ip_addr.file> /var/run/apache2/InterfaceName.txt");
		int status1 =system(syscommand1);
		int status2 =system(syscommand2);
		int ret1 = WEXITSTATUS(status1);
		int ret2 = WEXITSTATUS(status2);
							 
		if(0==ret1)
			{}
		else ShowAlert(search(lpublic,"bash_fail"));
		
		if(0==ret2)
			{}
		else ShowAlert(search(lpublic,"bash_fail"));
	
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
				if(strstr(temp,"lo")==NULL && strstr(temp,"eth0")==NULL && strstr(temp,"eth1")==NULL && strstr(temp,"eth2")==NULL && strstr(temp,"eth3")==NULL)
				{
					strcpy(IntfInfo[i],temp);
					i++;
					memset(temp,0,20);
				}
					//fprintf(stderr,"IntfInfo[%d]=%s",i,IntfInfo[i]);
			}
		fclose(ft);
		*Num=i;
		free(syscommand1);
		free(syscommand2);
		return 1;
	}

#ifndef ADD_ROUTE_MODULE
#define ADD_ROUTE_MODULE
#define IP_ADDR_LEN_PER_DOT 4
#endif
															 
int addroute_hand(struct list *lpublic,struct list *lcontrol)
{
	//char target_ip1[4],target_ip2[4],target_ip3[4],target_ip4[4];
	//char mask1[4],mask2[4],mask3[4],mask4[4];
	char * target_ip1 = (char *)malloc(IP_ADDR_LEN_PER_DOT);
	char * target_ip2 = (char *)malloc(IP_ADDR_LEN_PER_DOT);
	char * target_ip3 = (char *)malloc(IP_ADDR_LEN_PER_DOT);
	char * target_ip4 = (char *)malloc(IP_ADDR_LEN_PER_DOT);


	char * mask1 = (char *)malloc(IP_ADDR_LEN_PER_DOT);
	char * mask2 = (char *)malloc(IP_ADDR_LEN_PER_DOT);
	char * mask3 = (char *)malloc(IP_ADDR_LEN_PER_DOT);
	char * mask4 = (char *)malloc(IP_ADDR_LEN_PER_DOT);
	
	char next_hop1[4],next_hop2[4],next_hop3[4],next_hop4[4];
	unsigned long mask[4];
	unsigned long maskIDentify;
	int flag=0,flag_intf=0,net_hop=0; //net_hop为了添加0.0.0.0/0的无下一条的默认路由
	char * distance_SR=(char *)malloc(10);
	memset(distance_SR,0,10);
	char * staticRoute_mode=(char *)malloc(10);
	memset(staticRoute_mode,0,10);


    int ret=0;
	char target_IP[20];
	memset(target_IP,0,20);
																   
	char route_mask[20];
	memset(route_mask,0,20);
																   
	char next_hop[20];
	memset(next_hop,0,20);
															 
	char sysShell[100];
	memset(sysShell,0,100);
															 
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
															 
																 
	memset(mask1,0,4);
	cgiFormStringNoNewlines("mask1",mask1,4);
	strcat(route_mask,mask1);
	strcat(route_mask,".");		 
	mask[0]=atoi(mask1);
																	 
	memset(mask2,0,4);
	cgiFormStringNoNewlines("mask2",mask2,4);
	strcat(route_mask,mask2);
	strcat(route_mask,".");	 
	mask[1]=atoi(mask2);
																	 
	memset(mask3,0,4);
	cgiFormStringNoNewlines("mask3",mask3,4);
	strcat(route_mask,mask3);
	strcat(route_mask,".");	 
	mask[2]=atoi(mask3);
																	 
	memset(mask4,0,4);
	cgiFormStringNoNewlines("mask4",mask4,4);
	strcat(route_mask,mask4);		 
	mask[3]=atoi(mask4);
	//fprintf(stderr,"11111111111");
	target_ip1 = trim_zero(target_ip1);
	target_ip2 = trim_zero(target_ip2);
	target_ip3 = trim_zero(target_ip3);
	target_ip4 = trim_zero(target_ip4);

	mask1 = trim_zero(mask1);
	mask2 = trim_zero(mask2);
	mask3 = trim_zero(mask3);
	mask4 = trim_zero(mask4);
	//fprintf(stderr,"2222222222222");																 
	maskIDentify=(mask[0] << 24) + (mask[1] << 16) + (mask[2] << 8) + mask[3];
	if(!(strcmp(target_ip1,"0")||strcmp(target_ip2,"0")||strcmp(target_ip3,"0")||strcmp(target_ip4,"0") ||
 	      strcmp(mask1,"0")||strcmp(mask2,"0")||strcmp(mask3,"0")||strcmp(mask4,"0")))
 	    {
 	    	net_hop = 0;
 	    }
 	else
 	{
 		net_hop = 1;
    	if(0==checkMark(maskIDentify))
    		{
    			ShowAlert(search(lcontrol,"mask_incorrect"));
    			return 0;
    		}
		
	}

	int Nexthopchoice=0;
	cgiFormSelectSingle("nethop_type", nexthop, 3, &Nexthopchoice, 0);
	//fprintf(stderr,"Nexthopchoice=%d",Nexthopchoice);
	switch(Nexthopchoice)
		{
			case 0:
				{
                	memset(next_hop1,0,4);
                	cgiFormStringNoNewlines("next_hop1",next_hop1,4);
                	strcat(next_hop,next_hop1);
                	strcat(next_hop,".");
                	memset(next_hop2,0,4);
                	cgiFormStringNoNewlines("next_hop2",next_hop2,4);
                	strcat(next_hop,next_hop2);
                	strcat(next_hop,".");
                	memset(next_hop3,0,4);
                	cgiFormStringNoNewlines("next_hop3",next_hop3,4);
                	strcat(next_hop,next_hop3);
                	strcat(next_hop,".");
                	memset(next_hop4,0,4);
                	cgiFormStringNoNewlines("next_hop4",next_hop4,4);
                	strcat(next_hop,next_hop4);
                														 
                	if(!(strcmp(next_hop1,"")||strcmp(next_hop2,"")||strcmp(next_hop3,"")||strcmp(next_hop4,"")) )
                	 	{
                			flag=1;  //net_hop 都为空
                			//return 0;
                	 	}
                	else if(strcmp(next_hop1,"")&&strcmp(next_hop2,"")&&strcmp(next_hop3,"")&&strcmp(next_hop4,""))
                		{
                			flag=2;
                		}
                	else
                		{
                			ShowAlert(search(lcontrol,"ip_incorrect"));
                    		return 0;
                		}
				}
				//fprintf(stderr,"444444flag=%d-4444444",flag);
				break;
			case 1:
					cgiFormStringNoNewlines("route_intf",next_hop,20);
					flag_intf=1;
				break;
			case 2:
					strcat(next_hop,"null0");
					flag_intf=1;
				break;
		}
	//fprintf(stderr,"flag=%d--flag_intf=%d--net_hop=%d",flag,flag_intf,net_hop);

	int temp_dis=0,dis_ret=0;	
	cgiFormStringNoNewlines("distance_SR",distance_SR,10);
	dis_ret=str2ulong(distance_SR,&temp_dis,1,255);
  	cgiFormStringNoNewlines("staticRoute_mode",staticRoute_mode,10);
  	//fprintf(stderr,"dis_ret=%d--flag=%d--staticRoute_mode=%s",dis_ret,flag,staticRoute_mode);
  	if(dis_ret == -2)
  		{
  			ShowAlert(search(lcontrol,"no_num_error"));
			ret= -1 ;
			goto ret_error;
  		}
	else if(dis_ret == -3)
		{
			ShowAlert(search(lcontrol,"num_range_error"));
			ret= -1 ;
			goto ret_error;
		}
	else if(dis_ret == 0)
		{
        	if(flag==2)
        		{
	               	strcat(sysShell,"set_static_route.sh");
	               	strcat(sysShell," ");
	               	strcat(sysShell,target_IP);
	               	strcat(sysShell," ");
	               	strcat(sysShell,route_mask);
	               	strcat(sysShell," ");
	               	strcat(sysShell,next_hop);
	               	strcat(sysShell," ");
	               	strcat(sysShell,staticRoute_mode);
					if( strcmp(staticRoute_mode,"equalize") )
					{
	               		strcat(sysShell," ");
	               		strcat(sysShell,distance_SR);
					}
	               	strcat(sysShell," ");
	               	strcat(sysShell,">/var/run/apache2/addroute.txt");
	               	int status = system(sysShell);	 
	               	int ret = WEXITSTATUS(status);
					//fprintf(stderr,"sysShell=%s--flag=%d--dis_ret=%d",sysShell,flag,dis_ret);
	      			//////////////////////////////
	      			FILE *fd;
	               	char  temp[60];
	               	memset(temp,0,60);
	               							 
	               	if((fd=fopen("/var/run/apache2/addroute.txt","r"))==NULL)
	               		{
	               			ShowAlert(search(lpublic,"error_open"));
	               			return 0;
	               		}
	               	//int i=0;
	               	while((fgets(temp,60,fd)) != NULL)
	               		{
	               			if(strstr(temp,"% The system already has this static route")!=NULL)
	               				{
	               					ShowAlert(search(lcontrol,"already_has_route"));
	               					return -1;
	               				}
	               		}
					fclose(fd);
	      	         ///////////////////////////
	               	if(0==ret)
	               		ShowAlert(search(lcontrol,"add_route_suc"));
	               	else ShowAlert(search(lcontrol,"add_route_fail"));
        		}
      	else if(flag==1)
      		{
      			if(net_hop == 0)
      			{
        			strcat(sysShell,"set_static_route.sh");
                 	strcat(sysShell," ");
                 	strcat(sysShell,target_IP);
                 	strcat(sysShell," ");
                 	strcat(sysShell,route_mask);
                 	strcat(sysShell," ");
                 	strcat(sysShell,staticRoute_mode);
                 	strcat(sysShell," ");
                 	strcat(sysShell,distance_SR);
                 	strcat(sysShell," ");
                 	strcat(sysShell,">/var/run/apache2/addroute.txt");
					//fprintf(stderr,"sysShell=%s--flag=%d--net_hop=%d",sysShell,flag,net_hop);
    				
                   	int status = system(sysShell);	 
                   	int ret = WEXITSTATUS(status);
          			//////////////////////////////
          			FILE *fd;
                   	char  temp[60];
                   	memset(temp,0,60);
                   							 
                   	if((fd=fopen("/var/run/apache2/addroute.txt","r"))==NULL)
                   		{
                   			ShowAlert(search(lpublic,"error_open"));
                   			return 0;
                   		}
                   	//int i=0;
                   	while((fgets(temp,60,fd)) != NULL)
                   		{
                   			if(strstr(temp,"% The system already has this static route")!=NULL)
                   				{
                   					ShowAlert(search(lcontrol,"already_has_route"));
                   					return -1;
                   				}
                   		}
					fclose(fd);
          	         ///////////////////////////
                   	if(0==ret)
                   		ShowAlert(search(lcontrol,"add_route_suc"));
                   	else ShowAlert(search(lcontrol,"add_route_fail"));
      			}
				else
					ShowAlert(search(lcontrol,"INPUT_BADPARAM"));
      		}		
		else if( flag == 0 )
		{
			if( flag_intf == 1 )
			{
				strcat(sysShell,"set_static_route.sh");
	         	strcat(sysShell," ");
	         	strcat(sysShell,target_IP);
	         	strcat(sysShell," ");
	         	strcat(sysShell,route_mask);
	         	strcat(sysShell," ");
				
				if (strcmp(next_hop,"null0"))
	         		strcat(sysShell,next_hop);
				else
					strcat(sysShell,staticRoute_mode);

	         	strcat(sysShell," ");
	         	strcat(sysShell,distance_SR);
	         	strcat(sysShell," ");
	         	strcat(sysShell,">/var/run/apache2/addroute.txt");
				int status = system(sysShell);	 
	           	int ret = WEXITSTATUS(status);
				//fprintf(stderr,"sysShell=%s--flag=%d--flag_intf=%d--next_hop=%s",sysShell,flag,flag_intf,next_hop);
	  			//////////////////////////////
	  			FILE *fd;
	           	char  temp[60];
	           	memset(temp,0,60);
	           							 
	           	if((fd=fopen("/var/run/apache2/addroute.txt","r"))==NULL)
	           		{
	           			ShowAlert(search(lpublic,"error_open"));
	           			return 0;
	           		}
	           	//int i=0;
	           	while((fgets(temp,60,fd)) != NULL)
	           		{
	           			if(strstr(temp,"% The system already has this static route")!=NULL)
	           				{
	           					ShowAlert(search(lcontrol,"already_has_route"));
	           					return -1;
	           				}
	           		}
				fclose(fd);
	  	         ///////////////////////////
	           	if(0==ret)
	           		ShowAlert(search(lcontrol,"add_route_suc"));
	           	else ShowAlert(search(lcontrol,"add_route_fail"));
			}
		}
		}
ret_error:
	free(target_ip1);
	free(target_ip2);
	free(target_ip3);
	free(target_ip4);

	free(mask1);
	free(mask2);
	free(mask3);
	free(mask4);
	free(distance_SR);
	free(staticRoute_mode);
	return ret;
}
															 
// 输入已经转换为按主机字节序的 unsigned long
int checkMark(unsigned long mark)
{
	return ((((mark ^ (mark - 1)) >> 1) ^ mark) == -1);
}


int ShowIPRoute(char * routeInfo[],int * route_num,int * Kroute_num,int * Sroute_num,int * Croute_num)
{
	int i=0,c=0,k=0,s=0,j=0;
	char * showRoute=(char *)malloc(350);
	memset(showRoute,0,350);

	char  routePath[128];
	memset(routePath,0,128);
	char * revRouteInfo[4];

	
	strcat(showRoute,"show_route.sh 2>/dev/null | awk '{OFS=\"-\"}NR==4,NR==0{if($1~/S/){print $1,$2,$5,$6,$3,$7} else if($1~/K/){print $1,$2,$4,$5,$3} else if($1~/C/){print $1,$2,$6,$3} else if($1~/R/){print $1,$2,$5,$6,$3} else if($1~/O/){print $1,$2,$5,$7,$3}}'>/var/run/apache2/ip_route.txt");
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
	*route_num=i;
	fprintf(stderr,"route_num=%d",i);
	for(i=0;i<*route_num;i++)
		{
	
			revRouteInfo[0]=strtok(routeInfo[i],"-,");							
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
		}
	*Sroute_num=s;
	*Croute_num=c;
	*Kroute_num=k;
	//fprintf(stderr,"Sroute_num=%d-Croute_num=%d-Kroute_num=%d",s,c,k);
	fclose(fd);
	free(showRoute);

	return 1;
}

char* trim_zero(char * src)
{
	if (!strcmp(src,"00"))
	{
		memset(src, 0, 4);
		strcpy(src,"0");
	}

		return src;	
}


