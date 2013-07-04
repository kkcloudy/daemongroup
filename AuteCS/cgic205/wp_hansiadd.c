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
* wp_hansiadd.c
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
#include "ws_err.h"
#include "ws_ec.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"

#include "ws_dbus_list.h"
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vrrp.h"

int ShowHansiaddPage(char *m,char *n,struct list *lpublic,struct list *lcontrol);    

int cgiMain()
{  
  char *encry=(char *)malloc(HANSIUNL);  
  char *str;   
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lcontrol;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcontrol=get_chain_head("../htdocs/text/control.txt");

  char macChoice[10]; 
  memset(macChoice,0,10);
  cgiFormStringNoNewlines("SZ",macChoice,10);
  if(strcmp(macChoice,"")==0)
  	strcpy(macChoice,"1");

  if(cgiFormStringNoNewlines("UN", encry, HANSIUNL)!=cgiFormNotFound )  /*首次进入该页*/
  {
    str=dcryption(encry);
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else 
	  ShowHansiaddPage(encry,macChoice,lpublic,lcontrol);
  }
  else                    
  {   
   cgiFormStringNoNewlines("encry_newvrrp",encry,HANSIUNL);

	str=dcryption(encry);
    if(str==NULL)
      ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else 
	  ShowHansiaddPage(encry,macChoice,lpublic,lcontrol);
  } 
  free(encry);
  release(lpublic);  
  release(lcontrol);
  return 0;
}

