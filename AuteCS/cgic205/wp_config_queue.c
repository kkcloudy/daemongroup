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
* wp_config_queue.c
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
#include "ws_sysinfo.h"
#include "ws_dcli_qos.h"
#define AMOUNT 512


char * Queue_Scheduler[] = {
		"sp",
		"wrr",
		"sp+wrr"
};


int ShowQueryPage();

int cgiMain()
{
 ShowQueryPage();
 return 0;
}

int ShowQueryPage()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    	lcontrol=get_chain_head("../htdocs/text/control.txt"); 	
  	char *encry=(char *)malloc(BUF_LEN);
  	char *str;
  //FILE *fp;
  //char lan[3];
	  char policer_encry[BUF_LEN]; 
	  char addn[N];        
	  char * mode=(char *)malloc(AMOUNT);
	  memset(mode,0,AMOUNT);
	  int i=0;   
	  int cl=1;
  	int retu=0;
  	int select_flag=0;
  	//char menu[21]="menulist";
  	char* i_char=(char *)malloc(10);
  	char * group=(char * )malloc(10);
  	memset(group,0,10);
  	char * QueryID=(char * )malloc(10);
  	memset(QueryID,0,10);
  	char * wrr_weight=(char * )malloc(10);
  	memset(wrr_weight,0,10);
  	int qos_num=0;
  	char * queue_mode=(char * )malloc(10);
  	memset(queue_mode,0,10);
	char * radiobutton=(char *)malloc(10);
	memset(radiobutton,0,10);
	cgiFormString("radiobutton", radiobutton, 10);

  	struct qos_info receive_qos[MAX_QOS_PROFILE];
  	struct query_info query_info[8];
  	int flag[8];
  	for(i=0;i<8;i++)
    	{
    		flag[i]=0;
    	}
	
  	for(i=0;i<MAX_QOS_PROFILE;i++)
	{
		receive_qos[i].profileindex=0;
		receive_qos[i].dp=0;
		receive_qos[i].up=0;
		receive_qos[i].tc=10;
		receive_qos[i].dscp=0;
	}
  	ccgi_dbus_init();
  	memset(encry,0,BUF_LEN);
  	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  	{
    		str=dcryption(encry);
    		if(str==NULL)
    		{
	      		ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	      		return 0;
		}
		strcpy(addn,str);
		memset(policer_encry,0,BUF_LEN);                   /*清空临时变量*/
  	}
  	else
  	{
	     cgiFormStringNoNewlines("encry_configqueue",encry,BUF_LEN);
	     cgiFormStringNoNewlines("group",group,10);
	     cgiFormStringNoNewlines("QueryID",QueryID,10);
	     cgiFormStringNoNewlines("wrr_weight",wrr_weight,10);
	     cgiFormStringNoNewlines("queue_mode",queue_mode,10);
	     cgiFormStringNoNewlines("CheckUsr",addn,10);
	}

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"route_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	  "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	  "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
	  "#link{ text-decoration:none; font-size: 12px}"\
  	".ShowPolicy {overflow-x:hidden;  overflow:auto; width: 580px; height: 236px; clip: rect( ); padding-top: 2px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
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

 
  	 retu=checkuser_group(addn);

   	if(cgiFormSubmitClicked("submit_config_query") == cgiFormSuccess)
  	{
  		int ret=0;
  		if(strcmp(queue_mode,"sp")==0)
  		{
  			set_queue_scheduler(queue_mode);
  		}
  		else if(strcmp(queue_mode,"wrr")==0)
  		{
  			if(strcmp(wrr_weight,"")!=0)
  			{
     				set_queue_scheduler(queue_mode);
				if(strcmp(QueryID,"")!=0)
				{
	     			ret=set_wrr_queue(group,QueryID,wrr_weight,1);	     				
	     				
					if(ret==0)
					{
						ShowAlert(search(lcontrol,"config_queue_succ"));
					}
					else 
					{
						ShowAlert(search(lcontrol,"counterconfigfail"));
					}
				}
				else
				{
					ShowAlert(search(lcontrol,"pls_create_profile"));
				}
  			}
  			else
  			{
				ShowAlert(search(lcontrol,"weight_not_null"));
  			}
  				
  		}
  		else if(strcmp(queue_mode,"sp+wrr")==0)
  		{
  			char * temp=(char * )malloc(10);
  			memset(temp,0,10);
  			strcpy(temp,"hybrid");
			if(strcmp(radiobutton,"val")==0)
			{
	  			if(strcmp(wrr_weight,"")!=0)
	  			{
	     				set_queue_scheduler(temp);
					if(strcmp(QueryID,"")!=0)
					{
		   				ret=set_wrr_queue(group,QueryID,wrr_weight,2);		   				
						if(ret==0)
						{
							ShowAlert(search(lcontrol,"config_queue_succ"));
						}
						else 
						{
							ShowAlert(search(lcontrol,"counterconfigfail"));
						}
					}
					else
					{
						ShowAlert(search(lcontrol,"pls_create_profile"));
					}
				}
				else
				{
					ShowAlert(search(lcontrol,"weight_not_null"));
				}	
			}
			else if(strcmp(radiobutton,"sp")==0)
			{
				set_queue_scheduler(temp);
				if(strcmp(QueryID,"")!=0)
				{
					ret=set_wrr_queue(group,QueryID,"sp",2);					
				    if(ret==0)
					{
						ShowAlert(search(lcontrol,"config_queue_succ"));
					}
					else 
					{
						ShowAlert(search(lcontrol,"counterconfigfail"));
					}

				}
				else
				{
					ShowAlert(search(lcontrol,"pls_create_profile"));
				}
			}
			else
			{
				ShowAlert(search(lcontrol,"select_sp_weight"));
			}
  			free(temp);	
  		}
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
          "<td width=62 align=center><input id=but type=submit name=submit_config_query style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
          
		  if(cgiFormSubmitClicked("submit_config_query") != cgiFormSuccess)
		  {
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosModule.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
           
            }
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosModule.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
     			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"config_queue_list"));   /*突出显示*/
     			fprintf(cgiOut,"</tr>");
				int rowsCount=0;
				if(retu==0)
					rowsCount=16;
				else rowsCount=12;
				for(i=0;i<rowsCount+2;i++)
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}
				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
						"<table width=640 height=310 border=0 cellspacing=0 cellpadding=0>");
				  fprintf(cgiOut,"<tr height='35'>\n"\
				  				"<td style=\"font-size:14px\"><font color='red'><b>%s</b></font></td>"\
				  			  "</tr>\n",search(lcontrol,mode));
						if(retu==0)
						{
    						fprintf(cgiOut,"<tr>"\
    						"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"config_query_mode"));
    						fprintf(cgiOut,"</tr>"\
    						"<tr>"\
    						"<td><table frame=below rules=rows width=440 height=70 border=1>");
    						show_qos_profile(receive_qos,&qos_num,lcontrol);
    						for(i=0;i<qos_num;i++)
    						{
    							if(receive_qos[i].tc!=10)
    							{
    								flag[receive_qos[i].tc]=1;
								select_flag++;
    							}
    						}
    						fprintf(cgiOut,"<tr height=25 align=left  style=padding-top:8px>");
    						    fprintf(cgiOut,"<td id=td1 width=110>%s:</td>",search(lcontrol,"queue_mode"));
    						    fprintf(cgiOut,"<td  align=left style=padding-left:10px colspan=7><select name=queue_mode onchange=\"javascript:this.form.submit();\">");
    						    for(i=0;i<((get_product_id()==PRODUCT_ID_AU3K_BCM)?2:3);i++)
    							if(strcmp(Queue_Scheduler[i],queue_mode)==0)				/*显示上次选中的*/
    								fprintf(cgiOut,"<option value=%s selected=selected>%s",Queue_Scheduler[i],Queue_Scheduler[i]);
    							else				
    								fprintf(cgiOut,"<option value=%s>%s",Queue_Scheduler[i],Queue_Scheduler[i]);
    						    fprintf(cgiOut,"</select>");
    						    fprintf(cgiOut,"</td>");
    						    int Queue_Schedulerchoice=0;
    							cgiFormSelectSingle("queue_mode", Queue_Scheduler, 3, &Queue_Schedulerchoice, 0);
    						   	fprintf(cgiOut,"</tr>");
    							if(Queue_Schedulerchoice==2 || Queue_Schedulerchoice==1)
    							{
    								fprintf(cgiOut,"<tr height=25 align=left  style=padding-top:8px>");
    								fprintf(cgiOut,"<td  align=left style=padding-left:10px colspan=3>");
								if(get_product_id()!=PRODUCT_ID_AU3K_BCM)
								{
									fprintf(cgiOut,"<select name=group>");
        						    			fprintf(cgiOut,"<option value=%s>%s","group1","group1");
        						    			fprintf(cgiOut,"<option value=%s>%s","group2","group2");
        						    		fprintf(cgiOut,"</select>");
								}
								else
								{
									fprintf(cgiOut,"<input type=hidden name=group value=group1><b>group1</b>");
								}
        						    	fprintf(cgiOut,"</td>");
        						    fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"queue_TC"));
    								fprintf(cgiOut,"<td  align=left style=padding-left:10px colspan=3>\n");
								if(select_flag!=0)
								{
									fprintf(cgiOut,"<select name=QueryID>");
    									for(i=0;i<8;i++)
    									{
    										if(flag[i]==1)
    										{
    											fprintf(cgiOut,"<option value=%d>%d",i,i);
    										}
    									}
        						    		fprintf(cgiOut,"</select>");
								}
								else
								{
									fprintf(cgiOut,"<font color='red'>%s</font>",search(lcontrol,"not_create_qos_profile"));
								}
        						    fprintf(cgiOut,"</td>");
//***************************************************changed here**********************************************************
								if(Queue_Schedulerchoice==1)
								{
	    								fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"weight"));
	    								fprintf(cgiOut,"<td><input type=text name=wrr_weight size=8></td>");
								}
								else if(Queue_Schedulerchoice==2)
								{
	    								fprintf(cgiOut,"<td id=td1><input type=radio name=radiobutton value='val' checked />%s:</td>",search(lcontrol,"weight"));
	    								fprintf(cgiOut,"<td><input type=text name=wrr_weight size=8></td>");
									fprintf(cgiOut,"<td id=td1><input type=radio name=radiobutton value='sp' />sp</td>");
								}
