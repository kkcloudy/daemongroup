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
* ws_portal_container.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* shaojw@autelan.com
*
* DESCRIPTION: 
*
*
*******************************************************************************/
/*
* Copyright (c) 2008,Autelan - AuteCS
* All rights reserved.
*
*$Source: /rdoc/AuteCS/cgic205/portal/ws_portal_container.c,v $
*$Author: zhouyanmei $
*$Date: 2010/02/23 06:07:59 $
*$Revision: 1.10 $
*$State: Exp $
*
*
*/

#include "cgic.h"
#include "ws_portal_container.h"



STLabelHelper st_label_helper[] = {
	{"wp_user_portal.cgi","captive_Portal",0},
	{"wp_white_list.cgi","portal_white_list",0},
	{"wp_eag_conf.cgi","eag_title",0},
	{"wp_user_manage.cgi","user_mng",0},
	{"wp_nasid_byvlan.cgi","nasid_management",0},
	{"wp_multi_portal.cgi","multi_portal_management",0},	
	{"wp_black_list.cgi","portal_black_list",0},
	{"wp_portal_ftp.cgi","portal_ftp",0},
	{"wp_multi_radius.cgi","multi_radius_management",0},
	{"wp_wtpwlan_map_vlan.cgi","vlan_maping",0}
};
#define 	LABEL_ALL_NUM 	sizeof(st_label_helper)/sizeof(st_label_helper[0])



int init_portal_container( STPortalContainer **pp_stPortalContainer )
{
	STPortalContainer *pstPortalContainer=NULL;
	STLabel *pstLabel;
	char url[256]="";
	int iRet = 0;
	int i;

	pstPortalContainer = (STPortalContainer *)malloc(sizeof(STPortalContainer));
	if( NULL == pstPortalContainer )
	{
		iRet = WS_ERR_PORTAL_MEMORY;
		goto error1;
	}
	
//初始化常用公共变量
	memset( pstPortalContainer, 0,sizeof(STPortalContainer) );
	pstPortalContainer->lpublic=get_chain_head(LIST_FILE_PUBLIC);
	pstPortalContainer->llocal=get_chain_head(LIST_FILE_LOCAL);
	pstPortalContainer->lcon=get_chain_head(LIST_FILE_CONTROL);

	cgiFormStringNoNewlines("UN", pstPortalContainer->encry, sizeof(pstPortalContainer->encry));
	
	pstPortalContainer->username_encry=dcryption(pstPortalContainer->encry);
    if( NULL == pstPortalContainer->username_encry )
    {
	    //ShowErrorPage(search(stPageInfo.lpublic,"ill_user")); 	  /*用户非法*/
	    iRet = WS_ERR_PORTAL_ILLEGAL_USER;
		goto error2;
	}
	pstPortalContainer->iUserGroup = checkuser_group( pstPortalContainer->username_encry );

	pstPortalContainer->pstModuleContainer = MC_create_module_container();
	if( NULL == pstPortalContainer->pstModuleContainer )
	{
		iRet = WS_ERR_PORTAL_MEMORY;
		goto error3;
	}
	pstPortalContainer->fp = cgiOut;
//初始化完毕

//添加label
	for( i=0; i<LABEL_ALL_NUM; i++ )
	{
		if( 1 == st_label_helper[i].user_group_mask && 0 != pstPortalContainer->iUserGroup )
		{//如果当前用户不是管理员用户，并且该label只有管理员用户可见的时候，不添加该label，继续判断下一个label。
			continue;	
		}
		
		pstLabel = LB_create_label();
		if( NULL != pstLabel )
		{
			LB_setLabelName( pstLabel, search(pstPortalContainer->llocal, st_label_helper[i].name_key) );
		
			snprintf( url, sizeof(url), "%s?UN=%s", st_label_helper[i].cgi_name, pstPortalContainer->encry );
			LB_setLabelUrl( pstLabel, url ); 
			MC_addLabel( pstPortalContainer->pstModuleContainer, pstLabel );
		}
			
	}

	MC_setActiveLabel( pstPortalContainer->pstModuleContainer, 0 );
	
	MC_setOutPutFileHandle( pstPortalContainer->pstModuleContainer, pstPortalContainer->fp );
	
	MC_setOutPutFileHandle( pstPortalContainer->pstModuleContainer, cgiOut );
	
	MC_setModuleContainerDomainValue( pstPortalContainer->pstModuleContainer, PAGE_TITLE, search(pstPortalContainer->llocal,"portal_title") );
	MC_setModuleContainerDomainValue( pstPortalContainer->pstModuleContainer, MODULE_TITLE, search(pstPortalContainer->llocal,"portal_title") );
	MC_setModuleContainerDomainValue( pstPortalContainer->pstModuleContainer, FORM_ENCTYPE, "multipart/form-data" );

	MC_setModuleContainerDomainValue( pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, pstPortalContainer->encry );
	
	
	MC_setModuleContainerDomainValue( pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(pstPortalContainer->lpublic,"img_ok") );
	
	MC_setModuleContainerDomainValue( pstPortalContainer->pstModuleContainer, BTN_CANCEL_IMG, search(pstPortalContainer->lpublic,"img_cancel") );
	snprintf( url, sizeof(url), "wp_authentication.cgi?UN=%s", pstPortalContainer->encry );
	MC_setModuleContainerDomainValue( pstPortalContainer->pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_setModuleContainerDomainValue( pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );
	
	
	*pp_stPortalContainer = pstPortalContainer;
	return iRet;
	
	
error3:
error2:
	release( pstPortalContainer->lpublic );
	pstPortalContainer->lpublic = NULL;
	
	release( pstPortalContainer->llocal );
	pstPortalContainer->llocal = NULL;
	
	free( pstPortalContainer );
	pstPortalContainer = NULL;	
	
error1:
	*pp_stPortalContainer = NULL;
	
	return iRet;
}


int release_portal_container( STPortalContainer **pp_stPortalContainer )
{
	if( NULL == pp_stPortalContainer || NULL == *pp_stPortalContainer )	
	{
		return WS_ERR_PORTAL_PORINTER;	
	}
	
	if( NULL != (*pp_stPortalContainer)->lpublic )
	{
		release( (*pp_stPortalContainer)->lpublic );
		(*pp_stPortalContainer)->lpublic = NULL;
	}
	
	if( NULL != (*pp_stPortalContainer)->llocal )
	{
		release( (*pp_stPortalContainer)->llocal );
		(*pp_stPortalContainer)->llocal = NULL;
	}
	if( NULL != (*pp_stPortalContainer)->lcon )
	{
		release( (*pp_stPortalContainer)->lcon );
		(*pp_stPortalContainer)->lcon = NULL;
	}
	
	MC_destroy_module_container( &((*pp_stPortalContainer)->pstModuleContainer) );
	
	free( *pp_stPortalContainer );
	*pp_stPortalContainer = NULL;
	
	return 0;
}
