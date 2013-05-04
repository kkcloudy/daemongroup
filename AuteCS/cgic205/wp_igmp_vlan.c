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
* wp_igmp_vlan.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for igmp config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"
#include "util/npd_list.h"
#include "sysdef/npd_sysdef.h"
#include "npd/nam/npd_amapi.h"
#include "ws_ec.h"
#include "ws_public.h"
#include "ws_trunk.h"
#include <sys/types.h>
#include <sys/wait.h>
#include "ws_igmp_snp.h"


int show_vlan_detail();
int check_abc(char *ff);

int cgiMain()
{
	show_vlan_detail();
	return 0;
}

int show_vlan_detail()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 

    char *encry=(char *)malloc(BUF_LEN);				
    char *str;
    int i;
	int cl=1;
	
    char *IDForSearch=(char *)malloc(10);  /*获取vlan的id号*/
    memset(IDForSearch,0,10);
 
    char *IDnew=(char *)malloc(10);
	memset(IDnew,0,10);

	char *IDfixed=(char *)malloc(10);
	memset(IDfixed,0,10);
	
	unsigned short cur_vid;
	char igmp[N];
	memset(igmp,0,N);

	char tag[N];
	memset(tag,0,N);

	char porttype[N];
	memset(porttype,0,N);
	
    int ret;
	
    struct igmp_vlan igmp_head,*aq;
	struct igmp_port route_head,*rq;

	int igmp_num=0;
	int route_num=0;

	int ret_value = -1;
	int retu = -1;

  	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    str=dcryption(encry);
	retu=checkuser_group(str);//0 为管理员  
    if(str==NULL)
    {
      ShowErrorPage(search(lpublic,"ill_user")); /*用户非法*/
      return 0;
	}

  ccgi_dbus_init(); /*初始化dbus*/

  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>","IGMP SNOOPING");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");  
  fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
  fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
  fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");
  fprintf( cgiOut, "</head>"\
		  "<body>");

       
    	
    	
////////////////////////开启对应vlan的igmp功能
////////////// 欠缺条件是对vlan的查看，是否是现有的，只说没有此id就是了

		if(cgiFormSubmitClicked("submit_igmp") == cgiFormSuccess)
		{
			cgiFormStringNoNewlines("igmp",igmp,N); //判断是开启还是关闭该功能
			int ret = -1;
			cgiFormStringNoNewlines("vlanID", IDnew, 10);  //从上文的列表框中获得
			if(strcmp(IDnew,"")==0)
			ShowAlert(search(lcontrol,"igmp_vlan_id"));
			else{
			cur_vid=strtoul(IDnew,0,10);
			if(cur_vid != 1)
    		{
    			ret = config_igmp_snp_npd_vlan_ccgi(cur_vid,igmp);//开启/关闭VLAN的IGMP功能
    
    			switch(ret)
    			{
    				case 4:
    					ShowAlert(search(lcontrol,"igmp_no_sta_g"));//"您还未开启全局IGMP功能"
    					break;
    				case 5:
    					ShowAlert(search(lcontrol,"igmp_no_sta_vlan"));//("您还未开启此VLAN的IGMP功能");
    					break;
    				case 6:
    					ShowAlert(search(lcontrol,"igmp_sta_vlan"));//("您已开启此VLAN的IGMP功能");
    					break;
    				case 0:
    					ShowAlert(search(lpublic,"oper_succ"));
    					break;
    				case -1:
    					ShowAlert(search(lpublic,"oper_fail"));
    				default:
    					ShowAlert(search(lpublic,"oper_fail"));
    
    			}
    		}
			else
			  ShowAlert(search(lpublic,"igmp_vlan_f"));

			
		 }
      }