int ShowHansiaddPage(char *m,char *n,struct list *lpublic,struct list *lcontrol)
{  
    int i,upmaskbit,dwmaskbit;  
  /*define types for params*/
	char hspro[HANSIIPL];
	memset(hspro,0,HANSIIPL);

	char ulname[HANSIIPL];
	memset(ulname,0,HANSIIPL);

	char dlname[HANSIIPL];
	memset(dlname,0,HANSIIPL);

	char ulip[HANSIIPL];
	memset(ulip,0,HANSIIPL);

	char dlip[HANSIIPL];
	memset(dlip,0,HANSIIPL);

	char uip1[4],uip2[4],uip3[4],uip4[4];
	

	char dip1[4],dip2[4],dip3[4],dip4[4];
	
	char lprio[HANSIIPL];
	memset(lprio,0,HANSIIPL);

	char hbifname[HANSIIPL];
	memset(hbifname,0,HANSIIPL);
	char hbip[HANSIIPL];
	memset(hbip,0,HANSIIPL);
	char hbip1[4],hbip2[4],hbip3[4],hbip4[4];
	
	memset(hbip1,0,4);
	memset(hbip2,0,4);
	memset(hbip3,0,4);
	memset(hbip4,0,4);
	
	char upmask[HANSIIPL];
	memset(upmask,0,HANSIIPL);
	char downmask[HANSIIPL];
	memset(downmask,0,HANSIIPL);
	char upmk1[4];
	char upmk2[4];
	char upmk3[4];
	char upmk4[4];
	
	char dwmk1[1];
	char dwmk2[4];
	char dwmk3[4];
	char dwmk4[4];
	

	/*function return or params convert*/
	int op_ret=-1,convert_num=-1;

	char paramalert[128];
	memset(paramalert,0,128);

	DBusConnection *connection = NULL;
	ccgi_dbus_init();
	instance_parameter *paraHead2 = NULL;
	instance_parameter *p_q = NULL;
	list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
	char plotid[10] = {0};
	int pid = 0;
	cgiFormStringNoNewlines("plotid",plotid,sizeof(plotid));
	pid = atoi(plotid);
	if(0 == pid)
	{
		for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
		{
			pid = p_q->parameter.slot_id;
			connection = p_q->connection;
			break;
		}
	}
	else
	{
		for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
		{
			if(p_q->parameter.slot_id == pid)
			{
				connection = p_q->connection;
			}
		}
	}
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>VRRP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  	"<script language=javascript src=/ip.js>"\
  	"</script>"\
  "</head>"\
  "<body>");
 if(cgiFormSubmitClicked("hansiadd") == cgiFormSuccess)
 {
	//web page param with
	memset(hspro,0,HANSIIPL);
	cgiFormStringNoNewlines("profile",hspro,HANSIIPL);

	//heartbeatlink 
	cgiFormStringNoNewlines("hbifname",hbifname,HANSIIPL);
	cgiFormStringNoNewlines("hbip1",hbip1,4);
	
	cgiFormStringNoNewlines("hbip2",hbip2,4);
	cgiFormStringNoNewlines("hbip3",hbip3,4);
	cgiFormStringNoNewlines("hbip4",hbip4,4);

	sprintf(hbip,"%ld.%ld.%ld.%ld",strtoul(hbip1,0,10),strtoul(hbip2,0,10),strtoul(hbip3,0,10),strtoul(hbip4,0,10));


	memset(lprio,0,HANSIIPL);
	cgiFormStringNoNewlines("linkprio",lprio,HANSIIPL);


	ccgi_dbus_init();
	if(strcmp(hspro,"")==0)
	{
		memset(paramalert,0,128);
		sprintf(paramalert,"Hansi profile %s",search(lpublic,"param_null"));
		ShowAlert(paramalert);
	}
	else
	{
	   
		convert_num=strtoul(hspro,0,10);
		if(convert_num < 1 || convert_num > 16)	
		{
		    if(strcmp(n,"4")!=0)
	    	{
				ShowAlert(search(lpublic,"hs_param"));
	    	}
		}
		else
		{
			fprintf(stderr,"hspro=%s\n",hspro);
			fprintf(stderr,"pid=%d\n",pid);
			fprintf(stderr,"connection=%p\n",connection);
//			op_ret=ccgi_config_hansi_profile(hspro,pid,connection);
			//config current node sucessfully!
			if(op_ret==0)                         
			{
				if(strcmp(n,"0")==0) 
				{
					memset(paramalert,0,128);
					sprintf(paramalert,"Hansi profile %s",search(lpublic,"param_conf_succ"));
					ShowAlert(paramalert);
				}
				//up and down
				if(strcmp(n,"1")==0) 
				{
				    //up link 
					cgiFormStringNoNewlines("upname",ulname,HANSIIPL);
					memset(uip1,0,4);
					memset(uip2,0,4);
					memset(uip3,0,4);
					memset(uip4,0,4);
					cgiFormStringNoNewlines("upip1",uip1,4);
					cgiFormStringNoNewlines("upip2",uip2,4);
					cgiFormStringNoNewlines("upip3",uip3,4);
					cgiFormStringNoNewlines("upip4",uip4,4);
					sprintf(ulip,"%ld.%ld.%ld.%ld",strtoul(uip1,0,10),strtoul(uip2,0,10),strtoul(uip3,0,10),strtoul(uip4,0,10));
					//down link
					cgiFormStringNoNewlines("downname",dlname,HANSIIPL);
					memset(dip1,0,4);
					memset(dip2,0,4);
					memset(dip3,0,4);
					memset(dip4,0,4);
					cgiFormStringNoNewlines("downip1",dip1,4);
					cgiFormStringNoNewlines("downip2",dip2,4);
					cgiFormStringNoNewlines("downip3",dip3,4);
					cgiFormStringNoNewlines("downip4",dip4,4);
					sprintf(dlip,"%ld.%ld.%ld.%ld",strtoul(dip1,0,10),strtoul(dip2,0,10),strtoul(dip3,0,10),strtoul(dip4,0,10));
				   //upmask
				   	memset(upmk1,0,4);
					memset(upmk2,0,4);
					memset(upmk3,0,4);
					memset(upmk4,0,4);
					cgiFormStringNoNewlines("upmk1",upmk1,4);
					cgiFormStringNoNewlines("upmk2",upmk2,4);
					cgiFormStringNoNewlines("upmk3",upmk3,4);
					cgiFormStringNoNewlines("upmk4",upmk4,4);
					sprintf(upmask,"%ld.%ld.%ld.%ld",strtoul(upmk1,0,10),strtoul(upmk2,0,10),strtoul(upmk3,0,10),strtoul(upmk4,0,10));

				    //downmask
				   	memset(dwmk1,0,4);
					memset(dwmk2,0,4);
					memset(dwmk3,0,4);
					memset(dwmk4,0,4);
					cgiFormStringNoNewlines("dwmk1",dwmk1,4);
					cgiFormStringNoNewlines("dwmk2",dwmk2,4);
					cgiFormStringNoNewlines("dwmk3",dwmk3,4);
					cgiFormStringNoNewlines("dwmk4",dwmk4,4);
					sprintf(downmask,"%ld.%ld.%ld.%ld",strtoul(dwmk1,0,10),strtoul(dwmk2,0,10),strtoul(dwmk3,0,10),strtoul(dwmk4,0,10));
					fprintf(stderr,"ulip=%s\n",ulip);
					fprintf(stderr,"dlip=%s\n",dlip);
					fprintf(stderr,"upmask=%s\n",upmask);
					fprintf(stderr,"downmask=%s\n",downmask);

					if((strcmp(ulname,"")!=0) && (strcmp(dlname,"") !=0)&& (strcmp(lprio,"") !=0)&& (strcmp(uip1,"") !=0)&& (strcmp(uip2,"") !=0)&& (strcmp(uip3,"") !=0)&& (strcmp(uip4,"") !=0)&& (strcmp(dip1,"") !=0)&& (strcmp(dip2,"") !=0)&& (strcmp(dip3,"") !=0)&& (strcmp(dip4,"") !=0)&&(strcmp(upmk1,"") !=0)&&(strcmp(upmk2,"") !=0)&&(strcmp(upmk3,"") !=0)&&(strcmp(upmk4,"") !=0)&&(strcmp(dwmk1,"") !=0)&&(strcmp(dwmk2,"") !=0)&&(strcmp(dwmk3,"") !=0)&&(strcmp(dwmk4,"") !=0))
					{
						convert_num=strtoul(lprio,0,10);
						if(convert_num < 1 || convert_num > 255)
						{
							ShowAlert(search(lpublic,"hs_param"));
						}
						else
						{
						    upmaskbit=mask_bit(upmask);
							dwmaskbit=mask_bit(downmask);
							if((upmaskbit == -1)||(dwmaskbit == -1))
								ShowAlert(search(lcontrol,"dhcp_mask_err"));
							else
							{
//								op_ret=ccgi_downanduplink_ifname_mask(hspro,ulname, ulip, dlname, dlip, lprio,upmaskbit,dwmaskbit,connection);
								fprintf(stderr,"op_ret=%d\n",op_ret);
								if(op_ret==0)
								{
									memset(paramalert,0,128);
									sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_succ"));
									ShowAlert(paramalert);
								}
								else
								{
									memset(paramalert,0,128);
									sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_fail"));
									ShowAlert(paramalert);
								}
						    }
						}
					}
					else
					{
						ShowAlert(search(lpublic,"hs_param"));
					}
				}
				//uplink
				if(strcmp(n,"2")==0) 
				{
				    //up link 
					cgiFormStringNoNewlines("upname",ulname,HANSIIPL);
					memset(uip1,0,4);
					memset(uip2,0,4);
					memset(uip3,0,4);
					memset(uip4,0,4);
					cgiFormStringNoNewlines("upip1",uip1,4);
					cgiFormStringNoNewlines("upip2",uip2,4);
					cgiFormStringNoNewlines("upip3",uip3,4);
					cgiFormStringNoNewlines("upip4",uip4,4);
					sprintf(ulip,"%ld.%ld.%ld.%ld",strtoul(uip1,0,10),strtoul(uip2,0,10),strtoul(uip3,0,10),strtoul(uip4,0,10));

				   //upmask
				   	memset(upmk1,0,4);
					memset(upmk2,0,4);
					memset(upmk3,0,4);
					memset(upmk4,0,4);
					cgiFormStringNoNewlines("upmk1",upmk1,4);
					cgiFormStringNoNewlines("upmk2",upmk2,4);
					cgiFormStringNoNewlines("upmk3",upmk3,4);
					cgiFormStringNoNewlines("upmk4",upmk4,4);
					sprintf(upmask,"%ld.%ld.%ld.%ld",strtoul(upmk1,0,10),strtoul(upmk2,0,10),strtoul(upmk3,0,10),strtoul(upmk4,0,10));
					fprintf(stderr,"ulip=%s\n",ulip);
					fprintf(stderr,"upmask=%s\n",upmask);

					if((strcmp(ulname,"")!=0) && (strcmp(lprio,"") !=0)&& (strcmp(uip1,"") !=0)&& (strcmp(uip2,"") !=0)&& (strcmp(uip3,"") !=0)&& (strcmp(uip4,"") !=0)&&(strcmp(upmk1,"") !=0)&&(strcmp(upmk2,"") !=0)&&(strcmp(upmk3,"") !=0)&&(strcmp(upmk4,"") !=0))
					{
						convert_num=strtoul(lprio,0,10);
						if(convert_num < 1 || convert_num > 255)
						{
							ShowAlert(search(lpublic,"hs_param"));
						}
						else
						{
						    upmaskbit=mask_bit(upmask);
							if(upmaskbit == -1)
								ShowAlert(search(lcontrol,"dhcp_mask_err"));
                            else
                        	{
								fprintf(stderr,"op_ret=%d\n",op_ret);
//								op_ret=config_vrrp_uplink(hspro,ulname, ulip, lprio,upmaskbit,connection);
								if(op_ret==0)
								{
									memset(paramalert,0,128);
									sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_succ"));
									ShowAlert(paramalert);
								}
								else
								{
									memset(paramalert,0,128);
									sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_fail"));
									ShowAlert(paramalert);
								}
                        	}
						}
					}
					else	
					{
						ShowAlert(search(lpublic,"hs_param"));
					}
				}
				//downlink
				if(strcmp(n,"3")==0) 
				{
					//down link
					cgiFormStringNoNewlines("downname",dlname,HANSIIPL);
					memset(dip1,0,4);
					memset(dip2,0,4);
					memset(dip3,0,4);
					memset(dip4,0,4);
					cgiFormStringNoNewlines("downip1",dip1,4);
					cgiFormStringNoNewlines("downip2",dip2,4);
					cgiFormStringNoNewlines("downip3",dip3,4);
					cgiFormStringNoNewlines("downip4",dip4,4);
					sprintf(dlip,"%ld.%ld.%ld.%ld",strtoul(dip1,0,10),strtoul(dip2,0,10),strtoul(dip3,0,10),strtoul(dip4,0,10));
				    //downmask
				   	memset(dwmk1,0,4);
					memset(dwmk2,0,4);
					memset(dwmk3,0,4);
					memset(dwmk4,0,4);
					cgiFormStringNoNewlines("dwmk1",dwmk1,4);
					cgiFormStringNoNewlines("dwmk2",dwmk2,4);
					cgiFormStringNoNewlines("dwmk3",dwmk3,4);
					cgiFormStringNoNewlines("dwmk4",dwmk4,4);
					sprintf(downmask,"%ld.%ld.%ld.%ld",strtoul(dwmk1,0,10),strtoul(dwmk2,0,10),strtoul(dwmk3,0,10),strtoul(dwmk4,0,10));
					fprintf(stderr,"dlip=%s\n",dlip);
					fprintf(stderr,"downmask=%s\n",downmask);

					if((strcmp(dlname,"") !=0)&& (strcmp(lprio,"") !=0)&& (strcmp(dip1,"") !=0)&& (strcmp(dip2,"") !=0)&& (strcmp(dip3,"") !=0)&& (strcmp(dip4,"") !=0)&&(strcmp(dwmk1,"") !=0)&&(strcmp(dwmk2,"") !=0)&&(strcmp(dwmk3,"") !=0)&&(strcmp(dwmk4,"") !=0))
					{
						convert_num=strtoul(lprio,0,10);
						if(convert_num < 1 || convert_num > 255)
						ShowAlert(search(lpublic,"hs_param"));
						else
						{
							dwmaskbit=mask_bit(downmask);
							if(dwmaskbit == -1)
								ShowAlert(search(lcontrol,"dhcp_mask_err"));
							else
							{
//								op_ret=config_vrrp_downlink_mask(hspro,dlname,dlip, lprio,dwmaskbit,connection);
								fprintf(stderr,"op_ret=%d\n",op_ret);
								if(op_ret==0)
								{
									memset(paramalert,0,128);
									sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_succ"));
									ShowAlert(paramalert);
								}
								else
								{
									memset(paramalert,0,128);
									sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_fail"));
									ShowAlert(paramalert);
								}
							}
						}
					}
					else
					{
						ShowAlert(search(lpublic,"hs_param"));
					}
				}
				if((strcmp(hbifname,"")!=0))
				{
//					op_ret=config_vrrp_heartbeat_cmd_func(hspro, hbifname, hbip,connection);
					fprintf(stderr,"op_ret=%d\n",op_ret);
					if(op_ret==0)
					{
						memset(paramalert,0,128);
						sprintf(paramalert,"Hansi heartbeatlink %s",search(lpublic,"param_conf_succ"));
						ShowAlert(paramalert);
					}
					else
					{
						memset(paramalert,0,128);
						sprintf(paramalert,"Hansi heartbeatlink %s",search(lpublic,"param_conf_fail"));
						ShowAlert(paramalert);
					}
				}
			}
			else
			{
				memset(paramalert,0,128);
				sprintf(paramalert,"Hansi profile %s",search(lpublic,"param_conf_fail"));
				ShowAlert(paramalert);
			}
		}
	}
}

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>VRRP</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=hansiadd style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_hansilist.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
							"<td align=left id=tdleft><a href=wp_hansilist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"vr_list"));
						fprintf(cgiOut,"</tr>"); 
						
						fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_hansiidlist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"hs_list"));
						fprintf(cgiOut,"</tr>"); 

						 fprintf(cgiOut,"<tr height=26>"\
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"hs_create"));   /*突出显示*/
					fprintf(cgiOut,"</tr>");
					
						
			for(i=0;i<15;i++)
			{
				fprintf(cgiOut,"<tr height=25>"\
				"<td id=tdleft>&nbsp;</td>"\
				"</tr>");
			}
			fprintf(cgiOut,"</table>"\
			"</td>"\
			"<td align=left  valign=top border=0 style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:0px;\">");
			fprintf(cgiOut,"<table width=600 border=0 cellspacing=0 cellpadding=0>");
			fprintf(cgiOut,"<tr align=left>");
		    fprintf(cgiOut,"<td valign=top>SLOT ID:");
			fprintf( cgiOut, "<select name=insid onchange=slotid_change(this)>");
			for(p_q=paraHead2;(NULL != p_q);p_q=p_q->next)
			{
				if(p_q->parameter.slot_id == pid)
				{
					fprintf(cgiOut,"<option value=\"%d\" selected>%d</option>",p_q->parameter.slot_id,p_q->parameter.slot_id);
				}
				else
				{
					fprintf(cgiOut,"<option value=\"%d\">%d</option>",p_q->parameter.slot_id,p_q->parameter.slot_id);
				}		
			}
			fprintf( cgiOut, "</select>\n");	
			fprintf(cgiOut,"</td>");
			fprintf(cgiOut,"</tr>");
			fprintf( cgiOut,"<script type=text/javascript>\n");
			fprintf( cgiOut,"function slotid_change( obj )\n"\
			"{\n"\
			"var slotid = obj.options[obj.selectedIndex].value;\n"\
			"var url = 'wp_hansiadd.cgi?UN=%s&plotid='+slotid;\n"\
			"window.location.href = url;\n"\
			"}\n", m);
			fprintf( cgiOut,"</script>\n" );
			fprintf(cgiOut,"<tr height=30>\n");
			fprintf(cgiOut,"<td width=140>\n");
			fprintf(cgiOut,"%s:",search(lpublic,"hs_conftype"));
			fprintf(cgiOut,"</td>\n");
			fprintf(cgiOut,"<td colspan=3>\n");
			fprintf(cgiOut,"<select name=udtype onchange=port_sel_change(this)>");
			if(strcmp(n,"0")==0)
			fprintf(cgiOut,"<option value='0' selected=selected>%s</option>",search(lpublic,"hs_profile"));
			else
			fprintf(cgiOut,"<option value='0'>%s</option>",search(lpublic,"hs_profile"));
			if(strcmp(n,"1")==0)
			fprintf(cgiOut,"<option value='1' selected=selected>%s/%s</option>\n",search(lpublic,"hs_uplinks"),search(lpublic,"hs_downs"));
			else
			fprintf(cgiOut,"<option value='1'>%s/%s</option>\n",search(lpublic,"hs_uplinks"),search(lpublic,"hs_downs"));
			
			if(strcmp(n,"2")==0)
			fprintf(cgiOut,"<option value='2' selected=selected>%s</option>\n",search(lpublic,"hs_uplinks"));
			else
			fprintf(cgiOut,"<option value='2'>%s</option>\n",search(lpublic,"hs_uplinks"));
			
			if(strcmp(n,"3")==0)
			fprintf(cgiOut,"<option value='3' selected=selected>%s</option>\n",search(lpublic,"hs_downs"));
			else
			fprintf(cgiOut,"<option value='3'>%s</option>\n",search(lpublic,"hs_downs"));

			if(strcmp(n,"4")==0)
			fprintf(cgiOut,"<option value='4' selected=selected>%s</option>\n",search(lpublic,"hs_hblink"));
			else
			fprintf(cgiOut,"<option value='4'>%s</option>\n",search(lpublic,"hs_hblink"));
			
			fprintf(cgiOut,"</select>\n");
			fprintf(cgiOut,"</td>\n");
			fprintf(cgiOut,"</tr>\n");
			fprintf(cgiOut,"<script type=text/javascript>\n");
            fprintf(cgiOut,"function port_sel_change( obj )\n"\
	           	"{\n"\
		           	"var selectz = obj.options[obj.selectedIndex].value;\n"\
		           	"var url = 'wp_hansiadd.cgi?UN=%s&plotid=%d&SZ='+selectz;\n"\
		           	"window.location.href = url;\n"\
	           	"}\n", m,pid);
            fprintf(cgiOut,"</script>\n" );
			
			
				//pro
				fprintf(cgiOut,"<tr height=30>"\
				"<td width=140>%s:</td>",search(lpublic,"hs_profile"));            
				fprintf(cgiOut,"<td colspan=3><input name=profile maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" "\
				"onpaste=\"var s=clipboardData.getData('text'); if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\""\
				"ondragenter=\"return  false;\""\
				"style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" type=text size=8 value=\"\"/><font color=red>(1--16)</font></td>"\
				"</tr>");
			    if((strcmp(n,"1")==0) ||(strcmp(n,"2")==0))
				{
			        //up   link ifname
					fprintf(cgiOut,"<tr  height=30>"\
					"<td width=140>%s/IP/Mask :</td>",search(lpublic,"hs_uplink"));
					fprintf(cgiOut,"<td><input type=text name=upname style=\"width:80px\"></td>");
					fprintf(cgiOut,"&nbsp;&nbsp;<td>"\
					"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
					fprintf(cgiOut,"<input type=text  name=upip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
					fprintf(cgiOut,"<input type=text  name=upip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text  name=upip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text  name=upip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
					fprintf(cgiOut,"</div></td>");
					fprintf(cgiOut,"&nbsp;&nbsp;<td>"\
					"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
					fprintf(cgiOut,"<input type=text  name=upmk1 id=upmk1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
					fprintf(cgiOut,"<input type=text  name=upmk2 id=upmk2  value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text  name=upmk3 id=upmk3  value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text  name=upmk4 id=upmk4  value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
					fprintf(cgiOut,"</div></td>"\
					"</tr>");
				}
				if((strcmp(n,"1")==0) ||(strcmp(n,"3")==0))
				{
					//  down link ifname
					fprintf(cgiOut,"<tr  height=30>"\
					"<td width=140>%s/IP/Mask :</td>",search(lpublic,"hs_dlink"));
					fprintf(cgiOut,"<td><input type=text name=downname style=\"width:80px\"></td>");
					fprintf(cgiOut,"&nbsp;&nbsp;<td>"\
					"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
					fprintf(cgiOut,"<input type=text  name=downip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
					fprintf(cgiOut,"<input type=text  name=downip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text  name=downip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text  name=downip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
					fprintf(cgiOut,"</div></td>");
					fprintf(cgiOut,"&nbsp;&nbsp;<td>"\
					"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
					fprintf(cgiOut,"<input type=text  name=dwmk1 id=dwmk1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
					fprintf(cgiOut,"<input type=text  name=dwmk2 id=dwmk2  value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text  name=dwmk3 id=dwmk3  value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text  name=dwmk4 id=dwmk4  value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
					fprintf(cgiOut,"</div></td>");

                    fprintf(cgiOut,"</tr>\n");
				}
				if((strcmp(n,"4")!=0)&&(strcmp(n,"0")!=0))
				{
					fprintf(cgiOut,"<tr  height=30>\n");
					fprintf(cgiOut,"<td width=140>%s:</td>\n",search(lcontrol,"prior"));
					fprintf(cgiOut,"&nbsp;&nbsp;<td colspan=3><input name=linkprio maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" "\
					"onpaste=\"var s=clipboardData.getData('text'); if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\""\
					"ondragenter=\"return  false;\""\
					"style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" type=text size=8 value=\"\"/><font color=red>(1--255)</font></td>");
					fprintf(cgiOut,"</tr>");
				}
				if(strcmp(n,"0")!=0)
				{
	                //heartbeaklink 
					fprintf(cgiOut,"<tr  height=30>"\
					"<td width=140>%s %s/IP :</td>",search(lpublic,"hs_hblink"),search(lpublic,"inter"));
					fprintf(cgiOut,"<td><input type=text name=hbifname style=\"width:80px\"></td>");
					fprintf(cgiOut,"&nbsp;&nbsp;<td colspan=2>"\
					"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
					fprintf(cgiOut,"<input type=text  name=hbip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
					fprintf(cgiOut,"<input type=text  name=hbip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text  name=hbip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text  name=hbip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
					fprintf(cgiOut,"</div></td>");
					fprintf(cgiOut,"</tr>\n");
				}

  fprintf(cgiOut,"<input type=hidden name=encry_newvrrp value=%s>",m);
  fprintf(cgiOut,"<input type=hidden name=SZ value=%s>\n",n);
  fprintf(cgiOut,"<input type=hidden name=plotid value=%d>",pid);
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
free_instance_parameter_list(&paraHead2); 
return 0;
}
