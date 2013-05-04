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
* wp_policer.c
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

char * Policer_Mode[] = {
	"strict",
	"loose"
};



int ShowPolicerPage();

int cgiMain()
{
 ShowPolicerPage();
 return 0;
}

int ShowPolicerPage()
{
    int retz;
	ccgi_dbus_init();
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	 lpublic=get_chain_head("../htdocs/text/public.txt");
   	 lcontrol=get_chain_head("../htdocs/text/control.txt"); 	
	  char *encry=(char *)malloc(BUF_LEN);
	  char *str;

	  char * mode=(char *)malloc(AMOUNT);
	  memset(mode,0,AMOUNT);
	 int result;
	  //char policer_encry[BUF_LEN];        
	  char addn[N];
	  int i=0;   
	  int cl=1;
	  int retu=0;
	  int policer_num=0;
	  char menu[21]="menulist";
	  char* i_char=(char *)malloc(10);
	  char * deletepolicer=(char * )malloc(10);
	  memset(deletepolicer,0,10);
	  char * index=(char * )malloc(10);
	  memset(index,0,10);
	  char * CheckUsr=(char * )malloc(10);
	  memset(CheckUsr,0,10);
	  char * disablecounter = (char *)malloc(10);
	  memset(disablecounter,0,10);
	  char * counterindex = (char *)malloc(10);
	  memset(counterindex,0,10);
	  struct policer_info receive_policer[Policer_Num];
	  //policer mode
	  char * policer_mode=(char * )malloc(10);
	  memset(policer_mode,0,10);
	  //policer mode information
	  char * policer_mode_info=(char * )malloc(10);
	  memset(policer_mode_info,0,10);
  
	  for(i=0;i<Policer_Num;i++)
		{
	    	receive_policer[i].policer_index=0;
	    	 receive_policer[i].policer_state=(char *)malloc(20);
	    	 memset(receive_policer[i].policer_state,0,20);
	    	 
	    	 receive_policer[i].CounterState=(char *)malloc(20);
	    	 memset(receive_policer[i].CounterState,0,20);
	    	 
	    	 receive_policer[i].Out_Profile_Action=(char *)malloc(20);
	    	 memset(receive_policer[i].Out_Profile_Action,0,20);
	    	 
	    	 receive_policer[i].Policer_Mode=(char *)malloc(20);
	    	 memset(receive_policer[i].Policer_Mode,0,20);
	    	 
	    	 receive_policer[i].Policing_Packet_Size=(char *)malloc(20);
	    	 memset(receive_policer[i].Policing_Packet_Size,0,20);

	    }
  
	  
		memset(encry,0,BUF_LEN);
	       cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	       str=dcryption(encry);
	       if(str==NULL)
	       {
		      ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
		      return 0;
		}
	  strcpy(addn,str);
	 
	  cgiFormStringNoNewlines("DELRULE",deletepolicer,10);
	  cgiFormStringNoNewlines("INDEX",index,10);
	  cgiFormStringNoNewlines("CheckUsr",CheckUsr,10);
	  cgiFormStringNoNewlines("ENABLE",disablecounter,10);
	  cgiFormStringNoNewlines("COUNTERINDEX",counterindex,10);
	  cgiFormStringNoNewlines("policer_mode",policer_mode,10);
	  cgiFormStringNoNewlines("policer_mode_info",policer_mode_info,10);

	  if(strcmp(CheckUsr,"")!=0)
	  	retu=atoi(CheckUsr);
	 
	show_qos_mode(mode);

	  cgiHeaderContentType("text/html");
	  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"route_manage"));
	  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
		  	"<style type=text/css>"\
	  	  "#div1{ width:42px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
		  "#div2{ width:50px; height:15px; padding-left:5px; padding-top:3px}"\
		  "#link{ text-decoration:none; font-size: 12px}"\
	  	".ShowPolicy {overflow-x:hidden;  overflow:auto; width: 750px; height: 386px; clip: rect( ); padding-top: 2px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
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
	  if(strcmp(deletepolicer,"delete")==0)
	  {
	  	result=delete_policer(index);
		switch(result)
		{
			case 0:
				ShowAlert(search(lcontrol,"del_succ"));
				break;
			case -2:
				ShowAlert(search(lcontrol,"no_policer"));
				break;
			case -3:
				ShowAlert(search(lcontrol,"inuse_policer"));
				break;
			case -4:
				ShowAlert(search(lcontrol,"fail_del_policer"));
				break;
			default :
				ShowAlert(search(lpublic,"oper_fail"));
				break;
			}
	  }
	  if(cgiFormSubmitClicked("submit_policerlist") != cgiFormSuccess)
	  {
	  	retu=checkuser_group(str);
	  }
	  else if(cgiFormSubmitClicked("submit_policerlist") == cgiFormSuccess)
	  {
	  	 fprintf( cgiOut, "<script type='text/javascript'>\n" );
	 	 fprintf( cgiOut, "window.location.href='wp_qosModule.cgi?UN=%s';\n",encry);
	 	 fprintf( cgiOut, "</script>\n" );
	  }
	  
		if(strcmp(disablecounter,"disable")==0)
		{
			retz=counter_policer(counterindex,disablecounter,index);
			switch(retz)
			{
				case 0:
					ShowAlert(search(lpublic,"oper_succ"));
					break;
				default:
					ShowAlert(search(lpublic,"oper_fail"));
					break;
			}
		}

		if(cgiFormSubmitClicked("config_mode") == cgiFormSuccess)
		{	 
				
			if(strcmp(policer_mode,"strict")==0)
			{
			    	retz=set_strict_mode(policer_mode_info);
					switch(retz)
					{
						case 0:
							ShowAlert(search(lpublic,"oper_succ"));
							break;
						default:
							ShowAlert(search(lpublic,"oper_fail"));
							break;
					}
			}
			else if(strcmp(policer_mode,"loose")==0)
			{				
			    	retz=set_meter_loose_mode(policer_mode_info);
					switch(retz)
					{
						case 0:
							ShowAlert(search(lpublic,"oper_succ"));
							break;
						default:
							ShowAlert(search(lpublic,"oper_fail"));
							break;
					}
			}
		    	 fprintf(cgiOut, "<script type='text/javascript'>\n" );
		 	 fprintf(cgiOut, "window.location.href='wp_policer.cgi?UN=%s';\n",encry);
		 	 fprintf(cgiOut, "</script>\n" );
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
          "<td width=62 align=center><input id=but type=submit name=submit_policerlist style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
          
		
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
	#if 0
         		if(cgiFormSubmitClicked("submit_policerlist") != cgiFormSuccess)
         		{
         			fprintf(cgiOut,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"policer"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
         			if(retu==0)
         			{
            			fprintf(cgiOut,"<tr height=25>"\
            			"<td align=left id=tdleft><a href=wp_addpolicer.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_policer"));					   
            			fprintf(cgiOut,"</tr>");
         			}
				fprintf(cgiOut,"<tr height=26>"\
             			"<td align=left id=tdleft><a href=wp_qoscounter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"counter"));					   
             			fprintf(cgiOut,"</tr>");

         		}
         		else if(cgiFormSubmitClicked("submit_policerlist") == cgiFormSuccess) 			  
         		{
         		#endif
         			fprintf(cgiOut,"<tr height=26>"\
         			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"policer"));   /*突出显示*/
         			fprintf(cgiOut,"</tr>");
         			if(retu==0)
         			{
	             			fprintf(cgiOut,"<tr height=25>"\
	             			"<td align=left id=tdleft><a href=wp_addpolicer.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"add_policer"));					   
	             			fprintf(cgiOut,"</tr>");
         			}
				fprintf(cgiOut,"<tr height=26>"\
             			"<td align=left id=tdleft><a href=wp_qoscounter.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"counter"));					   
             			fprintf(cgiOut,"</tr>");
				
         		//}
         		int rowsCount=0;
         		if(retu==0)
         			rowsCount=15;
         		else
         			rowsCount=16;
				for(i=0;i<rowsCount;i++)
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}
				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
						"<table width=700 height=310 border=0 cellspacing=0 cellpadding=0>");
				 
			fprintf(cgiOut,"<tr>"\
							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">\n"\
								"<table width=%s>\n","90%");
									fprintf(cgiOut,"<tr>\n"\
										"<td id=sec1 style=\"font-size:14px\">%s</td>",search(lcontrol,"policy_map_info"));
							fprintf(cgiOut,"<td id=sec1 style=\"font-size:14px\" align='right'><font color='red'><b>%s</b></font></td>",search(lcontrol,mode));
						fprintf(cgiOut,"</tr>\n"\
								"</table>\n"\
							"</td>\n");
			fprintf(cgiOut,"</tr>");
						 //////////////////new/////////////////////
				if(checkuser_group(addn)==0)/*administrator*/
				{
				  fprintf(cgiOut,"<tr><td>&nbsp;</td></tr>");
				  fprintf(cgiOut,"<tr>");
				  	fprintf(cgiOut,"<td>");
				  		fprintf(cgiOut,"<table>");
				  
				  		fprintf(cgiOut,"<tr align=left  style=padding-top:8px>");
						    fprintf(cgiOut,"<td id=td1 width=110>%s:</td>",search(lcontrol,"config_policer_mode"));
						    fprintf(cgiOut,"<td  align=left style=padding-left:10px width=50><select name=policer_mode onchange=\"javascript:this.form.submit();\">");
						    for(i=0;i<2;i++)
							if(strcmp(Policer_Mode[i],policer_mode)==0)				/*显示上次选中的*/
								fprintf(cgiOut,"<option value=%s selected=selected>%s",Policer_Mode[i],Policer_Mode[i]);
							else				
								fprintf(cgiOut,"<option value=%s>%s",Policer_Mode[i],Policer_Mode[i]);
						    fprintf(cgiOut,"</select>\n");
						    fprintf(cgiOut,"</td>");
						    int policer_modechoice=0;
							cgiFormSelectSingle("policer_mode", Policer_Mode, 2, &policer_modechoice, 0);
			                            if(policer_modechoice==0)
			                            {
			                         
								fprintf(cgiOut,"<td  align=left style=padding-left:10px><select name=policer_mode_info>");
				  
				    				fprintf(cgiOut,"<option value=%s>%s","l1","L1");
				    				fprintf(cgiOut,"<option value=%s>%s","l2","L2");
				    				fprintf(cgiOut,"<option value=%s>%s","l3","L3");
				    				fprintf(cgiOut,"</select>");
				    				fprintf(cgiOut,"</td>");
							}
							else if(policer_modechoice==1)
							{
								fprintf(cgiOut,"<td  align=left style=padding-left:10px><select name=policer_mode_info>");

			    					fprintf(cgiOut,"<option value=%s>%s","0","0");
			    					fprintf(cgiOut,"<option value=%s>%s","1","1");
			    					fprintf(cgiOut,"<option value=%s>%s","2","2");
			    					fprintf(cgiOut,"</select>");
			    					fprintf(cgiOut,"</td>");
							}

							fprintf(cgiOut,"<td align=left style=padding-left:10px>");
								fprintf(cgiOut,"<input type=submit style=width:70px; height:36px  border=0 name=config_mode style=background-image:url(/images/SubBackGif.gif) value=\"%s\">",search(lcontrol,"mode_submit"));
							fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"</table>");
							fprintf(cgiOut,"</td>");
						fprintf(cgiOut,"</tr>");
					}
				  ///////////////////////////////////////
			fprintf(cgiOut,"<tr>"\
						   "<td align=left valign=top style=padding-top:18px>"\
						   "<div class=ShowPolicy><table width=688 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
						   fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 style=font-size:14px  id=td1 align=left>"\
							 "<th width=45 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"Index");
							 fprintf(cgiOut,"<th width=45 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"Policer_State"));
							 fprintf(cgiOut,"<th width=40 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"CIR");
							 fprintf(cgiOut,"<th width=40 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"CBS");
							 fprintf(cgiOut,"<th width=110 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"Counter State");
							 fprintf(cgiOut,"<th width=110 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),"Counter Index");
							 fprintf(cgiOut,"<th width=85 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"Out_Profile"));
							 fprintf(cgiOut,"<th width=100 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"Remap_QOS"));
							 fprintf(cgiOut,"<th width=50 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"Policer_Mode"));
							 fprintf(cgiOut,"<th width=50 style=font-size:12px><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"Policer_Size"));
							 fprintf(cgiOut,"<th width=13 style=font-size:12px>&nbsp;</th>");
							 fprintf(cgiOut,"</tr>");
							show_policer(receive_policer,&policer_num);
							
							for(i=0;i<policer_num;i++)
							{
								memset(menu,0,21);
							  	strcpy(menu,"menulist");
							  	sprintf(i_char,"%d",i+1);
							  	strcat(menu,i_char);

								fprintf(cgiOut,"<tr height=25 bgcolor=%s align=left>",setclour(cl));
								 fprintf(cgiOut,"<td style=font-size:12px>%u</td>",receive_policer[i].policer_index);
								 fprintf(cgiOut,"<td style=font-size:12px>%s</td>",receive_policer[i].policer_state);
								 fprintf(cgiOut,"<td style=font-size:12px>%ld</td>",receive_policer[i].cir);
								 fprintf(cgiOut,"<td style=font-size:12px>%ld</td>",receive_policer[i].cbs);
								 fprintf(cgiOut,"<td style=font-size:12px align='center'>%s</td>",receive_policer[i].CounterState);
								 if(receive_policer[i].CounterIndex==0)
								 {
								 	fprintf(cgiOut,"<td style=font-size:12px align='center'>%s</td>","No");
								 }
								 else
								 {
								 	fprintf(cgiOut,"<td style=font-size:12px align='center'>%d</td>",receive_policer[i].CounterIndex);
								 }
								 fprintf(cgiOut,"<td style=font-size:12px align='center'>%s</td>",receive_policer[i].Out_Profile_Action);
								 fprintf(cgiOut,"<td style=font-size:12px align='center'>%u</td>",receive_policer[i].Remap_QoSProfile);
								 fprintf(cgiOut,"<td style=font-size:12px>%s</td>",receive_policer[i].Policer_Mode);
								 fprintf(cgiOut,"<td style=font-size:12px>%s</td>",receive_policer[i].Policing_Packet_Size);
								 if(retu==0)
								 {
    								 fprintf(cgiOut,"<td align=left>");
    								 fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(policer_num-i),menu,menu);
    																   fprintf(cgiOut,"<img src=/images/detail.gif>"\
    																   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
    																   fprintf(cgiOut,"<div id=div1>");
																	   
    																  //if(cgiFormSubmitClicked("submit_policerlist") != cgiFormSuccess)
    																  // {
    																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_conpolicer.cgi?UN=%s&INDEX=%u&COUNTERINDEX=%d target=mainFrame>%s</a></div>",encry,receive_policer[i].policer_index,receive_policer[i].CounterIndex,search(lpublic,"configure"));
        																   	fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_policer.cgi?UN=%s&INDEX=%u&DELRULE=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,receive_policer[i].policer_index,"delete",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
        																   	if(receive_policer[i].CounterIndex!=0)
        																   	{
        																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_policer.cgi?UN=%s&INDEX=%u&COUNTERINDEX=%d&ENABLE=%s target=mainFrame>%s</a></div>",encry,receive_policer[i].policer_index,receive_policer[i].CounterIndex,"disable",search(lcontrol,"counter_disable"));
        																   	}
																		#if 0	
    																   	}
    																   else
    																   {
    																  
																	   
    																		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_conpolicer.cgi?UN=%s&INDEX=%u&COUNTERINDEX=%d target=mainFrame>%s</a></div>",encry,receive_policer[i].policer_index,receive_policer[i].CounterIndex,search(lpublic,"configure"));
        																   	fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_policer.cgi?UN=%s&INDEX=%u&DELRULE=%s target=mainFrame >%s</a></div>",encry,receive_policer[i].policer_index,"delete",search(lcontrol,"confirm_delete"),search(lcontrol,"delete"));
        																   	if(receive_policer[i].CounterIndex!=0)
        																   	{
        																   		fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_policer.cgi?UN=%s&INDEX=%u&COUNTERINDEX=%d&ENABLE=%s target=mainFrame>%s</a></div>",encry,receive_policer[i].policer_index,receive_policer[i].CounterIndex,"disable",search(lcontrol,"counter_disable"));
        																   	}
    																	//}
    																	 #endif
    																   fprintf(cgiOut,"</div>"\
    																   "</div>"\
    																   "</div>");
    																   fprintf(cgiOut,"</td>");
								}
								 else  fprintf(cgiOut,"<td>&nbsp;</td>");
							 fprintf(cgiOut,"</tr>");
								 cl=!cl;
							}
							 
						  	fprintf(cgiOut,"</table></div>"\
							 "</td>"\
						   "</tr>"\
						 
						 /*"<tr height=25 style=padding-top:2px>"\
			   			"<td style=font-size:14px;color:#FF0000> K - kernel route, C - connected, S - static, R - RIP, O - OSPF,I - ISIS, B - BGP, > - selected route, * - FIB route</td>"\
			   			"</tr>"\*/
			   			
						/*"<tr>"\
						"<td>"\
						"<table width=430 style=padding-top:2px>"\
						"<tr>");
						sprintf(pageNumCA,"%d",pageNum+1);
						sprintf(pageNumCD,"%d",pageNum-1);
						if(cgiFormSubmitClicked("submit_policerlist") != cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));
							}
						else if(cgiFormSubmitClicked("submit_policerlist") == cgiFormSuccess)
							{
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",policer_encry,pageNumCA,"PageDown",search(lcontrol,"page_down"));
								fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_srouter.cgi?UN=%s&PN=%s&SN=%s>%s</td>",policer_encry,pageNumCD,"PageUp",search(lcontrol,"page_up"));  
							}
						fprintf(cgiOut,"</tr></table></td>"\
						"</tr>"\*/
						 "<tr>");
						// if(cgiFormSubmitClicked("submit_policerlist") != cgiFormSuccess)
						// {
							   fprintf(cgiOut,"<td><input type=hidden name='UN' value=%s></td>",encry);
							   fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
						// }
						// else if(cgiFormSubmitClicked("submit_policerlist") == cgiFormSuccess)
						//{
						//	   fprintf(cgiOut,"<td><input type=hidden name='UN' value=%s></td>",encry);
						//	   fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
						//}
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
for(i=0;i<Policer_Num;i++)
  {

	  	free(receive_policer[i].policer_state);
	  	free(receive_policer[i].CounterState);
	  	free(receive_policer[i].Out_Profile_Action);
	  	free(receive_policer[i].Policer_Mode);
	  	free(receive_policer[i].Policing_Packet_Size);
 }
free(deletepolicer);
free(index);
free(policer_mode);
free(policer_mode_info);
free(disablecounter);
free(counterindex);
release(lpublic);  
release(lcontrol);
free(mode);
return 0;
}
