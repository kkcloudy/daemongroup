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
* wp_contrl.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system infos 
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_nm_status.h"
#include "ws_dcli_portconf.h"
#include "ws_stp.h"//modify by shaojunwu 20090408  pre is ws_stp.c
#include "ws_init_dbus.h"
#include "ws_trunk.h"
#include "ws_fdb.h"
#include "ws_dcli_acl.h"
#include "ws_dcli_qos.h"
#include "ws_sndr_cfg.h"
#include "ws_list_container.h"
#include "ws_secondary_container.h"  
#include "ws_dhcp_conf.h"
#include "ws_dcli_vlan.h"

#define Route_Num 40960
#define MAX_ACL_NUM 4096

int ShowIPRoute(char * routeInfo[],int * route_num,int * Kroute_num,int * Sroute_num,int * Croute_num);
int  init_pstControlSCCreateHelper();

//暂时12个             


int port_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //端口管理，都有
{
    
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	
	SI_set_label_name( p_item, search(p_pubinfo->local,"prt_manage")); 
	SI_set_label_img( p_item,"/images/PortMan.jpg");
	sprintf(temp_url,"wp_prtsur.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	///////////
	//ccgi_dbus_init();
	char num[10];
	memset(num,0,10);
	int pnum;
	pnum=count_eth_port_num();
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,search(p_pubinfo->local,"prt_manage"));			
	SI_set_summary_keyinfo( p_item,search(p_pubinfo->public,"port"));	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));
	sprintf(num,"%d",pnum);
	SI_set_summary_key( p_item,num);
	
	return 0;
}

int fdb_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //FDB，都有
{
    
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	
	SI_set_label_name( p_item, "FDB"); 
	SI_set_label_img( p_item,"/images/FDB.jpg");
	sprintf(temp_url,"wp_show_fdb.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	///////////
	//ccgi_dbus_init();
	char num[10];
	memset(num,0,10);
	int fdb_count = 0;
	fdb_count = get_fdb_count();
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,"FDB");			
	SI_set_summary_keyinfo( p_item,"FDB");	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));
	sprintf(num,"%d",fdb_count);
	SI_set_summary_key( p_item,num);
	
	return 0;
}


int vlan_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //vlan管理，都有
{
    
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	char name[60];
	memset(name,0,60);
	sprintf(name,"VLAN %s",search(p_pubinfo->public,"management"));
	SI_set_label_name( p_item, name); 
	SI_set_label_img( p_item,"/images/VLANMan.jpg");
	sprintf(temp_url,"wp_configvlan.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	///////////
	//ccgi_dbus_init();
	char num[10];
	memset(num,0,10);
	struct vlan_info_simple receive_vlan[MAX_VLAN_NUM];
	int i,port_num[MAX_VLAN_NUM],vlanNum=0;
	for(i=0;i<4095;i++)
  	{
  		receive_vlan[i].vlanId=0;
		receive_vlan[i].vlanName=(char*)malloc(21);
		memset(receive_vlan[i].vlanName,0,21);
  		port_num[i]=0;
  	}
	show_vlan_list(receive_vlan,port_num,&vlanNum);
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,name);			
	SI_set_summary_keyinfo( p_item,"VLAN");	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));
	sprintf(num,"%d",vlanNum-1);
	SI_set_summary_key( p_item,num);

	for(i=0;i<4095;i++)
	{
		free(receive_vlan[i].vlanName);
	}
	return 0;
}


int trunk_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //端口聚合，都有
{
    
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	
	SI_set_label_name( p_item, search(p_pubinfo->local,"trunk_manage")); 
	SI_set_label_img( p_item,"/images/TrunkMan.jpg");
	sprintf(temp_url,"wp_trunklis.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	///////////
	//ccgi_dbus_init();
	char num[10];
	memset(num,0,10);
	int trknum=0;
	int result1;
	struct trunk_profile thead;
	result1=show_trunk_list(&thead,&trknum);
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,search(p_pubinfo->local,"trunk_manage"));			
	SI_set_summary_keyinfo( p_item,"Trunk");	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));
	sprintf(num,"%d",trknum);
	SI_set_summary_key( p_item,num);
	
	if((result1==1)&&(trknum>0))
    Free_trunk_head(&thead);
	return 0;
}