///////////////////	传参到端口配置页面

		if(cgiFormSubmitClicked("view") == cgiFormSuccess)	 
			{

             cgiFormStringNoNewlines("vlanID", IDfixed, 10);  //获取框中的id号
             cgiFormStringNoNewlines("port",tag,N);
			 
			 if(strcmp(tag,"tag")==0)
			 	strcpy(porttype,"Tag");
			 if(strcmp(tag,"untag")==0)
			 	strcpy(porttype,"Untag");
			 
             if(strcmp(IDfixed,"")==0)
			 	ShowAlert(search(lcontrol,"igmp_vlan_id"));
			 else
    		{
    			 
                 ret=check_abc(IDfixed);
    			 if(ret==0)
        		 {
        		         int xx=strtoul(IDfixed,0,10);
						 if(xx !=1)
    					{
            			     fprintf( cgiOut, "<script type='text/javascript'>\n" );
                             fprintf( cgiOut, "window.location.href='wp_igmp_port.cgi?UN=%s&VID=%s&Tag=%s';\n", encry,IDfixed,porttype);
                             fprintf( cgiOut, "</script>\n" );
    					}
						 else
			               ShowAlert(search(lpublic,"igmp_vlan_f"));
        		  }
				 else
  				 {
                    ShowAlert(search(lcontrol,"igmp_id_str"));
  				 }
    		}
	    }
	
	//end
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>IGMP SNOOPING</font></td>");
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

	     fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry); //鉴权 
	     

	   
	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td width=62 align=center><input id=but type=submit name=submit_vlandetail style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
        fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
					     "<td align=left id=tdleft><a href=wp_config_igmp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san></font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"igmp_time_para"));
					    fprintf(cgiOut,"</tr>");
            		 
					  fprintf(cgiOut,"<tr height=26>"\
							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"igmp_conf"));
							fprintf(cgiOut,"</tr>");
                       
					    /*
						//新增条目
					    fprintf(cgiOut,"<tr height=25>"\
					     "<td align=left id=tdleft><a href=wp_igmp_view.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),"IGMP信息");
					    fprintf(cgiOut,"</tr>");
						*/

					if((cgiFormSubmitClicked("igmp_view") == cgiFormSuccess)|| (cgiFormSubmitClicked("conf_info") == cgiFormSuccess))
						{
						for(i=0;i<19;i++)
						{
							fprintf(cgiOut,"<tr height=25>"\
							  "<td id=tdleft>&nbsp;</td>"\
							"</tr>");
						}
						}
					   else
					   	{
                       for(i=0;i<10;i++)
						{
							fprintf(cgiOut,"<tr height=25>"\
							  "<td id=tdleft>&nbsp;</td>"\
							"</tr>");
						}

					   	}
					
				
					
				  fprintf(cgiOut,"</table>"\
              "</td>");

	fprintf(cgiOut, "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=600 height=295 border=0 cellspacing=0 cellpadding=0>");
 			
	    fprintf(cgiOut,"<tr>"\
		"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lcontrol,"igmp_des"));
		fprintf(cgiOut,"</tr>");

		fprintf(cgiOut,"<tr>");
	    fprintf(cgiOut,"<td align=center valign=top  style=padding-top:10px>");
		fprintf(cgiOut,"<table align=left width=400 border=0 cellspacing=0 cellpadding=0>");

	    fprintf(cgiOut,"<tr>\n");
		fprintf(cgiOut,"<td>%s\n","VLAN ID :");								
		fprintf(cgiOut,"<input type=text name=vlanID value=\"\" size=11 maxLength=4 />");								
		fprintf(cgiOut,"</td>\n");
		fprintf(cgiOut,"</tr>\n");

		fprintf(cgiOut,"<tr height=10><td></td></tr>\n");  //间隔开来

		fprintf(cgiOut,"<tr>");
		fprintf(cgiOut,"<td>%s:  \n",search(lcontrol,"igmp_port"));
		if(retu == 0)
			fprintf(cgiOut,"<select name=port style=width:80px>\n");
		else
			fprintf(cgiOut,"<select name=port disabled style=width:80px>\n");
		fprintf(cgiOut,"<option value=tag>Tag</option>\n");
		fprintf(cgiOut,"<option value=untag>Untag</option>\n");
		fprintf(cgiOut,"</select>");
		fprintf(cgiOut,"</td>");
		fprintf(cgiOut,"<td>\n");
		
		if(retu == 0)
			fprintf(cgiOut,"<input type=submit name=view value=\"%s\">",search(lcontrol,"port_configure"));
		else
			fprintf(cgiOut,"<input type=submit name=view disabled value=\"%s\">",search(lcontrol,"port_configure"));

		fprintf(cgiOut,"</td>\n");
		fprintf(cgiOut,"</tr>\n");

		
											
							
	    fprintf(cgiOut,"<tr height=30 style=padding-top:20px>");
		fprintf(cgiOut,"<td>%s","IGMP :&nbsp;&nbsp;&nbsp;&nbsp;");

		if(retu == 0)
			{
				fprintf(cgiOut,"<select name=igmp style=width:80px><option value=enable>start<option value=disable>stop</select></td><td>"\
				"<input type=submit style= height:22px  border=0 name=submit_igmp style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"start_stop"));		
			}
		else
			{
				fprintf(cgiOut,"<select name=igmp style=width:80px disabled><option value=enable>start<option value=disable>stop</select></td><td>"\
				"<input type=submit style= height:22px  disabled border=0 name=submit_igmp style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"start_stop"));		
			}
		fprintf(cgiOut,"</tr>");

        fprintf(cgiOut,"</table>\n");
		fprintf(cgiOut,"</td>\n");
		fprintf(cgiOut,"</tr>\n");
							 
						
							   
///////////////////////////////////////////////////////////
///显示组播组的详细信息  2009-03-03 zhouyanmei
///////////////////////////////////////////////////////////
							
						 
	
		fprintf(cgiOut,"<tr>"\
	  "<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">"\
	  "<input type=submit name=igmp_view value=\"%s\">",search(lcontrol,"igmp_view"));
      fprintf( cgiOut,"&nbsp;&nbsp;&nbsp;\n");
	  //fprintf( cgiOut,"<input type=submit name=conf_info value=\"%s\">",search(lcontrol,"igmp_view_conf"));
	  fprintf( cgiOut,"</td>\n");
	  fprintf(cgiOut,"</tr>");
	 
	if(cgiFormSubmitClicked("igmp_view") == cgiFormSuccess)
		{
	  fprintf(cgiOut,"<tr>"\
		"<td align=center valign=top  style=padding-top:10px>"\
			"<table align=left width=400 border=0 cellspacing=0 cellpadding=0>");
			int vlan_id;
			
			cgiFormStringNoNewlines("vlanID", IDfixed, 10);  //获取框中的id号								
            if(strcmp(IDfixed,"")==0)
		 	{
		 	   ShowAlert(search(lcontrol,"igmp_vlan_id"));
            }
		    else
			{
               ret=check_abc(IDfixed);
		       if(ret==0)
		       	{
			   	vlan_id = (int)strtoul(IDfixed,0,10);				
				//ret_value = dcli_show_igmp_snp_mcgroup_list((unsigned short)vlan_id);//显示组播组信息

				/////////新的存储链表的函数
				
				ret_value = dcli_show_igmp_snp_mcgroup_list(vlan_id,&igmp_head,&igmp_num);
                

				if(ret_value==0)
				{ 
				    fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px>"\
					"<th  style=font-size:12px>%s</th>","VLAN ID");
					fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VIDX");
					fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","GROUP_IP");
					//fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","PORT MEMBER LIST");
					fprintf(cgiOut,"</tr>");
         			for(aq=igmp_head.next;aq!=NULL;aq=aq->next)
         				{
                           
   			                fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));
               				fprintf(cgiOut,"<td style=font-size:12px align=center>%d</td>",vlan_id);
               				fprintf(cgiOut, "<td style=font-size:12px align=center>%d</td>",aq->vidx);
               				fprintf(cgiOut, "<td style=font-size:12px align=center>%-3u.%-3u.%-3u.%-3u</td>",aq->ip0,aq->ip1,aq->ip2,aq->ip3);
               			    fprintf(cgiOut,"</tr>");
               
               			    cl = !cl;
				      }

				}
			else{
				
			switch(ret_value)
			{
				case NPD_GROUP_NOTEXIST:
					fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcontrol,"group_no_exist"));
					break;
				case NPD_VLAN_ERR_HW:
					fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcontrol,"HW_error"));
					break;
				default :
					fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcontrol,"show_err"));
					break;

			}
				}
			
				ret_value = show_igmp_snp_mcgroup_router_port(vlan_id,&route_head,&route_num);
				
				if(ret_value==0)
				{
				    fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                 	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN");
                 	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TYPE");
                 	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","PORT");
                 	fprintf(cgiOut,"</tr>");
					
                    ////////////////////// 分开区别
					for(rq=route_head.next;rq!=NULL;rq=rq->next)
					{
		            		fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));
						    fprintf(cgiOut,"<td align = left>%s :%d</td>","VLAN ID",vlan_id);
							if(rq->un_slot != 0)
							{
							fprintf(cgiOut,"<td  align = left>%s  --  %s</td>","ROUTE_PORT","untag");
							fprintf(cgiOut, "<td align = left>%d/%d</td>",rq->un_slot,rq->un_port);
							}
							if(rq->slot != 0)
							{
							fprintf(cgiOut,"<td  align = left>%s  --  %s</td>","ROUTE_PORT","tag");
							fprintf(cgiOut, "<td align = left>%d/%d</td>",rq->slot,rq->port);
							}
							fprintf(cgiOut,"</tr>");
                    cl = !cl;
					}
					/////////////////////

					
				}
				else
				{
         			switch(ret_value)
         			{
         				case 1:  //未开启全局的igmp功能
         					fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcontrol,"igmp_no_sta_g"));
         					break;
         				case 3:  //没有路由端口
         					fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcontrol,"no_route_port"));
         					break;
         				case 4: //未开启vlan的igmp功能
         					fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcontrol,"igmp_no_sta_vlan"));
         					break;
         				case 5:
         					fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcontrol,"vlan_id_illegal"));
         					break;
         				case 6:
         					fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcontrol,"s_arp_vlan_notexist"));
         					break;
         				case -1:
         					fprintf(cgiOut,"<tr><td><font color=red>%s</font></td></tr>",search(lcontrol,"show_err"));
         					break;
         
         			}
		       	}
		  }
		else{
			ShowAlert(search(lcontrol,"igmp_id_str"));
			}
		 	}
		fprintf(cgiOut,"</table>");
 			fprintf(cgiOut,"</td>"\
 		  "</tr>");
     }	

