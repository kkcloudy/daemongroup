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
* wp_all_interface.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for port sub all interface deal
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_err.h"
#include "ws_init_dbus.h"
#include <sys/wait.h>

#include "ws_dcli_interface.h"


#define MAX_INTF_NUM 4095
#define INTF_NAME_LENTH  20
#define MAX_NETWORK_ONE_INTF  8


int ShowInterfacePage();
int nameToIP(char * intfname,char * ipnetwork[],int * net_num,struct list * lpublic);
int  interfaceInfo(char * intfname[],int * infoNum,struct list * lpublic);
int delete_interface(char * intf_name,char * net,struct list * lcon);


int cgiMain()
{
 	ShowInterfacePage();
 	return 0;
}

int ShowInterfacePage()
{ 
  struct list *lpublic;	/*解析public.txt文件的链表头*/
  struct list *lcontrol;	  /*解析system.txt文件的链表头*/	
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcontrol=get_chain_head("../htdocs/text/control.txt"); 
  
  char *encry=(char *)malloc(BUF_LEN);				
  char *str=NULL;
  int i,retu;
  int net_num,intf_num;
  char * select_opt = (char *)malloc(INTF_NAME_LENTH);
  memset(select_opt, 0, INTF_NAME_LENTH);
  char * delete_rule = (char *)malloc(10);
  memset(delete_rule, 0, 10);
  char * name_for_delete = (char *)malloc(INTF_NAME_LENTH);
  memset(name_for_delete, 0, INTF_NAME_LENTH);
  char * address_for_delete = (char *)malloc(INTF_NAME_LENTH);
  memset(address_for_delete, 0, INTF_NAME_LENTH);
  char * CheckUsr = (char *)malloc(20);
  memset(CheckUsr, 0, 20);

	struct intf_info
	{
		char intf_name[INTF_NAME_LENTH];
		char * network[MAX_NETWORK_ONE_INTF];
		struct intf_info * next;
	};
  
  char * intfName[MAX_INTF_NUM];
  char menu[21]="menulist";
  char* i_char=(char *)malloc(10);
  memset( i_char, 0 ,10);
	  
  for( i=0;i<MAX_INTF_NUM;i++ )
  {
  	 intfName[i] = (char *)malloc(INTF_NAME_LENTH);
	 memset( intfName[i], 0, INTF_NAME_LENTH );
  }
  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); 	       /*用户非法*/
      return 0;
	}
  }
  else
  {
  	cgiFormStringNoNewlines("encry_addusr",encry,BUF_LEN);
  }
  
	cgiFormStringNoNewlines("interface_all",select_opt,INTF_NAME_LENTH);
	cgiFormStringNoNewlines("DELETE_RULE",delete_rule,INTF_NAME_LENTH);
	cgiFormStringNoNewlines("CheckUsr", CheckUsr, 10);

	if(strcmp(CheckUsr,"")!=0)
		retu=atoi(CheckUsr);

	//////added 2009-3-11///////////////////
	if (!strcmp(select_opt, ""))
	{
		cgiFormStringNoNewlines("SELECT_OPT",select_opt,INTF_NAME_LENTH);
	}
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"prt_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	"<style type=text/css>"\
	"#div1{ width:42px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	"#div2{ width:40px; height:15px; padding-left:5px; padding-top:3px}"\
	"#link{ text-decoration:none; font-size: 12px}"\
	"</style>"\
	"</head>"\
	"<script src=/ip.js>"\
	"</script>"\
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
	if( strcmp(delete_rule , "delete") == 0 )  
	{
		cgiFormStringNoNewlines("INTNAME",name_for_delete,INTF_NAME_LENTH);
		cgiFormStringNoNewlines("NETWORK",address_for_delete,30);
		delete_interface(name_for_delete,address_for_delete,lcontrol);
	}
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )
		retu=checkuser_group(str);

	if(cgiFormSubmitClicked("submit_interface_all") == cgiFormSuccess)
	{
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_prtsur.cgi?UN=%s';\n", encry);
		fprintf( cgiOut, "</script>\n" );
	}
  fprintf(cgiOut,"<form name=form_interface method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
"<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcontrol,"prt_manage"));
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<input type=hidden name=UN value=%s>",encry);  
	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>"\
	"<td width=62 align=center><input id=but type=submit name=submit_interface_all style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			
	fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
					  "<td align=left id=tdleft><a href=wp_prtsur.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"prt_sur"));						
				fprintf(cgiOut,"</tr>"\
			  	"<tr height=25>\n"\
				  "<td align=left id=tdleft><a href=wp_prtarp.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>ARP<font><font id=%s> %s<font></a></td></tr>",encry,search(lpublic,"menu_san"),search(lpublic,"survey")); 					  
				if(retu==0)  /*管理员*/
				{
				  	fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_static_arp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",encry,search(lpublic,"menu_san"),search(lcontrol,"prt_static_arp"));					   
				  	fprintf(cgiOut,"\n");
					fprintf(cgiOut,"<tr height=25>\n"\
					    "<td align=left id=tdleft><a href=wp_prtcfg.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",encry,search(lpublic,"menu_san"),search(lcontrol,"prt_cfg"));

				  	fprintf(cgiOut,"<tr height=25>\n"\
				      "<td align=left id=tdleft><a href=wp_prtfuncfg.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"func_cfg"));                       
				  	fprintf(cgiOut,"</tr>");                    
				}
			  	fprintf(cgiOut,"<tr height=25>\n"\
				    "<td align=left id=tdleft><a href=wp_subintf.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",encry,search(lpublic,"menu_san"),search(lcontrol,"title_subintf"));
				if(retu==0)  /*管理员*/
				{
					fprintf(cgiOut,"<tr height=25>"\
							  "<td align=left id=tdleft><a href=wp_interface_bindip.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td></tr>",encry,search(lpublic,"menu_san"),search(lpublic,"config_interface"));
				}
				///added by tangsiqi
				fprintf(cgiOut,"<tr height=26>"\
						  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san></font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"interface"));   /*突出显示*/
				fprintf(cgiOut,"</tr>");
				for(i=0;i<2;i++)
				{
				  fprintf(cgiOut,"<tr height=25>"\
					"<td id=tdleft>&nbsp;</td>"\
				  "</tr>");
				}
				char * net_work[8];
				for( i = 0; i<8 ;i++ )
				{
					net_work[i] = (char *)malloc(30);
					memset( net_work[i] ,0, 30 );
				}
	
				interfaceInfo( intfName, &intf_num, lpublic);
				
			  fprintf(cgiOut,"</table>"\
				"</td>"\
				"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
                "<table border=0 width=350 cellspacing=0 cellpadding=0>");
			  fprintf(cgiOut,"<tr align=left><td>");
			  fprintf(cgiOut, "<select name=\"interface_all\" onchange=\"javascript:this.form.submit();\" style='height:auto'>");
				for(i=0;i<intf_num;i++)
				{
			 	     if(0 == strncmp(intfName[i],"ve",2))
				 	 {
				 	 	char *temp_ve = NULL;
						char temp_ve1[INTF_NAME_LENTH];

				 	 	temp_ve = strchr(intfName[i],'@');
						if(temp_ve)
						{
							memset(temp_ve1,0,INTF_NAME_LENTH);
							strncpy(temp_ve1,intfName[i],temp_ve-intfName[i]);
						
							memset(intfName[i],0,INTF_NAME_LENTH);
							strcpy(intfName[i],temp_ve1);
						}
				 	 }
				 
					 if( strcmp( intfName[i],select_opt ) == 0)
						fprintf(cgiOut,"<option value=%s selected=selected>%s",intfName[i],intfName[i]);
					 else				
						fprintf(cgiOut,"<option value=%s>%s",intfName[i],intfName[i]);
				}
			  fprintf(cgiOut,"</select></td></tr>");

			  if( strcmp(select_opt, "") != 0 )
			 	 nameToIP( select_opt , net_work, &net_num , lpublic );
			  else
			  	 nameToIP( intfName[0] , net_work, &net_num , lpublic );

			  fprintf(cgiOut,"<tr>"\
		      "<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:30px\">%s",search(lcontrol,"network_info"));
			  cgiFormStringNoNewlines("interface_all",select_opt,INTF_NAME_LENTH);
			  ccgi_dbus_init();
			  int vlan_ret=-1,eth_ret=-1;
			  vlan_ret=ccgi_intf_show_advanced_routing(1, 0,select_opt);
			  eth_ret=ccgi_intf_show_advanced_routing(0,1,select_opt);
				  
			  if((vlan_ret==5)||(eth_ret==5))
			  fprintf(cgiOut,"&nbsp;&nbsp;&nbsp;&nbsp;<font id=%s>%s</font></td>",search(lpublic,"menu_thead"),search(lpublic,"port_promis"));		

			  fprintf(cgiOut,"</tr>");
			  fprintf(cgiOut,"<tr style=padding-top:10px><td align=left style=\"background-color:#ffffff; border-right:0px solid #707070\">"\
			  "<table border=0 cellspacing=0 cellpadding=0>");
			  fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9  padding-top:5px align=left>"\
			  "<th width=100 style=font-size:12px align=left><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"intf_name"));
			  fprintf(cgiOut,"<th width=150 style=font-size:12px align=left><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"network"));

			  if(retu==0)  /*管理员*/
			  	  fprintf(cgiOut,"<th width=13>&nbsp;</th>");
			  fprintf(cgiOut,"</tr>");
			  int cl = 1; //控制颜色变化
			  for(i =0 ; i < net_num ; i++ )
			  	{
			  		memset(menu,0,21);
				  	strcpy(menu,"menulist");
				  	sprintf(i_char,"%d",i+1);
				  	strcat(menu,i_char);
			  		fprintf(cgiOut,"<tr height=25 bgcolor=%s  align=left>",setclour(cl));
					if( strcmp(select_opt, "") != 0 )							
						fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",select_opt);	
					
					//kehao modify  
					//else
						//fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",intfName[0]);
					else if( strcmp(delete_rule , "delete") == 0 )
						fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",name_for_delete);
					else
						fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",intfName[0]);
					fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",net_work[i]);
					if(retu==0)  /*管理员*/
					{
						fprintf(cgiOut,"<td align=center>");
        				  fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(net_num-i),menu,menu);
        				   fprintf(cgiOut,"<img src=/images/detail.gif>"\
        				   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
        				   fprintf(cgiOut,"<div id=div1>");
        					if( strcmp(select_opt, "") != 0 )
        					{
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_interface_bindip.cgi?UN=%s&INTF=%s target=mainFrame>%s</a></div>",encry,select_opt,search(lcontrol,"add"));
        			   			fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_all_interface.cgi?UN=%s&INTNAME=%s&NETWORK=%s&DELETE_RULE=%s&SELECT_OPT=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,select_opt,net_work[i],"delete",select_opt,search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
        					}
        			   		else
        			   		{
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_interface_bindip.cgi?UN=%s&INTF=%s target=mainFrame>%s</a></div>",encry,intfName[0],search(lcontrol,"add"));
								fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_all_interface.cgi?UN=%s&INTNAME=%s&NETWORK=%s&DELETE_RULE=%s&SELECT_OPT=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,intfName[0],net_work[i],"delete",intfName[0],search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
        			   		}
        				   fprintf(cgiOut,"</div>"\
        				   "</div>"\
        				   "</div>");
    					fprintf(cgiOut,"</td>");
			  		}
					  if( strcmp(select_opt, "") != 0 )
							fprintf(cgiOut,"<td><input type=hidden name=INTNAME value=%s></td>",select_opt);
					  else
					  		fprintf(cgiOut,"<td><input type=hidden name=INTNAME value=%s></td>",intfName[0]);
					  fprintf(cgiOut,"<td><input type=hidden name=NETWORK value=%s></td>",net_work[i]);
					
					fprintf(cgiOut,"</tr>");

					cl = !cl;
			  	}
			  fprintf(cgiOut,"</table></td></tr>");
    		  fprintf(cgiOut,"<tr>");
    		  fprintf(cgiOut,"<td><input type=hidden name=encry_addusr value=%s></td>",encry);
			  fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
    		  fprintf(cgiOut,"</tr></tr></table>");
	fprintf(cgiOut,"</td>"\
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


	for( i=0;i<MAX_INTF_NUM;i++ )
	{
		free( intfName[i] );
	}

	for( i = 0; i<8 ;i++ )
	{
		free( net_work[i] );
	}


	free(i_char);
	free(CheckUsr);
	free(address_for_delete);
	free(name_for_delete);
	free(select_opt);
	free(delete_rule);
	free(encry);
	release(lpublic);  
	release(lcontrol);	

	return 0;
}

int nameToIP(char * intfname,char * ipnetwork[],int * net_num,struct list * lpublic) // 接口多IP网段的读取
{
	int i=0;
	char buf[60],command[200];
	FILE * fd;

	sprintf(command,"show_intf_ip.sh %s 2>/dev/null | awk '{if($1~/inet/){print $2}}' ",intfname);
	fd = popen (command, "r");

	if(fd != NULL)
	{
		while((fgets(buf,60,fd)) != NULL)
		{
			if( strstr(buf,".") == NULL)
				{
					memset(buf,0,60);
					continue;
				}
			strncpy( ipnetwork[i] , buf , strlen(buf)-1);
			i++;
			memset(buf,0,60);
		}
		pclose(fd);
	}
	*net_num = i;
	return 0;
}


int  interfaceInfo(char * intfname[],int * infoNum,struct list * lpublic)
{
	FILE * ft;
	char * syscommand=(char *)malloc(300);
	memset(syscommand,0,300);
	int i = 0, j = 0,is_repeat = 0;
	sprintf(syscommand,"ip addr | awk 'BEGIN{FS=\":\";RS=\"(\^|\\n)[0-9]+:[ ]\"}{print $1}' | awk 'NR==2,NR==0{print}' ");
	ft = popen(syscommand,"r"); 
	char  temp[INTF_NAME_LENTH];
	memset(temp,0,INTF_NAME_LENTH);
	i=0;

	if(ft != NULL)
	{
		while((fgets(temp,INTF_NAME_LENTH-2,ft)) != NULL)
		{
			 is_repeat = 0;
			 for(j=0;j<i;j++)
			 {
			 	if(0 == strncmp(temp,intfname[j],strlen(temp)-1))
			 	{
			 		is_repeat = 1;
			 	}
			 }

			 if(0 == is_repeat)
			 {			 	 
				 strncpy(intfname[i],temp,strlen(temp)-1);
				 i++;
			 }
			 memset(temp,0,INTF_NAME_LENTH);
		}
		pclose(ft);
	}
	*infoNum=i;
	free(syscommand);
	return 1;
}

int delete_interface(char * intf_name,char * net,struct list * lcon)
{
	char * command = (char *)malloc(200);
	memset( command, 0, 200);

	if( intf_name != NULL && net!=NULL )
		{
			sprintf(command, "del_intf_ip.sh %s %s 1>/dev/null 2>/dev/null",intf_name,net);
			int status = system(command);
			int ret = WEXITSTATUS(status);
			
			if ( ret == 0 ) 
				ShowAlert(search(lcon,"no_intf_ip_suc"));
		}
	
	free(command);
	return 0;
}


