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
* wp_user_portal.c
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
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
#include <fcntl.h>
#include <sys/wait.h>
#include "ws_portal_container.h"
#include "ws_public.h"

#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vrrp.h"
#include "ws_eag_conf.h"
#include "ws_dbus_list_interface.h"

int ShowportalPage(struct list *lLicense,struct list *lsystem,struct list *lpublic);


typedef struct{
	int cur_id;
	int cur_id_state;
	char cur_id_record[256];
	char username[30];
	char ip_addr[20];
	char if_info[256];
	char ip1[4];
	char ip2[4];
	char ip3[4];
	char ip4[4];	
	char port[16];
	int reset_cur_id;
}STUserInput;

typedef struct{
//	STModuleContainer *pstModuleContainer;
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;
	struct list *lauth;
	struct list *lsystem;
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *fp;
	STUserInput stUserInput;
	
	int formProcessError;
} STPageInfo;


STPageInfo stPageInfo;
STPageInfo *pstPageInfo;





struct list *lpublic;
int cgiMain()
{
	struct list *lLicense;
	struct list *lsystem;
	struct list *lpublic;

	DcliWInit();
	ccgi_dbus_init();   
	
	pstPageInfo = &stPageInfo;
	
	memset( pstPageInfo, 0, sizeof(STPageInfo) );
	
	lsystem= get_chain_head("../htdocs/text/system.txt");
	lLicense =get_chain_head("../htdocs/text/authentication.txt");
	lpublic=get_chain_head("../htdocs/text/public.txt");
	ShowportalPage(lLicense,lsystem,lpublic);
	release(lLicense);
	release(lsystem); 
	release(lpublic); 
	return 0;
}