//////////////////////////////////////
///查看组播组的配置信息 2009-03-05
///
//////////////////////////////////////
/*
                             if(cgiFormSubmitClicked("conf_info") == cgiFormSuccess)
                         		{
                         	           fprintf(cgiOut,"<tr>"\
                         		          "<td align=center valign=top  style=padding-top:10px>"\
                         			         "<table align=left width=400 border=0 cellspacing=0 cellpadding=0>");
                         
                         		       //ret_value = show_igmp_vlan_list(&sum_head,&sum_num,&port_num);
									   sq=sum_head.next;
									   
                         		       if(ret_value==0)
                         		       	{
                                  	     for(i=0;i<sum_num;i++)
                                  			 {
                                              		fprintf(cgiOut,"<tr align=left>"\
                                  									"<td  align = left style=font-size:13px >%s</td>","vlanID");
                                  					fprintf(cgiOut, "<td align = left style=font-size:13px >%d</td>",sq->vlanId);
                                  					fprintf(cgiOut,"</tr>");
                                  
                                  			 }
                         		       	}
                              }	
							 */

				fprintf(cgiOut,"<tr><td>\n");
				fprintf(cgiOut,"<input type=hidden name=old_port value=\"%s\">",IDForSearch);
				fprintf(cgiOut,"</td></tr>\n");
					
				fprintf(cgiOut,"</table>"\

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
free(IDnew);
free(IDForSearch);
free(IDfixed);
if((ret_value==0)&&(igmp_num>0))
  Free_igmp_head(&igmp_head);

if((ret_value==0)&&(route_num>0))
  Free_igmp_port(&route_head);

//if((ret_value==0)&&(sum_num>0))
//  Free_igmp_sum(&sum_head);


release(lpublic);  
release(lcontrol);

return 0;
}

