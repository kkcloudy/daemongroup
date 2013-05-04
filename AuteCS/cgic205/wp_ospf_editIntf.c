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
* wp_ospf_editintf.c
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
#include "ws_dhcp_conf.h"


#include <sys/wait.h>
#define Info_Num 500
#define VIRTUAL_NUM 16

//将int 32的值转化成ip地址字符串
#define INET_NTOA(ip_int,addr_str)\
		{\
			unsigned int a1,a2,a3,a4;\
			unsigned int ip_uint = (unsigned int)ip_int;\
			a1 = (ip_uint&0xff000000)>>24;\
			a2 = (ip_uint&0x00ff0000)>>16;\
			a3 = (ip_uint&0x0000ff00)>>8;\
			a4 = (ip_uint&0x000000ff);\
			sprintf( addr_str, "%d.%d.%d.%d", a1,a2,a3,a4 );\
		}



int ShowEditIntfPage();
int ReadConfig_ospf(char * ospfInfo[],int * infoNum,struct list * lpublic);
int OSPF_ReadConfig_For_ABR(char * ospf_info[],int * infoNum,struct list * lpublic);

int executeconfig(char * intfName_par,char * address_par,char * area_par,char * auth_mode_param,struct list * lcontrol,struct list * lpublic);
char * turn_area_value(char * src);
char * trim_area_address(char * src);
int radiobutton_last = 0;


int cgiMain()
{
 ShowEditIntfPage();
 return 0;
}

int ShowEditIntfPage()
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
	char encry_EditIntf[BUF_LEN];

	//int IntfNum=0;

	char * Area=(char *)malloc(50);
	memset(Area,0,50);
	char * Area_type=(char *)malloc(30);
	memset(Area_type,0,30);


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

	char * MTU_check=(char *)malloc(10);
	memset(MTU_check,0,10);
	
	char * intfName=(char *)malloc(30);
	memset(intfName,0,30);
	char * address=(char *)malloc(60);
	memset(address,0,60);
	char * auth_key=(char *)malloc(30);
	memset(auth_key,0,30);
	char * auth_mode=(char *)malloc(10);
	memset(auth_mode,0,10);
	char * MD5key_id=(char *)malloc(30);
	memset(MD5key_id,0,30);

	char * intfNameLater=(char *)malloc(30);
	memset(intfNameLater,0,30);
	char * addressLater=(char *)malloc(60);
	memset(addressLater,0,60);
	char * AreaLater=(char *)malloc(50);
	memset(AreaLater,0,50);
	char * virtual_link=(char *)malloc(50);
	memset(virtual_link,0,50);
	
////////////////////////////area/////////////////////////////////
	char * AreaInfo[Info_Num];
	for(i=0;i<Info_Num;i++)
	{
		AreaInfo[i]=(char *)malloc(60);
		memset(AreaInfo[i],0,60);
	}
	
	char * filter_in=(char *)malloc(10);
	memset(filter_in,0,10);
	char * filter_out=(char *)malloc(10);
	memset(filter_out,0,10);
	char * Range_prefix=(char *)malloc(30);
	memset(Range_prefix,0,30);
	char * Range_type=(char *)malloc(20);
	memset(Range_type,0,20);
	char * Area_type_param=(char *)malloc(20);
	memset(Area_type_param,0,20);
	char * turn_back=(char *)malloc(50);
	memset(turn_back,0,50);
	

	char * ABR_OrNot=(char *)malloc(10);
	memset(ABR_OrNot,0,10);
