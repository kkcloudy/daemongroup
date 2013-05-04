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
* wp_qosmap.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for qos config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include <sys/wait.h>
#include "ws_init_dbus.h"

#include "ws_dcli_qos.h"

#define AMOUNT 512

int ShowQosMapPage();

int cgiMain()
{
 ShowQosMapPage();
 return 0;
}

int ShowQosMapPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 	
	char *encry=(char *)malloc(BUF_LEN);
	char *str=NULL;
	//FILE *fp;
	//char lan[3];
	char qosmap_encry[BUF_LEN]; 
	//char addn[N];         
	int i;   
	int retu=0,retz;
	int cl=1;
	int qos_num=0,map_num=0,up_num=0,dscp_num=0,dscpself_num=0;
	char menu[21]="menulist";
	char* i_char=(char *)malloc(10);
	char * delrule=(char *)malloc(10);
	memset(delrule,0,10);
	char * Index=(char *)malloc(10);
	memset(Index,0,10);

	char *mode=(char *)malloc(AMOUNT);
	memset(mode,0,AMOUNT);

	char * CheckUsr=(char *)malloc(10);
	memset(CheckUsr,0,10);
	struct qos_info receive_qos[MAX_QOS_PROFILE];
	struct mapping_info receive_map;
	receive_map.mapping_des= (char*)malloc(50);
	for(i=0;i<MAX_QOS_PROFILE;i++)
	{
		receive_qos[i].profileindex=0;
		receive_qos[i].dp=0;
		receive_qos[i].up=0;
		receive_qos[i].tc=0;
		receive_qos[i].dscp=0;
	}
	ccgi_dbus_init();
	if(cgiFormSubmitClicked("submit_qosmap") != cgiFormSuccess)
	{
		memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
			ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
			return 0;
		}
		//strcpy(addn,str);
		memset(qosmap_encry,0,BUF_LEN);                   /*清空临时变量*/
	}

	cgiFormStringNoNewlines("encry_routelist",qosmap_encry,BUF_LEN);
	cgiFormStringNoNewlines("DELRULE",delrule,BUF_LEN);
	cgiFormStringNoNewlines("INDEX",Index,BUF_LEN);

	cgiFormStringNoNewlines("CheckUsr",CheckUsr,10); 
	if(strcmp(CheckUsr,"")!=0)
		retu=atoi(CheckUsr);
	if(strcmp(delrule,"delete")==0)
	{
		retz=delete_qos_profile(Index,lcontrol);
		switch(retz)
		{
			case 0:
				ShowAlert(search(lpublic,"oper_succ"));
				break;
			case -1:
				ShowAlert(search(lpublic,"oper_fail"));
				break;
			case -2:
				ShowAlert(search(lcontrol,"illegal_input"));
				break;
			case -3:
				ShowAlert(search(lcontrol,"qos_profile_not_exist"));
				break;
			case -4:
				ShowAlert(search(lcontrol,"qos_profile_in_use"));
				break;
			case -5:
				ShowAlert(search(lpublic,"oper_fail"));
				break;
			default:
				break;
		}
	}
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"qos_manage"));
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	"<style type=text/css>"\
	"#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	"#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	"#link{ text-decoration:none; font-size: 12px}"\
	".ShowRoute {overflow-x:hidden;  overflow:auto; width: 580px; height: 386px; clip: rect( ); padding-top: 2px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
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
	if(cgiFormSubmitClicked("submit_qosmap") != cgiFormSuccess)
	{
		retu=checkuser_group(str);
	}
	if(cgiFormSubmitClicked("submit_qosmap") == cgiFormSuccess)
	{
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_contrl.cgi?UN=%s';\n", qosmap_encry);
		fprintf( cgiOut, "</script>\n" );
	}
	show_qos_mode(mode);

	fprintf(cgiOut,"<form method=post>"\
	"<div align=center>"\
	"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
	"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>QOS</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>"\
	"<td width=62 align=center><input id=but type=submit name=submit_qosmap style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  

	if(cgiFormSubmitClicked("submit_qosmap") != cgiFormSuccess)
	{
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosModule.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	}
	else                                         
	fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosModule.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",qosmap_encry,search(lpublic,"img_cancel"));
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
	"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>QOS </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"list"));   /*突出显示*/
	fprintf(cgiOut,"</tr>");
	if(cgiFormSubmitClicked("submit_qosmap") != cgiFormSuccess)
	{
		if(retu==0)
		{
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_addqos.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>QOS Profile</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));					   
			fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_addmap.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_map"));					   
			fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));					   
			fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_policymaplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"policy_map"));					   
			fprintf(cgiOut,"</tr>"\
			"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_createpolicy.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_policy"));
			fprintf(cgiOut,"</tr>");
		}
		else
		{
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));					   
			fprintf(cgiOut,"</tr>");

			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_policymaplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"policy_map"));					   
			fprintf(cgiOut,"</tr>");

		}
	}
	else if(cgiFormSubmitClicked("submit_qosmap") == cgiFormSuccess) 			  
	{
		if(retu==0)
		{
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_addqos.cgi?UN=%s target=mainFrame class=top style=color:#000000><font id=%s>%s</font><font id=yingwen_san>QOS Profile</font></a></td>",qosmap_encry,search(lpublic,"menu_san"),search(lcontrol,"add"));						
			fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_addmap.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",qosmap_encry,search(lpublic,"menu_san"),search(lcontrol,"add_map"));					   
			fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",qosmap_encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));					   
			fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_policymaplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",qosmap_encry,search(lpublic,"menu_san"),search(lcontrol,"policy_map"));					   
			fprintf(cgiOut,"</tr>"\
			"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_createpolicy.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",qosmap_encry,search(lpublic,"menu_san"),search(lcontrol,"add_policy"));
			fprintf(cgiOut,"</tr>");
		}
		else
		{
			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_qosmapinfo.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",qosmap_encry,search(lpublic,"menu_san"),search(lcontrol,"map_detail"));					   
			fprintf(cgiOut,"</tr>");

			fprintf(cgiOut,"<tr height=25>"\
			"<td align=left id=tdleft><a href=wp_policymaplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",qosmap_encry,search(lpublic,"menu_san"),search(lcontrol,"policy_map"));					   
			fprintf(cgiOut,"</tr>");

		}
	}
	int rowsCount=0;
	if(retu==0)
		rowsCount=13;
	else 
		rowsCount=16;
	for(i=0;i<rowsCount;i++)
	{
		fprintf(cgiOut,"<tr height=25>\n"\
		"<td id=tdleft>&nbsp;</td>\n"\
		"</tr>\n");
	}
	fprintf(cgiOut,"</table>\n"\
	"</td>\n"\
	"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">\n"\
	"<table width=640 height=310 border=0 cellspacing=0 cellpadding=0>\n"\
	"<tr>\n"\
	"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">\n"\
	"<table width=%s>\n","100%");
	fprintf(cgiOut,"<tr>\n"\
	"<td id=sec1 style=\"font-size:14px\">%s</td>\n",search(lcontrol,"QOS_info"));
	fprintf(cgiOut,"<td id=sec1 style=\"font-size:14px\" align='right'><font color=red><b>%s</b></font></td>\n",search(lcontrol,mode));
	fprintf(cgiOut,"</tr>\n"\
	"</table>\n"\
	"</td>\n");
	fprintf(cgiOut,"</tr>\n"\
	"<tr>\n"\
	"<td align=left valign=top style=padding-top:18px>\n"\
	"<div class=ShowRoute><table width=503 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>\n");
	fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 style=font-size:14px  id=td1 align='left'>\n"\
	"<th width=70 style=font-size:12px>%s</th>\n","INDEX");
	fprintf(cgiOut,"<th width=70 style=font-size:12px>%s</th>\n","DP");
	fprintf(cgiOut,"<th width=70 style=font-size:12px>%s</th>\n","UP");
	fprintf(cgiOut,"<th width=70 style=font-size:12px>%s</th>\n","TC");	
	fprintf(cgiOut,"<th width=70 style=font-size:12px>%s</th>\n","DSCP");
	//fprintf(cgiOut,"<th width=70 style=font-size:12px>%s</th>\n",search(lcontrol,"mapping_num"));
	fprintf(cgiOut,"<th width=13 style=font-size:12px>&nbsp;</th>\n");
	fprintf(cgiOut,"</tr>\n");
	for(i=0;i<MAX_QOS_PROFILE;i++)
	{
		receive_qos[i].profileindex=0;
		receive_qos[i].dp=0;
		receive_qos[i].up=0;
		receive_qos[i].tc=0;
		receive_qos[i].dscp=0;
	}
	show_qos_profile(receive_qos,&qos_num,lcontrol);
	for(i=0;i<qos_num;i++)
	{
		memset(menu,0,21);
		strcpy(menu,"menulist");
		sprintf(i_char,"%d",i+1);
		strcat(menu,i_char);
		map_num=0;
		receive_map=show_remap_table_byindex(&map_num,&up_num,&dscp_num,&dscpself_num,lcontrol);
		if(receive_qos[i].profileindex>0 && receive_qos[i].profileindex<128)
		{
			fprintf(cgiOut,"<tr height=25 bgcolor=%s>\n",setclour(cl));
			fprintf(cgiOut,"<td style=font-size:12px>%u</td>\n",receive_qos[i].profileindex);
			if(receive_qos[i].dp==0)
				fprintf(cgiOut,"<td style=font-size:12px>Green</td>\n");
			else if(receive_qos[i].dp==2)
				fprintf(cgiOut,"<td style=font-size:12px>Red</td>\n");
			fprintf(cgiOut,"<td style=font-size:12px>%u</td>\n",receive_qos[i].up);
			fprintf(cgiOut,"<td style=font-size:12px>%u</td>\n",receive_qos[i].tc);
			fprintf(cgiOut,"<td style=font-size:12px>%u</td>\n",receive_qos[i].dscp);
			//fprintf(cgiOut,"<td style=font-size:12px align=center>%d</td>\n",map_num);
			fprintf(cgiOut,"<td align=left>\n");
			if(retu==0)
			{
				fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">\n",(qos_num-i),menu,menu);
				fprintf(cgiOut,"<img src=/images/detail.gif>\n"\
				"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">\n",menu);
				fprintf(cgiOut,"<div id=div1>\n");
				if(cgiFormSubmitClicked("submit_qosmap") != cgiFormSuccess)
				{
					//fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosmapinfo.cgi?UN=%s&INDEX=%u target=mainFrame>%s</a></div>\n",encry,receive_qos[i].profileindex,search(lcontrol,"map_detail"));
					//f(retu==0)
					fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosmap.cgi?UN=%s&INDEX=%u&DELRULE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>\n",encry,receive_qos[i].profileindex,"delete",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
				}
				else
				{
					//fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosmapinfo.cgi?UN=%s&INDEX=%u target=mainFrame>%s</a></div>\n",qosmap_encry,receive_qos[i].profileindex,search(lcontrol,"map_detail"));
					//if(retu==0)
					fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosmap.cgi?UN=%s&INDEX=%u&DELRULE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>\n",qosmap_encry,receive_qos[i].profileindex,"delete",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
				}
				fprintf(cgiOut,"</div>\n");
				fprintf(cgiOut,"</div>\n"\
				"</div>\n");
			}
			fprintf(cgiOut,"</td>\n");
			fprintf(cgiOut,"</tr>\n");
			cl=!cl;
		}
	}

	fprintf(cgiOut,"</table></div>\n"\
	"</td>\n"\
	"</tr>\n"\						 

	"<tr>");
	if(cgiFormSubmitClicked("submit_qosmap") != cgiFormSuccess)
	{
		fprintf(cgiOut,"<td><input type=hidden name=encry_routelist value=%s></td>",encry);
		fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
	}
	else if(cgiFormSubmitClicked("submit_qosmap") == cgiFormSuccess)
	{
		fprintf(cgiOut,"<td><input type=hidden name=encry_routelist value=%s></td>",qosmap_encry);
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
	free(CheckUsr);
	free(i_char);
	free(delrule);
	free(receive_map.mapping_des);
	release(lpublic);  
	release(lcontrol);
	free(mode);
	return 0;
}
						 
