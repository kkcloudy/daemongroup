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
* wp_acl_port_info.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for acl port infos
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
#include "ws_dcli_acl.h"
#include "ws_init_dbus.h"

#define PATH_LENG 512  
#define SHOW_AMOUNT 64  //转换数组的容量

int ShowAclPortInfo(struct list *lpublic, struct list *lcontrol);   /*n代表加密后的字符串*/

int cgiMain()
{
 struct list *lpublic;
 struct list *lcontrol;

 	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");


 ShowAclPortInfo(lpublic,lcontrol);
 return 0;
}

int ShowAclPortInfo(struct list *lpublic,struct list *lcontrol)
{ 

 	// FILE *fp;
  	//char lan[3]; 
  	char * type_num=malloc(SHOW_AMOUNT);
  	char * index_num=malloc(SHOW_AMOUNT);
  	char * type=malloc(SHOW_AMOUNT);
  	memset(type,0,SHOW_AMOUNT);
  	memset(type_num,0,SHOW_AMOUNT);
  	memset(index_num,0,SHOW_AMOUNT);
 
  	struct acl_groupone_info * p_oneinfo;
  	p_oneinfo = (struct acl_groupone_info *)malloc(sizeof(struct acl_groupone_info));
  
  	if(p_oneinfo==NULL)
  	{
  		ShowErrorPage("no space");
		return 0;
  	}
  	int i,j,t=0;
  	char *encry=malloc(BUF_LEN);                /*存储从wp_usrmag.cgi带入的加密字符串*/
  	char *str = NULL;        
  	memset(encry,0,BUF_LEN);

	//char url_temp[128];
  	ccgi_dbus_init();
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  	str=dcryption(encry);
  	if(str==NULL)
  	{
    		ShowErrorPage(search(lpublic, "ill_user")); 	       /*用户非法*/
    		return 0;
  	}
	
 	 if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
	{

		cgiFormStringNoNewlines("INDEX",index_num,SHOW_AMOUNT);
		cgiFormStringNoNewlines("TYPE",type_num,SHOW_AMOUNT);
	}
	if(strcmp("0",type_num)==0)
	{	
		type="ingress";
	}else
	{
		type="egress";
	}
  //strcpy(addn,str);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>\n");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>\n");
  fprintf(cgiOut,"<title>Time</title>\n");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>\n"\
  "<style type=text/css>\n"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>\n"\
  "<script type=\"text/javascript\">\n");
	  fprintf(cgiOut,"</script>\n");
  fprintf(cgiOut,"</head>\n");

  fprintf(cgiOut,"<body>\n"\
  "<form id=Form1>\n"\
  "<div align=center>\n"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
  "<tr>\n"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>\n"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>\n"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>\n",search(lpublic,"title_style"),search(lcontrol,"portinfo"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>\n");