//**********************************************************************************************************************************************************
    								fprintf(cgiOut,"</tr>");
    							}
    						fprintf(cgiOut,"</table>"\
    						"</td>"\
    						"</tr>");
						}
						fprintf(cgiOut,"<tr style=padding-top:18px>"\
						"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"config_queue_list"));
						fprintf(cgiOut,"</tr>"\
						 "<tr>"\
						   "<td align=left valign=top style=padding-top:18px>"\
						   "<div class=ShowPolicy><table width=320 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
						   fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 style=font-size:14px  id=td1  align=left>"\
							 "<th width=90><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"queue_TC"));
							 fprintf(cgiOut,"<th width=120><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"Scheduling_Group"));
							 fprintf(cgiOut,"<th width=70><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"weight"));
							 fprintf(cgiOut,"</tr>");
							 for(i=0;i<8;i++)
			                              {
			                              	flag[i]=0;
			                              	query_info[i].QID=0;
			                              	query_info[i].Scheduling_group=(char * )malloc(10);
			                              	memset(query_info[i].Scheduling_group,0,10);
			                              	query_info[i].weight=0;
			                              }
							show_queue(query_info);
							for(i=0;i<8;i++)
							{
								/*memset(menu,0,21);
							  	strcpy(menu,"menulist");
							  	sprintf(i_char,"%d",i+1);
							  	strcat(menu,i_char);*/

								fprintf(cgiOut,"<tr height=25 bgcolor=%s align=left>",setclour(cl));
								 fprintf(cgiOut,"<td>%u</td>",query_info[i].QID);
								 fprintf(cgiOut,"<td>%s</td>",query_info[i].Scheduling_group);
								 fprintf(cgiOut,"<td>%u</td>",query_info[i].weight); 
								 fprintf(cgiOut,"</tr>");
								 cl=!cl;
							}
							 
						  	fprintf(cgiOut,"</table></div>"\
							 "</td>"\
						   "</tr>"\
						 
						"<tr>"\
						"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
						fprintf(cgiOut,"</tr>"\
						 "<tr height=25 style=padding-top:2px>"\
			   			"<td style=font-size:14px;color:#FF0000>%s</td>",search(lcontrol,"queue_TC_des"));
			   			fprintf(cgiOut,"</tr>"\
			   			
						/*"<tr>"\
						"<td>"\
						"<table width=430 style=padding-top:2px>"\
						"<tr>");
						sprintf(pageNumCA,"%d",pageNum+1);
						sprintf(pageNumCD,"%d",pageNum-1);
						if(cgiFormSubmitClicked("submit_config_query") != cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
							}
						else if(cgiFormSubmitClicked("submit_config_query") == cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",policer_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",policer_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));  
							}
						fprintf(cgiOut,"</tr></table></td>"\
						"</tr>"\*/
						 "<tr>");
						 if(cgiFormSubmitClicked("submit_config_query") != cgiFormSuccess)
						 {
						   fprintf(cgiOut,"<td colspan=6><input type=hidden name=encry_configqueue value=%s></td>",encry);
						   fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%s></td>",addn);
						 }
						 else if(cgiFormSubmitClicked("submit_config_query") == cgiFormSuccess)
							 {
							   fprintf(cgiOut,"<td colspan=6><input type=hidden name=encry_configqueue value=%s></td>",encry);
							   fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%s></td>",addn);
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
free(i_char);
for(i=0;i<8;i++)
{
	free(query_info[i].Scheduling_group);
}
free(group);
free(QueryID);
free(wrr_weight);
free(queue_mode);
release(lpublic);  
release(lcontrol);
free(mode);
return 0;
}

