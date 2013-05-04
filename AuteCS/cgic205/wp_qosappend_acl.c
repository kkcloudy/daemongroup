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
* wp_qosappend_acl.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for qos append acl
*
*
*******************************************************************************/
#include <stdio.h>
#include "ws_ec.h"
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_dcli_qos.h"
#include "ws_init_dbus.h"

#define PATH_LENG 512  
#define AMOUNT 64  //转换数组的容量

int configQosBaseAcl(struct list *lpublic, struct list *llocal);   /*n代表加密后的字符串*/

int cgiMain()
{
 	struct list *lpublic;
 	struct list *lcontrol;
 	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");
 	configQosBaseAcl(lpublic, lcontrol);
 return 0;
}

int configQosBaseAcl(struct list *lpublic, struct list *lcontrol)
{ 
	ccgi_dbus_init();				/*初始化*/
	char * index=malloc(PATH_LENG);
	char * rule = malloc(PATH_LENG);
	char * insert_index=malloc(PATH_LENG);
	
	memset(index,0,PATH_LENG);
	memset(rule,0,PATH_LENG);
	memset(insert_index,0,PATH_LENG);
	struct qos_info qos_list[MAX_QOS_PROFILE];
	int qos_number = 0;
	char * mode=(char *)malloc(AMOUNT);
	
	show_qos_profile(qos_list,&qos_number,lcontrol);
	//configQosMode("flow");
	show_qos_mode(mode);
	
  	int i,reback;
  	char *encry=malloc(BUF_LEN);                /*存储从wp_usrmag.cgi带入的加密字符串*/
  	char *str = NULL;        
  	memset(encry,0,BUF_LEN);
  	
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  	str=dcryption(encry);
  	if(str==NULL)
  	{
    		ShowErrorPage(search(lpublic, "ill_user")); 	       /*用户非法*/
    		return 0;
  	}

	if(cgiFormSubmitClicked("addAclQos")!=cgiFormSuccess)
	{
		cgiFormStringNoNewlines("INDEX", index,PATH_LENG);
	}
	else
	{
		cgiFormStringNoNewlines("insert_index",index,PATH_LENG);
	}
	cgiFormStringNoNewlines("rule",rule,PATH_LENG);
	
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>\n");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>\n");
  fprintf(cgiOut,"<title>%s</title>\n",search(lcontrol,"aclqos"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  "<style type=text/css>\n"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>\n"\
  "<script type=\"text/javascript\">\n");  
	  fprintf(cgiOut,"</script>\n");
  fprintf(cgiOut,"</head>\n");


			if(cgiFormSubmitClicked("addAclQos")==cgiFormSuccess)
			{
				char url_temp[128];
				if(strcmp(mode,"flow")==0)
				{
				
					if(strcmp(rule,"")==0)
					{
						ShowAlert(search(lcontrol,"rulenotnull"));
						sprintf( url_temp, "wp_qosappend_acl.cgi?UN=%s&INDEX=%s",  encry,index );
						//cgiHeaderLocation( url_temp );
						fprintf( cgiOut, "<script type='text/javascript'>\n" );
						fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
						fprintf( cgiOut, "</script>\n" );
					}
					else
					{
						reback=config_qos_base_acl_traffic(index,rule);
						switch(reback)
						{
							case CMD_FAILURE:
									{
										ShowAlert(search(lcontrol,"failgetreplay"));
										sprintf( url_temp, "wp_qosappend_acl.cgi?UN=%s&INDEX=%s",encry,index );
										fprintf( cgiOut, "<script type='text/javascript'>\n" );
										fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
										fprintf( cgiOut, "</script>\n" );									
									}
									break;
							case NPD_DBUS_ERROR_ALREADY_PORT:
									{
										ShowAlert(search(lcontrol,"notflow"));
										sprintf( url_temp, "wp_qosappend_acl.cgi?UN=%s&INDEX=%s",encry,index );
										fprintf( cgiOut, "<script type='text/javascript'>\n" );
										fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
										fprintf( cgiOut, "</script>\n" );
									}
									break;
							case NPD_DBUS_ERROR_NO_QOS_MODE:
									{
										ShowAlert(search(lcontrol,"notqos"));
										sprintf( url_temp, "wp_qosappend_acl.cgi?UN=%s&INDEX=%s",encry,index );
										fprintf( cgiOut, "<script type='text/javascript'>\n" );
										fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
										fprintf( cgiOut, "</script>\n" );
									}
									break;
							case NPD_DBUS_ERROR_HYBRID_FLOW:
									{
										ShowAlert(search(lcontrol,"modehybird"));
										sprintf( url_temp, "wp_qosappend_acl.cgi?UN=%s&INDEX=%s",encry,index );
										fprintf( cgiOut, "<script type='text/javascript'>\n" );
										fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
										fprintf( cgiOut, "</script>\n" );
									}
									break;
							case QOS_PROFILE_NOT_EXISTED:
									{
										ShowAlert(search(lcontrol,"profilenotexist"));
										sprintf( url_temp, "wp_qosappend_acl.cgi?UN=%s&INDEX=%s",encry,index );
										fprintf( cgiOut, "<script type='text/javascript'>\n" );
										fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
										fprintf( cgiOut, "</script>\n" );
									}
									break;
							case ACL_RULE_EXT_ONLY:
									{
										ShowAlert(search(lcontrol,"useextended"));
										sprintf( url_temp, "wp_qosappend_acl.cgi?UN=%s&INDEX=%s",encry,index );
										fprintf( cgiOut, "<script type='text/javascript'>\n" );
										fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
										fprintf( cgiOut, "</script>\n" );
									}
									break;
							case QOS_FAIL_TRAFFIC:
									{
										ShowAlert(search(lcontrol,"failtraffic"));
										sprintf( url_temp, "wp_qosappend_acl.cgi?UN=%s&INDEX=%s",encry,index );
										fprintf( cgiOut, "<script type='text/javascript'>\n" );
										fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
										fprintf( cgiOut, "</script>\n" );
									}
									break;
							case CMD_SUCCESS:
									{
										ShowAlert(search(lcontrol,"appendsucc"));
										sprintf( url_temp, "wp_aclall.cgi?UN=%s",  encry );
										fprintf( cgiOut, "<script type='text/javascript'>\n" );
										fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
										fprintf( cgiOut, "</script>\n" );
									}
									break;
						}
						/*if(reback==CMD_SUCCESS)
						{
							ShowAlert(search(lcontrol,"appendsucc"));
							sprintf( url_temp, "wp_aclall.cgi?UN=%s",  encry );
							fprintf( cgiOut, "<script type='text/javascript'>\n" );
							fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
							fprintf( cgiOut, "</script>\n" );
						}
						else
						{
							sprintf( url_temp, "wp_qosappend_acl.cgi?UN=%s&INDEX=%s",encry,index );
							fprintf( cgiOut, "<script type='text/javascript'>\n" );
							fprintf( cgiOut, "window.location.href='%s';\n", url_temp );
							fprintf( cgiOut, "</script>\n" );
						}*/
					}
				}
				else
				{	
					ShowAlert(search(lcontrol,"notflow"));
				}
			}



  fprintf(cgiOut,"<body>\n"\
  "<form id=Form1>\n"\
  "<div align=center>\n"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
  "<tr>\n"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>\n"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>\n"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>\n",search(lpublic,"title_style"),search(lcontrol,"aclqos"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>\n");
	    //if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
	     // ShowAlert(search(lpublic,"error_open"));
	   // fseek(fp,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
	    //fgets(lan,3,fp);
		//fclose(fp);
		
	  //  if(strcmp(lan,"ch")==0)
    //	{	
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>\n"\
          "<tr>\n"\
          "<td width=62 align=center><input id=but type=submit name=addAclQos style=background-image:url(/images/%s) value=""></td>\n",search(lpublic, "img_ok"));		  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>\n",encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>\n"\
          "</table>\n");
		//}
		/*
		else			
		{	
		  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>\n"\
		  "<tr>\n"\
		  "<td width=62 align=center><input id=but type=submit name=addAclQos style=background-image:url(/images/ok-en.jpg) value=""></td>\n");		  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_aclall.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>\n",encry);
		  fprintf(cgiOut,"</tr>\n"\
		  "</table>\n");
		}
		*/
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
                  "</tr>\n");
					
					//显示左侧功能表
					fprintf(cgiOut,"<tr height=26>\n"\
					  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>\n",search(lpublic,"menu_san"),search(lcontrol,"aclqos"));
					fprintf(cgiOut,"</tr>\n");
					//调整页面纵向长度
					for(i=0;i<7;i++) 
	              {
  				    fprintf(cgiOut,"<tr height=25>\n"\
                      "<td id=tdleft>&nbsp;</td>\n"\
                    "</tr>\n");
	              }
                fprintf(cgiOut,"</table>\n"\
				"</td>\n"\
				"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">\n"\
				"<table  border=0 cellspacing=0 cellpadding=0>\n"\
				"<tr>\n"\
				"<td align='left'>\n");
	fprintf(cgiOut,"<input type='hidden' name=UN value=%s>",encry);//鉴权
	 //MY CODE********************************************************************************************************	
		fprintf(cgiOut,"<table>\n");
			 for(i=0;i<3;i++)
			 {
			 fprintf(cgiOut,"<tr>"\
			 				"<td colspan='2'></td>\n"\
			 			"</tr>\n");
			 }
			  fprintf(cgiOut,"<tr height='35'>"\
			 				"<td colspan='3' style=\"font-size:14px\"><font color='red'><b>%s</b></font></td>"\
			 			"</tr>\n",search(lcontrol,mode));
			fprintf(cgiOut,"<tr>\n"\
							"<td align='left'>Qos Profile:</td>\n"\
							"<td align='left'>\n"\
								"<select name='rule'>\n"\
									"<option value=''>--%s--\n",search(lcontrol,"select"));
								for(i = 0;i<qos_number;i++)
								{
						fprintf(cgiOut,"<option value=%d>%d\n",qos_list[i].profileindex,qos_list[i].profileindex);
								}
					fprintf(cgiOut,"</select>\n"\
							"</td>\n"\
							"<td align='left'><font color=red>(1-127)</font></td>\n"
						"</tr>");
			for(i=0;i<3;i++)
			 {
			 fprintf(cgiOut,"<tr>"\
			 				"<td colspan='3'></td>\n"\
			 			"</tr>\n");
			 }
		fprintf(cgiOut,"</table>\n");
		//fprintf(cgiOut,"<div align='left'>%s</div>\n",search(lcontrol, "tips"));
		//fprintf(cgiOut,"<hr color='red'>\n");
		fprintf(cgiOut,"<table align='left'>\n");
				fprintf(cgiOut,"<tr>"\
			  					 "<td  id=sec1 style=\"padding-left:23px;width=600; border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
			       fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr  height=25 style=\"padding-left:23px; padding-top:2px\">\n"\
							"<td align='left' colspan='4'  style=font-size:14px;color:#FF0000><font color=red>%s</font></td>"\
						"</tr>\n"\
						"<tr  height=25 style=\"padding-left:23px; padding-top:2px\">\n"\
							"<td align='left' colspan='4'  style=font-size:14px;color:#FF0000><font color=red>%s</font></td>"\
						"</tr>\n",search(lcontrol,"aclqosone"),search(lcontrol,"aclqostwo"));
						fprintf(cgiOut,"<tr><td><input type=hidden name=insert_index value=%s></td></tr>\n",index);
		//fprintf(cgiOut,"<tr><td>sdfasdasdfa%s</td></tr>\n",index);
		fprintf(cgiOut,"</table>\n");
//*********************************************************END
fprintf(cgiOut,"</td>\n"\
		  	  "</tr>\n"\
      "</table>\n"\
              "</td>\n"\
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
"</body>\n"\
"</html>\n");
free(index);
free(rule);
free(insert_index);
//free(flow_type);

release(lpublic);
release(lcontrol);
free(encry);
return 0;
}

