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
#include "ws_init_dbus.h"
#include "ws_dbus_list.h"
#include "ws_dbus_list_interface.h"
#include "ws_dcli_vrrp.h"
#include "ws_local_hansi.h"
#include "ws_dcli_license.h"

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

int ShowHansiModPage(char *m,char *id,char *choice,struct list *lpublic,struct list *lcontrol,int slotid,DBusConnection *connection,int pid);    
//void ShowHansiClearPage(char *m,char *id,struct list *lpublic,struct list *lcontrol,int slotid,DBusConnection *connection);    

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
	

	if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
	{
		str=dcryption(encry);
		if(str==NULL)
			ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
		else 
		{
			if(strcmp(hstype,"1")==0)
				ShowHansiModPage(encry,hsid,macChoice,lpublic,lcontrol,pid,connection,pid);
//			else if(strcmp(hstype,"2")==0)
//				ShowHansiClearPage(encry,hsid,lpublic,lcontrol);
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
				ShowHansiModPage(encry,hsid,macChoice,lpublic,lcontrol,pid,connection,pid);
//			else if(strcmp(hstype,"2")==0)
//				ShowHansiClearPage(encry,hsid,lpublic,lcontrol);
		}

	} 
	free(encry);
	release(lpublic);  
	release(lcontrol);
	return 0;
}