int stp_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //5 ,STP，都有
{

	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	
	SI_set_label_name( p_item, "STP"); 
	SI_set_label_img( p_item,"/images/STP.jpg");
	sprintf(temp_url,"wp_select_stp.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	///////////
	char num[20];
	memset(num,0,20);
	int fdb_count = 0;
	fdb_count = get_fdb_count();
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,"STP");			
	SI_set_summary_keyinfo( p_item,search(p_pubinfo->local,"br_info"));	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));

	//ccgi_dbus_init();
	int stpmode;
	int ret=-1;
	bridge_info br_info;
	if(ccgi_get_brg_g_state(&stpmode)==1)
	{
			if(ccgi_get_br_info(&br_info)==0)
			{
				ret = 0;
			}
			else
			{
				ret =-1;
			}

	}
	else
	{
		ret =-1;
	}
	
   if(ret == -1)
   	sprintf(num,"%s",search(p_pubinfo->local,"no_start_br"));                      	
   else
   	sprintf(num,"%02x:%02x:%02x:%02x:%02x:%02x",br_info.design_br_mac[0],br_info.design_br_mac[1],br_info.design_br_mac[2],br_info.design_br_mac[3],br_info.design_br_mac[4],br_info.design_br_mac[5]);
 
	SI_set_summary_key( p_item,num);
	
	return 0;
}



int route_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //6 ,路由管理，都有
{

	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	
	SI_set_label_name( p_item, search(p_pubinfo->local,"route_manage")); 
	SI_set_label_img( p_item,"/images/RouteMan.jpg");
	sprintf(temp_url,"wp_srouter.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	///////////
	char num[20];
	memset(num,0,20);
	int i;
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,search(p_pubinfo->local,"route_manage"));			
	SI_set_summary_keyinfo( p_item,search(p_pubinfo->local,"rou_sta"));	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));

	int kr=0,cr=0,sr=0,rn=0;
	char * IpRouteItem[Route_Num]; 
  	for(i=0;i<Route_Num;i++)  //循环分空间
  		{
  			IpRouteItem[i]=(char *)malloc(60);
			memset(IpRouteItem[i],0,60);
  		}
    
	ShowIPRoute(IpRouteItem,&rn,&kr,&sr,&cr);
   	sprintf(num,"%d",rn);
	SI_set_summary_key( p_item,num);

	for(i=0;i<Route_Num;i++)  //循环释放
	{
		free(IpRouteItem[i]);
	}
	return 0;
}



