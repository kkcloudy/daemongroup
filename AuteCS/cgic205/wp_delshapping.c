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
* wp_delshapping.c
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
#include "ws_dcli_portconf.h"
#include "ws_dcli_qos.h"
#define AMOUNT 512
char * Shapping_Mode[] = {
		"PORT",
		"QUEUE"
};

char * switch7000[] = {
		"1/1","1/2","1/3","1/4","1/5","1/6","2/1","2/2","2/3","2/4","2/5","2/6","3/1","3/2","3/3","3/4","3/5","3/6","4/1","4/2","4/3","4/4","4/5","4/6"
		
};

char * switch5000[] = {
		"1/1","1/2","1/3","1/4","1/5","1/6","1/7","1/8","1/9","1/10","1/11","1/12","1/13","1/14","1/15","1/16","1/17","1/18","1/19","1/20","1/21","1/22","1/23","1/24",
		
};


int ShowDelShappingPage(); 


int cgiMain()
{
 ShowDelShappingPage();
 return 0;
}

int ShowDelShappingPage()
{
	ccgi_dbus_init();
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    	lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	//*************add show port function******************
	int num = 0,ret1=0;
	//int num = 0,m=0,ret1=0,ret2=0;
  	ETH_SLOT_LIST  head,*p;
  	ETH_PORT_LIST *pp;
	char * port = (char *)malloc(10);
	//*************************************************
	//FILE *fp;
	//char lan[3];
	char *encry=(char *)malloc(BUF_LEN);			  
	char *str;
	int i;
	int select_flag=0;
	
	int flag[8];
	int qos_num=0;
	struct qos_info receive_qos[MAX_QOS_PROFILE];
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
    unsigned int gu_index=0;
	char * Shapping_mode=(char *)malloc(10);
	memset(Shapping_mode,0,10);
	char * QueryID=(char *)malloc(10);
	memset(QueryID,0,10);
	char * bindPort=(char *)malloc(10);
	memset(bindPort,0,10);
	char * product=(char *)malloc(20);
  	memset(product,0,20);
	//char delshapping_encry[BUF_LEN];  
	memset(encry,0,BUF_LEN);

	char * mode=(char *)malloc(AMOUNT);
	memset(mode,0,AMOUNT);
	
	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
	{
	  str=dcryption(encry);
	  if(str==NULL)
	  {
		ShowErrorPage(search(lpublic,"ill_user"));	   /*用户非法*/
		return 0;
	  }
	}
	else
	{
      cgiFormStringNoNewlines("encry_delshape",encry,BUF_LEN);
      cgiFormStringNoNewlines("shapping_type",Shapping_mode,BUF_LEN);
      cgiFormStringNoNewlines("QueryID",QueryID,10);
      cgiFormStringNoNewlines("Shapping_port",bindPort,10);
  	}
  	
	product=readproductID();
   cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");
  if(cgiFormSubmitClicked("submit_delshapping") == cgiFormSuccess)
  {
  		get_one_port_index_QOS(bindPort,&gu_index);
    	if(strcmp(Shapping_mode,"QUEUE")==0)
    	{
    		if(strcmp(QueryID,"")!=0)
    		{
			int ret = del_queue_traffic(QueryID,gu_index);
			switch(ret)
			{
				case 0:
					ShowAlert(search(lcontrol,"del_succ"));
					break;
				case -2:
					ShowAlert(search(lcontrol,"port_error"));
					break;
				case -3:
					ShowAlert(search(lcontrol,"no_traffic_info"));
					break;
				case -4:
					ShowAlert(search(lcontrol,"fail_del_shap"));
					break;
				default:
					ShowAlert(search(lpublic,"oper_fail"));
					break;
			}
		}
		else
		{
			ShowAlert(search(lcontrol,"pls_create_profile"));
		}
    	}
	else if(strcmp(Shapping_mode,"PORT")==0)
	{
		int ret = del_traffic_shape(gu_index);
		switch(ret)
		{
			case 0:
				ShowAlert(search(lcontrol,"del_succ"));
				break;
			case -2:
				ShowAlert(search(lcontrol,"port_error"));
				break;
			case -3:
				ShowAlert(search(lcontrol,"no_traffic_info"));
				break;

			case -4:
				ShowAlert(search(lcontrol,"fail_del_shap"));
				break;
			default:
				ShowAlert(search(lpublic,"oper_fail"));
				break;
		}
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
	   //   ShowAlert(search(lpublic,"error_open"));
	   // fseek(fp,4,0); 						/*将文件指针移到离文件首4个字节处，即lan=之后*/
	   // fgets(lan,3,fp);	   
		//fclose(fp);
	   // if(strcmp(lan,"ch")==0)
    	//{	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_delshapping style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_delshapping") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_TrafficShapping.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_TrafficShapping.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
		/*}		
		else			
		{	
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_delshapping style=background-image:url(/images/ok-en.jpg) value=""></td>");		  
		  if(cgiFormSubmitClicked("submit_delshapping") != cgiFormSuccess)
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_TrafficShapping.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
		  else
		    fprintf(cgiOut,"<td width=62 align=left><a href=wp_TrafficShapping.cgi?UN=%s target=mainFrame><img src=/images/cancel-en.jpg border=0 width=62 height=20/></a></td>",encry);
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
        		  fprintf(cgiOut,"<tr height=25>"\
        			"<td align=left id=tdleft><a href=wp_TrafficShapping.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcontrol,"config_shape_mode"));					   
        		  fprintf(cgiOut,"</tr>"\
        		    "<tr height=26>"\
        			"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"del_shapping"));	 /*突出显示*/
        		  fprintf(cgiOut,"</tr>");
					for(i=0;i<5;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }

				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  "<table width=640 height=115 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						  "<td align=left valign=top>"\
						  "<table width=350 border=0 cellspacing=0 cellpadding=0  style=padding-top:18px>"\
						  "<tr height=30  align=left>");
				fprintf(cgiOut,"<td style=\"font-size:14px\" colspan='2'><font color='red'><b>%s</b></font></td>",search(lcontrol,mode));
			fprintf(cgiOut,"</tr>"\
							 "<tr height=30  align=left>"\
								"<td id=td1 align=left>%s:</td>",search(lcontrol,"Shapping_type"));
								fprintf(cgiOut,"<td>"\
								"<select name=shapping_type  onchange=\"javascript:this.form.submit();\">");
     						    for(i=0;i<2;i++)
     							if(strcmp(Shapping_Mode[i],Shapping_mode)==0)				/*显示上次选中的*/
     								fprintf(cgiOut,"<option value=%s selected=selected>%s",Shapping_Mode[i],Shapping_Mode[i]);
     							else				
     								fprintf(cgiOut,"<option value=%s>%s",Shapping_Mode[i],Shapping_Mode[i]);
     						    fprintf(cgiOut,"</select>");
								fprintf(cgiOut,"</td>"\
							  "</tr>"\
							  "<tr align=left height=30>");
							  int Shappingchoice=0;
     							cgiFormSelectSingle("shapping_type", Shapping_Mode, 2, &Shappingchoice, 0);
     							ret1=show_ethport_list(&head,&num);
  							p=head.next;
							fprintf(cgiOut,"<td id=td1 align=left width=90>%s:</td>\n",search(lcontrol,"config_egress_port"));
          						fprintf(cgiOut,"<td  align=left width=260><select name=Shapping_port>\n");
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
							fprintf(cgiOut,"</select>\n");
							  fprintf(cgiOut,"</tr>"\
							  "<tr align=left height=30>");
							  show_qos_profile(receive_qos,&qos_num,lcontrol);
							  
        							for(i=0;i<qos_num;i++)
        							{
        								if(receive_qos[i].tc!=10)
        								{
        									flag[receive_qos[i].tc]=1;
										select_flag++;
        								}
        							}
									
								if(Shappingchoice==1)
								{
        						    		fprintf(cgiOut,"<td id=td1 align=left width=90>%s:</td>",search(lcontrol,"queue_TC"));
											
    									fprintf(cgiOut,"<td  align=left width=260>\n");
									if(select_flag!=0)
									{
										fprintf(cgiOut,"<select name=QueryID>\n");
    										for(i=0;i<8;i++)
    										{
    											if(flag[i]==1)
    											{
    												fprintf(cgiOut,"<option value=%d>%d",i,i);
    											}
    										}
        						    			fprintf(cgiOut,"</select>");
									}
									else if(select_flag==0)
									{
										fprintf(cgiOut,"<font color='red'>%s</font>\n",search(lcontrol,"not_create_qos_profile"));
									}
        						    		fprintf(cgiOut,"</td>");
								}
								
							  fprintf(cgiOut,"</tr>"\
							  "<tr>");
							  /*if(cgiFormSubmitClicked("submit_delshapping") != cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_delshape value=%s></td>",encry);
							  }
							  else if(cgiFormSubmitClicked("submit_delshapping") == cgiFormSuccess)
							  {
								fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_delshape value=%s></td>",delshapping_encry);
							  }*/
							  fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_delshape value=%s></td>",encry);
							  fprintf(cgiOut,"</tr>"\
							"</table>"\
						  "</td>"\
						"</tr>"\
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
free(Shapping_mode);
free(QueryID);
free(bindPort);
free(product);
release(lpublic);  
release(lcontrol);
free(mode);

return 0;
}