/////////////////////////////////////////////////////////////////
	if(cgiFormSubmitClicked("submit_EditIntf") != cgiFormSuccess)
	{
	  memset(encry,0,BUF_LEN);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN);
	  
	  cgiFormStringNoNewlines("IFNAME", intfName, 30);
	  cgiFormStringNoNewlines("AREA",Area,50);
	  cgiFormStringNoNewlines("AREA_TYPE",Area_type,30);  
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
	  
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(encry_EditIntf,0,BUF_LEN);					 /*清空临时变量*/
	}
	else
	{
		cgiFormStringNoNewlines("md5key_id",MD5key_id,30);
	}
	fprintf(stderr,"AREA=%s-AREA_TYPE=%s-auth_mode=%s-auth_key=%s",Area,Area_type,auth_mode,auth_key);
	//fprintf(stderr,"intfName=%s-Area=%s-address=%s-Hello=%s-Dead=%s-Transmit=%s-Retransmit=%s-Priority=%s-Network_type=%s-MTU_check=%s",intfName,Area,address,Hello,Dead,Transmit,Retransmit,Priority,Network_type,MTU_check);
  cgiFormStringNoNewlines("encry_EditIntf",encry_EditIntf,BUF_LEN);
  cgiFormStringNoNewlines("intfNameLater",intfNameLater,30);
  cgiFormStringNoNewlines("addressLater",addressLater,60);
  cgiFormStringNoNewlines("AreaLater",AreaLater,50);
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
  
 			
      "if(obj.value==\"NoAuth\" || obj==\"a\")"\
      "{"\
      		"document.formedit.simplepwd.disabled=true;"\
      		"document.formedit.MD5pwd.disabled=true;"\
      		"document.formedit.MD5key_id.disabled=true;"\
      		"document.formedit.simplepwd.style.backgroundColor = \"#ccc\";"\
      		"document.formedit.MD5pwd.style.backgroundColor = \"#ccc\";"\
      		"document.formedit.MD5key_id.style.backgroundColor = \"#ccc\";"\

      "}"\
      "else if(obj.value==\"text\" || obj==\"b\" )"\
      "{"\
      		"document.formedit.simplepwd.disabled=false;"\
      		"document.formedit.MD5pwd.disabled=true;"\
      		"document.formedit.MD5key_id.disabled=true;"\
      		"document.formedit.simplepwd.style.backgroundColor = \"#fff\";"\
      		"document.formedit.MD5pwd.style.backgroundColor = \"#ccc\";"\
      		"document.formedit.MD5key_id.style.backgroundColor = \"#ccc\";"\
      "}"\
      "else if(obj.value==\"MD5\"  || obj==\"c\")"\
      "{"\
      		"document.formedit.simplepwd.disabled=true;"\
      		"document.formedit.MD5pwd.disabled=false;"\
      		"document.formedit.MD5key_id.disabled=false;"\
      		"document.formedit.MD5pwd.style.backgroundColor = \"#fff\";"\
		    "document.formedit.MD5key_id.style.backgroundColor = \"#fff\";"\
      		"document.formedit.simplepwd.style.backgroundColor = \"#ccc\";"\
      "}"\
      "if(document.formedit.IS_ABR.checked==true)"\
      "{"\

  			"document.formedit.advertise_param.disabled=false;"\
  			"document.formedit.route_range.disabled=false;"\
		
  			"document.formedit.src_ip1.disabled=false;"\
  			"document.formedit.src_ip2.disabled=false;"\
  			"document.formedit.src_ip3.disabled=false;"\
  			"document.formedit.src_ip4.disabled=false;"\
  			"document.formedit.masklen.disabled=false;"\
  			"document.formedit.masklen.style.backgroundColor = \"#fff\";"\
      		
      "}"\
      "else if(document.formedit.IS_ABR.checked==false)"\
      "{"\

  			"document.formedit.advertise_param.disabled=true;"\
			"document.formedit.route_range.disabled=true;"\

  			"document.formedit.src_ip1.disabled=true;"\
  			"document.formedit.src_ip2.disabled=true;"\
  			"document.formedit.src_ip3.disabled=true;"\
  			"document.formedit.src_ip4.disabled=true;"\
  			"document.formedit.masklen.disabled=true;"\
  			"document.formedit.masklen.style.backgroundColor = \"#ccc\";"\
      "}"\
     "if(document.formedit.area_type.value==\"nssa\")"\
      "{"\
    	"document.formedit.nssa_param.disabled=false;"\
      "}"\
     "else if(document.formedit.area_type.value==\"stub\")"\
      "{"\
    	"document.formedit.nssa_param.disabled=true;"\
      "}"\
  "}"\
  "</script>");
  
  if(strcmp(auth_mode,"Simple")==0)
  {
  	radiobutton_last = 1;
  	fprintf(cgiOut,"<body onload=changestate(\"b\")>");
  }
  else if(strcmp(auth_mode,"MD5")==0)
  {
  	radiobutton_last = 2;
  	fprintf(cgiOut,"<body onload=changestate(\"c\")>");
  }
  else 
  {
  	radiobutton_last = 0;
  	fprintf(cgiOut,"<body onload=changestate(\"a\")>");
  }
  
  //ccgi_dbus_init();
  if(cgiFormSubmitClicked("submit_EditIntf") == cgiFormSuccess)
  {
  	 fprintf(stderr,"intfNameLater=%s-addressLater=%s-AreaLater=%s",intfNameLater,addressLater,AreaLater);
  	 executeconfig(intfNameLater,addressLater,AreaLater,auth_mode,lcontrol,lpublic);
     fprintf( cgiOut, "<script type='text/javascript'>\n" );
   	 fprintf( cgiOut, "window.location.href='wp_ospf_intf.cgi?UN=%s';\n", encry_EditIntf);
   	 fprintf( cgiOut, "</script>\n" );
  }

  fprintf(cgiOut,"<form method=post name=formedit>"\
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
          "<td width=62 align=center><input id=but type=submit name=submit_EditIntf style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));	  
		  if(cgiFormSubmitClicked("submit_EditIntf") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_ospf_intf.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_ospf_intf.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry_EditIntf,search(lpublic,"img_cancel"));
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
                		if(cgiFormSubmitClicked("submit_EditIntf") != cgiFormSuccess)
                		{
                			fprintf(cgiOut,"<tr height=25>"\
                			"<td align=left id=tdleft><a href=wp_ospf_bcon.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));					   
                			fprintf(cgiOut,"</tr>");
                			fprintf(cgiOut,"<tr height=26>"\
                			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>OSPF</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"ospf_intf_conf"));   /*突出显示*/
                			fprintf(cgiOut,"</tr>"\
                			"<tr height=25>"\
                			"<td align=left id=tdleft><a href=wp_ospf_addintf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_ospf_intf"));
                			fprintf(cgiOut,"</tr>");
                 			
                		}
                		else if(cgiFormSubmitClicked("submit_EditIntf") == cgiFormSuccess)					
                		{
                			fprintf(cgiOut,"<tr height=25>"\
                			"<td align=left id=tdleft><a href=wp_ospf_bcon.cgi?UN=%s target=mainFrame style=color:#000000><font id=yingwen_san>OSPF</font><font id=%s>%s</font></a></td>",encry_EditIntf,search(lpublic,"menu_san"),search(lcontrol,"rip_conf"));						
                			fprintf(cgiOut,"</tr>");
                			fprintf(cgiOut,"<tr height=26>"\
                			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>OSPF</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"ospf_intf_conf"));   /*突出显示*/
                			fprintf(cgiOut,"</tr>"\
                			"<tr height=25>"\
                			"<td align=left id=tdleft><a href=wp_ospf_addintf.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",encry_EditIntf,search(lpublic,"menu_san"),search(lcontrol,"add_ospf_intf"));
                			fprintf(cgiOut,"</tr>");

                		}

					  for(i=0;i<20;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=340 border=0 cellspacing=0 cellpadding=0>");
					  	fprintf(cgiOut,"<tr>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"interface_info"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						  "<td align=left valign=top style=padding-top:10px>"\
						  "<table width=500 border=0 cellspacing=0 cellpadding=0>");
					/////////////////////////////////////////area_readconfig/////////////////////////////////////////////	
							int area_info_num=0,j=0,ospf_router_num=0;
							char * area_temp = turn_area_value(Area);
							fprintf(stderr,"area_temp=%s",area_temp);

							Area_ReadConfig(area_temp,AreaInfo,&area_info_num,lpublic);
							char * ef_area = NULL;
							ef_area  = trim_area_address(area_temp);
							fprintf(stderr,"ef_area=%s",ef_area);
							char * rev_info[5];
							for(i=0;i<area_info_num;i++)
								{
									rev_info[0]=strtok(AreaInfo[i],"*");
									if(strstr(rev_info[0],"##In#fliter-list") != NULL)
										{
											j=0;
                     						while(rev_info[j]!=NULL && j<1)
                 								{                    									
                 									rev_info[j+1]=strtok(NULL,"*#");
                 									j++;
                 								}
											strcpy(filter_in,rev_info[1]);
											fprintf(stderr,"filter_in=%s",filter_in);
										}
									else if(strstr(rev_info[0],"##Out#fliter-list") != NULL)
										{
											j=0;
                     						while(rev_info[j]!=NULL && j<1)
                 								{                    									
                 									rev_info[j+1]=strtok(NULL,"*#");
                 									j++;
                 								}
											strcpy(filter_out,rev_info[1]);
											fprintf(stderr,"filter_out=%s",filter_out);
										}
									else if(strstr(rev_info[0],"###Range#prefix") != NULL)
										{
											j=0;
                     						while(rev_info[j]!=NULL && j<1)
                 								{                    									
                 									rev_info[j+1]=strtok(NULL,"*#");
                 									j++;
                 								}
											//if( strstr() != NULL)
											strcpy(Range_prefix,rev_info[1]);
											fprintf(stderr,"Range_prefix=%s",Range_prefix);
										}
									else if(strstr(rev_info[0],"###Range#type") != NULL)
										{
											j=0;
                     						while(rev_info[j]!=NULL && j<1)
                 								{                    									
                 									rev_info[j+1]=strtok(NULL,"*#");
                 									j++;
                 								}
											strcpy(Range_type,rev_info[1]);
											fprintf(stderr,"Range_type=%s",Range_type);
										}
									else if(strstr(rev_info[0],"###Statu") != NULL)
										{
											j=0;
                     						while(rev_info[j]!=NULL && j<1)
                 								{                    									
                 									rev_info[j+1]=strtok(NULL,"*");
                 									j++;
                 								}
											strcpy(Area_type_param,rev_info[1]);
											fprintf(stderr,"Area_type_param=%s",Area_type_param);
										}
									else if(strstr(rev_info[0],"###Number#of#NSSA#LSA") != NULL)
										{
											if(rev_info[1]!=NULL)
											{
												strcpy(turn_back,rev_info[1]);
											}
											fprintf(stderr,"turn_back=%s",turn_back);
										}
									
									
								}
							if(strstr(Area_type_param,",#no#summary")!=NULL)
								{
									//fprintf(stderr,"1111111xx");
									memset(Area_type_param,0,20);
									strcpy(Area_type_param,"no-summary");
								}
							
							char * Range_prefix_ip1=(char *)malloc(4);
							memset(Range_prefix_ip1,0,4);
							char * Range_prefix_ip2=(char *)malloc(4);
							memset(Range_prefix_ip2,0,4);
							char * Range_prefix_ip3=(char *)malloc(4);
							memset(Range_prefix_ip3,0,4);
							char * Range_prefix_ip4=(char *)malloc(4);
							memset(Range_prefix_ip4,0,4);
							char * Range_prefix_mask=(char *)malloc(4);
							memset(Range_prefix_mask,0,4);
							int is_same_subnet = 0;
							char *tempt;
							char temp[20];
							char source_ip[20];
							//char source_mask[20];
							char source_subnet[20];
							//char temp_source_mask[20];
							char target_ip[20];
							char target_subnet[20];
							char *endptr = NULL;  
							int temp_mask = 0;
							int range_mask = 0;
							char prefix_mask[20];

							char * virtual_prefix_ip1=(char *)malloc(4);
							memset(virtual_prefix_ip1,0,4);
							char * virtual_prefix_ip2=(char *)malloc(4);
							memset(virtual_prefix_ip2,0,4);
							char * virtual_prefix_ip3=(char *)malloc(4);
							memset(virtual_prefix_ip3,0,4);
							char * virtual_prefix_ip4=(char *)malloc(4);
							memset(virtual_prefix_ip4,0,4);

							char * virtual_prefix_area=(char *)malloc(10);
							memset(virtual_prefix_area,0,10);
							
							if (strlen(Range_prefix) > 0)
							{
								memset(target_ip,0,20);
								strcpy(target_ip,Range_prefix);
								
								rev_info[0]=strtok(Range_prefix,".");
								rev_info[1]=strtok(NULL,".");
								rev_info[2]=strtok(NULL,".");
								rev_info[3]=strtok(NULL,"./");
								rev_info[4]=strtok(NULL,"/");
								strcpy(Range_prefix_ip1,rev_info[0]);
								strcpy(Range_prefix_ip2,rev_info[1]);
								strcpy(Range_prefix_ip3,rev_info[2]);
								strcpy(Range_prefix_ip4,rev_info[3]);
								strcpy(Range_prefix_mask,rev_info[4]);

								temp_mask=strtoul(Range_prefix_mask,&endptr,10);
								range_mask = 0xffffffff^((1<<(32-temp_mask))-1);
								memset(prefix_mask,0,20);
								INET_NTOA(range_mask,prefix_mask);
								//fprintf(stderr,"\n#####prefix_mask=%s####\n",prefix_mask);
								memset(target_subnet,0,20);
								get_sub_net(target_ip,prefix_mask,target_subnet);
								//fprintf(stderr,"\n#####target_subnet=%s####\n",target_subnet);

								memset(temp,0,20);
								strcpy(temp,address);
								tempt=strtok(temp,"/");
								if(tempt!=NULL)
								{
									memset(source_ip,0,20);
									strcpy(source_ip,tempt);
									/*tempt=strtok(NULL,"/");
									memset(temp_source_mask,0,20);
									strcpy(temp_source_mask,tempt);
									fprintf(stderr,"\n#####temp_source_mask=%s####\n",temp_source_mask);

									temp_mask=strtoul(temp_source_mask,&endptr,10);
									range_mask = 0xffffffff^((1<<(32-temp_mask))-1);
									memset(source_mask,0,20);
									INET_NTOA(range_mask,source_mask);
									fprintf(stderr,"\n#####source_mask=%s####\n",source_mask);*/

									memset(source_subnet,0,20);
									get_sub_net(source_ip,prefix_mask,source_subnet);
									//fprintf(stderr,"\n#####source_subnet=%s####\n",source_subnet);

									if(strcmp(source_subnet,target_subnet)==0)
									{
										is_same_subnet = 1;
									}
								}
								
							}
							
			//////////////////////////////////end area_readconfig for virsual_link////////////////////////////////////////////////
							for (i=0;i<Info_Num;i++)
							{
								memset(AreaInfo[i],0,60);
							}

							ReadConfig_ospf(AreaInfo,&ospf_router_num,lpublic);

							for (i=0;i<ospf_router_num;i++)
							{
								rev_info[0]=strtok(AreaInfo[i],"-");
								if (strstr(rev_info[0],"area")!=NULL)
								{
									j=0;
             						while (rev_info[j]!=NULL && j<2)
         								{                    									
         									rev_info[j+1]=strtok(NULL,"-\n");
         									j++;
         								}
									strcpy(virtual_prefix_area,rev_info[1]);
									fprintf(stderr,"virtual_prefix_area=%s",virtual_prefix_area);
									if (strstr(Area,virtual_prefix_area) != NULL && strstr(rev_info[2],"virtual") != NULL)
									{
										rev_info[3]=strtok(NULL,"-\n");
										rev_info[4]=strtok(NULL,"-\n");
										strcpy(virtual_link,rev_info[4]);
									}
									fprintf(stderr,"virtual_link=%s",virtual_link);
								}
							}
		
							if (strcmp(virtual_link,"")!=0)
							{
								rev_info[0]=strtok(virtual_link,".");
								rev_info[1]=strtok(NULL,".");
								rev_info[2]=strtok(NULL,".");
								rev_info[3]=strtok(NULL,"./");
								
								strcpy(virtual_prefix_ip1,rev_info[0]);
								strcpy(virtual_prefix_ip2,rev_info[1]);
								strcpy(virtual_prefix_ip3,rev_info[2]);
								strcpy(virtual_prefix_ip4,rev_info[3]);

							}

			//////////////////////////////////////////for ABR/////////////////////////////////////////////////////////////
							for (i=0;i<Info_Num;i++)
							{
								memset(AreaInfo[i],0,60);
							}
							OSPF_ReadConfig_For_ABR(AreaInfo,&ospf_router_num,lpublic);
							for (i=0; i<ospf_router_num; i++)
								{
									rev_info[0]=strtok(AreaInfo[i],"*");
									if (strstr(rev_info[0],"#Router#type") != NULL)
										{
											j=0;
                     						while (rev_info[j]!=NULL && j<1)
                 								{                    									
                 									rev_info[j+1]=strtok(NULL,"*#");
                 									j++;
                 								}
											strcpy(ABR_OrNot,rev_info[1]);
											fprintf(stderr,"ABR_OrNot=%s",ABR_OrNot);
										}
								}
							
							fprintf(cgiOut,"<tr align=left height=35>"\
							"<td width=10 style=color:red>*</td>");
							fprintf(cgiOut,"<td align=left width=60>%s: </td>",search(lcontrol,"interface"));
							fprintf(cgiOut,"<td align=left width=60>%s</td>",intfName);
							fprintf(cgiOut,"<td align=right width=60>%s: </td>",search(lcontrol,"network"));
							fprintf(cgiOut,"<td align=left width=125>%s</td>",address);
							fprintf(cgiOut,"<td align=right width=60>%s: </td>",search(lcontrol,"area_ID"));
							fprintf(cgiOut,"<td align=left width=125>%s</td>",Area);
							fprintf(cgiOut,"</tr>"\
							
							"</table>"\
						  "</td>"\
						"</tr>"\
						"<tr>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-top:12px\">%s</td>",search(lcontrol,"interface_set"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td>"\
						"<table width=500 border=0 cellspacing=0 cellpadding=0>"\
						"<tr height=35 style=padding-top:5px>"\
						"<td width=10>&nbsp;</td>");
						fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"intf_net_type"));
						fprintf(cgiOut,"<td align=left width=120 colspan=4>");
						fprintf(cgiOut, "<select name=if_netType>");
						//fprintf(stderr,"Network_type=%s",Network_type);
						if (strcmp(Network_type,"NBMA")==0)
							{
                            	fprintf(cgiOut, "<option value=non-broadcast>NBMA");
        						fprintf(cgiOut, "<option value=broadcast>%s",search(lcontrol,"broadcast"));
        						fprintf(cgiOut, "<option value=point-to-multipoint>%s",search(lcontrol,"point_to_mulpoint"));
        						fprintf(cgiOut, "<option value=point-to-point>%s",search(lcontrol,"point_to_point"));
							}
						else if (strcmp(Network_type,"BROADCAST")==0)
							{
								fprintf(cgiOut, "<option value=broadcast>%s",search(lcontrol,"broadcast"));
								fprintf(cgiOut, "<option value=non-broadcast>NBMA");	
        						fprintf(cgiOut, "<option value=point-to-multipoint>%s",search(lcontrol,"point_to_mulpoint"));
        						fprintf(cgiOut, "<option value=point-to-point>%s",search(lcontrol,"point_to_point"));
							}
						else if (strcmp(Network_type,"POINTOMULTIPOINT")==0)
							{
								fprintf(cgiOut, "<option value=point-to-multipoint>%s",search(lcontrol,"point_to_mulpoint"));
								fprintf(cgiOut, "<option value=broadcast>%s",search(lcontrol,"broadcast"));
								fprintf(cgiOut, "<option value=non-broadcast>NBMA");	
        						fprintf(cgiOut, "<option value=point-to-point>%s",search(lcontrol,"point_to_point"));
							}
						else if (strcmp(Network_type,"POINTOPOINT")==0)
							{
								fprintf(cgiOut, "<option value=point-to-point>%s",search(lcontrol,"point_to_point"));
								fprintf(cgiOut, "<option value=broadcast>%s",search(lcontrol,"broadcast"));
								fprintf(cgiOut, "<option value=non-broadcast>NBMA");	
        						fprintf(cgiOut, "<option value=point-to-multipoint>%s",search(lcontrol,"point_to_mulpoint"));
							}
						else
							{
								fprintf(cgiOut, "<option value=broadcast>%s",search(lcontrol,"broadcast"));
								fprintf(cgiOut, "<option value=non-broadcast>NBMA");	
        						fprintf(cgiOut, "<option value=point-to-multipoint>%s",search(lcontrol,"point_to_mulpoint"));
        						fprintf(cgiOut, "<option value=point-to-point>%s",search(lcontrol,"point_to_point"));
							}
                     	fprintf(cgiOut, "</select>");
						fprintf(cgiOut,"</td>"\
						"</tr>"\
						"<tr align=left height=35>"\
						"<td width=10>&nbsp;</td>");
						
						fprintf(cgiOut,"<td align=left width=160>%s: </td>",search(lcontrol,"hello_interval"));
						if(strcmp(Hello,"")!=0 && strcmp(Hello,"30")!=0)
							fprintf(cgiOut, "<td width=90 align=left><input type=text name=hello_text size=6 value=%s></td>",Hello);
						else
							fprintf(cgiOut, "<td width=90 align=left><input type=text name=hello_text size=6 value=10></td>");
						
						fprintf(cgiOut,"<td align=left width=160 colspan=2>%s: </td>",search(lcontrol,"dead_interval"));
						if(strcmp(Dead,"")!=0)
							fprintf(cgiOut, "<td width=90 align=left><input type=text name=dead_text size=6 value=%s></td>",Dead);
						else
							fprintf(cgiOut, "<td width=90 align=left><input type=text name=dead_text size=6 value=40></td>");
						fprintf(cgiOut,"</tr>");
						
						fprintf(cgiOut,"<tr align=left height=35>"\
						"<td width=10>&nbsp;</td>");
						fprintf(cgiOut,"<td align=left width=160>%s: </td>",search(lcontrol,"LSA_transmit"));
						if(strcmp(Transmit,"")!=0)
							fprintf(cgiOut, "<td width=90 align=left><input type=text name=LSA_transmit size=6 value=%s></td>",Transmit);
						else
							fprintf(cgiOut, "<td width=90 align=left><input type=text name=LSA_transmit size=6 value=1></td>");
						
						fprintf(cgiOut,"<td align=left width=160 colspan=2>%s: </td>",search(lcontrol,"LSA_retransmit"));
						if(strcmp(Retransmit,"")!=0)
							fprintf(cgiOut, "<td width=90 align=left><input type=text name=LSA_retransmit size=6 value=%s></td>",Retransmit);
						else
							fprintf(cgiOut, "<td width=90 align=left><input type=text name=LSA_retransmit size=6 value=5></td>");
						fprintf(cgiOut,"</tr>");

						fprintf(cgiOut,"<tr align=left height=35>");
						fprintf(stderr,"@@MTU_check=%s",MTU_check);
						if(strcmp(MTU_check,"disabled")==0)
							fprintf(cgiOut, "<td colspan=2><input type=checkbox name=mtu_ignore value=mtu_ig checked>%s</td>",search(lcontrol,"mtu_ignore"));
						else
							fprintf(cgiOut, "<td colspan=2><input type=checkbox name=mtu_ignore value=mtu_ig>%s</td>",search(lcontrol,"mtu_ignore"));
						
						fprintf(cgiOut, "<td align=right>%s: </td>",search(lcontrol,"prior"));
						if(strcmp(Priority,"")!=0)
							fprintf(cgiOut, "<td align=left><input type=text name=Priority size=6 value=%s></td>",Priority);
						else
							fprintf(cgiOut, "<td align=left><input type=text name=Priority size=6 value=></td>");
						fprintf(cgiOut, "<td align=right>%s: </td>",search(lcontrol,"cost"));
						if(strcmp(Cost,"")!=0)
							fprintf(cgiOut, "<td><input type=text name=Cost size=6 value=%s></td>",Cost);
						else
							fprintf(cgiOut, "<td><input type=text name=Cost size=6 value=></td>");
						fprintf(cgiOut,"</tr>"\
						/*"<tr align=left height=35>");
						fprintf(cgiOut, "<td>&nbsp;</td>");

						fprintf(cgiOut,"</tr>"\*/
						"</table>"\
						"</td>"\
						"</tr>");
/////////////////////////////////////area configure///////////////////////////////////////////////////////////////////////////////////
						fprintf(stderr,"Area_type=%s-Area_type_param=%s-filter_in=%s=filter_out=%s",Area_type,Area_type_param,filter_in,filter_out);
						fprintf(cgiOut,"<tr>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-top:12px\">%s</td>",search(lcontrol,"area_set"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td>"\

						"<table width=500 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>");
						fprintf(stderr,"ABR_OrNot=%s",ABR_OrNot);
						if(strstr(ABR_OrNot,"ABR")!=NULL || strcmp(ABR_OrNot,"ASBR")==0)
							fprintf(cgiOut,"<td colspan=5><input type=checkbox name=IS_ABR value=area_abr checked onclick=changestate(this) disabled>%s</td>",search(lcontrol,"IS_ABR"));
						else
							fprintf(cgiOut,"<td colspan=5><input type=checkbox name=IS_ABR value=area_abr onclick=changestate(this) disabled>%s</td>",search(lcontrol,"IS_ABR"));
						fprintf(cgiOut,"</tr>"\
						"<tr height=35 style=padding-top:5px>");
						//"<td width=10>&nbsp;</td>");
						//fprintf(cgiOut,"<td align=left width=100>%s: </td>",search(lcontrol,"area_type"));
						if(strcmp(Area_type,"NSSA")==0 || strcmp(Area_type,"Stub")==0)
							fprintf(cgiOut,"<td width=110><input type=checkbox name=area_type_check checked value=area_type_check>%s</td>",search(lcontrol,"area_type"));
						else
							fprintf(cgiOut,"<td width=110><input type=checkbox name=area_type_check value=area_type_check>%s</td>",search(lcontrol,"area_type"));
						fprintf(cgiOut,"<td align=left width=40>");
						fprintf(stderr,"Area_type=%s-Area_type_param=%s",Area_type,Area_type_param);
						if(strcmp(Area_type,"NSSA")==0)
							{
								fprintf(cgiOut, "<select name=area_type  onchange=changestate(this)>");
		                     	fprintf(cgiOut, "<option value=nssa>nssa");
								fprintf(cgiOut, "<option value=stub>stub");
								fprintf(cgiOut, "<option value=None>None");
		                     	fprintf(cgiOut, "</select>");
							}
						else if(strcmp(Area_type,"Stub")==0)
							{
								fprintf(cgiOut, "<select name=area_type  onchange=changestate(this)>");
								fprintf(cgiOut, "<option value=stub>stub");
								fprintf(cgiOut, "<option value=None>None");
		                     	fprintf(cgiOut, "<option value=nssa>nssa");
		                     	fprintf(cgiOut, "</select>");
							}
						else
							{
								fprintf(cgiOut, "<select name=area_type  onchange=changestate(this)>");
								fprintf(cgiOut, "<option value=None>None");
								fprintf(cgiOut, "<option value=stub>stub");
		                     	fprintf(cgiOut, "<option value=nssa>nssa");
		                     	fprintf(cgiOut, "</select>");
							}
							
						fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"<td align=left width=120>");
						fprintf(cgiOut, "<select name=nssa_param>");
                     	fprintf(cgiOut, "<option value=translate-candidate>translate-candidate");
						fprintf(cgiOut, "<option value=translate-never>translate-never");
						fprintf(cgiOut, "<option value=translate-always>translate-always");
                     	fprintf(cgiOut, "</select>");
						fprintf(cgiOut,"</td>");
						if(strcmp(Area_type_param,"no-summary")==0)
							fprintf(cgiOut,"<td width=240 align=left><input type=checkbox name=no_summary checked value=no_summary>no-summary</td>");
						else
							fprintf(cgiOut,"<td width=240 align=left><input type=checkbox name=no_summary value=no_summary>no-summary</td>");	
						fprintf(cgiOut,"</tr>"\
						"</table>"\
						"</td>"\
						"</tr>"\

						"<tr>"\
						"<td>"\
						"<table width=500 border=0 cellspacing=0 cellpadding=0>"\
						"<tr align=left height=35>");
						if ((strlen(Range_prefix) > 0) && (is_same_subnet == 1))
				        	fprintf(cgiOut,"<td width=110><input type=checkbox name=route_range value=route_range checked>%s</td>",search(lcontrol,"route_range"));
						else
							fprintf(cgiOut,"<td width=110><input type=checkbox name=route_range value=route_range >%s</td>",search(lcontrol,"route_range"));
						fprintf(cgiOut,"<td align=left width=140>"\
					    "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
						if ((strlen(Range_prefix) > 0) && (is_same_subnet == 1))
						{
							fprintf(cgiOut,"<input type=text name=src_ip1 maxlength=3 class=a3 value=%s onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",Range_prefix_ip1,search(lpublic,"ip_error"));
     					    fprintf(cgiOut,"<input type=text name=src_ip2 maxlength=3 class=a3 value=%s onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",Range_prefix_ip2,search(lpublic,"ip_error"));
     					    fprintf(cgiOut,"<input type=text name=src_ip3 maxlength=3 class=a3 value=%s onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",Range_prefix_ip3,search(lpublic,"ip_error"));
     					    fprintf(cgiOut,"<input type=text name=src_ip4 maxlength=3 class=a3 value=%s onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",Range_prefix_ip4,search(lpublic,"ip_error"));
						}
						else
						{
							fprintf(cgiOut,"<input type=text name=src_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
    					    fprintf(cgiOut,"<input type=text name=src_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
    					    fprintf(cgiOut,"<input type=text name=src_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
    					    fprintf(cgiOut,"<input type=text name=src_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
						}
					   
					    fprintf(cgiOut,"</div>");
					    fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"<td width=10 style=padding-left:2px align=center>%s</td>","/");
						if ((strlen(Range_prefix) > 0) && (is_same_subnet == 1))
    						fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=masklen size=3 value=%s></td>",Range_prefix_mask);
						else
							fprintf(cgiOut,"<td  align=left style=padding-left:2px  width=15><input type=text name=masklen size=3></td>");
						fprintf(cgiOut,"<td align=left width=230 style=padding-left:2px>");
						fprintf(cgiOut, "<select name=advertise_param>");
                     	fprintf(cgiOut, "<option value=advertise>advertise");
						fprintf(cgiOut, "<option value=not-advertise>not-advertise");
                     	fprintf(cgiOut, "</select>");
						fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"</tr>"\

						"<tr align=left height=35>");
						if(strcmp(virtual_link,"")!=0)
				        	fprintf(cgiOut,"<td><input type=checkbox name=virtual_link checked value=virtual_link>%s</td>",search(lcontrol,"virtual_link"));
						else
							fprintf(cgiOut,"<td><input type=checkbox name=virtual_link value=virtual_link>%s</td>",search(lcontrol,"virtual_link"));
						fprintf(cgiOut,"<td align=left>"\
					    "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
						if(strcmp(virtual_link,"")!=0)
						{
						    fprintf(cgiOut,"<input type=text name=vlink_ip1 maxlength=3 class=a3 value=%s onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",virtual_prefix_ip1,search(lpublic,"ip_error"));
						    fprintf(cgiOut,"<input type=text name=vlink_ip2 maxlength=3 class=a3 value=%s onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",virtual_prefix_ip2,search(lpublic,"ip_error"));
						    fprintf(cgiOut,"<input type=text name=vlink_ip3 maxlength=3 class=a3 value=%s onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",virtual_prefix_ip3,search(lpublic,"ip_error"));
						    fprintf(cgiOut,"<input type=text name=vlink_ip4 maxlength=3 class=a3 value=%s onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",virtual_prefix_ip4,search(lpublic,"ip_error"));
						}
						else
						{
							fprintf(cgiOut,"<input type=text name=vlink_ip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						    fprintf(cgiOut,"<input type=text name=vlink_ip2 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						    fprintf(cgiOut,"<input type=text name=vlink_ip3 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
						    fprintf(cgiOut,"<input type=text name=vlink_ip4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
						}
					    fprintf(cgiOut,"</div>");
					    fprintf(cgiOut,"</td>");

						fprintf(cgiOut,"<td style=padding-left:2px align=left colspan=3>(%s)</td>",search(lcontrol,"Router_ID"));
						fprintf(cgiOut,"</tr>"\

						"</table>"\
						"</td>"\
						"</tr>"\

						"<tr>"\
						"<td>"\
						"<table>"\
						"<tr>");
						
						if(strcmp(filter_in,"")!=0)
							fprintf(cgiOut,"<td><input type=checkbox name=distribute_in checked onclick=check_ospf_enable(this)>%s: </td>",search(lcontrol,"distribute_in"));
						else
							fprintf(cgiOut,"<td><input type=checkbox name=distribute_in onclick=check_ospf_enable(this)>%s: </td>",search(lcontrol,"distribute_in"));
						
						fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"acl_index"));
						
						if(strcmp(filter_in,"")!=0)
							fprintf(cgiOut,"<td align=left><input type=text name=acl_index_in size=6 value=%s></td>",filter_in);
						else
							fprintf(cgiOut,"<td align=left><input type=text name=acl_index_in size=6 ></td>");
						fprintf(cgiOut,"</tr>"\
						"<tr>");
						if(strcmp(filter_out,"")!=0)
							fprintf(cgiOut,"<td><input type=checkbox name=distribute_out checked onclick=check_ospf_enable(this)>%s: </td>",search(lcontrol,"distribute_out"));
						else
							fprintf(cgiOut,"<td><input type=checkbox name=distribute_out onclick=check_ospf_enable(this)>%s: </td>",search(lcontrol,"distribute_out"));
						fprintf(cgiOut,"<td align=left>%s: </td>",search(lcontrol,"acl_index"));
						if(strcmp(filter_out,"")!=0)
							fprintf(cgiOut,"<td align=left><input type=text name=acl_index_out value=%s size=6 ></td>",filter_out);
						else
							fprintf(cgiOut,"<td align=left><input type=text name=acl_index_out size=6 ></td>");
						fprintf(cgiOut,"</tr>"\
						"</table>"\
						
						"</td>"\
						"</tr>"\
						"<tr height=35>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-top:12px\">%s</td>",search(lcontrol,"Authentication"));
							fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td>"\
						"<table>");
						if(strcmp(auth_mode,"Simple")==0)
							{
								radiobutton_last = 1;
        						fprintf(cgiOut,"<tr align=left height=35>"\
        						"<td><input type=radio name=radiobutton value=NoAuth onclick=\"changestate(this);\"></td>"\
        						"<td>None</td>"\
        						"</tr>"\
        						"<tr height=35 align=left>");
        						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=text onclick=\"changestate(this);\"  checked></td>");
        						fprintf(cgiOut,"<td>%s: </td>",search(lcontrol,"simple_passwd"));
								if(strcmp(auth_key,"")!=0)
        							fprintf(cgiOut,"<td><input type=text name=simplepwd maxLength=8 value=%s></td>",auth_key);
								else
									fprintf(cgiOut,"<td><input type=text name=simplepwd maxLength=8></td>");
								fprintf(cgiOut,"<td colspan=4><font color=red>(%s%d%s)</font></td>",search(lpublic,"most1"),8,search(lpublic,"most2"));
        						fprintf(cgiOut,"</tr>"\
        						
        						"<tr align=left height=35>");
        						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=MD5 onclick=\"changestate(this);\"></td>");
        						fprintf(cgiOut,"<td>%s: </td>",search(lcontrol,"MD5_passwd"));
        						fprintf(cgiOut,"<td><input type=text name=MD5pwd maxLength=16></td>"\
								"<td><font color=red>(%s%d%s)</font></td>",search(lpublic,"most1"),16,search(lpublic,"most2"));
								fprintf(cgiOut,"<td style=padding-left:5px> Key-ID: </td>"\
								"<td><input type=text size=6 name=MD5key_id maxLength=3></td>"\
								"<td><font color=red>(1--255)</font></td>"\
        						"</tr>");
							}
						else if(strcmp(auth_mode,"MD5")==0)
							{
								radiobutton_last = 2;
								fprintf(cgiOut,"<tr align=left height=35>"\
        						"<td><input type=radio name=radiobutton value=NoAuth onclick=\"changestate(this);\"></td>"\
        						"<td>None</td>"\
        						"</tr>"\
        						"<tr height=35 align=left>");
        						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=text onclick=\"changestate(this);\"  ></td>");
        						fprintf(cgiOut,"<td>%s: </td>",search(lcontrol,"simple_passwd"));
								fprintf(cgiOut,"<td><input type=text name=simplepwd maxLength=8></td>");
								fprintf(cgiOut,"<td colspan=4><font color=red>(%s%d%s)</font></td>",search(lpublic,"most1"),8,search(lpublic,"most2"));
        						fprintf(cgiOut,"</tr>"\
        						
        						"<tr align=left height=35>");
        						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=MD5 onclick=\"changestate(this);\"  checked></td>");
        						fprintf(cgiOut,"<td>%s: </td>",search(lcontrol,"MD5_passwd"));
								if(strcmp(auth_key,"")!=0)
								{
    								fprintf(cgiOut,"<td><input type=text name=MD5pwd maxLength=16 value=%s></td>",auth_key);
									fprintf(cgiOut,"<td><font color=red>(%s%d%s)</font></td>",search(lpublic,"most1"),16,search(lpublic,"most2"));
									fprintf(cgiOut,"<td style=padding-left:5px> Key-ID: </td>"\
									"<td><input type=text size=6 name=MD5key_id maxLength=3 value=%s></td>",MD5key_id);
									fprintf(cgiOut,"<td><font color=red>(1--255)</font></td>");
								}
        						else
    							{
									fprintf(cgiOut,"<td><input type=text name=MD5pwd maxLength=16></td>");
									fprintf(cgiOut,"<td><font color=red>(%s%d%s)</font></td>",search(lpublic,"most1"),16,search(lpublic,"most2"));
									fprintf(cgiOut,"<td style=padding-left:5px> Key-ID: </td>"\
									"<td><input type=text size=6 name=MD5key_id maxLength=3></td>");
									fprintf(cgiOut,"<td><font color=red>(1--255)</font></td>");
    							}
        						fprintf(cgiOut,"</tr>");
							}
						else
							{
								radiobutton_last = 0;
								fprintf(cgiOut,"<tr align=left height=35>"\
        						"<td><input type=radio name=radiobutton value=NoAuth onclick=\"changestate(this);\" checked></td>"\
        						"<td>None</td>"\
        						"</tr>"\
        						"<tr height=35 align=left>");
        						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=text onclick=\"changestate(this);\"  ></td>");
        						fprintf(cgiOut,"<td>%s: </td>",search(lcontrol,"simple_passwd"));
								fprintf(cgiOut,"<td><input type=text name=simplepwd maxLength=8></td>");
								fprintf(cgiOut,"<td colspan=4><font color=red>(%s%d%s)</font></td>",search(lpublic,"most1"),8,search(lpublic,"most2"));
        						fprintf(cgiOut,"</tr>"\
        						
        						"<tr align=left height=35>");
        						fprintf(cgiOut,"<td><input type=radio name=radiobutton value=MD5 onclick=\"changestate(this);\" ></td>");
        						fprintf(cgiOut,"<td>%s: </td>",search(lcontrol,"MD5_passwd"));
								fprintf(cgiOut,"<td><input type=text name=MD5pwd maxLength=16></td>"\
								"<td><font color=red>(%s%d%s)</font></td>",search(lpublic,"most1"),16,search(lpublic,"most2"));
								fprintf(cgiOut,"<td style=padding-left:5px> Key-ID: </td>"\
								"<td><input type=text size=6 name=MD5key_id maxLength=3></td>"\
								"<td><font color=red>(1--255)</font></td>"\
        						"</tr>");
							}
						fprintf(cgiOut,"</table>"\
						"</td>"\
						"</tr>"\
						"<tr>");
						if(cgiFormSubmitClicked("submit_EditIntf") != cgiFormSuccess)
    						 {
    						   fprintf(cgiOut,"<td><input type=hidden name=encry_EditIntf value=%s></td>",encry);
							   fprintf(cgiOut,"<td><input type=hidden name=intfNameLater value=%s></td>",intfName);
							   fprintf(cgiOut,"<td><input type=hidden name=addressLater value=%s></td>",address);
							   fprintf(cgiOut,"<td><input type=hidden name=AreaLater value=%s></td>",Area);	   
    						 }
						else if(cgiFormSubmitClicked("submit_EditIntf") == cgiFormSuccess)
							 {
							   fprintf(cgiOut,"<td><input type=hidden name=encry_EditIntf value=%s></td>",encry_EditIntf);
							   fprintf(cgiOut,"<td><input type=hidden name=intfNameLater value=%s></td>",intfNameLater);
							   fprintf(cgiOut,"<td><input type=hidden name=addressLater value=%s></td>",addressLater);
							   fprintf(cgiOut,"<td><input type=hidden name=AreaLater value=%s></td>",AreaLater);
							 }
						fprintf(cgiOut,"<td><input type=hidden name=md5key_id value=%s></td>",MD5key_id);
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
"</table>");
fprintf(cgiOut,"<input type=hidden name=radiobutton_last value=%d>",radiobutton_last);
fprintf(cgiOut,"</div>"\
"</form>"\
"</body>"\
"</html>");  
free(encry);

release(lpublic);  
release(lcontrol);

for(i=0;i<Info_Num;i++)
{
	free(AreaInfo[i]);
}
if(area_temp[0] == '0' )
	free(area_temp);
free(virtual_link);

free(virtual_prefix_ip1);
free(virtual_prefix_ip2);
free(virtual_prefix_ip3);
free(virtual_prefix_ip4);
free(virtual_prefix_area);

free(Range_prefix_ip1);
free(Range_prefix_ip2);
free(Range_prefix_ip3);
free(Range_prefix_ip4);
free(Range_prefix_mask);

free(turn_back);
free(MD5key_id);
free(ABR_OrNot);
free(filter_in);
free(filter_out);
free(Range_prefix);
free(Range_type);
free(Area_type_param);
free(Range_prefix_ip1);
free(Range_prefix_ip2);
free(Range_prefix_ip3);
free(Range_prefix_ip4);
free(Range_prefix_mask);
free(Area_type);
free(intfNameLater);
free(addressLater);
free(AreaLater);
free(auth_mode);
free(auth_key);
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


#if 0
int executeconfig(char * IntfInfo[],int num,struct list * lcontrol,struct list * lpublic)
{
	char * intfNamesys=(char * )malloc(300);
	memset(intfNamesys,0,300);
	char * intfName_par=(char *)malloc(20);
	memset(intfName_par,0,20);
	char * areaID=(char *)malloc(30);
	memset(areaID,0,30);
	char * OSPF_loop=(char *)malloc(30);
	memset(OSPF_loop,0,30);
	
	char * address_par;
	int flag=0;
	
	cgiFormStringNoNewlines("OSPF_intf",intfName_par,20);
	cgiFormStringNoNewlines("areaID",areaID,30);
	address_par=nameToIP(intfName_par,lpublic);
	fprintf(stderr,"address_par=%s",address_par);

	fprintf(stderr,"flag=%d",flag);
	if(flag!=1)
		{
        	strcat(intfNamesys,"ospf_network.sh");
        	strcat(intfNamesys," ");
        	strcat(intfNamesys,"on");
        	strcat(intfNamesys," ");
        	strcat(intfNamesys,address_par);
        	strcat(intfNamesys," ");
        	strcat(intfNamesys,areaID);
        	int status = system(intfNamesys);
        	int ret = WEXITSTATUS(status);
            					 
            	if(-3==ret)
            		ShowAlert(search(lpublic,"bash_fail"));
////////////////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("OSPF_loop",OSPF_loop,30);
				
		
		ShowAlert(search(lcontrol,"ospf_add_suc"));
		}
	else ShowAlert(search(lcontrol,"ospf_intf_exist"));


	free(OSPF_loop);
	free(areaID);
	free(intfName_par);
}
#endif

int ReadConfig_ospf(char * ospfInfo[],int * infoNum,struct list * lpublic)
{
	int i;
	char * command=(char *)malloc(250);
	memset(command,0,250);
	strcat(command,"show_run_conf.sh | awk 'BEGIN{FS=\"\\n\";RS=\"!\"}/router ospf/{print}'| awk '{OFS=\"-\";ORS=\"-\\n\"}{$1=$1;print}'  >/var/run/apache2/OSPF_temp_Info.txt");
	int status = system(command);
	int ret = WEXITSTATUS(status);

	if(0==ret)
		{}
	else ShowAlert(search(lpublic,"bash_fail"));

	FILE *fd;
	char  temp[60];
	memset(temp,0,60);
	if((fd=fopen("/var/run/apache2/OSPF_temp_Info.txt","r"))==NULL)
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

int Area_ReadConfig(char * areaID,char * area_info[],int * infoNum,struct list * lpublic)
{
	int i;
	char * command=(char *)malloc(200);
	memset(command,0,200);
	sprintf(command,"ospf_show_ip_ospf.sh | sed 's/:/*/g' | sed 's/ /#/g' | awk 'BEGIN{FS=\"\\n\";RS=\"\\n\\n\";}/###Area#ID\\*#%s/{print}' >/var/run/apache2/OSPF_AREA.txt",areaID);
	//fprintf(stderr,"command=%s",command);
	int status = system(command);
	int ret = WEXITSTATUS(status);
	if(0==ret)
		{}
	else ShowAlert("occur an error!");

	FILE *fd;
	char  temp[256];
	memset(temp,0,256);
	if((fd=fopen("/var/run/apache2/OSPF_AREA.txt","r"))==NULL)
		{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i=0;
	while((fgets(temp,256,fd)) != NULL)
		{
			strcat(area_info[i],temp);
			i++;
			memset(temp,0,256);
		}
	fclose(fd);
	*infoNum=i;
	
	free(command);
	return 1;
}
int OSPF_ReadConfig_For_ABR(char * ospf_info[],int * infoNum,struct list * lpublic)
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




int executeconfig(char * intfName_par,char * address_par,char * area_par,char * auth_mode_param,struct list * lcontrol,struct list * lpublic)
{
	char * intfNamesys=(char * )malloc(300);
	memset(intfNamesys,0,300);

	
	char * if_netType=(char *)malloc(30);
	memset(if_netType,0,30);
	
	char * hello_text=(char *)malloc(10);
	memset(hello_text,0,10);
	char * dead_text=(char *)malloc(10);
	memset(dead_text,0,10);
	char * transmit=(char *)malloc(10);
	memset(transmit,0,10);
	char * retransmit=(char *)malloc(10);
	memset(retransmit,0,10);
	char * Priority=(char *)malloc(10);
	memset(Priority,0,10);
	char * Cost=(char *)malloc(10);
	memset(Cost,0,10);
	char * area_type=(char *)malloc(10);
	memset(area_type,0,10);
	char * nssa_param=(char *)malloc(30);
	memset(nssa_param,0,30);
	char * advertise_param=(char *)malloc(30);
	memset(advertise_param,0,30);
	char * acl_index_out=(char *)malloc(10);
	memset(acl_index_out,0,10);
	char * acl_index_in=(char *)malloc(10);
	memset(acl_index_in,0,10);
	

	char  radiobutton[10]={0};

	char * simplepwd=(char *)malloc(20);
	memset(simplepwd,0,20);
	char * MD5key_id=(char *)malloc(20);
	memset(MD5key_id,0,20);
	char *endptr = NULL;  
	int md5key = 0;
	


	char dip1[4],dip2[4],dip3[4],dip4[4];
	char * dstip=(char *)malloc(20);
	memset(dstip,0,20);
	char * dmasklength=(char *)malloc(10);
	memset(dmasklength,0,10);
	//////////////////////////////////////////////////

	char * Adjacent_neighbor_count=(char *)malloc(10);
	memset(Adjacent_neighbor_count,0,10);

	
	//fprintf(stderr,"flag=%d",flag);
	////////////////////////////这里开始作用脚本///////////////////////////////////////

	int status=0,ret=0;
	
			if(strcmp(area_par,"")!=0)
				{
        	    	strcat(intfNamesys,"ospf_network.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,"on");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,address_par);
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,area_par);
					strcat(intfNamesys," ");
        	    	strcat(intfNamesys,">/dev/null");
        	    	status = system(intfNamesys);
        	    	ret = WEXITSTATUS(status);
        	        					 
        	        	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
				}
			else
				{
					ShowAlert(search(lcontrol,"area_not_null"));
					return -1;
				}
      /////////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("if_netType",if_netType,30);
			//fprintf(stderr,"if_netType=%s",if_netType);
			strcat(intfNamesys,"ospf_if_network.sh");
	    	strcat(intfNamesys," ");
	    	strcat(intfNamesys,intfName_par);
	    	strcat(intfNamesys," ");
			strcat(intfNamesys,"on");
			strcat(intfNamesys," ");
			strcat(intfNamesys,if_netType);
			strcat(intfNamesys," ");
			strcat(intfNamesys,">/dev/null");
			//fprintf(stderr,"intfNamesys=%s",intfNamesys);
			status = system(intfNamesys);
        	ret = WEXITSTATUS(status);
            					 
            	if(-3==ret)
	        	{
	        		ShowAlert(search(lpublic,"ospf_config_fail"));
					return -1;
	        	}
		///////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("hello_text",hello_text,10);
			
			strcat(intfNamesys,"ospf_if_hellointervla.sh");
	    	strcat(intfNamesys," ");
	    	strcat(intfNamesys,intfName_par);
	    	strcat(intfNamesys," ");
			strcat(intfNamesys,"on");
			strcat(intfNamesys," ");
			strcat(intfNamesys,hello_text);
			strcat(intfNamesys," ");
			strcat(intfNamesys,"non");
			strcat(intfNamesys," ");
			strcat(intfNamesys,">/dev/null");
			status = system(intfNamesys);
        	ret = WEXITSTATUS(status);
            					 
            	if(-3==ret)
	        	{
	        		ShowAlert(search(lpublic,"ospf_config_fail"));
					return -1;
	        	}
		
		//////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("dead_text",dead_text,10);
			//if(strcmp(dead_text,"40")!=0)
				{
        			strcat(intfNamesys,"ospf_if_deadinterval.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,intfName_par);
        	    	strcat(intfNamesys," ");
        			strcat(intfNamesys,"on");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,dead_text);
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,"non");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,">/dev/null");
        			status = system(intfNamesys);
                	ret = WEXITSTATUS(status);
                    					 
                	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
				}
		///////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("LSA_transmit",transmit,10);
			//if(strcmp(transmit,"1")!=0)
				{
        			strcat(intfNamesys,"ospf_if_tran.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,intfName_par);
        	    	strcat(intfNamesys," ");
        			strcat(intfNamesys,"on");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,transmit);
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,"non");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,">/dev/null");
        			status = system(intfNamesys);
                	ret = WEXITSTATUS(status);
                    					 
                	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
				}
		//////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("LSA_retransmit",retransmit,10);
			//if(strcmp(retransmit,"5")!=0)
				{
        			strcat(intfNamesys,"ospf_if_tran_inter.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,intfName_par);
        	    	strcat(intfNamesys," ");
        			strcat(intfNamesys,"on");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,retransmit);
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,"non");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,">/dev/null");
        			status = system(intfNamesys);
                	ret = WEXITSTATUS(status);
                    					 
                	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
				}

		/////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
         	int result=0;
         	char **responses;
         	result = cgiFormStringMultiple("mtu_ignore", &responses);
         	if(responses[0])
         	{
         		strcat(intfNamesys,"ospf_if_mtu.sh");
				strcat(intfNamesys," ");
        	    strcat(intfNamesys,intfName_par);
         		strcat(intfNamesys," ");
         		strcat(intfNamesys,"on");
         		strcat(intfNamesys," ");
         		strcat(intfNamesys,"non");
				strcat(intfNamesys," ");
             	strcat(intfNamesys,">/dev/null");
         		system(intfNamesys);
				status = system(intfNamesys);
            	ret = WEXITSTATUS(status);
                					 
            	if(-3==ret)
	        	{
	        		ShowAlert(search(lpublic,"ospf_config_fail"));
					return -1;
	        	}
         	}
			else
			{
				strcat(intfNamesys,"ospf_if_mtu.sh");
				strcat(intfNamesys," ");
        	    strcat(intfNamesys,intfName_par);
         		strcat(intfNamesys," ");
         		strcat(intfNamesys,"off");
         		strcat(intfNamesys," ");
         		strcat(intfNamesys,"non");
				strcat(intfNamesys," ");
             	strcat(intfNamesys,">/dev/null");
         		system(intfNamesys);
				status = system(intfNamesys);
            	ret = WEXITSTATUS(status);
                					 
            	if(-3==ret)
	        	{
	        		ShowAlert(search(lpublic,"ospf_config_fail"));
					return -1;
	        	}
			}
		////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("Priority",Priority,10);
			//if(strcmp(Priority,"1")!=0)
				{
        			strcat(intfNamesys,"ospf_if_pri.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,intfName_par);
        	    	strcat(intfNamesys," ");
        			strcat(intfNamesys,"on");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,Priority);
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,"non");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,">/dev/null");
        			status = system(intfNamesys);
                	ret = WEXITSTATUS(status);
                    					 
                	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
				}
		//////////////////////////////////////////////////////////////////////
			memset(intfNamesys,0,300);
			cgiFormStringNoNewlines("Cost",Cost,10);
			if(strcmp(Cost,"")!=0)
				{
        			strcat(intfNamesys,"ospf_if_cost.sh");
        	    	strcat(intfNamesys," ");
        	    	strcat(intfNamesys,intfName_par);
        	    	strcat(intfNamesys," ");
        			strcat(intfNamesys,"on");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,Cost);
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,"non");
        			strcat(intfNamesys," ");
        			strcat(intfNamesys,">/dev/null");
        			status = system(intfNamesys);
                	ret = WEXITSTATUS(status);
                    					 
                	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
				}
				
		///////////////////////////下面是区域//////////////////////////////////////////
			memset(intfNamesys,0,300);
         	result=0;
         	char **responses2;
			char **responses1;
         	result = cgiFormStringMultiple("IS_ABR", &responses2);
			cgiFormStringNoNewlines("area_type",area_type,10);
			cgiFormStringNoNewlines("nssa_param",nssa_param,30);
			
			char **responses3;
         	result = cgiFormStringMultiple("no_summary", &responses3);
			result=0;
			result = cgiFormStringMultiple("area_type_check", &responses1);
         	//if(responses2[0])
         	//fprintf(stderr,"area_type=%s-nssa_param=%s",area_type,nssa_param);
         	if(responses1[0])
         		{
            		if(strcmp(area_type,"nssa")==0)
            			{
            				strcat(intfNamesys,"ospf_nssa.sh");
                	    	strcat(intfNamesys," ");
                	    	strcat(intfNamesys,area_par);
                	    	strcat(intfNamesys," ");
                			strcat(intfNamesys,"on");
       					if(responses2[0] && strcmp(nssa_param,"")!=0)
       						{
       		         			strcat(intfNamesys," ");
       		         			strcat(intfNamesys,nssa_param);
       						}
       					if(responses2[0] && responses3[0])
       						{
       		         			strcat(intfNamesys," ");
       		         			strcat(intfNamesys,"no-summary");
       						}
                			strcat(intfNamesys," ");
                			strcat(intfNamesys,">/dev/null");
                			status = system(intfNamesys);
                        	ret = WEXITSTATUS(status);
                            					 
                        	if(-3==ret)
	        	        	{
	        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
	        	        	}

							
                 		}
       				else
         				{
         					strcat(intfNamesys,"ospf_stub.sh");
                  	    	strcat(intfNamesys," ");
                  	    	strcat(intfNamesys,area_par);
                  	    	strcat(intfNamesys," ");
                  			strcat(intfNamesys,"on");
         					if(responses2[0] && responses3[0])
         						{
         		         			strcat(intfNamesys," ");
         		         			strcat(intfNamesys,"no-summary");
         						}
                  			strcat(intfNamesys," ");
                  			strcat(intfNamesys,">/dev/null");
                  			status = system(intfNamesys);
                          	ret = WEXITSTATUS(status);
                              					 
                          	if(-3==ret)
	        	        	{
	        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
	        	        	}
         				}
         		}
			else
				{
					if(strcmp(area_type,"nssa")==0)
						{
							strcat(intfNamesys,"ospf_nssa.sh");
                	    	strcat(intfNamesys," ");
                	    	strcat(intfNamesys,area_par);
                	    	strcat(intfNamesys," ");
                			strcat(intfNamesys,"off");
							strcat(intfNamesys," ");
                  			strcat(intfNamesys,">/dev/null");
							status = system(intfNamesys);
                          	ret = WEXITSTATUS(status);
                              					 
                          	if(-1==ret)
                          	{
                          		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
                          	}
						}
					else
						{
							strcat(intfNamesys,"ospf_stub.sh");
                	    	strcat(intfNamesys," ");
                	    	strcat(intfNamesys,area_par);
                	    	strcat(intfNamesys," ");
                			strcat(intfNamesys,"off");
							strcat(intfNamesys," ");
                			strcat(intfNamesys,"non");
							strcat(intfNamesys," ");
                  			strcat(intfNamesys,">/dev/null");
							status = system(intfNamesys);
                          	ret = WEXITSTATUS(status);
                              					 
                          	if(-3==ret)
	        	        	{
	        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
	        	        	}
						}
				}
		////////////////////////////////聚合//////////////////////////////////////////
				memset(intfNamesys,0,300);
                char **responses4;
         		result = cgiFormStringMultiple("route_range", &responses4);
     			memset(dip1,0,4);
             	cgiFormStringNoNewlines("src_ip1",dip1,4);	 
             	strcat(dstip,dip1);
             	strcat(dstip,".");
             	memset(dip2,0,4);
             	cgiFormStringNoNewlines("src_ip2",dip2,4); 
             	strcat(dstip,dip2);  
             	strcat(dstip,".");
             	memset(dip3,0,4);
             	cgiFormStringNoNewlines("src_ip3",dip3,4); 
             	strcat(dstip,dip3);  
             	strcat(dstip,".");
             	memset(dip4,0,4);
             	cgiFormStringNoNewlines("src_ip4",dip4,4);
             	strcat(dstip,dip4);

				cgiFormStringNoNewlines("masklen",dmasklength,10);
				
             	sprintf(dstip,"%s/%s",dstip,dmasklength);

				cgiFormStringNoNewlines("advertise_param",advertise_param,30);
				//fprintf(stderr,"advertise_param=%s-dmasklength=%s",advertise_param,dmasklength);
				if(responses4[0])
					{
						strcat(intfNamesys,"ospf_range.sh");
	         	    	strcat(intfNamesys," ");
	         	    	strcat(intfNamesys,area_par);
	         	    	strcat(intfNamesys," ");
	         			strcat(intfNamesys,"on");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,dstip);
						if(strcmp(advertise_param,"")!=0)
							{
								strcat(intfNamesys," ");
	         					strcat(intfNamesys,advertise_param);
							}
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,">/dev/null");
						status = system(intfNamesys);
                     	ret = WEXITSTATUS(status);
                         					 
                     	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
					}
				else
					{
						strcat(intfNamesys,"ospf_range.sh");
	         	    	strcat(intfNamesys," ");
	         	    	strcat(intfNamesys,area_par);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"off");
	         	    	strcat(intfNamesys," ");
	         			strcat(intfNamesys,dstip);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,">/dev/null");
						status = system(intfNamesys);
                    	ret = WEXITSTATUS(status);
                        					 
                    	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
					}
				
		///////////////////////////虚连接//////////////////////////////////////
				memset(intfNamesys,0,300);
				memset(dmasklength,0,10);
				memset(dstip,0,20);
                char **responses5;
				result = cgiFormStringMultiple("virtual_link", &responses5);
     			memset(dip1,0,4);
             	cgiFormStringNoNewlines("vlink_ip1",dip1,4);	 
             	strcat(dstip,dip1);
             	strcat(dstip,".");
             	memset(dip2,0,4);
             	cgiFormStringNoNewlines("vlink_ip2",dip2,4); 
             	strcat(dstip,dip2);  
             	strcat(dstip,".");
             	memset(dip3,0,4);
             	cgiFormStringNoNewlines("vlink_ip3",dip3,4); 
             	strcat(dstip,dip3);  
             	strcat(dstip,".");
             	memset(dip4,0,4);
             	cgiFormStringNoNewlines("vlink_ip4",dip4,4);
             	strcat(dstip,dip4);
             	//sprintf(dstip,"%s/%s",dstip,dmasklength);
				//fprintf(stderr,"##dstip=%s",dstip);
				if(responses5[0])
					{

						strcat(intfNamesys,"ospf_virtual-link.sh");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"on");
	         	    	strcat(intfNamesys," ");
	         	    	strcat(intfNamesys,area_par);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,dstip);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,">/dev/null");
						status = system(intfNamesys);
                     	ret = WEXITSTATUS(status);
                         					 
                     	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
					}
				else
					{
						strcat(intfNamesys,"ospf_virtual-link.sh");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"off");
	         	    	strcat(intfNamesys," ");
	         	    	strcat(intfNamesys,area_par);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,dstip);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,">/dev/null");
						status = system(intfNamesys);
                     	ret = WEXITSTATUS(status);
                         					 
                     	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
					}
		///////////////////////////////////////过滤///////////////////////////////
				memset(intfNamesys,0,300);
				char **responses6;
				result = cgiFormStringMultiple("distribute_in", &responses6);
				char **responses7;
				result = cgiFormStringMultiple("distribute_out", &responses7);
				cgiFormStringNoNewlines("acl_index_in",acl_index_in,10);
				cgiFormStringNoNewlines("acl_index_out",acl_index_out,10);
				//fprintf(stderr,"acl_index_in=%s-acl_index_out=%s",acl_index_in,acl_index_out);
				if(responses6[0])
					{
						strcat(intfNamesys,"ospf_filter_list.sh");
	         	    	strcat(intfNamesys," ");
	         	    	strcat(intfNamesys,area_par);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"on");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,acl_index_in);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"in");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,">/dev/null");
						status = system(intfNamesys);
                     	ret = WEXITSTATUS(status);
                         					 
                     	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
					}
				else
					{
						strcat(intfNamesys,"ospf_filter_list.sh");
	         	    	strcat(intfNamesys," ");
	         	    	strcat(intfNamesys,area_par);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"off");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,acl_index_in);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"in");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,">/dev/null");
						status = system(intfNamesys);
                     	ret = WEXITSTATUS(status);
                         					 
                     	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
					}
				memset(intfNamesys,0,300);
				if(responses7[0])
					{
						strcat(intfNamesys,"ospf_filter_list.sh");
	         	    	strcat(intfNamesys," ");
	         	    	strcat(intfNamesys,area_par);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"on");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,acl_index_out);
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,"out");
						strcat(intfNamesys," ");
	         			strcat(intfNamesys,">/dev/null");
						status = system(intfNamesys);
                     	ret = WEXITSTATUS(status);
                         					 
                     	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
					}
				else
					{
						strcat(intfNamesys,"ospf_filter_list.sh");
             	    	strcat(intfNamesys," ");
             	    	strcat(intfNamesys,area_par);
    					strcat(intfNamesys," ");
             			strcat(intfNamesys,"off");
    					strcat(intfNamesys," ");
             			strcat(intfNamesys,acl_index_out);
    					strcat(intfNamesys," ");
             			strcat(intfNamesys,"out");
    					strcat(intfNamesys," ");
             			strcat(intfNamesys,">/dev/null");
    					status = system(intfNamesys);
                     	ret = WEXITSTATUS(status);
                         					 
                     	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
					}
		///////////////////////////////////认证///////////////////////////////////////////

				char str_radiobutton_last[10]={0};
				memset(intfNamesys,0,300);
				cgiFormString("radiobutton", radiobutton, 10);
				cgiFormString("radiobutton_last", str_radiobutton_last, 10);
				radiobutton_last = atoi(str_radiobutton_last);
				cgiFormString("md5key_id", MD5key_id, 20);
				//fprintf(stderr,"radiobutton=%s--radiobutton_last=%d---auth_mode_param=%s\n",radiobutton, radiobutton_last, auth_mode_param);
				char md5_tmp[20]={0};
				
             	if(strcmp(radiobutton,"NoAuth")==0)
             	{
             		
					//fprintf(stderr,"MD5key_id=%s", MD5key_id);
					if (radiobutton_last == 1)
					{
	             		memset(intfNamesys,0,300);
	                  	sprintf(intfNamesys,"ospf_if_authentication_key.sh %s OFF 123 NON >/dev/null ",intfName_par);
						//fprintf(stderr, "22222222ospf_if_authentication_key intfNamesys=%s----\n", intfNamesys);
	                  	status = system(intfNamesys);
	                 	ret = WEXITSTATUS(status);
	                     					 
	                 	if(-3==ret)
	        	        	{
	        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
	        	        	}
					}
					else if (radiobutton_last == 2)
					{
	             		memset(intfNamesys,0,300);
	                  	sprintf(intfNamesys,"ospf_if_md5_key.sh %s off %s non non >/dev/null",intfName_par,MD5key_id);
						//fprintf(stderr, "1111ospf_if_md5_key intfNamesys=%s----\n", intfNamesys);
	                  	status = system(intfNamesys);
	                 	ret = WEXITSTATUS(status);
	                     					 
	                 	if(-3==ret)
	        	        	{
	        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
	        	        	}
					}
					memset(intfNamesys,0,300);
                          	sprintf(intfNamesys,"ospf_if_authentication.sh %s off 123 non >/dev/null",intfName_par);
                          	status = system(intfNamesys);
                         	ret = WEXITSTATUS(status);
                             					 
                         	if(-3==ret)
	        	        	{
	        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
	        	        	}

             	}
             	else if(strcmp(radiobutton,"text")==0)
             	{
             		cgiFormString("simplepwd", simplepwd, 20);
					if(strcmp(simplepwd,"")==0)
					{
						ShowAlert(search(lpublic,"pass_not_null"));
						return -1;
					}
					
					if(strlen(simplepwd)>8)
					{
						ShowAlert(search(lpublic,"pass_too_long"));
						return -1;
					}
					
             		if (radiobutton_last == 2)
					{
	             		memset(intfNamesys,0,300);
	                  	sprintf(intfNamesys,"ospf_if_md5_key.sh %s off %s non non >/dev/null",intfName_par,MD5key_id);
						//fprintf(stderr, "1111ospf_if_md5_key intfNamesys=%s----\n", intfNamesys);
	                  	status = system(intfNamesys);
	                 	ret = WEXITSTATUS(status);
	                     					 
	                 	if(-3==ret)
	        	        	{
	        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
	        	        	}
					}
             		
					memset(intfNamesys,0,300);
             		strcat(intfNamesys,"ospf_if_authentication_key.sh");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,intfName_par);
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"ON");
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,simplepwd);
					strcat(intfNamesys," ");
             		strcat(intfNamesys,"NON");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,">/dev/null");
					status = system(intfNamesys);
                 	ret = WEXITSTATUS(status);
                    			 
                 	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}

					memset(intfNamesys,0,300);
					strcat(intfNamesys,"ospf_if_authentication.sh");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,intfName_par);
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"on");
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"text");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,"text_param4");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,">/dev/null");
             		status = system(intfNamesys);
                 	ret = WEXITSTATUS(status);
                     		 
                 	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
             

             	}
             	else if(strcmp(radiobutton,"MD5")==0)
             	{
             		cgiFormString("MD5pwd", simplepwd, 20);
					cgiFormString("MD5key_id", MD5key_id, 20);
					if(strcmp(simplepwd,"")==0)
					{
						ShowAlert(search(lpublic,"pass_not_null"));
						return -1;
					}
					
					if(strlen(simplepwd)>16)
					{
						ShowAlert(search(lpublic,"pass_too_long"));
						return -1;
					}
					
					if(strcmp(MD5key_id,"")==0)
					{
						ShowAlert(search(lpublic,"key_id_not_null"));
						return -1;
					}
					md5key=strtoul(MD5key_id,&endptr,10);
					if((md5key<1)||(md5key>255))
					{
						ShowAlert(search(lpublic,"key_id_illegal"));
						return -1;
					}	
					
             		if (radiobutton_last == 1)
					{
	             		memset(intfNamesys,0,300);
	                  	sprintf(intfNamesys,"ospf_if_authentication_key.sh %s OFF 123 NON >/dev/null",intfName_par);
						//fprintf(stderr, "22222222ospf_if_authentication_key intfNamesys=%s----\n", intfNamesys);
	                  	status = system(intfNamesys);
	                 	ret = WEXITSTATUS(status);
	                     					 
	                 	if(-3==ret)
        	        	{
        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
							return -1;
        	        	}
					}
					
					memset(intfNamesys,0,300);
             		strcat(intfNamesys,"ospf_if_md5_key.sh");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,intfName_par);
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"on");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,MD5key_id);
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,simplepwd);
					strcat(intfNamesys," ");
             		strcat(intfNamesys,"non");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,">/dev/null");
					//fprintf(stderr,"MD5 intfNamesys=%s\n",intfNamesys);
             		status = system(intfNamesys);
                 	ret = WEXITSTATUS(status);
                     					 
                 	if(-3==ret)
    	        	{
    	        		ShowAlert(search(lpublic,"ospf_config_fail"));
						return -1;
    	        	}
             		
             		memset(intfNamesys,0,300);
             		strcat(intfNamesys,"ospf_if_authentication.sh");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,intfName_par);
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"on");
             		strcat(intfNamesys," ");
             		strcat(intfNamesys,"message-digest");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,"non");
					strcat(intfNamesys," ");
             		strcat(intfNamesys,">/dev/null");
             		status = system(intfNamesys);
                 	ret = WEXITSTATUS(status);
                     					 
                 	if(-3==ret)
		        	{
		        		ShowAlert(search(lpublic,"ospf_config_fail"));
						return -1;
		        	}
	
             	}
				else
				{
					//fprintf(stderr, "ospf_if auth_mode_param=%s----\n", auth_mode_param);
						if(strcmp(auth_mode_param,"Simple")==0)
                  		{
                          	memset(intfNamesys,0,300);
                          	sprintf(intfNamesys,"ospf_if_authentication_key.sh %s OFF 123 NON >/dev/null",intfName_par);
                          	status = system(intfNamesys);
                         	ret = WEXITSTATUS(status);
                             					 
                         	if(-3==ret)
	        	        	{
	        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
	        	        	}
                          	
                          	memset(intfNamesys,0,300);
                          	sprintf(intfNamesys,"ospf_if_authentication.sh %s off message-digest non >/dev/null",intfName_par);
                          	status = system(intfNamesys);
                         	ret = WEXITSTATUS(status);
                             					 
                         	if(-3==ret)
	        	        	{
	        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
	        	        	}
                  		}
                  	else if(strcmp(auth_mode_param,"MD5")==0)
                  		{
                  			cgiFormString("MD5key_id", MD5key_id, 20);
                  			memset(intfNamesys,0,300);
                          	sprintf(intfNamesys,"ospf_if_md5_key.sh %s off %s non non >/dev/null",intfName_par,MD5key_id);
							//fprintf(stderr, "ospf_if_md5_key intfNamesys=%s----\n", intfNamesys);
                          	status = system(intfNamesys);
                        	ret = WEXITSTATUS(status);
                            					 
                        	if(-3==ret)
	        	        	{
	        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
	        	        	}
                          	
                          	memset(intfNamesys,0,300);
                          	sprintf(intfNamesys,"ospf_if_authentication.sh %s off 123 non >/dev/null",intfName_par);
							//fprintf(stderr, "ospf_if_authentication intfNamesys=%s----\n", intfNamesys);
                          	status = system(intfNamesys);
                         	ret = WEXITSTATUS(status);
                             					 
                         	if(-3==ret)
	        	        	{
	        	        		ShowAlert(search(lpublic,"ospf_config_fail"));
								return -1;
	        	        	}
						}
				}

	
	
	////////////全部脚本正常执行则显示成功//////////////////////////
	ShowAlert(search(lcontrol,"ospf_config_suc"));

	free(intfNamesys);
	free(MD5key_id);
	free(simplepwd);
	free(transmit);
	free(retransmit);
	free(acl_index_in);
	free(acl_index_out);
	free(advertise_param);
	free(dmasklength);
	free(dstip);
	free(nssa_param);
	free(area_type);
	free(Priority);
	free(Cost);
	free(dead_text);
	free(hello_text);
	free(Adjacent_neighbor_count);
	free(if_netType);
	return 0;	
}

char * turn_area_value(char * src)
{
	if(NULL == src)
		return NULL;

	if ( NULL != strchr(src,'.'))
		return src;
	else
		{
			char * temp = (char *)malloc(30);
			memset(temp, 0, 30);
			sprintf(temp, "0.0.0.%s", src);
			return temp;
		}
}

char * trim_area_address(char * src)
{
	unsigned int temp = 0;
	while((*(src+temp+1)) != '\0')
	{
		if(isspace(*(src+temp)) && (*(src+temp)) != '\0')
			temp++;
		if( ((*(src+temp)) == '.' || (*(src+temp)) == '0') && (*(src+temp)) != '\0' && (*(src+temp+1)) != '\0' )
			temp++;
	}
	return (char *)(src+temp);
}