int ShowHansiModPage(char *m,char *id,char * choice,struct list *lpublic,struct list *lcontrol,int slotid,DBusConnection *connection,int pid)  
{  
	int i,retu,hspro_num=0,upmaskbit,dwmaskbit;
	int flag1=0,flag2=0,flag3=0,flag4=0,flag5=0;
	char max_type[10]={0};
	char max_num[10]={0};
	char max_ap_type[10]={0};
	char max_ap_num[10]={0};
	

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

	Z_VRRP_web zvrrp;
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

	int retu1 = 0;
	int k = 0;
	vrrp_link_ip_web *uq=NULL;
	vrrp_link_ip_web *dq=NULL;
	vrrp_link_ip_web *vq=NULL;
	char optstr[10] = {0};
	char infstr[64] = {0};
	char intfip[32] = {0};
	char temp[32] = {0};

	char hmd_switch[10];
	char link_switch[10];
	char preenpt[10];
	memset(hmd_switch,0,10);
	memset(link_switch,0,10);
	memset(preenpt,0,10);

	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>VRRP</title>");
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	"<style type=text/css>"\
	".a3{width:30;border:0; text-align:center}"\
	"#div1{ width:80px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
	"#div2{ width:78px; height:15px; padding-left:5px; padding-top:3px}"\
	"#link{ text-decoration:none; font-size: 12px}"\
	".divlis {overflow-x:hidden;	overflow:auto; width: 690; height: 100px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	"</style>"\
	"<script language=javascript src=/ip.js>"\
	"</script>"\
	"</head>"\
	"<body>");
	cgiFormStringNoNewlines("OPT",optstr,sizeof(optstr));
	cgiFormStringNoNewlines("INF",infstr,sizeof(infstr));
	cgiFormStringNoNewlines("FIP",intfip,sizeof(intfip));
	int ret = 0;
	ccgi_dbus_init();
	if((0 != strcmp(infstr,""))&&(0 != strcmp(intfip,"")))
	{
		if(0 == strcmp(optstr,"1"))
		{
			ret = config_vrrp_link_add_vip_web("delete", id, "uplink", infstr, intfip, connection);
			fprintf(stderr,"ret 111111=%d\n",ret);
		}		
		else if(0 == strcmp(optstr,"2"))
		{
			ret = config_vrrp_link_add_vip_web("delete", id, "downlink", infstr, intfip, connection);
			fprintf(stderr,"ret 2222222222=%d\n",ret);
		}
		else if(0 == strcmp(optstr,"3"))
		{
			ret = config_vrrp_link_add_vip_web("delete", id, "vgateway", infstr, intfip, connection);
			fprintf(stderr,"ret 3333333333=%d\n",ret);
		}
		if(33 != ret)
		{
			ShowAlert(ccgi_vrrp_err_msg[ret]);
		}
				
	}
	if(cgiFormSubmitClicked("hansiadd") == cgiFormSuccess)
	{
		//vgateway
		
		cgiFormStringNoNewlines("vgatewayname",vgatewayname,10);	
		cgiFormStringNoNewlines("vgateway1",vgateway1,10);	
		cgiFormStringNoNewlines("vgateway2",vgateway2,10);	
		cgiFormStringNoNewlines("vgateway3",vgateway3,10);	
		cgiFormStringNoNewlines("vgateway4",vgateway4,10);	
		sprintf(vgateway,"%ld.%ld.%ld.%ld",strtoul(vgateway1,0,10),strtoul(vgateway2,0,10),strtoul(vgateway3,0,10),strtoul(vgateway4,0,10));
		fprintf(stderr,"vgateway1=%s,vgateway=%s",vgateway1,vgateway);
		cgiFormStringNoNewlines("vgmk1",vgmk1,10);	
		cgiFormStringNoNewlines("vgmk2",vgmk2,10);	
		cgiFormStringNoNewlines("vgmk3",vgmk3,10);	
		cgiFormStringNoNewlines("vgmk4",vgmk4,10);
		sprintf(vgmk,"%ld.%ld.%ld.%ld",strtoul(vgmk1,0,10),strtoul(vgmk2,0,10),strtoul(vgmk3,0,10),strtoul(vgmk4,0,10));
		
		fprintf(stderr,"vgmk=%s",vgmk);
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
		fprintf(stderr,"ulip=%s",ulip);

		cgiFormStringNoNewlines("downip1",dip1,4);
		cgiFormStringNoNewlines("downip2",dip2,4);
		cgiFormStringNoNewlines("downip3",dip3,4);
		cgiFormStringNoNewlines("downip4",dip4,4);
		sprintf(dlip,"%ld.%ld.%ld.%ld",strtoul(dip1,0,10),strtoul(dip2,0,10),strtoul(dip3,0,10),strtoul(dip4,0,10));
		fprintf(stderr,"dlip=%s",dlip);

		cgiFormStringNoNewlines("hbip1",hip1,4);
		cgiFormStringNoNewlines("hbip2",hip2,4);
		cgiFormStringNoNewlines("hbip3",hip3,4);
		cgiFormStringNoNewlines("hbip4",hip4,4);
		sprintf(hbip,"%ld.%ld.%ld.%ld",strtoul(hip1,0,10),strtoul(hip2,0,10),strtoul(hip3,0,10),strtoul(hip4,0,10));
		fprintf(stderr,"hbip=%s",hbip);

		memset(hsprio,0,10);
		cgiFormStringNoNewlines("hsprio",hsprio,10);

		memset(hblink,0,30);
		cgiFormStringNoNewlines("hblink",hblink,30);

		memset(hstime,0,10);
		cgiFormStringNoNewlines("hstime",hstime,10);

		memset(vmac,0,10);
		cgiFormStringNoNewlines("vmac",vmac,10);
		
		memset(hmd_switch,0,10);
		cgiFormStringNoNewlines("Hmd_switch",hmd_switch,10);
		
		memset(link_switch,0,10);
		cgiFormStringNoNewlines("link_detect_switch",link_switch,10);
		
		memset(preenpt,0,10);
		cgiFormStringNoNewlines("hs_preempt",preenpt,10);
		
		memset(max_ap_type,0,10);
		cgiFormStringNoNewlines("max_ap_type",max_ap_type,10);
		memset(max_ap_num,0,10);
		cgiFormStringNoNewlines("max_ap_num",max_ap_num,10);


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
		fprintf(stderr,"upmask=%s",upmask);

	    //downmask
		cgiFormStringNoNewlines("dwmk1",dwmk1,4);
		cgiFormStringNoNewlines("dwmk2",dwmk2,4);
		cgiFormStringNoNewlines("dwmk3",dwmk3,4);
		cgiFormStringNoNewlines("dwmk4",dwmk4,4);
		sprintf(downmask,"%ld.%ld.%ld.%ld",strtoul(dwmk1,0,10),strtoul(dwmk2,0,10),strtoul(dwmk3,0,10),strtoul(dwmk4,0,10));
		fprintf(stderr,"downmask=%s",downmask);



		op_ret=ccgi_config_hansi_profile_web(id,slotid,connection);
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
							op_ret=ccgi_downanduplink_ifname_mask_web(id,ulname, ulip, dlname, dlip, hsprio,upmaskbit,dwmaskbit,connection);
							if(op_ret==0)
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_succ"));
								ShowAlert(paramalert);

							}
							else
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s:%s",search(lpublic,"param_conf_fail"),ccgi_vrrp_err_msg[op_ret]);
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
							//op_ret=config_vrrp_uplink(id,ulname, ulip, hsprio,upmaskbit,connection);
							memset(temp,0,sizeof(temp));
							snprintf(temp,sizeof(temp)-1,"%s/%d",ulip,upmaskbit);
							op_ret = config_vrrp_link_add_vip_web("add", id, "uplink", ulname, temp, connection);
							if(op_ret==33)
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_succ"));
								ShowAlert(paramalert);

							}
							else
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s:%s",search(lpublic,"param_conf_fail"),ccgi_vrrp_err_msg[op_ret]);
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
							memset(temp,0,sizeof(temp));
							snprintf(temp,sizeof(temp)-1,"%s/%d",dlip,dwmaskbit);
							op_ret = config_vrrp_link_add_vip_web("add", id, "downlink", dlname, temp, connection);
							if(op_ret==33)
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s",search(lpublic,"param_conf_succ"));
								ShowAlert(paramalert);

							}
							else
							{
								memset(paramalert,0,128);
								sprintf(paramalert,"UpandDown link %s:%s",search(lpublic,"param_conf_fail"),ccgi_vrrp_err_msg[op_ret]);
								ShowAlert(paramalert);
							}
						}
					}
				}
				else					
				 ShowAlert(search(lpublic,"hs_param"));
		    }
	    //single 
		//deal with hansi priority
		if(strcmp(hsprio,"")!=0)
		{
			convert_num=strtoul(hsprio,0,10);
			if(convert_num < 1 || convert_num > 255)
				ShowAlert(search(lpublic,"hs_param"));
			else
			{
				op_ret=ccgi_config_hansi_priority_web(id,hsprio,connection);
				if(op_ret==0)
				{
					memset(paramalert,0,128);
					sprintf(paramalert,"Hansi priority %s",search(lpublic,"param_conf_succ"));
					ShowAlert(paramalert);
				}
				else
				{
					memset(paramalert,0,128);
					sprintf(paramalert,"Hansi priority %s:%s",search(lpublic,"param_conf_fail"),ccgi_vrrp_err_msg[op_ret]);
					ShowAlert(paramalert);
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
				if(convert_num < 1 || convert_num > ADVERTIME)
					ShowAlert(search(lpublic,"hs_param"));
				else
				{
					op_ret=ccgi_config_hansi_advertime_web(id,hstime,connection);
					if(op_ret==0)
					{
						memset(paramalert,0,128);
						sprintf(paramalert,"Hansi advertime %s",search(lpublic,"param_conf_succ"));
						ShowAlert(paramalert);
					}
					else
					{
						memset(paramalert,0,128);
						sprintf(paramalert,"Hansi advertime %s:%s",search(lpublic,"param_conf_fail"),ccgi_vrrp_err_msg[op_ret]);
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
				op_ret=config_vrrp_heartbeat_cmd_func_web(id, hblink, hbip,connection);
				if(op_ret==0)
				{
					memset(paramalert,0,128);
					sprintf(paramalert,"Hansi heartbeatlink %s",search(lpublic,"param_conf_succ"));
					ShowAlert(paramalert);
				}
				else
				{
					memset(paramalert,0,128);
					sprintf(paramalert,"Hansi heartbeatlink %s:%s",search(lpublic,"param_conf_fail"),ccgi_vrrp_err_msg[op_ret]);
					ShowAlert(paramalert);
				}
			}
	    }
		if(strcmp(choice,"7")==0) 
		    {
		    	int dwmaskbit=0;
				char mask_tmp[20] = {0};
				dwmaskbit=mask_bit(vgmk);
		    	
				if( (strcmp(vgatewayname,"") !=0)&&(strcmp(vgateway1,"") !=0)&& (strcmp(vgateway2,"") !=0)&& (strcmp(vgateway3,"") !=0)&& (strcmp(vgateway4,"") !=0)&&(strcmp(vgmk1,"") !=0)&&(strcmp(vgmk2,"") !=0)&&(strcmp(vgmk3,"") !=0)&&(strcmp(vgmk4,"") !=0))
				{
						if(dwmaskbit == -1)
							ShowAlert(search(lcontrol,"dhcp_mask_err"));
						else
						{
							sprintf(mask_tmp,"%s/%d",vgateway,dwmaskbit);
							
							op_ret=config_vrrp_link_add_vip_web("add",id,"vgateway",vgatewayname,mask_tmp,connection);
							if(op_ret==33) 
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
									sprintf(paramalert,"%s:%s",search(lpublic,"vgateway_conf_failure"),ccgi_vrrp_err_msg[op_ret]);
									ShowAlert(paramalert);
								}
							}
						}
					
				}
				else					
				 ShowAlert(search(lpublic,"hs_param"));
				
		    }
		
			if(strcmp(choice,"8")==0) 
			{
				if(strcmp(hmd_switch,"none")!=0)
					flag1=set_hansi_check_state_cmd_web(hmd_switch,pid,connection);
				fprintf(stderr,"flag1=%d\n",flag1);
				if(strcmp(link_switch,"none")!=0)
					flag2=config_vrrp_multi_link_detect_web(link_switch,id,connection);
				fprintf(stderr,"flag2=%d\n",flag2);
				if(strcmp(preenpt,"none")!=0)
					flag3=config_vrrp_preempt_web(preenpt,id,connection);
				fprintf(stderr,"flag3=%d\n",flag3);
				if((strcmp(max_ap_num,"")!=0) && (strcmp(max_ap_type,"")!=0))
				{
					fprintf(stderr,"max_ap_type=%s\n",max_ap_type);
					fprintf(stderr,"max_ap_num=%s\n",max_ap_num);
					fprintf(stderr,"id=%s\n",id);
					fprintf(stderr,"pid=%d\n",pid);

				flag4=license_assign_cmd_web(max_ap_type,max_ap_num,"hansi",pid,id,connection);
				}
				fprintf(stderr,"flag4=%d\n",flag4);

				
				if((flag1==0) &&(flag2==0) &&(flag3==0) &&(flag4==0))
					ShowAlert(search(lpublic,"config_sucess"));
				if(flag1!=0)
					ShowAlert(search(lpublic,"Hmd_switch_fail"));
				if(flag2!=0)
					ShowAlert(search(lpublic,"link_detect_switch_fail"));
				if(flag3!=0)
					ShowAlert(search(lpublic,"hs_preempt_failed"));
				if(flag4!=0)
				{
					if(flag4==-10)
						ShowAlert("HMD dbus license number not enough");
					else if(flag4==-11)
						ShowAlert("HMD dbus license type not exist");					
					else if(flag4==-12)
						ShowAlert("HMD dbus slot id not exist");
					else if(flag4==-13)
						ShowAlert("HMD dbus command not support");
					else if(flag4==-14)
						ShowAlert("HMD dbus set num more than specefication");
					else
						ShowAlert(search(lpublic,"max_ap_num_fail"));
				}
			}
	   
		}
		else
		{
			memset(paramalert,0,128);
			sprintf(paramalert,"Hansi profile %s:%s",search(lpublic,"param_conf_fail"),ccgi_vrrp_err_msg[op_ret]);
			ShowAlert(paramalert);
		}
	}  

	if(cgiFormSubmitClicked("starts") == cgiFormSuccess)
	{
		retu=config_vrrp_service_web(id, "enable",connection);
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
		retu=config_vrrp_service_web(id, "disable",connection);
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
	"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"vr_conf"));   /*突出显示*/
	fprintf(cgiOut,"</tr>");

	for(i=0;i<15;i++)
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
	retu1 = ccgi_show_hansi_profile_web(&zvrrp,hspro_num,slotid,connection);    

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
		"<td colspan=3>%d-%s</td>"\
		"</tr>",search(lpublic,"hs_profile"),slotid,id);
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
	
	if(strcmp(choice,"8")==0)
		fprintf(cgiOut,"<option value='8' selected=selected>%s</option>\n",search(lpublic,"other_set"));
	else
		fprintf(cgiOut,"<option value='8'>%s</option>\n",search(lpublic,"other_set"));
	
	fprintf(cgiOut,"</select>\n");
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"</tr>\n");
	fprintf(cgiOut,"<script type=text/javascript>\n");
    fprintf(cgiOut,"function port_sel_change( obj )\n"\
       	"{\n"\
           	"var selectz = obj.options[obj.selectedIndex].value;\n"\
           	"var url = 'wp_hansimod.cgi?UN=%s&ID=%s&TYPE=%s&plotid=%d&SZ='+selectz;\n"\
           	"window.location.href = url;\n"\
       	"}\n", m,id,"1",slotid);
    fprintf(cgiOut,"</script>\n" );
	if((strcmp(choice,"1")==0) || (strcmp(choice,"2")==0))
	{
		//up   link ifname
		fprintf(cgiOut,"<tr  height=30>"\
		"<td width=140>%s/IP/Mask :</td>",search(lpublic,"hs_uplink"));
		fprintf(cgiOut,"<td><input type=text name=upname style=\"width:80px\"></td>");
		fprintf(cgiOut,"<td>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=upip1  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=upip2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error"));
		fprintf(cgiOut,"<input type=text  name=upip3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error"));
		fprintf(cgiOut,"<input type=text  name=upip4  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">",search(lpublic,"ip_error"));
		fprintf(cgiOut,"</div></td>");
		fprintf(cgiOut,"<td>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=upmk1 id=upmk1  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=upmk2 id=upmk2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error"));
		fprintf(cgiOut,"<input type=text  name=upmk3 id=upmk3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=upmk4 id=upmk4  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"</div></td>"\
		"</tr>");
	}
	if((strcmp(choice,"1")==0) || (strcmp(choice,"3")==0))
	{
		//  down link ifname
		fprintf(cgiOut,"<tr  height=30>"\
		"<td width=140>%s/IP/Mask :</td>",search(lpublic,"hs_dlink"));
		fprintf(cgiOut,"<td><input type=text name=downname style=\"width:80px\"></td>");//zvrrp.downlink_ifname
		fprintf(cgiOut,"<td>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=downip1 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=downip2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error"));
		fprintf(cgiOut,"<input type=text  name=downip3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error"));
		fprintf(cgiOut,"<input type=text  name=downip4  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">",search(lpublic,"ip_error"));
		fprintf(cgiOut,"</div></td>");
		fprintf(cgiOut,"<td>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=dwmk1 id=dwmk1  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=dwmk2 id=dwmk2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=dwmk3 id=dwmk3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=dwmk4 id=dwmk4 maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"</div></td>");
		fprintf(cgiOut,"</tr>");
	}
	if((strcmp(choice,"1")==0) || (strcmp(choice,"3")==0)||(strcmp(choice,"2")==0)||(strcmp(choice,"4")==0))
	{
		//priority
		fprintf(cgiOut,"<tr height=30>"\
		"<td width=140>%s:</td>",search(lcontrol,"prior"));
		fprintf(cgiOut,"<td colspan=3 align=left><input name=hsprio maxLength=3 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\""\
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
		fprintf(cgiOut,"<td colspan=3 align=left><input name=hstime maxLength=6 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\""\
		"onpaste=\"var s=clipboardData.getData('text'); if(!/\\D/.test(s)) value=s.replace(/^0*/,'');   return   false;\""\
		"ondragenter=\"return  false;\" "\
		"style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" type=text size=8 value=\"%d\"/><font color=red>(1--%d)</font></td>"\
		"</tr>",zvrrp.advert,ADVERTIME);
   	}
   if(strcmp(choice,"6")==0)
   {

		int ip_1 = 0,ip_2 = 0,ip_3 = 0,ip_4 = 0;
		sscanf(zvrrp.hbip,"%d.%d.%d.%d",&ip_1,&ip_2,&ip_3,&ip_4);
		//heart blink
		fprintf(cgiOut,"<tr  height=30>"\
		"<td width=140>%s/IP :</td>",search(lpublic,"inter"));
		fprintf(cgiOut,"<td><input type=text name=hblink style=\"width:80px\" value=\"%s\"></td>",zvrrp.hbinf);
		fprintf(cgiOut,"<td colspan=2>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=hbip1  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),ip_1); 
		fprintf(cgiOut,"<input type=text  name=hbip2  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),ip_2);
		fprintf(cgiOut,"<input type=text  name=hbip3  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">.",search(lpublic,"ip_error"),ip_3);
		fprintf(cgiOut,"<input type=text  name=hbip4  maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"%d\">",search(lpublic,"ip_error"),ip_4);
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
		fprintf(cgiOut,"<td colspan=2>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
		fprintf(cgiOut,"<input type=text  name=vgmk1   maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=vgmk2   maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=vgmk3   maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">.",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"<input type=text  name=vgmk4   maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=\"\">",search(lpublic,"ip_error")); 
		fprintf(cgiOut,"</div></td>");
		fprintf(cgiOut,"</tr>");

		///
	//	int gw_i=0;
		fprintf(cgiOut,"<tr height=30>");
		fprintf(cgiOut,"<td width=140>%s:</td>",search(lpublic,"exist_vgate"));
		fprintf(cgiOut,"<td colspan=3></td>");
		fprintf(cgiOut,"</tr>");
	}
	if(strcmp(choice,"8")==0)
	{
	
		 fprintf(cgiOut,"<tr  height=30>"\
		 "<td width=140>%s:</td>",search(lpublic,"Hmd_switch"));
		 fprintf(cgiOut,"<td colspan=3>\n");
		 fprintf(cgiOut,"<select name=Hmd_switch style=\"width:80px\" >");
		 fprintf(cgiOut,"<option value='none' selected=selected> </option>\n");
		 fprintf(cgiOut,"<option value='enable'>enable</option>\n");
		 fprintf(cgiOut,"<option value='disable'>disable</option>\n");
		 fprintf(cgiOut,"</select>"\
		 				"</td>");
		 fprintf(cgiOut,"<tr height=30>"\
		 	"<td width=140>%s:</td>",search(lpublic,"link_detect_switch"));
		 fprintf(cgiOut,"<td colspan=3>\n");
		 fprintf(cgiOut,"<select name=link_detect_switch style=\"width:80px\" >");
		 fprintf(cgiOut,"<option value='none' selected=selected> </option>\n");
		 fprintf(cgiOut,"<option value='on'>on</option>\n");
		 fprintf(cgiOut,"<option value='off'>off</option>\n");
		 fprintf(cgiOut,"</select>"\
		 				"</td>");
		 fprintf(cgiOut,"<tr height=30>"\
		 	"<td width=140>%s:</td>",search(lpublic,"hs_preempt"));
		 fprintf(cgiOut,"<td colspan=3>\n");
		 fprintf(cgiOut,"<select name=hs_preempt style=\"width:80px\" >");
		 fprintf(cgiOut,"<option value='none' selected=selected> </option>\n");
		 fprintf(cgiOut,"<option value='yes'>yes</option>\n");
		 fprintf(cgiOut,"<option value='no'>no</option>\n");
		 fprintf(cgiOut,"</select>"\
		 				"</td>");
		 fprintf(cgiOut,"<tr height=30>"\
		 	"<td width=140>%s:</td>",search(lpublic,"max_ap_num"));
		 fprintf(cgiOut,"<td align=left>%s<input name=max_ap_type maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\""\
		 "onpaste=\"var s=clipboardData.getData('text'); if(!/\\D/.test(s)) value=s.replace(/^0*/,'');	 return   false;\""\
		 "ondragenter=\"return	false;\" "\
		 "style=\"ime-mode:disabled\" onkeyup=\"if(/(^0+)/.test(value))value=value.replace(/^0*/,'')\" type=text size=8 value=\"%s\"/></td>"\
		 ,search(lpublic,"type"),max_type);

		 fprintf(cgiOut,"<td align=left>%s<input name=max_ap_num maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" size=8"\
		 "</td></tr>",search(lpublic,"num"),max_num);
	 }


	
	fprintf(cgiOut,"<tr><td colspan=4>");
	/////div for uplink
	fprintf(cgiOut,"<div class=divlis><table valign=top rules=rows width=570 border=1>");
	//uplink ifname
	if(retu1 == 0)
	{
		uq = zvrrp.uplink_list;
		k = 0;
		while(NULL!=uq)
		{
			k++;
			if(k == 1)
			{
		        fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>%s/IP</td>"\
						  "<td id=td2>%s/%s</td>",search(lpublic,"hs_uplink"),uq->ifname,uq->link_ip);
				fprintf(cgiOut,"<td><a href=wp_hansimod.cgi?UN=%s&ID=%s&plotid=%d&TYPE=1&OPT=1&INF=%s&FIP=%s target=mainFrame>%s</a></td>",m,id,slotid,uq->ifname,uq->link_ip,search(lpublic,"delete"));
				fprintf(cgiOut,"</tr>");	
			}
			else
			{
		        fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>&nbsp;</td>"\
						  "<td id=td2>%s/%s</td>",uq->ifname,uq->link_ip);
				fprintf(cgiOut,"<td><a href=wp_hansimod.cgi?UN=%s&ID=%s&plotid=%d&TYPE=1&OPT=1&INF=%s&FIP=%s target=mainFrame>%s</a></td>",m,id,slotid,uq->ifname,uq->link_ip,search(lpublic,"delete"));
				fprintf(cgiOut,"</tr>");	
			}
			uq = uq->next;
		}
		
		//downlink ifname
		dq = zvrrp.downlink_list;
		k = 0;
		while(NULL!=dq)
		{

			k++;
			if(k == 1)
			{
			   fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>%s/IP</td>"\
						  "<td id=td2>%s/%s</td>",search(lpublic,"hs_dlink"),dq->ifname,dq->link_ip);
				fprintf(cgiOut,"<td><a href=wp_hansimod.cgi?UN=%s&ID=%s&plotid=%d&TYPE=1&OPT=2&INF=%s&FIP=%s target=mainFrame>%s</a></td>",m,id,slotid,dq->ifname,dq->link_ip,search(lpublic,"delete"));
			   fprintf(cgiOut,"</tr>");
			}
			else
			{
			   fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>&nbsp;</td>"\
						  "<td id=td2>%s/%s</td>",dq->ifname,dq->link_ip);
				fprintf(cgiOut,"<td><a href=wp_hansimod.cgi?UN=%s&ID=%s&plotid=%d&TYPE=1&OPT=2&INF=%s&FIP=%s target=mainFrame>%s</a></td>",m,id,slotid,dq->ifname,dq->link_ip,search(lpublic,"delete"));
			   fprintf(cgiOut,"</tr>");
			}
		   dq = dq->next;

		}
		//downlink ifname
		vq = zvrrp.vgatewaylink_list;
		k = 0;
		while(NULL!=vq)
		{

			k++;
			if(k == 1)
			{
			   fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>%s/IP</td>"\
						  "<td id=td2>%s/%s</td>",search(lpublic,"vgateway"),vq->ifname,vq->link_ip);
				fprintf(cgiOut,"<td><a href=wp_hansimod.cgi?UN=%s&ID=%s&plotid=%d&TYPE=1&OPT=3&INF=%s&FIP=%s target=mainFrame>%s</a></td>",m,id,slotid,vq->ifname,vq->link_ip,search(lpublic,"delete"));
			   fprintf(cgiOut,"</tr>");
			}
			else
			{
			   fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>&nbsp;</td>"\
						  "<td id=td2>%s/%s</td>",vq->ifname,vq->link_ip);
				fprintf(cgiOut,"<td><a href=wp_hansimod.cgi?UN=%s&ID=%s&plotid=%d&TYPE=1&OPT=3&INF=%s&FIP=%s target=mainFrame>%s</a></td>",m,id,slotid,vq->ifname,vq->link_ip,search(lpublic,"delete"));
			   fprintf(cgiOut,"</tr>");
			}
		   vq = vq->next;

		}
		
		fprintf(cgiOut,"</table></div></td></tr>");
	}

	fprintf(cgiOut,"<input type=hidden name=encry_newvrrp value=%s>",m);
	fprintf(cgiOut,"<input type=hidden name=ID value=%s>",id);
	fprintf(cgiOut,"<input type=hidden name=plotid value=%d>",pid);
	fprintf(cgiOut,"<input type=hidden name=TYPE value=%s>","1");
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
	free_ccgi_show_hansi_profile_web(&zvrrp);
	return 0;
}

