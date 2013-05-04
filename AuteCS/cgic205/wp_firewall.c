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
* wp_firewall.c
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
#include "ws_tcrule.h"
#include "ws_firewall.h"
#include "ws_sndr_cfg.h"
#include "ws_list_container.h"
#include "ws_secondary_container.h"  
//#include "dcli_main.h"  

#define _MANAGE_FIREWALL_ 1

#ifdef _MANAGE_FIREWALL_ 
DBusConnection *dcli_dbus_connection = NULL;

int firewall_fill_summary( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )
{
	p_item->fp=cgiOut;
	
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	SI_set_label_name( p_item, search(p_pubinfo->local,"title_fw")); 
	SI_set_label_img( p_item,"/images/Firewall.jpg");
	sprintf(temp_url,"wp_fwruleview.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	SI_set_summary_title( p_item,search(p_pubinfo->local,"title_fw") );			
	SI_set_summary_keyinfo( p_item, search(p_pubinfo->local,"firewall_state") );	
	//SI_set_summary_keyvalue( p_item, search(lper,"sys_scan") );
	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));	 
	int ret;
	//ret=fwServiceStatus();//原来的
	//DBusConnection *connection;
	u_long service_status = 0;
	fwRule *rule_array = NULL;
	u_long rule_num = 0;
	ret = ac_manage_show_firewall_rule(dcli_dbus_connection, &service_status, &rule_array, &rule_num);
	//ret = ac_manage_show_firewall_rule(connection, NULL, &rule_array, &rule_num);
	//ret=ac_manage_show_firewall_rule(DBusConnection *connection, u_long *service_status, fwRule **rule_array, u_long *rule_num);
    if( ret==0  )
	  SI_set_summary_key( p_item,search(p_pubinfo->local,"fw_running") );
	else
	  SI_set_summary_key( p_item,search(p_pubinfo->local,"fw_stoped") );
	
	return 0;
}

int portflow_fill_summary( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )
{
  
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	int ret;
	char num[10];
	memset(num,0,10);
	
	SI_set_label_name( p_item, search(p_pubinfo->local,"title_tc")); 
	SI_set_label_img( p_item,"/images/LiuKong.jpg");
	sprintf(temp_url,"wp_tcruleview.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	SI_set_summary_title( p_item, search(p_pubinfo->local,"title_tc") );		
	SI_set_summary_keyinfo( p_item, search(p_pubinfo->local,"tc_rulenum") );	
	//SI_set_summary_keyvalue( p_item, search(lper,"sys_scan") );
	ret=getTcRuleNum();
	sprintf(num,"%d",ret);
	SI_set_summary_key( p_item, num);	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));

	return 0;
}
STSCCreateHelper pstControlSCCreateHelper[] = {
	#if SNDR_FIREWALL_ITEM
	{firewall_fill_summary},
	#endif

	#if SNDR_TRAFFIC_ITEM
	{portflow_fill_summary}
	#endif
};

#define HELPER_ITEM_NUM  sizeof(pstControlSCCreateHelper)/sizeof(pstControlSCCreateHelper[0])

#endif

int cgiMain()
{
#ifdef _MANAGE_FIREWALL_ 
    char *encry=(char *)malloc(BUF_LEN);              
    char *str;    

	STPubInfoForItem stPubInfoForItem;
	memset( &stPubInfoForItem, 0, sizeof(stPubInfoForItem));  
    struct list *lpublic;  
    lpublic=get_chain_head("../htdocs/text/public.txt");
    struct list *local;   
    local=get_chain_head("../htdocs/text/firewall.txt"); 

	stPubInfoForItem.public = lpublic;
	stPubInfoForItem.local = local;
	
    memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	strcpy( stPubInfoForItem.encry, encry );
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
    		SC_writeHtml(pstControlSndrContainer);  
    		release_sndr_module_container( pstControlSndrContainer );  
    	}
    }
	
    free(encry);
	release(lpublic);  
	release(local);
#endif	
	return 0;
}

