/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* wp_wlan.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
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
#include "wcpss/asd/asd.h"
#include "ws_security.h"
#include "wcpss/wid/WID.h"
#include "ws_dcli_wqos.h"
#include "ws_sta.h" 
#include "ws_sndr_cfg.h"
#include "ws_list_container.h"
#include "ws_secondary_container.h"  
#include "ws_dcli_ebr.h"
#include "ws_dcli_vrrp.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

static int ShowContRlPage(struct secondary_module_container *p );
//一共是八个
int wlan_security_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //1 ,无线安全，都有
{
	struct list *lsecu = NULL;     /*解析security.txt文件的链表头*/  
	lsecu=get_chain_head("../htdocs/text/security.txt");

	p_item->fp=cgiOut;

	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128] = { 0 };
	memset(temp_url,0,sizeof(temp_url));
	
	SI_set_label_name( p_item, search(lsecu,"wlan_sec")); 
	SI_set_label_img( p_item,"/images/WlanSecur.jpg");
	snprintf(temp_url,sizeof(temp_url)-1,"wp_seculis.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  

	struct dcli_security *shead = NULL;  
	int result1 = 0;
	int sec_num = 0;
	char num[10] = { 0 };
	memset(num,0,sizeof(num));
	if(p_pubinfo->ins_para)
	{
		result1=show_security_list(p_pubinfo->ins_para->parameter, p_pubinfo->ins_para->connection,&shead,&sec_num);
	}
	if(result1 == 1)
	  snprintf(num,sizeof(num)-1,"%d",sec_num);
	else	  
	  strncpy(num,"0",sizeof(num)-1);
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	SI_set_summary_title( p_item,search(lsecu,"wlan_sec")); 		
	SI_set_summary_keyinfo( p_item, search(lsecu,"security"));	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));
	
	SI_set_summary_key( p_item,num);
	
	if(result1 == 1)
	{
		Free_security_head(shead);
	}
	
	release(lsecu); 
	return 0;
}


int wlan_list_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //2,wlan 都有
{
    
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128] = { 0 };
	memset(temp_url,0,sizeof(temp_url));
	char num[10] = { 0 };
	memset(num,0,sizeof(num));
	
	SI_set_label_name( p_item, "WLAN"); 
	SI_set_label_img( p_item,"/images/WLAN.jpg");
	snprintf(temp_url,sizeof(temp_url)-1,"wp_wlanlis.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
    int wnum = 0;        /*WLAN个数*/
	DCLI_WLAN_API_GROUP *WLANINFO = NULL;
	int result2 = 0;
	if(p_pubinfo->ins_para)
	{
		result2=show_wlan_list(p_pubinfo->ins_para->parameter, p_pubinfo->ins_para->connection,&WLANINFO);	  
	}
	if(result2 == 1)
	{
		wnum = WLANINFO->wlan_num;
	}
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));
	SI_set_summary_title( p_item, "WLAN" );		
	SI_set_summary_keyinfo( p_item, "WLAN");	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));	
	
	snprintf(num,sizeof(num)-1,"%d",wnum);
	
	SI_set_summary_key( p_item, num);	
	if(result2 == 1)
	{
		Free_wlan_head(WLANINFO);
	}
	
	return 0;
}


int ap_list_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //3,ap 都有
{
   
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128] = { 0 };
	memset(temp_url,0,sizeof(temp_url));
	char num[10] = { 0 };
	memset(num,0,sizeof(num));
	
	SI_set_label_name( p_item, search(p_pubinfo->local,"ap")); 
	SI_set_label_img( p_item,"/images/AP.jpg");
	snprintf(temp_url,sizeof(temp_url)-1,"wp_wtplis.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	DCLI_WTP_API_GROUP_ONE *wtphead = NULL;
    int wtpnum = 0;      /*WTP个数*/
	int result3 = 0;
	if(p_pubinfo->ins_para)
	{
		result3=show_wtp_list_new_cmd_func(p_pubinfo->ins_para->parameter, p_pubinfo->ins_para->connection,&wtphead); 
	}
	if(result3 == 1)
	{
		if((wtphead)&&(wtphead->WTP_INFO))
		{
			wtpnum = wtphead->WTP_INFO->list_len;
		}
	}
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));
	SI_set_summary_title( p_item, search(p_pubinfo->local,"ap") );		
	SI_set_summary_keyinfo( p_item, "AP");	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));	
	
	snprintf(num,sizeof(num)-1,"%d",wtpnum);
	
	SI_set_summary_key( p_item, num);
	
	if(result3==1)
	{
      Free_wtp_list_new_head(wtphead);
	}
	
	return 0;
}

