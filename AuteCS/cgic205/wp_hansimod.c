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
* wp_hansimod.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for vrrp mod
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
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"

unsigned char *ccgi_vrrp_err_msg[] = {	\
/*   0 */	"Vrrp_Error_none",
/*   1 */	"Vrrp_General_failure",
/*   2 */	"Vrrp_Profile_Out_Of_Range",
/*   3 */	"Vrrp_Profile_Has_Already_Exist",
/*   4 */	"Vrrp_Profile_Not_Exist",
/*   5 */	"Vrrp_Memory_Malloc_Failed",
/*   6 */	"Vrrp_Bad_Parameter_Input",
/*   7 */	"Vrrp_Not_Configured",
/*	 8 */	"Vrrp_Disabled_First",
/*   9 */	"Vrrp_Virtual_gateway_Error",
/*	 A */	"Vrrp_Not_Appropriate_Mode",
/*	 B */	"Vrrp_Interface_Name_Error",
/*	 C */	"Vrrp_Virtual_IP_Setted",
/*	 D */	"Vrrp_Virtual_IP_Not_Setted",
/*	 E */	"Vrrp_Virtual_IP_Delete_Error",
/*	 F */	"Vrrp_Interface_Has_Exist",
/*	 10 */	"Vrrp_Interface_Not_Exist",
/*    11 */	"Vrrp_No_More_Items",
/*	 12 */	"Vrrp_Virtual_Mac_Should_Be_Enable",
};

int ShowHansiModPage(char *m,char *id,char *choice,struct list *lpublic,struct list *lcontrol);    
void ShowHansiClearPage(char *m,char *id,struct list *lpublic,struct list *lcontrol);

int cgiMain()
{  
	char *encry=(char *)malloc(BUF_LEN);  
	char *str;   
	struct list *lpublic;   /*解析public.txt文件的链表头*/
	struct list *lcontrol;     /*解析wlan.txt文件的链表头*/  
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");
	char hsid[10];
	memset(hsid,0,10);
	char hstype[10];
	memset(hstype,0,10);
	char macChoice[10]; 
	memset(macChoice,0,10);
	cgiFormStringNoNewlines("SZ",macChoice,10);
	if(strcmp(macChoice,"")==0)
	 strcpy(macChoice,"1");
	ccgi_dbus_init();/*初始化dbus*/
	cgiFormStringNoNewlines("ID",hsid,10);
	cgiFormStringNoNewlines("TYPE",hstype,10);

	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
	{
		str=dcryption(encry);
		if(str==NULL)
			ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
		else 
		{
			if(strcmp(hstype,"1")==0)
				ShowHansiModPage(encry,hsid,macChoice,lpublic,lcontrol);
			else if(strcmp(hstype,"2")==0)
				ShowHansiClearPage(encry,hsid,lpublic,lcontrol);
		}

	}
	else                    
	{      
		cgiFormStringNoNewlines("encry_newvrrp",encry,BUF_LEN);
		str=dcryption(encry);	
		if(str==NULL)
			ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
		else 
		{
			if(strcmp(hstype,"1")==0)
				ShowHansiModPage(encry,hsid,macChoice,lpublic,lcontrol);
			else if(strcmp(hstype,"2")==0)
				ShowHansiClearPage(encry,hsid,lpublic,lcontrol);
		}

	} 
	free(encry);
	release(lpublic);  
	release(lcontrol);
	return 0;
}

