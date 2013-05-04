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
* wp_qosmapinfo.c
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
#include "ws_init_dbus.h"

#include "ws_dcli_qos.h"


int ShowMapDetailPage(); 
int addvlan_hand(struct list *lpublic); 

int cgiMain()
{
 ShowMapDetailPage();
 return 0;
}

int ShowMapDetailPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	int retz;
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str=NULL;
	char * index=(char *)malloc(10);
	memset(index,0,10);

	char * indexself=(char *)malloc(10);
	memset(indexself,0,10);
	
	unsigned int indextemp=0;
	char *endptr = NULL;  
	int i;
	int map_num=0,up_num=0,dscp_num=0,dscpself_num=0;
	char menu[21]="menulist";
  	char* i_char=(char *)malloc(10);
  	
  	char* DelMap=(char *)malloc(10);
	memset(DelMap,0,10);
	
	char* maptype=(char *)malloc(20);
	memset(maptype,0,20);
	
	char qosmapdta_encry[BUF_LEN];  
	struct mapping_info receive_map;
	int retu=0;

	char* CheckUsr=(char *)malloc(10);
	memset(CheckUsr,0,10);

	ccgi_dbus_init();
	if(cgiFormSubmitClicked("submit_qosmapdta") != cgiFormSuccess)
	{
	  memset(encry,0,BUF_LEN);
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
	  }
	  memset(qosmapdta_encry,0,BUF_LEN);					 /*清空临时变量*/
	}

  cgiFormStringNoNewlines("encry_qosmapdta",qosmapdta_encry,BUF_LEN);
  cgiFormStringNoNewlines("INDEX",index,10);
  cgiFormStringNoNewlines("CheckUsr",CheckUsr,10);

  if(strcmp(CheckUsr,"")!=0)
  	retu=atoi(CheckUsr);
  indextemp=strtoul(index,&endptr,10);
   cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "<style type=text/css>"\
  	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
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
  if(cgiFormSubmitClicked("submit_qosmapdta") != cgiFormSuccess)
  {
  	 retu=checkuser_group(str);
  }
  if(cgiFormSubmitClicked("submit_qosmapdta") == cgiFormSuccess)
  {
	  fprintf(cgiOut,"<script type='text/javascript'>\n");
	  fprintf(cgiOut,"window.location.href='wp_qosmap.cgi?UN=%s';\n",qosmapdta_encry);
	  fprintf(cgiOut,"</script>\n");
  }
  cgiFormStringNoNewlines("DELRULE", DelMap, 10);
  cgiFormStringNoNewlines("MAPTYPE",maptype, 20);
  cgiFormStringNoNewlines("INDEXSELF",indexself, 20);
  if(strcmp(DelMap,"delete")==0)
  {	
  	if(strcmp(maptype,"up-to-profile")==0)
  	{
  		retz=del_up_to_profile(indexself,lcontrol);
		switch(retz)
		{
			case 0:
				ShowAlert(search(lpublic,"oper_succ"));
				break;
			case -3:
				ShowAlert(search(lcontrol,"del_map_up_qos_fail"));   
				break;
			default:
				ShowAlert(search(lpublic,"oper_fail"));
				break;
		}
  	}
  	else if(strcmp(maptype,"dscp-to-profile")==0)
  	{
  		retz=del_dscp_to_profile(indexself,lcontrol);
		switch(retz)
		{
			case 0:
				ShowAlert(search(lpublic,"oper_succ"));
				break;
			case -3:
				ShowAlert(search(lcontrol,"del_map_dscp_qos_fail"));
				break;
			default:
				ShowAlert(search(lpublic,"oper_fail"));
				break;				
		}
  	}
  	else if(strcmp(maptype,"dscp-to-dscp")==0)
	{
		retz=del_dscp_to_dscp(indexself,lcontrol);
		switch(retz)
		{
			case 0:
				ShowAlert(search(lpublic,"oper_succ"));
				break;
			case -3:
				ShowAlert(search(lcontrol,"del_map_dscp_dscp_fail"));
				break;
			default:
				ShowAlert(search(lpublic,"oper_fail"));
				break;
		}
	}
  }
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>QOS</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	#if 0
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
	    if(strcmp(lan,"ch")==0)
    	{	
    	#endif
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
          //"<td width=62 align=center><input id=but type=submit name=submit_qosmapdta style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_qosmapdta") != cgiFormSuccess)
		  {
		  	fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
            		fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  }
		  else
		  {
     			fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",qosmapdta_encry,search(lpublic,"img_ok"));
     			fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",qosmapdta_encry,search(lpublic,"img_cancel"));
		  }
		  fprintf(cgiOut,"</tr>"\
          "</table>");
		  #if 0
		}		
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_qosmapdta style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		  if(cgiFormSubmitClicked("submit_qosmapdta") != cgiFormSuccess)
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  else
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosmap.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",qosmapdta_encry);
		  fprintf(cgiOut,"</tr>"\
		  "</table>");
		}
		#endif
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
            		if(cgiFormSubmitClicked("submit_qosmapdta") != cgiFormSuccess)
            		{
            			if(retu==0)
            			{
					fprintf(cgiOut,"<tr height=25>"\
                 			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>QOS </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"list"));					   
                 			fprintf(cgiOut,"</tr>");
							
    					fprintf(cgiOut,"<tr height=25>"\
                 			"<td align=left id=tdleft><a href=wp_addqos.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>QOS Profile</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add"));					   
                 			fprintf(cgiOut,"</tr>");
                 			fprintf(cgiOut,"<tr height=25>"\
                 			"<td align=left id=tdleft><a href=wp_addmap.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_map"));					   
                 			fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=26>"\
         				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>QOS </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"map_detail"));   /*突出显示*/
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
                 			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"list"));					   
                 			fprintf(cgiOut,"</tr>");
				
					fprintf(cgiOut,"<tr height=26>"\
         				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>QOS </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"map_detail"));   /*突出显示*/
         				fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
                 			"<td align=left id=tdleft><a href=wp_policymaplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"policy_map"));					   
                 			fprintf(cgiOut,"</tr>");
				}

            		}
            		else if(cgiFormSubmitClicked("submit_qosmapdta") == cgiFormSuccess)				
            		{
            			if(retu==0)
            			{
					fprintf(cgiOut,"<tr height=25>"\
                			"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame class=top style=color:#000000><font id=%s>QOS %s</font><font id=yingwen_san>QOS Profile</font></a></td>",qosmapdta_encry,search(lpublic,"menu_san"),search(lcontrol,"list"));						
                			fprintf(cgiOut,"</tr>");
						
    					fprintf(cgiOut,"<tr height=25>"\
                			"<td align=left id=tdleft><a href=wp_addqos.cgi?UN=%s target=mainFrame class=top style=color:#000000><font id=%s>%s</font><font id=yingwen_san>QOS Profile</font></a></td>",qosmapdta_encry,search(lpublic,"menu_san"),search(lcontrol,"add"));						
                			fprintf(cgiOut,"</tr>");
                			fprintf(cgiOut,"<tr height=25>"\
                			"<td align=left id=tdleft><a href=wp_addmap.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",qosmapdta_encry,search(lpublic,"menu_san"),search(lcontrol,"add_map"));					   
                			fprintf(cgiOut,"</tr>");

					fprintf(cgiOut,"<tr height=26>"\
         				"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>QOS </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"map_detail"));   /*突出显示*/
         				fprintf(cgiOut,"</tr>");
							
                			fprintf(cgiOut,"<tr height=25>"\
                			"<td align=left id=tdleft><a href=wp_policymaplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",qosmapdta_encry,search(lpublic,"menu_san"),search(lcontrol,"policy_map"));					   
                			fprintf(cgiOut,"</tr>"\
                			"<tr height=25>"\
                   			"<td align=left id=tdleft><a href=wp_createpolicy.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",qosmapdta_encry,search(lpublic,"menu_san"),search(lcontrol,"add_policy"));
                   			fprintf(cgiOut,"</tr>");
               			}
               			else
               			{
						fprintf(cgiOut,"<tr height=25>"\
                				"<td align=left id=tdleft><a href=wp_qosmap.cgi?UN=%s target=mainFrame class=top><font id=%s>QOS %s</font></a></td>",qosmapdta_encry,search(lpublic,"menu_san"),search(lcontrol,"list"));					   
                				fprintf(cgiOut,"</tr>");
						

						fprintf(cgiOut,"<tr height=26>"\
         					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>QOS </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"map_detail"));   /*突出显示*/
         					fprintf(cgiOut,"</tr>");
						
               				fprintf(cgiOut,"<tr height=25>"\
                				"<td align=left id=tdleft><a href=wp_policymaplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",qosmapdta_encry,search(lpublic,"menu_san"),search(lcontrol,"policy_map"));					   
                				fprintf(cgiOut,"</tr>");
               			}

            		}

            		int rowsCount=0;
            		if(retu==0)
            			rowsCount=4;
            		else
            			rowsCount=7;
					for(i=0;i<rowsCount;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>");
							map_num=0;
							receive_map=show_remap_table_byindex(&map_num,&up_num,&dscp_num,&dscpself_num,lcontrol);
							fprintf(cgiOut,"<table frame=below rules=rows style=overflow:auto width=320 border=1>");
     					   	fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 style=font-size:14px  id=td1>"\
							 "<th width=120 style=font-size:12px>%s</th>",search(lcontrol,"Map_Type"));
							 fprintf(cgiOut,"<th width=80 style=font-size:12px>%s</th>",search(lcontrol,"Map_src"));
							 fprintf(cgiOut,"<th width=100 style=font-size:12px>%s</th>",search(lcontrol,"Map_dst"));
							 if(retu==0)
							 {
							 	fprintf(cgiOut,"<th width=13 style=font-size:12px>&nbsp;</th>");
							 }
							 fprintf(cgiOut,"</tr>");

							if(up_num!=0)
							{
    							 for(i=0;i<up_num;i++)
    							 {
    							 	memset(menu,0,21);
									strcpy(menu,"menulist");
									sprintf(i_char,"%d",i+1);
									strcat(menu,i_char);
              					   	fprintf(cgiOut,"<tr align=left>"\
                      					   "<td id=td1 align=center>%s</td>","up-to-profile");
                      					   	fprintf(cgiOut,"<td id=td2 align=center>%d</td>",receive_map.flag[i]);
                      					   	fprintf(cgiOut,"<td id=td2 align=center>%d</td>",receive_map.profileindex[i]);
                      					   	if(retu==0)
                      					   	{
                          					   	fprintf(cgiOut,"<td align=center>");
        										fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(up_num+dscp_num+dscpself_num-i),menu,menu);
        																   fprintf(cgiOut,"<img src=/images/detail.gif>"\
        																   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
        																   fprintf(cgiOut,"<div id=div1>");
        																   	if(cgiFormSubmitClicked("submit_qosmapdta") != cgiFormSuccess)
        																   	{
        																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosmapinfo.cgi?UN=%s&INDEX=%s&INDEXSELF=%u&DELRULE=%s&MAPTYPE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,index,receive_map.flag[i],"delete","up-to-profile",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
        																   	}
        																   	else if(cgiFormSubmitClicked("submit_qosmapdta") == cgiFormSuccess)
        																   	{
        																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosmapinfo.cgi?UN=%s&INDEX=%s&INDEXSELF=%u&DELRULE=%s&MAPTYPE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",qosmapdta_encry,index,receive_map.flag[i],"delete","up-to-profile",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
        																   	}
        																   fprintf(cgiOut,"</div>"\
        																   "</div>"\
        																   "</div>");
        										 	fprintf(cgiOut,"</td>");
    										 }
    										 //else
    										 //fprintf(cgiOut,"<td>&nbsp;</td>");
                      					   	fprintf(cgiOut,"</tr>");
                 				}
             				}
             				if(dscp_num!=0)
							{
    							 for(i=0;i<dscp_num;i++)
    							 {
    							 	memset(menu,0,21);
									strcpy(menu,"menulist");
									sprintf(i_char,"%d",up_num+i+1);
									strcat(menu,i_char);
    							 
              					   	fprintf(cgiOut,"<tr align=left>"\
                      					   "<td id=td1 align=center>%s</td>","dscp-to-profile");
                      					   	fprintf(cgiOut,"<td id=td2 align=center>%d</td>",receive_map.flag[up_num+i]);
                      					   	fprintf(cgiOut,"<td id=td2 align=center>%d</td>",receive_map.profileindex[up_num+i]);
									if(retu==0)
									{
								 			fprintf(cgiOut,"<td align=center>");
    										fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(dscp_num+dscpself_num-i),menu,menu);
    																   fprintf(cgiOut,"<img src=/images/detail.gif>"\
    																   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
    																   fprintf(cgiOut,"<div id=div1>");
    																   	if(cgiFormSubmitClicked("submit_qosmapdta") != cgiFormSuccess)
    																   	{
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosmapinfo.cgi?UN=%s&INDEX=%s&INDEXSELF=%u&DELRULE=%s&MAPTYPE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,index,receive_map.flag[up_num+i],"delete","dscp-to-profile",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
    																   	}
    																   	else if(cgiFormSubmitClicked("submit_qosmapdta") == cgiFormSuccess)
    																   	{
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosmapinfo.cgi?UN=%s&INDEX=%s&INDEXSELF=%u&DELRULE=%s&MAPTYPE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",qosmapdta_encry,index,receive_map.flag[up_num+i],"delete","dscp-to-profile",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
    																   	}
    																   fprintf(cgiOut,"</div>"\
    																   "</div>"\
    																   "</div>");
    										 	fprintf(cgiOut,"</td>");
									}
                      					   	fprintf(cgiOut,"</tr>");
                 				}
             				}
             				if(dscpself_num!=0)
							{
    							 for(i=0;i<dscpself_num;i++)
    							 {
    							 	memset(menu,0,21);
									strcpy(menu,"menulist");
									sprintf(i_char,"%d",up_num+dscp_num+i+1);
									strcat(menu,i_char);
              					   	fprintf(cgiOut,"<tr align=left>"\
                      					   "<td id=td1 align=center>%s</td>","dscp-to-dscp");
                      					   	fprintf(cgiOut,"<td id=td2 align=center>%d</td>",receive_map.flag[up_num+dscp_num+i]);
                      					   	fprintf(cgiOut,"<td id=td2 align=center>%d</td>",receive_map.profileindex[up_num+dscp_num+i]);
									if(retu==0)
									{
                      					  	fprintf(cgiOut,"<td align=center>");
    										fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(dscpself_num-i),menu,menu);
    																   fprintf(cgiOut,"<img src=/images/detail.gif>"\
    																   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
    																   fprintf(cgiOut,"<div id=div1>");
    																   	if(cgiFormSubmitClicked("submit_qosmapdta") != cgiFormSuccess)
    																   	{
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosmapinfo.cgi?UN=%s&INDEX=%s&INDEXSELF=%u&DELRULE=%s&MAPTYPE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,index,receive_map.flag[up_num+dscp_num+i],"delete","dscp-to-dscp",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
    																   	}
    																   	else if(cgiFormSubmitClicked("submit_qosmapdta") == cgiFormSuccess)
    																   	{
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_qosmapinfo.cgi?UN=%s&INDEX=%s&INDEXSELF=%u&DELRULE=%s&MAPTYPE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",qosmapdta_encry,index,receive_map.flag[up_num+dscp_num+i],"delete","dscp-to-dscp",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
    																   	}
    																   fprintf(cgiOut,"</div>"\
    																   "</div>"\
    																   "</div>");
    										 	fprintf(cgiOut,"</td>");
									}
                      					   	fprintf(cgiOut,"</tr>");
                 				}
             				}

					  fprintf(cgiOut,"</table>"\

						  "</td>"\
						"</tr>"\
					"<tr>");
					if(cgiFormSubmitClicked("submit_qosmapdta") != cgiFormSuccess)
					  {
						fprintf(cgiOut,"<td><input type=hidden name=encry_qosmapdta value=%s></td>",encry);
						fprintf(cgiOut,"<td><input type=hidden name=INDEX value=%s></td>",index);
						fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
					  }
					else if(cgiFormSubmitClicked("submit_qosmapdta") == cgiFormSuccess)
					  {
						fprintf(cgiOut,"<td><input type=hidden name=encry_qosmapdta value=%s></td>",qosmapdta_encry);
						fprintf(cgiOut,"<td><input type=hidden name=INDEX value=%s></td>",index);
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
free(receive_map.mapping_des);
free(i_char);
free(DelMap);
free(maptype);
free(index);
free(indexself);
release(lpublic);  
release(lcontrol);
return 0;
}