int ap_monitor_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //4,ap监控 偏左
{
   	
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128] = { 0 };
	memset(temp_url,0,sizeof(temp_url));	
	char name[60] = { 0 };
	memset(name,0,sizeof(name));
	snprintf(name,sizeof(name)-1,"AP %s",search(p_pubinfo->public,"monitor"));
	SI_set_label_name( p_item,name); 
	SI_set_label_img( p_item,"/images/ApMonitor.jpg");
	snprintf(temp_url,sizeof(temp_url)-1,"wp_wtpmor.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	/////////////////////////
	//char num[10];
	//memset(num,0,sizeof(num));
	//SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));
	//SI_set_summary_title( p_item, search(p_pubinfo->local,"ap") );		
	//SI_set_summary_keyinfo( p_item, "AP");	
	//SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));	
	
	//snprintf(num,sizeof(num)-1,"%d",wtpnum);	
	//SI_set_summary_key( p_item, num);
	
	
	return 0;
}



int switch_in_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //5,接入用户 都有
{
   
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128] = { 0 };
	memset(temp_url,0,sizeof(temp_url));
	char num[10] = { 0 };
	memset(num,0,sizeof(num));
	
	SI_set_label_name( p_item, search(p_pubinfo->local,"station")); 
	SI_set_label_img( p_item,"/images/Station.jpg");
	snprintf(temp_url,sizeof(temp_url)-1,"wp_stasumary.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	struct dcli_ac_info *ac = NULL;
	int result4 = 0;	
	if(p_pubinfo->ins_para)
	{
		result4=show_sta_summary(p_pubinfo->ins_para->parameter, p_pubinfo->ins_para->connection,&ac,NULL);
	}
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));
	SI_set_summary_title( p_item, search(p_pubinfo->local,"station"));		
	SI_set_summary_keyinfo( p_item, search(p_pubinfo->local,"station"));	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));	

	if(result4 == 1)
	  snprintf(num,sizeof(num)-1,"%d",ac->num_sta);
	else
	  strncpy(num,"0",sizeof(num)-1);
	
	SI_set_summary_key( p_item, num);
	
	if(result4 == 1)
		Free_sta_summary(ac);
	
	return 0;
}




int rf_control_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //6,RF管理 都有
{
  
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128] = { 0 };
	memset(temp_url,0,sizeof(temp_url));
	char num[10] = { 0 };
	memset(num,0,sizeof(num));
	char name[60] = { 0 };
	memset(name,0,sizeof(name));
	snprintf(name,sizeof(name)-1,"RF %s",search(p_pubinfo->public,"management"));
	SI_set_label_name( p_item,name); 
	SI_set_label_img( p_item,"/images/Radio.jpg");
	snprintf(temp_url,sizeof(temp_url)-1,"wp_radiolis.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	DCLI_RADIO_API_GROUP_ONE *head = NULL;
	int result5 = 0;	
	if(p_pubinfo->ins_para)
	{
		result5=show_radio_list(p_pubinfo->ins_para->parameter, p_pubinfo->ins_para->connection,&head);
	}
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));
	SI_set_summary_title( p_item, "Radio");		
	SI_set_summary_keyinfo( p_item, "Radio");	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));	
	
	if(result5 == 1)
		snprintf(num,sizeof(num)-1,"%d",head->radio_num);
	else
	  	strncpy(num,"0",sizeof(num)-1);
	SI_set_summary_key( p_item, num);
	
	if(result5 == 1)
	{
    	Free_radio_head(head);
	}
	return 0;
}


int advanced_param_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //7,高级参数配置 偏左
{
  
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128] = { 0 };
	memset(temp_url,0,sizeof(temp_url));	
	
	SI_set_label_name( p_item, search(p_pubinfo->local,"adv_conf")); 
	SI_set_label_img( p_item,"/images/adv.jpg");
	snprintf(temp_url,sizeof(temp_url)-1,"wp_wcsumary.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	/////////////////////////
	//char num[10];
	//memset(num,0,sizeof(num));
	//SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));
	//SI_set_summary_title( p_item, search(p_pubinfo->local,"ap") );		
	//SI_set_summary_keyinfo( p_item, "AP");	
	//SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));	
	
	//snprintf(num,sizeof(num)-1,"%d",wtpnum);	
	//SI_set_summary_key( p_item, num);
	
	return 0;
}




