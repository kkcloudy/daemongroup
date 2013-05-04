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
* wp_TrafficShapping.c
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
#include "ws_dcli_portconf.h"
#include "ws_dcli_qos.h"

#define AMOUNT 512

char * Shapping_Mode[] = {
		"PORT",
		"QUERY"
};

char * switch7000[] = {
		"1/1","1/2","1/3","1/4","1/5","1/6","2/1","2/2","2/3","2/4","2/5","2/6","3/1","3/2","3/3","3/4","3/5","3/6","4/1","4/2","4/3","4/4","4/5","4/6"
		
};

char * switch5000[] = {
		"1/1","1/2","1/3","1/4","1/5","1/6","1/7","1/8","1/9","1/10","1/11","1/12","1/13","1/14","1/15","1/16","1/17","1/18","1/19","1/20","1/21","1/22","1/23","1/24"
		
};

char * kORm[] = {
	"k",
	"m"
};


int ShowTrafficShappingPage();

int cgiMain()
{
 ShowTrafficShappingPage();
 return 0;
}

int ShowTrafficShappingPage()
{
	 ccgi_dbus_init();
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 	
  char *encry=(char *)malloc(BUF_LEN);
  char *str;
 // FILE *fp;
  //char lan[3];
  char addn[N];         
  int i=0;   
  int cl=1;
  //char menu[21]="menulist";
  char* i_char=(char *)malloc(10);
  char * max_rate=(char * )malloc(10);
  memset(max_rate,0,10);
  char * QueryID=(char * )malloc(10);
  memset(QueryID,0,10);
  char * burstsize=(char * )malloc(10);
  memset(burstsize,0,10);
  char * product=(char *)malloc(20);
  memset(product,0,20);
  char * bindPort=(char *)malloc(20);
  memset(bindPort,0,20);
  strcpy(bindPort,"1/1");
  unsigned int gu_index=0;

  int temp;

  char * k_m = (char *)malloc(10);
  memset(k_m,0,10);
  
  char * mode=(char *)malloc(AMOUNT);
  memset(mode,0,AMOUNT);
  
  int retu=0;
  //char * CheckUsr=(char * )malloc(10);
  //memset(CheckUsr,0,10);
  
  	//*************add show port function******************
	int num = 0,ret1=0;
  	ETH_SLOT_LIST  head,*p;
  	ETH_PORT_LIST *pp;
	char * port = (char *)malloc(10);
	//*************************************************

  int qos_num=0;
  char * Shapping_mode=(char * )malloc(10);
  memset(Shapping_mode,0,10);
  struct qos_info receive_qos[MAX_QOS_PROFILE];
  struct Shapping_info shapping_info[8];
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
 
  product=readproductID();
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
  }
  else
  {
     cgiFormStringNoNewlines("encry_configqueue",encry,BUF_LEN);
     cgiFormStringNoNewlines("max_rate",max_rate,10);
     cgiFormStringNoNewlines("QueryID",QueryID,10);
     cgiFormStringNoNewlines("burstsize",burstsize,10);
     cgiFormStringNoNewlines("Shapping_mode",Shapping_mode,10);
     cgiFormStringNoNewlines("Shapping_port",bindPort,10);
     cgiFormStringNoNewlines("CheckUsr",addn,N);
     cgiFormStringNoNewlines("k_m",k_m,10);
     //if(strcmp(CheckUsr,"")!=0)
     	// retu=atoi(CheckUsr);
     //fprintf(stderr,"QueryID=%s",QueryID);
     //fprintf(stderr,"bindPort=%s-Shapping_mode=%s-max_rate=%s-burstsize=%s",bindPort,Shapping_mode,max_rate,burstsize);
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
  "</head>");
    fprintf(cgiOut,"<script type=\"text/javascript\">");
    fprintf(cgiOut,"function popMenu(objId)"\
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
			 "}");
	fprintf(cgiOut,"function isNum(strNum)"\
				"{"\
					"var re = /^[0-9]*[1-9][0-9]*$/;\n"\
					"if(strNum!=\"\"&&strNum!=null)\n"\
					"{\n"\
						"if(re.test(strNum)==false)\n"\
						"{\n"\
							"alert(\"%s\");\n"\
							"document.all.max_rate.value = \"\";\n"\
  							"document.all.max_rate.focus();\n"\
						"}\n"\
					"}\n"\
				"}\n",search(lcontrol,"num_wrong"));
	fprintf(cgiOut,"</script>"\
  "<body>");

  	//if(cgiFormSubmitClicked("submit_config_query") != cgiFormSuccess)
  	//{
  		//fprintf(stderr,"addn=%s",addn);
  		retu=checkuser_group(addn);
  	//}
    if(cgiFormSubmitClicked("submit_config_query") == cgiFormSuccess)
    {
    		temp=atoi(burstsize);
		get_one_port_index_QOS(bindPort,&gu_index);
		if(strcmp(burstsize,"")!=0)
		{
			if(strcmp(k_m,"k")==0)
			{
				if(temp<1||temp>4096)
				{
					ShowAlert(search(lcontrol,"illegal_k"));
				}
				else
				{
					if(strcmp(Shapping_mode,"QUERY")==0)
					{	
						int ret=set_traffic_queue_attr(QueryID,max_rate,burstsize,gu_index,k_m,lcontrol);
						if(ret==0)
						{
							ShowAlert(search(lcontrol,"appendsucc"));
						}
						else
							ShowAlert(search(lpublic,"oper_fail"));
					}
					else if(strcmp(Shapping_mode,"PORT")==0)
					{
						int ret= set_traffic_shape(max_rate,burstsize,gu_index,k_m,lcontrol);
						if(ret==0)
						{
							ShowAlert(search(lcontrol,"appendsucc"));
						}
						else
							ShowAlert(search(lpublic,"oper_fail"));
					}
				}
			}
			else if(strcmp(k_m,"m")==0)
			{
				if(temp<1||temp>1000)
				{
					ShowAlert(search(lcontrol,"illegal_m"));
				}
				else
				{
					if(strcmp(Shapping_mode,"QUERY")==0)
					{	
						int ret=set_traffic_queue_attr(QueryID,max_rate,burstsize,gu_index,k_m,lcontrol);
						if(ret==0)
						{
							ShowAlert(search(lcontrol,"appendsucc"));
						}
						else
							ShowAlert(search(lpublic,"oper_fail"));
					}
					else if(strcmp(Shapping_mode,"PORT")==0)
					{
						int ret=set_traffic_shape(max_rate,burstsize,gu_index,k_m,lcontrol);
						if(ret==0)
						{
							ShowAlert(search(lcontrol,"appendsucc"));
						}
						else
							ShowAlert(search(lpublic,"oper_fail"));
					}
				}
			}
		}	
		else
		{
			ShowAlert(search(lcontrol,"burst_null"));
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
	   // if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
	    //  ShowAlert(search(lpublic,"error_open"));
	   // fseek(fp,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
	   // fgets(lan,3,fp);	   
	//	fclose(fp);
	  //  if(strcmp(lan,"ch")==0)
    	//{
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
		/*}		
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_config_query style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		  if(cgiFormSubmitClicked("submit_config_query") != cgiFormSuccess)
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosModule.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  else
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_qosModule.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  fprintf(cgiOut,"</tr>"\
		  "</table>");
		}*/
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
     			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"config_shape_mode"));   /*突出显示*/
     			fprintf(cgiOut,"</tr>");
     			if(retu==0)
     			{
          			fprintf(cgiOut,"<tr height=25>"\
                 			"<td align=left id=tdleft><a href=wp_delshapping.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"del_shapping"));					   
                 		  fprintf(cgiOut,"</tr>");
            	}
            	int rowsCount=0;
            	if(retu==0)
            		rowsCount=16;
            	else rowsCount=15;
				for(i=0;i<rowsCount+5;i++)
					{
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					}
				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
						"<table width=640 height=230 border=0 cellspacing=0 cellpadding=0>");
						int Shappingchoice=0;
						//fprintf(cgiOut,"retu=%d",retu);
						fprintf(cgiOut,"<tr height='35'>\n"\
									"<td style=\"font-size:14px\"><font color='red'><b>%s</b></font></td>\n"\
									"</tr>\n",search(lcontrol,mode));
					if(retu==0)
					{
     			fprintf(cgiOut,"<tr>"\
     							"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lcontrol,"config_shape_mode"));
     			fprintf(cgiOut,"</tr>"\
     						"<tr>"\
     							"<td>"\
     								"<table frame=below width=640 rules=rows height=70 border=1>");
     						show_qos_profile(receive_qos,&qos_num,lcontrol);
     						for(i=0;i<qos_num;i++)
     						{
     							if(receive_qos[i].tc!=10)
     								flag[receive_qos[i].tc]=1;
     						}
     						fprintf(cgiOut,"<tr height=25 align=left  style=padding-top:8px>");
     							fprintf(cgiOut,"<td id=td1 width=120>%s:</td>",search(lcontrol,"Shapping_mode"));
     							fprintf(cgiOut,"<td  align=left width=60>"\
											"<select name=Shapping_mode onchange=\"javascript:this.form.submit();\">");
     						    for(i=0;i<2;i++)
     						    {
	     						    	if(strcmp(Shapping_Mode[i],Shapping_mode)==0)				/*显示上次选中的*/
	     							{
	     								fprintf(cgiOut,"<option value=%s selected=selected>%s",Shapping_Mode[i],Shapping_Mode[i]);
	     							}
	     							else
	     							{
	     								fprintf(cgiOut,"<option value=%s>%s",Shapping_Mode[i],Shapping_Mode[i]);
	     							}
     						    }
     						    		fprintf(cgiOut,"</select>");
     						    fprintf(cgiOut,"</td>");
     							cgiFormSelectSingle("Shapping_mode", Shapping_Mode, 2, &Shappingchoice, 0);
						ret1=show_ethport_list(&head,&num);
  						p=head.next;
         					fprintf(cgiOut,"<td id=td1 align=left width=100>%s:</td>",search(lcontrol,"config_egress_port"));
           					fprintf(cgiOut,"<td  align=left width=380 colspan=6>"\
								 		"<select name=Shapping_port>");
						if(p!=NULL)
							{
								while(p!=NULL)
								{
									pp=p->port.next;
									while(pp!=NULL)
									{
										memset(port,0,10);
										sprintf(port,"%d/%d",p->slot_no,pp->port_no);
          									fprintf(cgiOut,"<option value=%s>%s</option>\n",port,port);
										pp=pp->next;
									}
									p=p->next;
								}
							 }
						 fprintf(cgiOut,"</select>");
           					fprintf(cgiOut,"</td>");
						#if 0
						if(0==strcmp(product,"Switch7000"))
      						{
           					     fprintf(cgiOut,"<td id=td1 align=left width=100>%s:</td>",search(lcontrol,"config_egress_port"));
           					     fprintf(cgiOut,"<td  align=left width=380 colspan=6>"\
								 	     		"<select name=Shapping_port>");
           						    for(i=0;i<24;i++)
           						    {
           						    	if(strcmp(switch7000[i],bindPort)==0)
           						    	{
           								fprintf(cgiOut,"<option value=%s selected=selected>%s",switch7000[i],switch7000[i]);
           						    	}
           							else 
           							{
           								fprintf(cgiOut,"<option value=%s>%s",switch7000[i],switch7000[i]);
           							}
           						    }
           						    fprintf(cgiOut,"</select>");
           						  fprintf(cgiOut,"</td>");
      						}
      						else if(0==strcmp(product,"Switch5000"))
      						{
      						     	fprintf(cgiOut,"<td id=td1 align=left width=80>%s:</td>",search(lcontrol,"config_egress_port"));
           						fprintf(cgiOut,"<td  align=left width=400 colspan=6>"\
											"<select name=Shapping_port>");
           						    for(i=0;i<24;i++)
           						    if(strcmp(switch5000[i],bindPort)==0)
           						    	fprintf(cgiOut,"<option value=%s selected=selected>%s",switch5000[i],switch5000[i]);
           						    else
           								fprintf(cgiOut,"<option value=%s>%s",switch5000[i],switch5000[i]);
           						    fprintf(cgiOut,"</select>");
           						    fprintf(cgiOut,"</td>");
      						}
						#endif
     						   	fprintf(cgiOut,"</tr>");
     						   	fprintf(cgiOut,"<tr height=25 align=left  style=padding-top:8px>");
     						   	//fprintf(cgiOut,"Shapping_Mode[Shappingchoice]=%s",Shapping_Mode[Shappingchoice]);
     							if(strcmp(Shapping_Mode[Shappingchoice],"QUERY")==0)
     							{
         						    fprintf(cgiOut,"<td id=td1 width=140>%s:</td>",search(lcontrol,"queue_TC"));
							//  if(qos_num!=0)
							//  {
	     							    fprintf(cgiOut,"<td align=left width=60><select name=QueryID style='width:60;height:auto'>");
	     							    for(i=0;i<8;i++)
	     							    {
	     								//if(flag[i]==1)
	     								//{
	     									fprintf(cgiOut,"<option value=%d>%d</option>",i,i);
	     								//}
	     							    }
	         						    fprintf(cgiOut,"</select>");
	         						    fprintf(cgiOut,"</td>");
							
							  //  }
							   // else
							   // {
							    //	fprintf(cgiOut,"<td align=left width=60><font color='red'>%s</font></td>",search(lcontrol,"no_tc"));
							   // }
							
     							}
     							fprintf(cgiOut,"<td id=td1 width=140>%s:</td>",search(lcontrol,"MAX_RATE"));
     							fprintf(cgiOut,"<td width=60><input type=text name=max_rate size=8 onblur=\"isNum(this.value);\"></td>");
     							fprintf(cgiOut,"<td id=td1 width=100>%s:</td>",search(lcontrol,"burstsize"));
     							fprintf(cgiOut,"<td width=30><input type=text name=burstsize size=8></td>");
							#if 0
	     							if(strcmp(Shapping_Mode[Shappingchoice],"QUERY")==0)
	     							{
	     								fprintf(cgiOut,"<td style=color:red width=80>(1-4096)</td>");
	     							}
	     							else
	     							{
	     								fprintf(cgiOut,"<td style=color:red width=70>(1-4096)</td>");
	     							}
							#endif
							//*********add k/m*************
							fprintf(cgiOut,"<td id=td1 width=20>%s:</td>\n","k/m");
							fprintf(cgiOut,"<td width=120>\n"\
											"<select name='k_m'>\n");
							for(i=0;i<2;i++)
							{
								fprintf(cgiOut,"<option value='%s'>%s</option>\n",kORm[i],kORm[i]);
							}
								fprintf(cgiOut,"</select>\n"\
										"</td>\n");
							//*******************************
     							fprintf(cgiOut,"</tr>");
     						fprintf(cgiOut,"</table>"\
     						"</td>"\
     						"</tr>");
     					}
     					else
     					{
     						fprintf(cgiOut,"<tr><td><table width=480  height=50 border=0>");
     						fprintf(cgiOut,"<tr height=30 align=left  style=padding-top:8px>");
 							cgiFormSelectSingle("Shapping_mode", Shapping_Mode, 2, &Shappingchoice, 0);
						ret1=show_ethport_list(&head,&num);
  						p=head.next;
         					fprintf(cgiOut,"<td id=td1 align=left width=100>%s:</td>",search(lcontrol,"config_egress_port"));
           					fprintf(cgiOut,"<td  align=left width=380 colspan=6>"\
								 		"<select name=Shapping_port>");
						if(p!=NULL)
						{
							while(p!=NULL)
							{
								pp=p->port.next;
								while(pp!=NULL)
								{
									memset(port,0,10);
									sprintf(port,"%d/%d",p->slot_no,pp->port_no);
          								fprintf(cgiOut,"<option value=%s>%s</option>\n",port,port);
									pp=pp->next;
								}
								p=p->next;
							}
						}
						 fprintf(cgiOut,"</select>");
           					fprintf(cgiOut,"</td>");

							#if 0
							if(0==strcmp(product,"Switch7000"))
 							{
      						    fprintf(cgiOut,"<td id=td1 align=left width=80>%s:</td>",search(lcontrol,"config_egress_port"));
      							fprintf(cgiOut,"<td  align=left width=400><select name=Shapping_port>");
      						    for(i=0;i<24;i++)
      						    if(strcmp(switch7000[i],bindPort)==0)
      								fprintf(cgiOut,"<option value=%s selected=selected>%s",switch7000[i],switch7000[i]);
      							else 
      								fprintf(cgiOut,"<option value=%s>%s",switch7000[i],switch7000[i]);
      						    fprintf(cgiOut,"</select>");
      						    fprintf(cgiOut,"</td>");
 						    }
 							else if(0==strcmp(product,"Switch5000"))
 							{
 								fprintf(cgiOut,"<td id=td1 align=left width=80>%s:</td>",search(lcontrol,"config_egress_port"));
      							fprintf(cgiOut,"<td  align=left width=400><select name=Shapping_port>");
      						    for(i=0;i<24;i++)
      						    if(strcmp(switch5000[i],bindPort)==0)
      						    	fprintf(cgiOut,"<option value=%s selected=selected>%s",switch5000[i],switch5000[i]);
      						    else
      								fprintf(cgiOut,"<option value=%s>%s",switch5000[i],switch5000[i]);
      						    fprintf(cgiOut,"</select>");
      						    fprintf(cgiOut,"</td>");
 							}
							#endif
 						   	fprintf(cgiOut,"</tr></table></td></tr>");
    
     					}
     					
						fprintf(cgiOut,"<tr style=padding-top:8px>"\
						"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s (%s:%s)</td>",search(lcontrol,"config_shape_list"),search(lcontrol,"config_egress_port"),bindPort);
						fprintf(cgiOut,"</tr>");
						for(i=0;i<8;i++)
                          {
							shapping_info[i].burstsize=0;
							shapping_info[i].maxrate=0;
							shapping_info[i].port_burstsize=0;
							shapping_info[i].port_maxrate=0;
							shapping_info[i].QOS_ID=0;
							shapping_info[i].Port_Shaping_status=(char * )malloc(10);
							memset(shapping_info[i].Port_Shaping_status,0,10);
							shapping_info[i].Shaping_status=(char * )malloc(10);
							memset(shapping_info[i].Port_Shaping_status,0,10);
                          }
                  		gu_index=0;
                  		//fprintf(stderr,"bindPort=%s",bindPort);
                        get_one_port_index_QOS(bindPort,&gu_index);
						show_traffic_shape(shapping_info,gu_index);
						fprintf(cgiOut,"<tr align=left style=padding-top:5px>"\
						"<td>"\
						"<table  width=640 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						"<td id=td1  width=70>%s:</td>",search(lcontrol,"Port_Shaping"));
					   fprintf(cgiOut,"<td id=td2  width=70>%s</td>",shapping_info[0].Port_Shaping_status);
					   fprintf(cgiOut,"<td id=td1  width=100>%s:</td>",search(lcontrol,"PORT_Max_Rate"));
					   fprintf(cgiOut,"<td id=td2  width=50>%ld</td>",shapping_info[0].port_maxrate);
					   fprintf(cgiOut,"<td id=td1  width=110>%s:</td>",search(lcontrol,"PORT_Burst_Size"));
					   fprintf(cgiOut,"<td id=td2  width=240>%u</td>",shapping_info[0].port_burstsize);
					   fprintf(cgiOut,"</tr>"\
					   "</table></td></tr>");

							
						   fprintf(cgiOut,"<tr align=left valign=top style=padding-top:15px>"\
						   "<td>"\
						   "<div class=ShowPolicy><table width=420 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");
						   	fprintf(cgiOut,"<tr height=30 bgcolor=#eaeff9 style=font-size:14px  id=td1 align=left>"\
							 "<th width=70><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"queue_TC"));
							 fprintf(cgiOut,"<th width=70><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"Status"));
							 fprintf(cgiOut,"<th width=120><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"MAX_RATE"));
							 fprintf(cgiOut,"<th width=120 colspan=3><font id=%s>%s</font></th>", search(lpublic,"menu_thead"),search(lcontrol,"burstsize"));
							 fprintf(cgiOut,"</tr>");

							for(i=0;i<8;i++)
							{
								fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));
								 fprintf(cgiOut,"<td style=font-size:12px align=left>%u</td>",shapping_info[i].QOS_ID);
								 fprintf(cgiOut,"<td style=font-size:12px align=left>%s</td>",shapping_info[i].Shaping_status);
								 fprintf(cgiOut,"<td style=font-size:12px align=left>%ld</td>",shapping_info[i].maxrate);
								 fprintf(cgiOut,"<td style=font-size:12px align=left colspan=3>%u</td>",shapping_info[i].burstsize); 
								 fprintf(cgiOut,"</tr>");
								 cl=!cl;
							}
    					fprintf(cgiOut,"</table></div>"\
    					 "</td>"\
    				   "</tr>");

						fprintf(cgiOut,"<tr style=padding-top:15px>"\
						"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\" >%s</td>",search(lpublic,"description"));
						fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25 style=padding-top:2px>"\
			   			"<td style=font-size:14px;color:#FF0000>%s</td>",search(lcontrol,"km_mean"));
			   			fprintf(cgiOut,"</tr>");
						 fprintf(cgiOut,"<tr height=25 style=padding-top:2px>"\
			   			"<td style=font-size:14px;color:#FF0000>%s</td>",search(lcontrol,"queue_TC_des"));
			   			fprintf(cgiOut,"</tr>");
			   			fprintf(cgiOut,"<tr height=25 style=padding-top:2px>"\
			   			"<td style=font-size:14px;color:#FF0000>%s</td>",search(lcontrol,"burstsize_real_value"));
			   			fprintf(cgiOut,"</tr>");
						fprintf(cgiOut,"<tr height=25 style=padding-top:2px>"\
			   			"<td style=font-size:14px;color:#FF0000>%s</td>",search(lcontrol,"burstsize_km"));
			   			fprintf(cgiOut,"</tr>");
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
						 fprintf(cgiOut,"<tr>");
						  fprintf(cgiOut,"<td colspan=8><input type=hidden name=encry_configqueue value=%s></td>",encry);
						  fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%s></td>",addn);
						  //fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);
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
//*****************************************************
if((ret1==0)&&(num>0))
{
	Free_ethslot_head(&head);
}

free(port);
//********************************************************

free(encry);
free(i_char);
for(i=0;i<8;i++)
{
	free(shapping_info[i].Port_Shaping_status);
	free(shapping_info[i].Shaping_status);
}
free(max_rate);
free(QueryID);
//free(CheckUsr);
free(burstsize);
free(product);
free(bindPort);
free(Shapping_mode);
release(lpublic);  
release(lcontrol);
free(mode);
free(k_m);
return 0;
}

