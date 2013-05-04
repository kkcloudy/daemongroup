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
* wp_conpolicer.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
*
*
*
*******************************************************************************/

#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"
#include "ws_sysinfo.h"
#include "ws_dcli_qos.h"

char * Out_Profile[] = {
	"keep",
	"drop",
	"remap"
};

#define SHOW_AMOUNT 15  //转换数组的容量

int ShowConfigPolicyPage(); 
int addvlan_hand(struct list *lpublic); 

int cgiMain()
{
 ShowConfigPolicyPage();
 return 0;
}



int ShowConfigPolicyPage()
{    
    int retz;
	ccgi_dbus_init();
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	int ret,result;
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	char * index=(char *)malloc(10);
	unsigned int indextemp=0;
	char *endptr = NULL;  
	int i,back;
	char configpolicy_encry[BUF_LEN];

	char * out_profile_action=(char * )malloc(20);
	memset(out_profile_action,0,20);
	char * policer_enable=(char * )malloc(10);
	memset(policer_enable,0,10);
	

	
	char * remap_qos=(char * )malloc(10);
	memset(remap_qos,0,10);
	//////////////////////new/////////////////
	char * inNum[SHOW_AMOUNT]={"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15"};
	unsigned int count_flag = 0;
	char * temp = malloc(BUF_LEN);
	char * counterindex=malloc(BUF_LEN);
	memset(counterindex,0,BUF_LEN);
	memset(temp,0,BUF_LEN);
	struct counter_info * counter[SHOW_AMOUNT];
	for(i=0;i<SHOW_AMOUNT;i++)
	{
  		counter[i] = (struct counter_info *)malloc(sizeof(struct counter_info));	
		
		if(counter[i]==NULL)
	  	{
	  		ShowErrorPage("no space");
			return 0;
	  	}
		memset(counter[i],0,sizeof(struct counter_info));
 	}

	char *con_index = (char *)malloc(BUF_LEN);
	memset(con_index,0,BUF_LEN);
	
	
	///////////////////////////
	

	//////////////////
	for(i=0;i<15;i++)
	{
		back=show_counter(inNum[i],counter[i]);
		if(back==0)
		{
			count_flag++;
		}
	}
	/////////////////
	memset(encry,0,BUF_LEN);
	memset(index,0,10);
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
	{
		  cgiFormStringNoNewlines("INDEX",index,10);
		  str=dcryption(encry);
		  if(str==NULL)
		  {
			ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
			return 0;
		  }
		  memset(configpolicy_encry,0,BUF_LEN);					 /*清空临时变量*/
	}
	else
	{
	       cgiFormStringNoNewlines("encry_addvlan",encry,BUF_LEN);
	       cgiFormStringNoNewlines("INDEXLATER",index,10);

	       cgiFormStringNoNewlines("out_profile_action",out_profile_action,10);
	       cgiFormStringNoNewlines("policer_enable",policer_enable,10);
	       
	       cgiFormStringNoNewlines("remap_qos",remap_qos,10);
	       indextemp=strtoul(index,&endptr,10);
   	}
   cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");
	if(cgiFormSubmitClicked("submit_configpolicy") == cgiFormSuccess)
	{
		if(strcmp(out_profile_action,"keep")==0 || strcmp(out_profile_action,"drop")==0)
		{
			ret=set_out_profile_action(out_profile_action,index,lcontrol);
			if(ret==0)
				ShowAlert(search(lcontrol,"config_suc"));
			else
				ShowAlert(search(lcontrol,"config_fail"));	
		}
		else if(strcmp(out_profile_action,"remap")==0)
		{
			ret=set_out_profile_remap(remap_qos,index,lcontrol);
			switch(ret)
			{
				case 0:
					ShowAlert(search(lcontrol,"config_suc"));
					break;
				case -2:
					ShowAlert(search(lcontrol,"qos_not_existed"));
					break;
				default :
					ShowAlert(search(lcontrol,"config_fail"));
					break;
			}
		}
		result=set_policer_enable(index,policer_enable);
		switch(result)
		{
			case 0:
				ShowAlert(search(lcontrol,"en_policer"));
				break;
			case -2:
				ShowAlert(search(lcontrol,"no_policer"));
				break;
			default:
				ShowAlert(search(lcontrol,"Operation_Fail"));
				break;
		}
		
	}

	if(cgiFormSubmitClicked("counter_enable")==cgiFormSuccess)
	{	
		cgiFormStringNoNewlines("counter",con_index,BUF_LEN);
		if(strcmp(con_index,"")!=0)
		{
			retz=counter_policer(con_index,"enable",index);
			switch(retz)
			{
				case 0:
				{
					ShowAlert(search(lcontrol,"counterconfigsucc"));
					fprintf(cgiOut, "<script type='text/javascript'>\n" );
					fprintf(cgiOut, "window.location.href='wp_policer.cgi?UN=%s';\n", encry);
					fprintf(cgiOut, "</script>\n" );
					break;
				}
				default:
					ShowAlert(search(lpublic,"oper_fail"));
					break;
			}
		}
		else
		{
			ShowAlert(search(lcontrol,"counter_notnull"));
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
	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_configpolicy style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_configpolicy") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_policer.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_policer.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"config_policer"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
         			fprintf(cgiOut,"<tr height=25>"\
         			"<td align=left id=tdleft><a href=wp_addpolicer.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_policer"));					   
         			fprintf(cgiOut,"</tr>");
				fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_qoscounter.cgi?UN=%s target=mainFrame style=color:#000000><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"counter"));						 
            		 	 fprintf(cgiOut,"</tr>");

					for(i=0;i<4;i++)
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

     					   	fprintf(cgiOut,"<table frame=below rules=rows width=500 border=1>");	
     			
             				
							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
						    fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"out_profile_action"));
						    fprintf(cgiOut,"<td  align=left style=padding-left:10px><select name=out_profile_action onchange=\"javascript:this.form.submit();\">");
								
						    for(i=0;i<((get_product_id()==PRODUCT_ID_AU3K_BCM)?2:3);i++)
							if(strcmp(Out_Profile[i],out_profile_action)==0)				/*显示上次选中的*/
								fprintf(cgiOut,"<option value=%s selected=selected>%s",Out_Profile[i],Out_Profile[i]);
							else				
								fprintf(cgiOut,"<option value=%s>%s",Out_Profile[i],Out_Profile[i]);
						    	
						    fprintf(cgiOut,"</select>");
						    fprintf(cgiOut,"</td>");
						    int Out_Profilechoice=0;
							cgiFormSelectSingle("out_profile_action", Out_Profile, 3, &Out_Profilechoice, 0);
							if(Out_Profilechoice==2)
							{
								fprintf(cgiOut,"<td id=td1>%s:</td>","QOS Profile Index");
								fprintf(cgiOut,"<td><input type=text name=remap_qos size=8></td>");
							}
							else
							{
								fprintf(cgiOut,"<td id=td1 width='1'>&nbsp;</td>");
								fprintf(cgiOut,"<td id=td1 width='1'>&nbsp;</td>");
							}
						   	fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
						    fprintf(cgiOut,"<td id=td1>%s:</td>",search(lcontrol,"policer_enable"));
						    if(Out_Profilechoice==2)
						    	fprintf(cgiOut,"<td  align=left style=padding-left:10px colspan=3><select name=policer_enable>");
						    else fprintf(cgiOut,"<td  align=left style=padding-left:10px colspan=3><select name=policer_enable>");
						    fprintf(cgiOut,"<option value=%s>%s","enable","enable");
						    fprintf(cgiOut,"<option value=%s>%s","disable","disable");
						    fprintf(cgiOut,"</select>");
						    fprintf(cgiOut,"</td>"\
						   	"</tr>");
						   ////////////////////////////////
						   cgiFormStringNoNewlines("COUNTERINDEX",counterindex,BUF_LEN);
						   fprintf(cgiOut,"<tr align='left'  style='padding-top:8px'>\n");
							   fprintf(cgiOut,"<td id='td1'>Counter Index:</td>\n");
							   fprintf(cgiOut,"<td  align='left' style='padding-left:10px'>\n");
								fprintf(cgiOut,"<select name='counter'>\n");
									fprintf(cgiOut,"<option value=''>--%s--</option>\n",search(lcontrol,"select"));
								for(i=0;i<15;i++)
								{	
									if(counter[i]->index!=0)
									{
										fprintf(cgiOut,"<option value=%d>%d</option>\n",counter[i]->index,counter[i]->index);
									}
								}
								fprintf(cgiOut,"</select>\n");
							   fprintf(cgiOut,"</td>\n");
							   fprintf(cgiOut,"<td  align='left' style='padding-left:10px'><input type='submit' name='counter_enable' value='%s' style='width:60px;height:default'></td>\n",search(lcontrol,"counter_enable"));
							   if(strcmp(counterindex,"0")!=0)
							   {
							   	fprintf(cgiOut,"<td  align='left' style='padding-left:10px'><font color=red>Counter %s %s</font></td>\n",counterindex,search(lcontrol,"counter_enable"));
							   }
							   else
							   {
							   	fprintf(cgiOut,"<td  align='left' style='padding-left:10px'><font color=red>%s</font></td>\n",search(lcontrol,"no_counter"));
							   }
						   	fprintf(cgiOut,"</tr>\n");
						///////////////////////////////////
		fprintf(cgiOut,"</table>"\
						"</td>"\
						"</tr>");
						fprintf(cgiOut,"<tr>");
					  if(cgiFormSubmitClicked("submit_configpolicy") != cgiFormSuccess)
					  {
						fprintf(cgiOut,"<td><input type=hidden name=encry_addvlan value=%s></td>",encry);
						fprintf(cgiOut,"<td><input type=hidden name=INDEXLATER value=%s></td>",index);
						fprintf(cgiOut,"<td><input type=hidden name=COUNTERINDEX value=%s></td>",counterindex);						
					  }
					  else if(cgiFormSubmitClicked("submit_configpolicy") == cgiFormSuccess)
					  {
						fprintf(cgiOut,"<td><input type=hidden name=encry_addvlan value=%s></td>",encry);
						fprintf(cgiOut,"<td><input type=hidden name=INDEXLATER value=%s></td>",index);
						fprintf(cgiOut,"<td><input type=hidden name=COUNTERINDEX value=%s></td>",counterindex);
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
free(out_profile_action);
free(policer_enable);


for(i=0;i<15;i++)
{
	free(counter[i]);
}
free(counterindex);
free(con_index);
release(lpublic);  
release(lcontrol);
return 0;
}