int ShowHansiModPage(char *m,char *id,char * choice,struct list *lpublic,struct list *lcontrol)
{  
	int i,retu,hspro_num=0,upmaskbit,dwmaskbit;

    char ulname[10];
	memset(ulname,0,10);

	char dlname[10];
	memset(dlname,0,10);
	
	char vgatewayname[10];
	char vgateway1[4],vgateway2[4],vgateway3[4],vgateway4[4];
	char vgmk1[4];
	char vgmk2[4];
	char vgmk3[4];
	char vgmk4[4];
	char vgateway[30];
	char vgmk[HANSIIPL];
	memset(vgatewayname,0,10);	
	memset(vgateway1,0,4);
	memset(vgateway2,0,4);
	memset(vgateway3,0,4);
	memset(vgateway4,0,4);
	memset(vgmk1,0,4);
	memset(vgmk2,0,4);
	memset(vgmk3,0,4);
	memset(vgmk4,0,4);
	memset(vgateway,0,30);
	memset(vgmk,0,HANSIIPL);

	char ulip[30];
	memset(ulip,0,30);

	char hbip[30];
	memset(hbip,0,30);

	char dlip[30];
	memset(dlip,0,30);

	char hip1[4],hip2[4],hip3[4],hip4[4];
	memset(hip1,0,4);
	memset(hip2,0,4);
	memset(hip3,0,4);
	memset(hip4,0,4);
	
	char uip1[4],uip2[4],uip3[4],uip4[4];
	memset(uip1,0,4);
	memset(uip2,0,4);
	memset(uip3,0,4);
	memset(uip4,0,4);
	

	char dip1[4],dip2[4],dip3[4],dip4[4];
	memset(dip1,0,4);
	memset(dip2,0,4);
	memset(dip3,0,4);
	memset(dip4,0,4);

	char hsprio[10];
	memset(hsprio,0,10);

	char hstime[10];
	memset(hstime,0,10);

	char hblink[30];
	memset(hblink,0,30);

	char vmac[10];
	memset(vmac,0,10);
	/*function return or params convert*/
	int op_ret=-1,convert_num=-1;
	char paramalert[128];
	memset(paramalert,0,128);

	Z_VRRP zvrrp;
	memset(&zvrrp,0,sizeof(zvrrp));

	char upmask[HANSIIPL];
	memset(upmask,0,HANSIIPL);
	char downmask[HANSIIPL];
	memset(downmask,0,HANSIIPL);
	char upmk1[4];
	char upmk2[4];
	char upmk3[4];
	char upmk4[4];
	memset(upmk1,0,4);
	memset(upmk2,0,4);
	memset(upmk3,0,4);
	memset(upmk4,0,4);
	char dwmk1[1];
	char dwmk2[4];
	char dwmk3[4];
	char dwmk4[4];
	memset(dwmk1,0,4);
	memset(dwmk2,0,4);
	memset(dwmk3,0,4);
	memset(dwmk4,0,4);


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
		//vgateway
		
		cgiFormStringNoNewlines("vgatewayname",vgatewayname,10);	
		cgiFormStringNoNewlines("vgateway1",vgateway1,10);	
		cgiFormStringNoNewlines("vgateway2",vgateway2,10);	
		cgiFormStringNoNewlines("vgateway3",vgateway3,10);	
		cgiFormStringNoNewlines("vgateway4",vgateway4,10);	
		sprintf(vgateway,"%ld.%ld.%ld.%ld",strtoul(vgateway1,0,10),strtoul(vgateway2,0,10),strtoul(vgateway3,0,10),strtoul(vgateway4,0,10));
		//fprintf(stderr,"vgateway1=%s,vgateway=%s",vgateway1,vgateway);
		cgiFormStringNoNewlines("vgmk1",vgmk1,10);	
		cgiFormStringNoNewlines("vgmk2",vgmk2,10);	
		cgiFormStringNoNewlines("vgmk3",vgmk3,10);	
		cgiFormStringNoNewlines("vgmk4",vgmk4,10);
		sprintf(vgmk,"%ld.%ld.%ld.%ld",strtoul(vgmk1,0,10),strtoul(vgmk2,0,10),strtoul(vgmk3,0,10),strtoul(vgmk4,0,10));
		
		//up or down link 
		cgiFormStringNoNewlines("upname",ulname,10);
		cgiFormStringNoNewlines("downname",dlname,10);

		memset(uip1,0,4);
		memset(uip2,0,4);
		memset(uip3,0,4);
		memset(uip4,0,4);

		memset(hip1,0,4);
		memset(hip2,0,4);
		memset(hip3,0,4);
		memset(hip4,0,4);

		memset(dip1,0,4);
		memset(dip2,0,4);
		memset(dip3,0,4);
		memset(dip4,0,4);

		memset(ulip,0,30);
		memset(dlip,0,30);

		cgiFormStringNoNewlines("upip1",uip1,4);
		cgiFormStringNoNewlines("upip2",uip2,4);
		cgiFormStringNoNewlines("upip3",uip3,4);
		cgiFormStringNoNewlines("upip4",uip4,4);
		sprintf(ulip,"%ld.%ld.%ld.%ld",strtoul(uip1,0,10),strtoul(uip2,0,10),strtoul(uip3,0,10),strtoul(uip4,0,10));

		cgiFormStringNoNewlines("downip1",dip1,4);
		cgiFormStringNoNewlines("downip2",dip2,4);
		cgiFormStringNoNewlines("downip3",dip3,4);
		cgiFormStringNoNewlines("downip4",dip4,4);
		sprintf(dlip,"%ld.%ld.%ld.%ld",strtoul(dip1,0,10),strtoul(dip2,0,10),strtoul(dip3,0,10),strtoul(dip4,0,10));

		cgiFormStringNoNewlines("hbip1",hip1,4);
		cgiFormStringNoNewlines("hbip2",hip2,4);
		cgiFormStringNoNewlines("hbip3",hip3,4);
		cgiFormStringNoNewlines("hbip4",hip4,4);
		sprintf(hbip,"%ld.%ld.%ld.%ld",strtoul(hip1,0,10),strtoul(hip2,0,10),strtoul(hip3,0,10),strtoul(hip4,0,10));

		memset(hsprio,0,10);
		cgiFormStringNoNewlines("hsprio",hsprio,10);

		memset(hblink,0,30);
		cgiFormStringNoNewlines("hblink",hblink,30);

		memset(hstime,0,10);
		cgiFormStringNoNewlines("hstime",hstime,10);

		memset(vmac,0,10);
		cgiFormStringNoNewlines("vmac",vmac,10);


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
		cgiFormStringNoNewlines("dwmk1",dwmk1,4);
		cgiFormStringNoNewlines("dwmk2",dwmk2,4);
		cgiFormStringNoNewlines("dwmk3",dwmk3,4);
		cgiFormStringNoNewlines("dwmk4",dwmk4,4);
		sprintf(downmask,"%ld.%ld.%ld.%ld",strtoul(dwmk1,0,10),strtoul(dwmk2,0,10),strtoul(dwmk3,0,10),strtoul(dwmk4,0,10));



		op_ret=ccgi_config_hansi_profile(id);
		if(op_ret==0)
		{
		    //up and down
		    if(strcmp(choice,"1")==0) 
		    {
				if((strcmp(ulname,"")!=0) && (strcmp(dlname,"") !=0)&& (strcmp(hsprio,"") !=0)&& (strcmp(uip1,"") !=0)&& (strcmp(uip2,"") !=0)&& (strcmp(uip3,"") !=0)&& (strcmp(uip4,"") !=0)&& (strcmp(dip1,"") !=0)&& (strcmp(dip2,"") !=0)&& (strcmp(dip3,"") !=0)&& (strcmp(dip4,"") !=0)&&(strcmp(upmk1,"") !=0)&&(strcmp(upmk2,"") !=0)&&(strcmp(upmk3,"") !=0)&&(strcmp(upmk4,"") !=0)&&(strcmp(dwmk1,"") !=0)&&(strcmp(dwmk2,"") !=0)&&(strcmp(dwmk3,"") !=0)&&(strcmp(dwmk4,"") !=0))
				{
				    convert_num=strtoul(hsprio,0,10);
					if(convert_num < 1 || convert_num > 255)
		                  ShowAlert(search(lpublic,"hs_param"));
					else
					{
						upmaskbit=mask_bit(upmask);
						dwmaskbit=mask_bit(downmask);
						if((upmaskbit == -1)||(dwmaskbit == -1))
							ShowAlert(search(lcontrol,"dhcp_mask_err"));
                        else
                    	{
							op_ret=ccgi_downanduplink_ifname_mask(id,ulname, ulip, dlname, dlip, hsprio,upmaskbit,dwmaskbit);
							if(op_ret==0)
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_succ"));
								ShowAlert(paramalert);

							}
							else
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s:%s",search(lpublic,"param_conf_fail"),search(lpublic,ccgi_vrrp_err_msg[op_ret]));
								ShowAlert(paramalert);
							}
                    	}
					}
				}
				else					
				 ShowAlert(search(lpublic,"hs_param"));
		    }
			//uplink
			if(strcmp(choice,"2")==0) 
		    {
				if((strcmp(ulname,"")!=0) &&  (strcmp(hsprio,"") !=0)&& (strcmp(uip1,"") !=0)&& (strcmp(uip2,"") !=0)&& (strcmp(uip3,"") !=0)&& (strcmp(uip4,"") !=0)&&(strcmp(upmk1,"") !=0)&&(strcmp(upmk2,"") !=0)&&(strcmp(upmk3,"") !=0)&&(strcmp(upmk4,"") !=0))
				{
				    convert_num=strtoul(hsprio,0,10);
					if(convert_num < 1 || convert_num > 255)
		                  ShowAlert(search(lpublic,"hs_param"));
					else
					{
					    upmaskbit=mask_bit(upmask);
						if(upmaskbit == -1)
							ShowAlert(search(lcontrol,"dhcp_mask_err"));
                        else
                    	{
							op_ret=config_vrrp_uplink(id,ulname, ulip, hsprio,upmaskbit);
							if(op_ret==0)
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_succ"));
								ShowAlert(paramalert);

							}
							else
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s:%s",search(lpublic,"param_conf_fail"),search(lpublic,ccgi_vrrp_err_msg[op_ret]));
								ShowAlert(paramalert);
							}
                    	}
					}
				}
				else					
				 ShowAlert(search(lpublic,"hs_param"));
		    }
			//downlink
			if(strcmp(choice,"3")==0) 
		    {
				if( (strcmp(dlname,"") !=0)&& (strcmp(hsprio,"") !=0)&& (strcmp(dip1,"") !=0)&& (strcmp(dip2,"") !=0)&& (strcmp(dip3,"") !=0)&& (strcmp(dip4,"") !=0)&&(strcmp(dwmk1,"") !=0)&&(strcmp(dwmk2,"") !=0)&&(strcmp(dwmk3,"") !=0)&&(strcmp(dwmk4,"") !=0))
				{
				    convert_num=strtoul(hsprio,0,10);
					if(convert_num < 1 || convert_num > 255)
		                  ShowAlert(search(lpublic,"hs_param"));
					else
					{
						dwmaskbit=mask_bit(downmask);
						if(dwmaskbit == -1)
							ShowAlert(search(lcontrol,"dhcp_mask_err"));
						else
						{
							op_ret=config_vrrp_downlink_mask(id,dlname,dlip, hsprio,dwmaskbit);
							if(op_ret==0)
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_succ"));
								ShowAlert(paramalert);

							}
							else
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s:%s",search(lpublic,"param_conf_fail"),search(lpublic,ccgi_vrrp_err_msg[op_ret]));
								ShowAlert(paramalert);
							}
						}
					}
				}
				else					
				 ShowAlert(search(lpublic,"hs_param"));
		    }
	    //single 
	    if(strcmp(choice,"4")==0)
	    {
			//deal with hansi priority
			if(strcmp(hsprio,"")!=0)
			{
				convert_num=strtoul(hsprio,0,10);
				if(convert_num < 1 || convert_num > 255)
					ShowAlert(search(lpublic,"hs_param"));
				else
				{
					op_ret=ccgi_config_hansi_priority(id,hsprio);
					if(op_ret==0)
					{
						memset(paramalert,0,128);
						sprintf(paramalert,"Hansi priority %s",search(lpublic,"param_conf_succ"));
						ShowAlert(paramalert);
					}
					else
					{
						memset(paramalert,0,128);
						sprintf(paramalert,"Hansi priority %s:%s",search(lpublic,"param_conf_fail"),search(lpublic,ccgi_vrrp_err_msg[op_ret]));
						ShowAlert(paramalert);
					}
				}
			}
	    }
	    //single
	    if(strcmp(choice,"5")==0)
	    {
			//deal with hansi advertime
			if(strcmp(hstime,"")!=0)
			{
				convert_num=strtoul(hstime,0,10);
				if(convert_num < 1 || convert_num > 255)
					ShowAlert(search(lpublic,"hs_param"));
				else
				{
					op_ret=ccgi_config_hansi_advertime(id,hstime);
					if(op_ret==0)
					{
						memset(paramalert,0,128);
						sprintf(paramalert,"Hansi advertime %s",search(lpublic,"param_conf_succ"));
						ShowAlert(paramalert);
					}
					else
					{
						memset(paramalert,0,128);
						sprintf(paramalert,"Hansi advertime %s:%s",search(lpublic,"param_conf_fail"),search(lpublic,ccgi_vrrp_err_msg[op_ret]));
						ShowAlert(paramalert);
					}
				}
			}
	    }
		//single
	    if(strcmp(choice,"6")==0)
	    {
			//deal with hansi heartbeatlink
			if(strcmp(hblink,"")!=0)
			{
				op_ret=config_vrrp_heartbeat_cmd_func(id, hblink, hbip);
				if(op_ret==0)
				{
					memset(paramalert,0,128);
					sprintf(paramalert,"Hansi heartbeatlink %s",search(lpublic,"param_conf_succ"));
					ShowAlert(paramalert);
				}
				else
				{
					memset(paramalert,0,128);
					sprintf(paramalert,"Hansi heartbeatlink %s:%s",search(lpublic,"param_conf_fail"),search(lpublic,ccgi_vrrp_err_msg[op_ret]));
					ShowAlert(paramalert);
				}
			}
	    }
		if(strcmp(choice,"7")==0) 
		    {
		    	int dwmaskbit=0;
				char mask_tmp[20] = {0};
				char del_add[10]={0};
				dwmaskbit=mask_bit(vgmk);
		    	
				cgiFormStringNoNewlines("del_add",del_add,10);
		  //  	fprintf(stderr,"vbvb vgatewayname=%s,vgateway=%s,vgmk=%s",vgatewayname,vgateway,vgmk);
				if( (strcmp(vgatewayname,"") !=0)&&(strcmp(vgateway1,"") !=0)&& (strcmp(vgateway2,"") !=0)&& (strcmp(vgateway3,"") !=0)&& (strcmp(vgateway4,"") !=0)&&(strcmp(vgmk1,"") !=0)&&(strcmp(vgmk2,"") !=0)&&(strcmp(vgmk3,"") !=0)&&(strcmp(vgmk4,"") !=0))
				{
						if(dwmaskbit == -1)
							ShowAlert(search(lcontrol,"dhcp_mask_err"));
						else
						{
							sprintf(mask_tmp,"%s/%d",vgateway,dwmaskbit);
							
							//op_ret=config_vrrp_gateway(id,vgatewayname,mask_tmp);
							op_ret=config_vrrp_link_add_vip(del_add,id,"vgateway",vgatewayname,mask_tmp);
							//fprintf(stderr,"op_ret=%d\n",op_ret);
							if(op_ret==0) 
							{
								ShowAlert(search(lpublic,"vgateway_conf_succ"));

							}							
							else
							{
								if(op_ret==-4) 
								{
									ShowAlert(search(lpublic,"hs_param"));

								}
								if(op_ret==-5) 
								{
									ShowAlert(search(lpublic,"ip_error"));

								}
								else
								{
									memset(paramalert,0,128);
									sprintf(paramalert,"%s:%s",search(lpublic,"vgateway_conf_failure"),search(lpublic,ccgi_vrrp_err_msg[op_ret]));
									ShowAlert(paramalert);
								}
							}
						}
					
				}
				else					
				 ShowAlert(search(lpublic,"hs_param"));
				
		    }
	   
		}
		else
		{
			memset(paramalert,0,128);
			sprintf(paramalert,"Hansi profile %s:%s",search(lpublic,"param_conf_fail"),search(lpublic,ccgi_vrrp_err_msg[op_ret]));
			ShowAlert(paramalert);
		}
	}  

	if(cgiFormSubmitClicked("starts") == cgiFormSuccess)
	{
		retu=config_vrrp_service(id, "enable");
		if(retu==0)
		{
			ShowAlert(search(lpublic,"oper_succ"));
		}
		else
		{
			ShowAlert(search(lpublic,"oper_fail"));
		}
	}
	if(cgiFormSubmitClicked("ends") == cgiFormSuccess)
	{
		retu=config_vrrp_service(id, "disable");
		if(retu==0)
		{
			ShowAlert(search(lpublic,"oper_succ"));
		}
		else
		{
			ShowAlert(search(lpublic,"oper_fail"));
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


	fprintf(cgiOut,"<tr height=26>"\
	"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"hs_conf"));   /*突出显示*/
	fprintf(cgiOut,"</tr>");

	for(i=0;i<7;i++)
	{
		fprintf(cgiOut,"<tr height=25>"\
		"<td id=tdleft>&nbsp;</td>"\
		"</tr>");
	}
	fprintf(cgiOut,"</table>"\
	"</td>"\
	"<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
	"<table width=600 border=0 cellspacing=0 cellpadding=0>");

	hspro_num=strtoul(id,0,10);
	ccgi_show_hansi_profile(&zvrrp, hspro_num);    
	//ccgi_show_hansi_profile_detail(&zvrrp, hspro_num);     

	fprintf(cgiOut,"<tr  height=30>\n");
	fprintf(cgiOut,"<td>\n");
	fprintf(cgiOut,"<input type=submit name=starts value=\"%s\">",search(lpublic,"hs_starts"));
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"<td colspan=3>\n");
	fprintf(cgiOut,"<input type=submit name=ends value=\"%s\">",search(lpublic,"hs_ends"));
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"</tr>\n");

    fprintf(cgiOut,"<tr height=30>\n"
		"<td width=140>%s</td>"\
		"<td colspan=3>%s</td>"\
		"</tr>",search(lpublic,"hs_profile"),id);
	fprintf(cgiOut,"<tr height=30>\n");
	fprintf(cgiOut,"<td width=140>\n");
	fprintf(cgiOut,"%s:",search(lpublic,"hs_conftype"));
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"<td colspan=3>\n");
	fprintf(cgiOut,"<select name=udtype style=\"width:80px\" onchange=port_sel_change(this)>");
	if(strcmp(choice,"1")==0)
	fprintf(cgiOut,"<option value='1' selected=selected>%s/%s</option>\n",search(lpublic,"hs_uplinks"),search(lpublic,"hs_downs"));
	else
	fprintf(cgiOut,"<option value='1'>%s/%s</option>\n",search(lpublic,"hs_uplinks"),search(lpublic,"hs_downs"));
	
	if(strcmp(choice,"2")==0)
	fprintf(cgiOut,"<option value='2' selected=selected>%s</option>\n",search(lpublic,"hs_uplinks"));
	else
	fprintf(cgiOut,"<option value='2'>%s</option>\n",search(lpublic,"hs_uplinks"));
	
	if(strcmp(choice,"3")==0)
	fprintf(cgiOut,"<option value='3' selected=selected>%s</option>\n",search(lpublic,"hs_downs"));
	else
	fprintf(cgiOut,"<option value='3'>%s</option>\n",search(lpublic,"hs_downs"));
	
	if(strcmp(choice,"4")==0)
	fprintf(cgiOut,"<option value='4' selected=selected>%s</option>\n",search(lcontrol,"prior"));
	else
	fprintf(cgiOut,"<option value='4'>%s</option>\n",search(lcontrol,"prior"));

	if(strcmp(choice,"5")==0)
	fprintf(cgiOut,"<option value='5' selected=selected>%s</option>\n",search(lpublic,"hs_adtime"));
	else
	fprintf(cgiOut,"<option value='5'>%s</option>\n",search(lpublic,"hs_adtime"));
	
	if(strcmp(choice,"6")==0)
	fprintf(cgiOut,"<option value='6' selected=selected>%s</option>\n",search(lpublic,"hs_hblink"));
	else
	fprintf(cgiOut,"<option value='6'>%s</option>\n",search(lpublic,"hs_hblink"));

	if(strcmp(choice,"7")==0)
		fprintf(cgiOut,"<option value='7' selected=selected>%s</option>\n",search(lpublic,"vgateway"));
	else
		fprintf(cgiOut,"<option value='7'>%s</option>\n",search(lpublic,"vgateway"));
	
	fprintf(cgiOut,"</select>\n");
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"</tr>\n");
	fprintf(cgiOut,"<script type=text/javascript>\n");
    fprintf(cgiOut,"function port_sel_change( obj )\n"\
       	"{\n"\
           	"var selectz = obj.options[obj.selectedIndex].value;\n"\
           	"var url = 'wp_hansimod.cgi?UN=%s&ID=%s&TYPE=%s&SZ='+selectz;\n"\
           	"window.location.href = url;\n"\
       	"}\n", m,id,"1");
    fprintf(cgiOut,"</script>\n" );
	if((strcmp(choice,"1")==0) || (strcmp(choice,"2")==0))
	{
		//up   link ifname
		fprintf(cgiOut,"<tr  height=30>"\
		"<td width=140>%s/IP/Mask :</td>",search(lpublic,"hs_uplink"));
		fprintf(cgiOut,"<td><input type=text name=upname style=\"width:80px\" value=\"%s\"></td>",zvrrp.uplink_ifname);
		fprintf(cgiOut,"<td>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=upip1  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),zvrrp.ulip1); 
		fprintf(cgiOut,"<input type=text  name=upip2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),zvrrp.ulip2);
		fprintf(cgiOut,"<input type=text  name=upip3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),zvrrp.ulip3);
		fprintf(cgiOut,"<input type=text  name=upip4  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">",search(lpublic,"ip_error"),zvrrp.ulip4);
		fprintf(cgiOut,"</div></td>");
		fprintf(cgiOut,"<td>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=upmk1 id=upmk1  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\">.",search(lpublic,"ip_error"),""); 
		fprintf(cgiOut,"<input type=text  name=upmk2 id=upmk2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\">.",search(lpublic,"ip_error"),"");
		fprintf(cgiOut,"<input type=text  name=upmk3 id=upmk3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\">.",search(lpublic,"ip_error"),""); 
		fprintf(cgiOut,"<input type=text  name=upmk4 id=upmk4  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\">",search(lpublic,"ip_error"),""); 
		fprintf(cgiOut,"</div></td>"\
		"</tr>");
	}
	if((strcmp(choice,"1")==0) || (strcmp(choice,"3")==0))
	{
		//  down link ifname
		fprintf(cgiOut,"<tr  height=30>"\
		"<td width=140>%s/IP/Mask :</td>",search(lpublic,"hs_dlink"));
		fprintf(cgiOut,"<td><input type=text name=downname style=\"width:80px\" value=\"%s\"></td>",zvrrp.downlink_ifname);
		fprintf(cgiOut,"<td>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=downip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),zvrrp.dlip1); 
		fprintf(cgiOut,"<input type=text  name=downip2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),zvrrp.dlip2);
		fprintf(cgiOut,"<input type=text  name=downip3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),zvrrp.dlip3);
		fprintf(cgiOut,"<input type=text  name=downip4  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">",search(lpublic,"ip_error"),zvrrp.dlip4);
		fprintf(cgiOut,"</div></td>");
		fprintf(cgiOut,"<td>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=dwmk1 id=dwmk1  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\">.",search(lpublic,"ip_error"),""); 
		fprintf(cgiOut,"<input type=text  name=dwmk2 id=dwmk2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\">.",search(lpublic,"ip_error"),""); 
		fprintf(cgiOut,"<input type=text  name=dwmk3 id=dwmk3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\">.",search(lpublic,"ip_error"),""); 
		fprintf(cgiOut,"<input type=text  name=dwmk4 id=dwmk4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%s\">",search(lpublic,"ip_error"),""); 
		fprintf(cgiOut,"</div></td>");
		fprintf(cgiOut,"</tr>");
	}
	if((strcmp(choice,"1")==0) || (strcmp(choice,"3")==0)||(strcmp(choice,"2")==0)||(strcmp(choice,"4")==0))
	{
		//priority
		fprintf(cgiOut,"<tr height=30>"\
		"<td width=140>%s:</td>",search(lcontrol,"prior"));
		fprintf(cgiOut,"<td colspan=3><input name=hsprio maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\""\
		"onpaste=\"var s=clipboardData.getData('text'); if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\""\
		"ondragenter=\"return  false;\" "\
		"style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" type=text size=8 value=\"%d\"/><font color=red>(1--255)</font></td>"\
		"</tr>",zvrrp.priority);
	}
   if(strcmp(choice,"5")==0)
   {

		//advertime
		fprintf(cgiOut,"<tr height=30>"\
		"<td width=140>%s:</td>",search(lpublic,"hs_adtime"));
		fprintf(cgiOut,"<td colspan=3><input name=hstime maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\""\
		"onpaste=\"var s=clipboardData.getData('text'); if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\""\
		"ondragenter=\"return  false;\" "\
		"style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" type=text size=8 value=\"%d\"/><font color=red>(1--255)</font></td>"\
		"</tr>",zvrrp.advert);
   	}
   if(strcmp(choice,"6")==0)
   {
		//heart blink
		fprintf(cgiOut,"<tr  height=30>"\
		"<td width=140>%s/IP :</td>",search(lpublic,"inter"));
		fprintf(cgiOut,"<td><input type=text name=hblink style=\"width:80px\" value=\"%s\"></td>",zvrrp.hbinf);
		fprintf(cgiOut,"<td colspan=2>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=hbip1  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),zvrrp.hbip1); 
		fprintf(cgiOut,"<input type=text  name=hbip2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),zvrrp.hbip2);
		fprintf(cgiOut,"<input type=text  name=hbip3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),zvrrp.hbip3);
		fprintf(cgiOut,"<input type=text  name=hbip4  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">",search(lpublic,"ip_error"),zvrrp.hbip4);
		fprintf(cgiOut,"</div></td>"\
		"</tr>");
   	}

	if((strcmp(choice,"7")==0))
	{
		//  vgateway ifname
		fprintf(cgiOut,"<tr  height=30>");
		fprintf(cgiOut,"<td width=140>%s/IP/Mask :</td>",search(lpublic,"port"));
		fprintf(cgiOut,"<td><input type=text name=vgatewayname style=\"width:80px\" value=\"\"></td>");
		fprintf(cgiOut,"<td>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");

		fprintf(cgiOut,"<input type=text  name=vgateway1  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">." ,search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=vgateway2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">." ,search(lpublic,"ip_error"));
		fprintf(cgiOut,"<input type=text  name=vgateway3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">." ,search(lpublic,"ip_error"));
		fprintf(cgiOut,"<input type=text  name=vgateway4  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">"  ,search(lpublic,"ip_error"));

		fprintf(cgiOut,"</div></td>");
		fprintf(cgiOut,"<td>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=vgmk1   maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=vgmk2   maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=vgmk3   maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=vgmk4   maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"</div></td>");

		fprintf(cgiOut,"<td><select name=del_add>");
		fprintf(cgiOut,"<option value=add>%s</option>",search(lpublic,"ntp_add"));
		fprintf(cgiOut,"<option value=delete>%s</option>",search(lpublic,"delete"));		
		fprintf(cgiOut,"</select></td>");
		
		fprintf(cgiOut,"</tr>");

		///
		int gw_i=0;
		fprintf(cgiOut,"<tr height=30>");
		fprintf(cgiOut,"<td width=140>%s:</td>",search(lpublic,"exist_vgate"));
		
		fprintf(cgiOut,"<td colspan=2><table>");
		for(gw_i=0;gw_i<zvrrp.gw_number;gw_i++)
			{
				fprintf(cgiOut,"<tr>");		
				fprintf(cgiOut,"<td>%s %s/%d</td>",zvrrp.gw[gw_i].gwname,zvrrp.gw[gw_i].gwip,zvrrp.gw[gw_i].gw_mask);		
				fprintf(cgiOut,"</tr>");
			}
		fprintf(cgiOut,"</table></td>");
		fprintf(cgiOut,"</tr>");
	}
	
	/*
	fprintf(cgiOut,"<tr height=30>"\
	"<td width=140>%s MAC:</td>",search(lcontrol,"virtual"));
	if(strcmp(zvrrp.macstate,"yes")==0)
	fprintf(cgiOut,"<td colspan=3><input type=checkbox name=vmac checked/></td>");
	else
	fprintf(cgiOut,"<td colspan=3><input type=checkbox name=vmac /></td>");
	fprintf(cgiOut,"</tr>");
	*/

	fprintf(cgiOut,"<tr>");
	fprintf(cgiOut,"<td><input type=hidden name=encry_newvrrp value=%s></td>",m);
	fprintf(cgiOut,"<td><input type=hidden name=ID value=%s></td>",id);
	fprintf(cgiOut,"<td><input type=hidden name=TYPE value=%s></td>","1");
	fprintf(cgiOut,"</tr>");
	fprintf(cgiOut,"<tr><td colspan=3><input type=hidden name=SZ value=%s></td>",choice);
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
	return 0;
}


void ShowHansiClearPage(char *m,char *id,struct list *lpublic,struct list *lcontrol)
{
	cgiHeaderContentType("text/html");	
	fprintf( cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( cgiOut, "<head> \n" );
	fprintf( cgiOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );
	//下面三句话用于禁止页面缓存
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");

	fprintf( cgiOut, "<title>%s</title> \n", search( lcontrol, "del" ) );
	fprintf( cgiOut, "<link rel=stylesheet href=/style.css type=text/css> \n" );
	fprintf( cgiOut, "<style type=text/css> \n" );
	fprintf( cgiOut, ".usrlis {overflow-x:hidden; overflow:auto; width: 416px; height: 270px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} \n" );
	fprintf( cgiOut, "</style> \n" );
	fprintf( cgiOut, "<style type=text/css> \n" );
	fprintf( cgiOut, "tr.even td { \n" );
	fprintf( cgiOut, "background-color: #eee; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "tr.odd td { \n" );
	fprintf( cgiOut, "background-color: #fff; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "tr.changed td { \n" );
	fprintf( cgiOut, "background-color: #ffd; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, " \n" ); 
	fprintf( cgiOut, "tr.new td { \n" );  
	fprintf( cgiOut, "background-color: #dfd; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "</style> \n" );
	fprintf( cgiOut, "</head> \n" );		
	fprintf( cgiOut, "<body> \n" );


	int ret;

	//ccgi_config_hansi_profile(id);
	ret=delete_hansi_profile(id);
	if(ret==0)
	{
		ShowAlert(search(lpublic,"oper_succ"));
	}
	else
	{
		ShowAlert(search(lpublic,"oper_fail"));
	}
	fprintf( cgiOut, "<script type='text/javascript'>\n" );
	fprintf( cgiOut, "window.location.href='wp_hansilist.cgi?UN=%s';\n", m);
	fprintf( cgiOut, "</script>\n" );	
	fprintf( cgiOut, "</body>\n" );
	fprintf( cgiOut, "</html>\n" );
}