int qos_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //8,无线QOS 都有
{
    
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128] = { 0 };
	memset(temp_url,0,sizeof(temp_url));
	char num[10] = { 0 };
	memset(num,0,sizeof(num));
	char name[60] = { 0 };
	memset(name,0,sizeof(name));
	snprintf(name,sizeof(name)-1,"%s QOS",search(p_pubinfo->local,"wireless"));
	SI_set_label_name( p_item,name); 
	SI_set_label_img( p_item,"/images/wqos.jpg");
	snprintf(temp_url,sizeof(temp_url)-1,"wp_wqoslis.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	DCLI_WQOS *wqos = NULL;
	int result6 = 0;	
	if(p_pubinfo->ins_para)
	{
		result6=show_wireless_qos_profile_list(p_pubinfo->ins_para->parameter, p_pubinfo->ins_para->connection,&wqos);
	}
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));
	SI_set_summary_title( p_item, name);		
	SI_set_summary_keyinfo( p_item, name);	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));	

	if(result6 == 1)
	  snprintf(num,sizeof(num)-1,"%d",wqos->qos_num);
	else
	  strncpy(num,"0",sizeof(num)-1);
	
	SI_set_summary_key( p_item, num);
	
	if(result6 == 1)
	{
		Free_qos_head(wqos);		
	}
	return 0;
}


int ebr_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //9,EBR都有
{
    
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128] = { 0 };
	memset(temp_url,0,sizeof(temp_url));	
	char num[10] = { 0 };
	memset(num,0,sizeof(num));
	char name[60] = { 0 };
	memset(name,0,sizeof(name));
	snprintf(name,sizeof(name)-1,"EBR %s",search(p_pubinfo->public,"management"));
	SI_set_label_name( p_item,name); 
	SI_set_label_img( p_item,"/images/ebr.jpg");
	snprintf(temp_url,sizeof(temp_url)-1,"wp_ebrlis.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
		
	DCLI_EBR_API_GROUP  * ebrinfo = NULL;   
	int result7=0;	
	if(p_pubinfo->ins_para)
	{
		result7=show_ethereal_bridge_list(p_pubinfo->ins_para->parameter, p_pubinfo->ins_para->connection,&ebrinfo);
	}	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));
	SI_set_summary_title( p_item, name);		
	SI_set_summary_keyinfo( p_item, name);	
	SI_set_summary_keyvalue( p_item,search(p_pubinfo->public,"total"));	
	if(result7 == 1)
		snprintf(num,sizeof(num)-1,"%d",ebrinfo->ebr_num);
	else
		snprintf(num,sizeof(num)-1,"%d",0);
	
	SI_set_summary_key( p_item, num);
	
	if(result7 == 1)
	{
		Free_ethereal_bridge_head(ebrinfo);
	}
	return 0;
}



int wids_fill( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )  //9,EBR都有
{
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128] = { 0 };
	memset(temp_url,0,sizeof(temp_url));
	char name[60] = { 0 };
	memset(name,0,sizeof(name));
	snprintf(name,sizeof(name)-1,"%s %s",search(p_pubinfo->local,"wids"),search(p_pubinfo->public,"management"));
	SI_set_label_name( p_item,name); 
	SI_set_label_img( p_item,"/images/wids.jpg");
	snprintf(temp_url,sizeof(temp_url)-1,"wp_showWids.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	return 0;
}


STSCCreateHelper pstControlSCCreateHelper[] = {
	#if  SNDR_SECURITY_ITEM
	{wlan_security_fill},
	#endif 

	#if  SNDR_WLAN_ITEM
	{wlan_list_fill},
	#endif 

	#if  SNDR_POINT_ITEM
	{ap_list_fill},
	#endif 

	/*#if  SNDR_MONITOR_ITEM
	{ap_monitor_fill},
	#endif */

	#if  SNDR_STATION_ITEM
	{switch_in_fill},
	#endif 

	#if  SNDR_RF_ITEM
	{rf_control_fill},
	#endif 

	#if  SNDR_ADVANCED_ITEM
	{advanced_param_fill},
	#endif 

	#if  SNDR_WLAN_QOS_ITEM
	{qos_fill},
	#endif 

	#if  SNDR_EBR_ITEM
	{ebr_fill},
	#endif

	#if SNDR_WIDS_ITEM
	{wids_fill}
	#endif
};

#define HELPER_ITEM_NUM  sizeof(pstControlSCCreateHelper)/sizeof(pstControlSCCreateHelper[0])