int ShowportalPage(struct list *lLicense,struct list *lsystem,struct list *lpublic)
{
	int i = 0;
	int interfaceNum = 0;
	int ret = 0;
	int cl = 0;
	char menu[21]="";
	char i_char[10]="";
	eag_captive_intfs captive_intfs;
	dbus_parameter parameter;
	instance_parameter *paraHead1 = NULL;
	instance_parameter *pq = NULL;
	void *ccgi_connection = NULL;
	char temp[10] = { 0 };
	memset( &captive_intfs, 0, sizeof(captive_intfs) );	
	int hs_flag = 0;
	char plotid[10] = {0};	
	char delstr[64] = {0};
	char infstr[64] = {0};

	memset(pstPageInfo->encry,0,BUF_LEN);
	cgiFormStringNoNewlines("UN", pstPageInfo->encry, BUF_LEN); 
	pstPageInfo->username_encry=dcryption(pstPageInfo->encry); 

	if(pstPageInfo->username_encry==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user")); 	       /*用户非法*/
		return 0;
	}
	pstPageInfo->iUserGroup = checkuser_group(pstPageInfo->username_encry);
	cgiHeaderContentType("text/html");

	
	memset(plotid,0,sizeof(plotid));
	cgiFormStringNoNewlines("plotid", plotid, sizeof(plotid)); 

	list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB);
	if (NULL == paraHead1) {
		return 0;
	}
	if(strcmp(plotid, "") == 0)
	{
		parameter.instance_id = paraHead1->parameter.instance_id;
		parameter.local_id = paraHead1->parameter.local_id;
		parameter.slot_id = paraHead1->parameter.slot_id;
		snprintf(plotid,sizeof(plotid)-1,"%d-%d-%d",parameter.slot_id, parameter.local_id, parameter.instance_id);
	}
	else
	{
		get_slotID_localID_instanceID(plotid, &parameter);
	}
	ccgi_ReInitDbusConnection(&ccgi_connection, parameter.slot_id, DISTRIBUTFAG);
	
 
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>\n");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>\n");
	//下面三句话用于禁止页面缓存
	
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");
 	  
  fprintf(cgiOut,"<title>%s</title>","Captive Protal");  
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
	"<style type=text/css>"\
	"#div1{ width:58px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	"#div2{ width:56px; height:15px; padding-left:3px; padding-top:3px}"\
	"#link{ text-decoration:none; font-size: 12px}"\
    ".usrlis {overflow-x:hidden;	overflow:auto; width: 690; height: 220px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	"</style>"\
  "<script language=javascript src=/ip.js>\n"\
  	"</script>\n"\
  	"<script type=\"text/javascript\">\n"\
			"function popMenu(objId)\n"\
			"{\n"\
			   "var obj = document.getElementById(objId);\n"\
			   "if (obj.style.display == 'none')\n"\
			   "{\n"\
				 "obj.style.display = 'block';\n"\
			   "}\n"\
			   "else\n"\
			   "{\n"\
				 "obj.style.display = 'none';\n"\
			   "}\n"\
		   "}");
  fprintf(cgiOut,"</script>\n"\
  	"</head>\n"\
  	"<body>");
  	fprintf(stderr, "--------------------------------------plotid=%s", plotid);
	char *cgiRequestMethod = NULL;
	cgiGetenv_func(&cgiRequestMethod, "REQUEST_METHOD");
	if((cgiFormSubmitClicked("inf_add") == cgiFormSuccess) && (0 == strcmp(cgiRequestMethod,"POST")))
	{
	#if 0
			memset(plotid,0,sizeof(plotid));
			p_id = 0;
			memset(infstr,0,sizeof(infstr));
			cgiFormStringNoNewlines("port_id",plotid, sizeof(plotid)); 
			p_id = atoi(plotid);
			if(0 == p_id)
			{
				hs_flag = HANSI_LOCAL;
			}
			else
			{
				hs_flag = HANSI_REMOTE;
			}
	#endif
		cgiFormStringNoNewlines("inf_str",infstr, sizeof(infstr)); 
		fprintf(stderr, "inf_add, %s", infstr);	
		ret = eag_add_captive_intf(ccgi_connection, parameter.local_id,
					parameter.instance_id, 4, infstr);
	}
	cgiFormStringNoNewlines("DELRULE",delstr, sizeof(delstr)); 
	if(0 == strcmp(delstr,"delete"))
	{
	#if 0
		memset(plotid,0,sizeof(plotid));
		p_id = 0;
		cgiFormStringNoNewlines("plotid",plotid, sizeof(plotid)); 

		p_id = atoi(plotid);
		if(0 == p_id)
		{
			hs_flag = HANSI_LOCAL;
		}
		else
		{
			hs_flag = HANSI_REMOTE;
		}
	#endif
		cgiFormStringNoNewlines("INF",infstr, sizeof(infstr)); 
		ret = eag_del_captive_intf(ccgi_connection, parameter.local_id,
						parameter.instance_id, 4, infstr);

	}

  fprintf(cgiOut,"<form method=post>\n" );
  fprintf( cgiOut, "<div align=center>\n"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
"<tr>\n"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>\n"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>\n"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),"Captive Portal");
    fprintf(cgiOut,"<td width=690px align=right valign=bottom background=/images/di22.jpg>");
	  {   
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>\n"\
		"<tr>\n" );
		fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=inf_add style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_authentication.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",pstPageInfo->encry,search(lpublic,"img_cancel"));

		fprintf(cgiOut,"</tr>\n"\
		"</table>");
	  } 	  
	fprintf(cgiOut,"</td>\n"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>\n"\
"</tr>\n"\
"<tr>\n"\
    "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>\n"\
	"<tr>\n"\
        "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>\n"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>\n"\
		  "<tr height=4 valign=bottom>\n"\
              "<td width=120>&nbsp;</td>\n"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>\n"\
		  "</tr>\n"\
		  "<tr>\n"\
              "<td><table width=120 border=0 cellspacing=0 cellpadding=0>\n"\
				 "<tr height=25>\n"\
				  "<td id=tdleft>&nbsp;</td>\n"\
				"</tr>");				
		 
				
				//user manage
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_user_manage.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "user_mng") );								
				
				//eag
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_eag_conf.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "eag_title") );

				//multi portal
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_multi_portal.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "multi_portal_management") );	

				//multi raidus
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_multi_radius.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry, search(lpublic,"menu_san"),search( lLicense, "multi_radius_management") );	

				//captive interface
				fprintf(cgiOut,"<tr height=26>\n"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\" ><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lLicense,"captive_Portal"));	/*突出显示*/
				fprintf(cgiOut,"</tr>");
				
				//white list
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_white_list.cgi?UN=%s&portal_id=0' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "portal_white_list") );
				
				//black list
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_black_list.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "portal_black_list") );	

				//nas
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_nasid_byvlan.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "nasid_management") );				

				//vlan map
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_wtpwlan_map_vlan.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry, search(lpublic,"menu_san"),search( lLicense, "vlan_maping") );	
			
				//pdc
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_pdc_conf.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry,search(lpublic,"menu_san"), search( lLicense, "pdc_conf") );				

				//rdc
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_rdc_conf.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry, search(lpublic,"menu_san"),search( lLicense, "rdc_conf") );	

				// portal ftp
				fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_portal_ftp.cgi?UN=%s' target=mainFrame class=top><font id=%s>%s</font></td></tr> \n",pstPageInfo->encry, search(lpublic,"menu_san"),search( lLicense, "portal_ftp") );	
				
				for(i=0;i<4+interfaceNum;i++)
				{
				  fprintf(cgiOut,"<tr height=25>\n"\
					"<td id=tdleft>&nbsp;</td>\n"\
				  	"</tr>");
				}
			  fprintf(cgiOut,"</table>\n"\
			"</td>\n"\
			"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">\n"\
			  "<table border=0 cellspacing=0 cellpadding=0 width=650 valign=top>");

	
/****************************************************************************************************
		add by shaojunwu     2008-9-4 17:50:01
		for get  interface information
*********************************************************************************************************/	
    fprintf(cgiOut,"<tr height=30 valign=top>");
	fprintf(cgiOut,"<td width=200 valign>%s</td>",search(lLicense,"plot_idz"));
	fprintf(cgiOut,"<td width=450><select name=plotid onchange=plotid_change(this)>");
	for (pq=paraHead1;(NULL != pq);pq=pq->next)
	{
		memset(temp,0,sizeof(temp));
		snprintf(temp,sizeof(temp)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);
		fprintf(stderr, "-----------------------------------------------------temp=%s\n", temp);
		if (strcmp(plotid, temp) == 0)
			fprintf(cgiOut,"<option value='%s' selected>%s</option>\n",temp,temp);
		else	       
			fprintf(cgiOut,"<option value='%s'>%s</option>\n",temp,temp);
	}
	fprintf(cgiOut,"</select></td>");
	fprintf(cgiOut,"</tr>\n" );
	fprintf(cgiOut,"<script type=text/javascript>\n");
   	fprintf(cgiOut,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_user_portal.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(cgiOut,"</script>\n" );
	
	fprintf(cgiOut,"<tr>");
	fprintf(cgiOut,"<td width=200>%s</td>\n", search(lLicense,"interface") );
	fprintf(cgiOut,"<td width=450><select name=inf_str>");
	infi  interf;
	interface_list_ioctl (0,&interf);
	char dupinf[20] = {0};
	infi * q ;
	q = interf.next;
	while(q)
	{
	    memset(dupinf,0,sizeof(dupinf));
		if(NULL != q->next)
		{
			strcpy(dupinf,q->next->if_name);
		}
		if( !strcmp(q->if_name,"lo") )
		{
			q = q->next;
			continue;
		}
		if( !strcmp(q->if_name,dupinf) )
		{
			q = q->next;
			continue;
		}
        fprintf(cgiOut,"<option value=%s>%s</option>",q->if_name,q->if_name);		
		q = q->next;
	}
    free_inf(&interf);
	fprintf(cgiOut,"</select></td>");
	fprintf(cgiOut,"</tr>\n" );

	fprintf(cgiOut,"<tr><td colspan=2><div class=usrlis><table>");
	
	fprintf(cgiOut,"<tr height=30 align=left bgcolor=#eaeff9>");
	fprintf(cgiOut,"<th width=100>Index</th>");
	fprintf(cgiOut,"<th width=150>%s</th>",search(lLicense,"interface"));
	fprintf(cgiOut,"<th width=100>&nbsp;</th>");
	fprintf(cgiOut,"</tr>");

	ret = eag_get_captive_intfs(ccgi_connection, parameter.local_id,
						parameter.instance_id, &captive_intfs);

	if( EAG_RETURN_OK == ret )
	{
		if( captive_intfs.curr_ifnum > 0 )
		{
			for( i=0; i < captive_intfs.curr_ifnum; i++ )
			{						
				memset(menu,0,21);
				strcpy(menu,"menulist");
				sprintf(i_char,"%d",i+1);
				strcat(menu,i_char);

				fprintf(cgiOut,"<tr align=left bgcolor=%s>", setclour(cl));
				fprintf(cgiOut,"<td>%d</td>",i+1);
				fprintf(cgiOut,"<td>%s</td>",captive_intfs.cpif[i]);
				fprintf(cgiOut,"<td>");
				fprintf(cgiOut, "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(10240-i),menu,menu);
				fprintf(cgiOut, "<img src=/images/detail.gif>"\
				"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
				fprintf(cgiOut, "<div id=div1>");
				fprintf(cgiOut, "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_user_portal.cgi?UN=%s&DELRULE=delete&INF=%s target=mainFrame>%s</a></div>", pstPageInfo->encry, captive_intfs.cpif[i],search(lpublic,"delete"));
				fprintf(cgiOut, "</div>"\
				"</div>"\
				"</div>");
				fprintf(cgiOut,"</td>");
				fprintf(cgiOut,"</tr>");
				cl = !cl;
			}
		}
	}

	fprintf(cgiOut,"</table></div></td></tr>");
/*********************************************************************************************
		end of add by shaojunwu

*********************************************************************************************/

	fprintf(cgiOut,"<input type=hidden name=UN value=%s>",pstPageInfo->encry);
	fprintf(cgiOut,"<input type=hidden name=plotid value=%s>",plotid);
	fprintf(cgiOut,"</tr></table>");

	fprintf(cgiOut,"</td>\n"\
		  "</tr>\n"\
		  "<tr height=4 valign=top>\n"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>\n"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>\n"\
		  "</tr>\n"\
		"</table>\n"\
	  "</td>\n"\
	  "<td width=15 background=/images/di999.jpg>&nbsp;</td>\n"\
	"</tr>\n"\
  "</table></td>\n"\
"</tr>\n"\
"<tr>\n"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>\n"\
  "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>\n"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>\n"\
"</tr>\n"\
"</table>\n"\
"</div>\n"\
"</form>\n"\
"</body>\n");

fprintf( cgiOut, "</html>");

free_instance_parameter_list(&paraHead1);


return 0;
}