//	    if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
//	      ShowAlert(search(lpublic,"error_open"));
//	    fseek(fp,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
//	    fgets(lan,3,fp);
//		fclose(fp);
//		
	   // if(strcmp(lan,"ch")==0)
    	//{	
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>\n"\
          "<tr>\n"\
          "<td width=62 align=center><input id=but type=submit name=set_time style=background-image:url(/images/%s) value=""></td>\n",search(lpublic,"img_ok"));		  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_aclgrplist.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>\n",encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>\n"\
          "</table>\n");
		/*}		
		else			
		{	
		  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>\n"\
		  "<tr>\n"\
		  "<td width=62 align=center><input id=but type=submit name=set_time style=background-image:url(/images/ok-en.jpg) value=""></td>\n");		  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_aclgrplist.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>\n",encry);
		  fprintf(cgiOut,"</tr>\n"\
		  "</table>\n");
		}*/		
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
		
					show_acl_group_one(type,index_num, p_oneinfo);
				//return 0;
					//show port information
					fprintf(cgiOut,"<tr height=26>\n"\
					  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>\n",search(lpublic,"menu_san"),search(lcontrol,"portinfo"));
					fprintf(cgiOut,"</tr>\n");
					if(((p_oneinfo->acl_count)>=(p_oneinfo->bind_by_port_count))&&((p_oneinfo->acl_count)>=(p_oneinfo->vlan_count)))
					{
						t=(p_oneinfo->acl_count)+10;
					}
					else if(((p_oneinfo->bind_by_port_count)>=(p_oneinfo->acl_count))&&((p_oneinfo->bind_by_port_count)>=(p_oneinfo->vlan_count)))
					{
						t=(p_oneinfo->bind_by_port_count)+10;
					}
					else if(((p_oneinfo->vlan_count)>=(p_oneinfo->acl_count))&&((p_oneinfo->vlan_count)>=(p_oneinfo->bind_by_port_count)))
					{
						t=(p_oneinfo->vlan_count)+10;
					}
					for(i=0;i<t;i++) 
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

	 //MY CODE********************************************************************************************************	
	 		fprintf(cgiOut,"<input type='hidden' name=UN value=%s>",encry);//鉴权
						

fprintf(cgiOut,"<table align=left border=0>");
fprintf(cgiOut,"<tr>"\
			     "<td colspan='4'><div align='center' style=font-size:14px color:#272727><b>%s</b></div></td>\n",search(lcontrol,"information"));
fprintf(cgiOut,"</tr>\n");
fprintf(cgiOut," <tr>\n"\
			    "<td></td>\n");
fprintf(cgiOut,"<td>\n"\
				"<table width=%s border=1  frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>\n","100%");
			if(type=="ingress")
			{
		fprintf(cgiOut,"<tr>\n"\
						"<td align='left' style=\"background-color:#f9fafe\">%s:</td>\n",search(lcontrol,"ingroup"));
			fprintf(cgiOut,"<td align='left' style=\"background-color:#f9fafe\">&nbsp;&nbsp;%u&nbsp;&nbsp;</td>\n"\
					"</tr>\n",p_oneinfo->groupIndex);
			}
			else
			{
		fprintf(cgiOut,"<tr>\n"\
						"<td align='left' style=\"background-color:#f9fafe\">%s:</td>\n",search(lcontrol,"egroup"));
			fprintf(cgiOut,"<td align='left' style=\"background-color:#f9fafe\">&nbsp;&nbsp;%u&nbsp;&nbsp;</td>\n"\
					"</tr>\n",p_oneinfo->groupIndex);		
			}
		fprintf(cgiOut,"<tr>\n"\
						"<td align='left' style=\"background-color:#ffffff\">%s:</td>\n",search(lcontrol,"bindcount"));
			fprintf(cgiOut,"<td align='left' style=\"background-color:#ffffff\">&nbsp;&nbsp;%u&nbsp;&nbsp;</td>\n"\
					"</tr>\n",p_oneinfo->bind_by_port_count);
		fprintf(cgiOut,"<tr>\n"\
						"<td align='left' style=\"background-color:#f9fafe\">%s:</td>\n",search(lcontrol,"aclcount"));
			fprintf(cgiOut,"<td align='left' style=\"background-color:#f9fafe\">&nbsp;&nbsp;%u&nbsp;&nbsp;</td>\n"\
					"</tr>\n",p_oneinfo->acl_count);
		fprintf(cgiOut,"<tr>\n"\
						"<td align='left' style=\"background-color:#f9fafe\">%s:</td>\n",search(lcontrol,"vlancount"));
			fprintf(cgiOut,"<td align='left' style=\"background-color:#f9fafe\">&nbsp;&nbsp;%u&nbsp;&nbsp;</td>\n"\
					"</tr>\n",p_oneinfo->vlan_count);
	fprintf(cgiOut,"</table>"\	
			"</td>\n");
fprintf(cgiOut,"<tr  height=15><td colspan=4>&nbsp;</td></tr>\n"\
			  "<tr>\n"\
			    "<td valign='top'>\n"\
				  "<p align='center' style=font-size:14px color:#272727><b>%s&nbsp;&nbsp;&nbsp;</b></p>\n"\
				  "<table width=%s border=1  frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>\n",search(lcontrol,"portinfo"),"100%");
  		if((p_oneinfo->bind_by_port_count)!=0)
  		{
  			for(i=0;i<(p_oneinfo->bind_by_port_count);i++)
  			{
  				if(i%2==0)
  				{
	       		fprintf(cgiOut,"<tr>\n"\
			        				"<td align='center' style=\"background-color:#f9fafe\">%s:</td>\n",search(lcontrol,"bindport"));
			        	fprintf(cgiOut,"<td align='center' style=\"background-color:#f9fafe\">%u/%u</td>\n"\
			      				"</tr>\n",p_oneinfo->bind_by_slot[i],p_oneinfo->bind_by_port[i]);
  				}
				else
  				{
  				fprintf(cgiOut,"<tr>\n"\
			        				"<td align='center' style=\"background-color:#ffffff\">%s:</td>\n",search(lcontrol,"bindport"));
			        	fprintf(cgiOut,"<td align='center' style=\"background-color:#ffffff\">%u/%u</td>\n"\
			      				"</tr>\n",p_oneinfo->bind_by_slot[i],p_oneinfo->bind_by_port[i]);
  				}
  			}
  		}
		else
  		{
  		fprintf(cgiOut,"<tr>\n"\
			        		"<td align='center'  style=\"background-color:#f9fafe\">%s</td>\n"\
			      		"</tr>\n",search(lcontrol,"noport"));
  		}
	   fprintf(cgiOut,"</table>\n"\
	   		 "</td>\n");
	 fprintf(cgiOut,"<td valign='top' colspan='2'>\n"\
				  "<p align='center' style=font-size:14px color:#272727><b>%s&nbsp;&nbsp;&nbsp;</b></p>\n"\
				  "<table width=%s border=1  frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>\n",search(lcontrol,"vlaninfo"),"100%");
  		if((p_oneinfo->vlan_count)!=0)
  		{
  			for(i=0;i<(p_oneinfo->vlan_count);i++)
  			{
  				if(i%2==0)
  				{
	       		fprintf(cgiOut,"<tr>\n"\
			        				"<td align='center' style=\"background-color:#f9fafe\">%s:</td>\n",search(lcontrol,"bindvlan"));
			        	fprintf(cgiOut,"<td align='center' style=\"background-color:#f9fafe\">%u</td>\n"\
			      				"</tr>\n",p_oneinfo->bind_by_vlan[i]);
  				}
				else
  				{
  				fprintf(cgiOut,"<tr>\n"\
			        				"<td align='center' style=\"background-color:#ffffff\">%s:</td>\n",search(lcontrol,"bindvlan"));
			        	fprintf(cgiOut,"<td align='center' style=\"background-color:#ffffff\">%u</td>\n"\
			      				"</tr>\n",p_oneinfo->bind_by_vlan[i]);
  				}
  			}
  		}
		else
  		{
  		fprintf(cgiOut,"<tr>\n"\
			        		"<td align='center'  style=\"background-color:#f9fafe\">%s</td>\n"\
			      		"</tr>\n",search(lcontrol,"novlan"));
  		}
	   fprintf(cgiOut,"</table>\n"\
	   		 "</td>\n");
fprintf(cgiOut,"<td valign='top'><p align='center' style=font-size:14px color:#272727><b>%s&nbsp;&nbsp;&nbsp;</b></p>\n"\
			    "<table width=%s border=1  frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>\n",search(lcontrol,"aclinfo"),"100%");
	   if(p_oneinfo->acl_count!=0)
	   {
	   	for(j=0;(j<p_oneinfo->acl_count);j++)
	   	{
	   		if(j%2==0)
	   		{
      fprintf(cgiOut,"<tr>\n"\
			        	"<td align='center' style=\"background-color:#f9fafe\">%s:</td>\n",search(lcontrol,"aclindex"));
		fprintf(cgiOut,"<td align='center' style=\"background-color:#f9fafe\">%u</td>\n"\
			      "</tr>\n",p_oneinfo->index[j]);
	   		}
			else
			{
	 fprintf(cgiOut,"<tr>\n"\
			        	"<td align='center' style=\"background-color:#ffffff\">%s:</td>\n",search(lcontrol,"aclindex"));
		fprintf(cgiOut,"<td align='center' style=\"background-color:#ffffff\">%u</td>\n"\
			      "</tr>\n",p_oneinfo->index[j]);
			}
	   	}
	   }
	   else
	   {
      fprintf(cgiOut,"<tr>\n"\
			        	"<td align='center'  style=\"background-color:#f9fafe\">%s</td>\n"\
			      "</tr>\n",search(lcontrol,"noindex"));
	   }
fprintf(cgiOut,"</table></td>\n"\
			  "</tr>\n");
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
free(p_oneinfo);
free(type);
free(type_num);
free(index_num);

release(lpublic);
release(lcontrol);
free(encry);
return 0;
}