int cgiMain()
{
	int instRun = DCLI_VRRP_INSTANCE_NO_CREATED;
    char encry[BUF_LEN] = { 0 };              
    char *str = NULL; 	
	dbus_parameter parameter;
	instance_parameter *paraHead1 = NULL;
	char plotid[10] = { 0 };
	memset(plotid,0,sizeof(plotid));
	
	DcliWInit();
	ccgi_dbus_init();   
	
    cgiFormStringNoNewlines("plotid", plotid, 10); 
	if(strcmp(plotid,"")==0)
	{
		list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB);
	}
	else
	{
		get_slotID_localID_instanceID(plotid,&parameter);
	}

	STPubInfoForItem stPubInfoForItem;
	memset( &stPubInfoForItem, 0, sizeof(stPubInfoForItem));  
    struct list *lpublic = NULL;  
    lpublic=get_chain_head("../htdocs/text/public.txt");
    struct list *local = NULL;   
    local=get_chain_head("../htdocs/text/wlan.txt"); 

	stPubInfoForItem.public = lpublic;
	stPubInfoForItem.local = local;	
	if(strcmp(plotid,"")==0)
	{
		if(paraHead1)
		{
			stPubInfoForItem.ins_para = paraHead1;
		}
	}
	else
	{
		get_instance_dbus_connection(parameter, &(stPubInfoForItem.ins_para), INSTANCE_STATE_WEB);
	}

    memset(encry,0,sizeof(encry));
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	strncpy( stPubInfoForItem.encry, encry, sizeof(stPubInfoForItem.encry)-1 );
    str=dcryption(encry);
    if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else
    {
    	STSndrContainer *pstControlSndrContainer = NULL;	
    	pstControlSndrContainer = create_sndr_module_container_helper( &stPubInfoForItem, pstControlSCCreateHelper, HELPER_ITEM_NUM );
    	if( NULL != pstControlSndrContainer )
    	{   
    	    pstControlSndrContainer->lpublic=lpublic;
			pstControlSndrContainer->local=local;
    	    pstControlSndrContainer->fp=cgiOut;
            MC_setPageCallBack_z(pstControlSndrContainer, (EX_SHOW_CALL_BACK_NZ) ShowContRlPage,NULL);
    		SC_writeHtml(pstControlSndrContainer);  
    		release_sndr_module_container( pstControlSndrContainer );  
    	}
    }

	free_instance_parameter_list(&(stPubInfoForItem.ins_para));
	free_instance_parameter_list(&paraHead1);
	release(lpublic);  
	release(local);	
	destroy_ccgi_dbus();
	return 0;
}

static int ShowContRlPage(struct secondary_module_container *p )
{  	
	struct list *lpublic = p->lpublic;
	char encryz[50] = { 0 };
	memset(encryz,0,sizeof(encryz));
	char plotid[10] = { 0 };
	memset(plotid,0,sizeof(plotid));
	instance_parameter *paraHead2 = NULL;
	instance_parameter *pq = NULL;
	char temp[10] = { 0 };

	list_instance_parameter(&paraHead2, INSTANCE_STATE_WEB);
    cgiFormStringNoNewlines("UN", encryz, 50); 
    cgiFormStringNoNewlines("plotid", plotid, 10); 

	/****************************end Fillet rectangular top******************************/
	fprintf( cgiOut, "<tr><td>\n");
	fprintf( cgiOut, "%s ID:&nbsp;&nbsp;",search(lpublic,"instance"));
	fprintf( cgiOut, "<select name=insid onchange=plotid_change(this)>");
    for(pq=paraHead2;(NULL != pq);pq=pq->next)
    {
       memset(temp,0,sizeof(temp));
	   snprintf(temp,sizeof(temp)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);
       if(strcmp(plotid,temp) == 0)
		   fprintf(cgiOut,"<option value='%s' selected>%s</option>\n",temp,temp);
	   else	       
	       fprintf(cgiOut,"<option value='%s'>%s</option>\n",temp,temp);
    }
	fprintf( cgiOut, "</select>\n");
	fprintf( cgiOut, "</td></tr>\n");	
	fprintf(cgiOut,"<script type=text/javascript>\n");
   	fprintf(cgiOut,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_wlan.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", encryz);
    fprintf(cgiOut,"</script>\n" );
	/******************************Fillet rectangular bottom********************************/	  

	free_instance_parameter_list(&paraHead2);
	return 0;	
}