int acl_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //7 ,acl管理，都有
{
    
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	char name[60];
	memset(name,0,60);
	sprintf(name,"ACL %s",search(p_pubinfo->public,"management"));
	SI_set_label_name( p_item, name); 
	SI_set_label_img( p_item,"/images/ACLMan.jpg");
	sprintf(temp_url,"wp_aclall.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	///////////
	//ccgi_dbus_init();
	char num[10];
	memset(num,0,10);
	struct acl_info receive_acl[MAX_ACL_NUM];
	int i,aclNum=0;
    for(i=0;i<MAX_ACL_NUM;i++)
    {
	  receive_acl[i].ruleIndex=0;
	  receive_acl[i].groupIndex=0;	  
	  receive_acl[i].ruleType=(char *)malloc(20);
	  memset(receive_acl[i].ruleType,0,20);
	  receive_acl[i].protype=(char *)malloc(20);
	  memset(receive_acl[i].protype,0,20);
	  receive_acl[i].dip=(char *)malloc(20);
	  memset(receive_acl[i].dip,0,20);
	  receive_acl[i].sip=(char *)malloc(20);
	  memset(receive_acl[i].sip,0,20);
	  receive_acl[i].srcport=0; 
	  receive_acl[i].dstport=0;
	  receive_acl[i].icmp_code=0;
	  receive_acl[i].icmp_type=0;
	  receive_acl[i].actype=(char *)malloc(50);
	  memset(receive_acl[i].actype,0,50);
	  receive_acl[i].dmac=(char *)malloc(30);
	  memset(receive_acl[i].dmac,0,30);
	  receive_acl[i].smac=(char *)malloc(30);
	  memset(receive_acl[i].smac,0,30);
	  receive_acl[i].vlanid=0; 
	  receive_acl[i].source_port=(char *)malloc(30);
	  memset(receive_acl[i].source_port,0,30);
	  receive_acl[i].redirect_port=(char *)malloc(30);
	  memset(receive_acl[i].redirect_port,0,30);
	  receive_acl[i].analyzer_port=(char *)malloc(30);
	  memset(receive_acl[i].analyzer_port,0,30);
	  receive_acl[i].policerId=0;
	  receive_acl[i].up=0;
	  receive_acl[i].dscp=0;
	  receive_acl[i].egrUP=0;
	  receive_acl[i].egrDSCP=0;
	  receive_acl[i].qosprofileindex=0;
	  receive_acl[i].upmm=(char *)malloc(30);
	  memset(receive_acl[i].upmm,0,30);
	  receive_acl[i].dscpmm=(char *)malloc(30);
	  memset(receive_acl[i].dscpmm,0,30);
  
  }  	
	show_acl_allinfo(receive_acl,&aclNum);
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,name);	
	memset(name,0,10);
	sprintf(name,"ACL %s",search(p_pubinfo->local,"rule"));
	SI_set_summary_keyinfo( p_item,name);	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));
	
	sprintf(num,"%d",aclNum);
	SI_set_summary_key( p_item,num);

	for(i=0;i<MAX_ACL_NUM;i++)
  {
	free(receive_acl[i].ruleType);
	free(receive_acl[i].protype);
	free(receive_acl[i].dip);	
	free(receive_acl[i].sip);	

	free(receive_acl[i].actype);	
	free(receive_acl[i].dmac);
	free(receive_acl[i].smac);

	free(receive_acl[i].source_port);
	free(receive_acl[i].redirect_port);
	free(receive_acl[i].analyzer_port);

	free(receive_acl[i].upmm);
	free(receive_acl[i].dscpmm);

}
	return 0;
}


int vrrp_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //8 ,FDB，偏左
{
    
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	
	SI_set_label_name( p_item, "VRRP"); 
	SI_set_label_img( p_item,"/images/VRRP.jpg");
	sprintf(temp_url,"wp_hansilist.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  


	return 0;
}




int qos_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //9 ,qos管理，都有
{
    
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	char name[60];
	memset(name,0,60);
	sprintf(name,"QOS %s",search(p_pubinfo->public,"management"));
	SI_set_label_name( p_item, name); 
	SI_set_label_img( p_item,"/images/QOSMan.jpg");
	sprintf(temp_url,"wp_qosModule.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	///////////
	//ccgi_dbus_init();
	char num[10];
	memset(num,0,10);
	int i,qos_num=0;
  
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,name);		
	SI_set_summary_keyinfo( p_item,"QOS Profile");	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));
	
    struct qos_info receive_qos[MAX_QOS_PROFILE];
	for(i=0;i<MAX_QOS_PROFILE;i++)
  		{
  			receive_qos[i].profileindex=0;
  			receive_qos[i].dp=0;
  			receive_qos[i].up=0;
  			receive_qos[i].tc=0;
  			receive_qos[i].dscp=0;
  		}
	show_qos_profile(receive_qos,&qos_num,p_pubinfo->local);
	sprintf(num,"%d",qos_num);
	SI_set_summary_key( p_item,num);

  
	return 0;
}



int dhcp_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //10 ,DHCP，都有
{
    
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	
	SI_set_label_name( p_item, "DHCP"); 
	SI_set_label_img( p_item,"/images/DHCP.jpg");
	sprintf(temp_url,"wp_dhcpsumary.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	///////////
	//ccgi_dbus_init();
	
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,"DHCP");			
	SI_set_summary_keyinfo( p_item,search(p_pubinfo->local,"subnet"));	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));

	#if 0
	FILE * fp= fopen("/var/run/apache2/count_sub.tmp","r");
	if(fp == NULL)
		{
			fprintf(stderr,"can not open /var/run/apache2/count_sub.tmp");
			return 0;
		}
	char buf[10];
	fgets(buf,10,fp);
	sprintf(num,"%s",buf);
	fclose(fp);
	#endif
	char num[10] = { 0 };
	int nodenum = 0;
	get_show_xml_nodenum(DHCP_XML, DHCP_NODE_CONF, &nodenum);
	sprintf(num,"%d",nodenum);
	SI_set_summary_key( p_item,num);
	return 0;
}





int igmp_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //11 ,igmp管理，都有
{
    
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	char name[60];
	memset(name,0,60);
	sprintf(name,"IGMP %s",search(p_pubinfo->public,"snooping"));
	SI_set_label_name( p_item, name); 
	SI_set_label_img( p_item,"/images/IGMPMan.jpg");
	sprintf(temp_url,"wp_config_igmp.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	///////////
	//ccgi_dbus_init();
	char num[10];
	memset(num,0,10);
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,name);		
	SI_set_summary_keyinfo( p_item,"IGMP SNOOPING VLAN");	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));
	
  	int vlan_count = 0;
	iShow_igmp_vlan_count(&vlan_count);
	sprintf(num,"%d",vlan_count);
	SI_set_summary_key( p_item,num);

  
	return 0;
}


int tool_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //12 ,工具管理，偏左
{
    
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	
	SI_set_label_name( p_item, search(p_pubinfo->public,"tool")); 
	SI_set_label_img( p_item,"/images/tool.jpg");
	sprintf(temp_url,"wp_command.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	///////////	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,search(p_pubinfo->public,"tool"));		
	SI_set_summary_keyinfo( p_item,search(p_pubinfo->public,"tool"));	
	//SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));
	
	SI_set_summary_key( p_item,"Ping | Traceroute");

  
	return 0;
}
//SNDR_VLAN_ITEM

int fast_forward_contrl_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )
{
	
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	
	SI_set_label_name( p_item, search(p_pubinfo->public,"fast_for_set")); 
	SI_set_label_img( p_item,"/images/LiuKong.jpg");
	sprintf(temp_url,"wp_fast_set.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	/////////// 
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,search(p_pubinfo->public,"fast_for_set")); 	
	SI_set_summary_keyinfo( p_item,search(p_pubinfo->public,"fast_for_set"));	
	
	SI_set_summary_key( p_item,"Set | Show");

  
	return 0;
}


//#define HELPER_ITEM_NUM  sizeof(pstControlSCCreateHelper)/sizeof(pstControlSCCreateHelper[0])

static int HELPER_ITEM_NUM = 0;
#define PST_MAX_NUM 20
STSCCreateHelper pstControlSCCreateHelper[PST_MAX_NUM] = {0};

int  init_pstControlSCCreateHelper()
{
	int product_id = 0;
	product_id = get_product_id();

	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = port_contrl_fill;
		HELPER_ITEM_NUM++;
	}

	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = fdb_contrl_fill;
		HELPER_ITEM_NUM++;
	}
	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		if((product_id != PRODUCT_ID_AX5K_E) && (PRODUCT_ID_AX5608 != product_id))
		{
			pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api  = vlan_contrl_fill;
			HELPER_ITEM_NUM++;

		}	
	}

	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = trunk_contrl_fill;
		HELPER_ITEM_NUM++;
	}
	
	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = stp_contrl_fill;
		HELPER_ITEM_NUM++;
	}
	
	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = route_contrl_fill;
		HELPER_ITEM_NUM++;
	}
	
	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = acl_contrl_fill;
		HELPER_ITEM_NUM++;
	}
	
	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = vrrp_contrl_fill;
		HELPER_ITEM_NUM++;
	}
	
	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = qos_contrl_fill;
		HELPER_ITEM_NUM++;
	}
	
	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = dhcp_contrl_fill;
		HELPER_ITEM_NUM++;
	}
	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = igmp_contrl_fill;
		HELPER_ITEM_NUM++;
	}
	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = tool_contrl_fill;
		HELPER_ITEM_NUM++;
	}
	if (HELPER_ITEM_NUM < PST_MAX_NUM)
	{
		pstControlSCCreateHelper[HELPER_ITEM_NUM].fill_data_api = fast_forward_contrl_fill;
		HELPER_ITEM_NUM++;
	}
	return HELPER_ITEM_NUM;

	};


int cgiMain()
{
    char *encry=(char *)malloc(BUF_LEN);              
    char *str;    
	STPubInfoForItem stPubInfoForItem;
	memset( &stPubInfoForItem, 0, sizeof(stPubInfoForItem));  
    struct list *lpublic;  
    lpublic=get_chain_head("../htdocs/text/public.txt");
    struct list *local;   
    local=get_chain_head("../htdocs/text/control.txt"); 
	int itemnum = 0;
	stPubInfoForItem.public = lpublic;
	stPubInfoForItem.local = local;
	
	ccgi_dbus_init();
	itemnum = init_pstControlSCCreateHelper();
	memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	strcpy( stPubInfoForItem.encry, encry );
    str=dcryption(encry);
    if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else
    {
    	STSndrContainer *pstControlSndrContainer = NULL;	
    	pstControlSndrContainer = create_sndr_module_container_helper( &stPubInfoForItem, pstControlSCCreateHelper, itemnum );
    	if( NULL != pstControlSndrContainer )
    	{   
    	    pstControlSndrContainer->lpublic=lpublic;
			pstControlSndrContainer->local=local;
    	    pstControlSndrContainer->fp=cgiOut;
    	    
    		SC_writeHtml(pstControlSndrContainer);  
    		release_sndr_module_container( pstControlSndrContainer );  
    	}
    }
    free(encry);
	release(lpublic);  
	release(local);	
	return 0;
}

int ShowIPRoute(char * routeInfo[],int * route_num,int * Kroute_num,int * Sroute_num,int * Croute_num)
{
	int i=0,c=0,k=0,s=0,j=0;
	char * showRoute=(char *)malloc(350);
	memset(showRoute,0,350);

	char  routePath[128];
	memset(routePath,0,128);
	char * revRouteInfo[4];



	strcat(showRoute,"show_route.sh 2>/dev/null | awk '{OFS=\"-\"}NR==4,NR==0{if($1~/S/){print $1,$2,$5,$6,$3,$7} else if($1~/K/){print $1,$2,$4,$5,$3} else if($1~/C/){print $1,$2,$6,$3} else if($1~/R/){print $1,$2,$5,$6,$3} else if($1~/O/){print $1,$2,$5,$7,$3}}'>/var/run/apache2/ip_route.txt");
	system(showRoute);
	FILE *fd;
	char  temp[60];
	memset(temp,0,60);
	if((fd=fopen("/var/run/apache2/ip_route.txt","r"))==NULL)
		{
			//ShowAlert(search(lpublic,"error_open"));
			return 0;
		}
	i=0;
	while((fgets(temp,60,fd)) != NULL)
		{
			strcat(routeInfo[i],temp);
			i++;
			memset(temp,0,60);
		}
	*route_num=i;

	for(i=0;i<*route_num;i++)
		{

			revRouteInfo[0]=strtok(routeInfo[i],"-,");
			//fprintf(stderr,"555revRouteInfo[0]=%s44444",revRouteInfo[0]);
			j=0;
			
			while(revRouteInfo[j]!=NULL && j<3)
				{
					revRouteInfo[j+1]=strtok(NULL,"-,");
					j++;
				}
			if(strstr(revRouteInfo[0],"C")!=NULL)
			c++;
			else if(strstr(revRouteInfo[0],"K")!=NULL)
				k++;
			else if(strstr(revRouteInfo[0],"S")!=NULL)
				s++;
		}
	*Sroute_num=s;
	*Croute_num=c;
	*Kroute_num=k;

	fclose(fd);
	free(showRoute);
	
	return 1;
}



