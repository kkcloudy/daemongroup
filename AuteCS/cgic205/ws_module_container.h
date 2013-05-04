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
* ws_module_container.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
/*
* Copyright (c) 2008,Autelan - AuteCS
* All rights reserved.
*
*$Source: /rdoc/accapi/nm/app/snmp/ws_module_container.h,v $
*$Author: shaojunwu $
*$Date: 2010/05/31 11:51:50 $
*$Revision: 1.1 $
*$State: Exp $
*
*$Log: ws_module_container.h,v $
*Revision 1.1  2010/05/31 11:51:50  shaojunwu
*modify for some ws has  html output for can't contain in libnm
*
*Revision 1.5  2010/02/09 07:16:54  zhouyanmei
*添加版权注释
*
*Revision 1.4  2010/02/09 06:56:46  zhouyanmei
*添加版权注释
*
*Revision 1.3  2008/12/19 03:53:44  tangsiqi
*新增函数LB_changelabelName_Byindex,LB_changelabelUrl_Byindex
*
*Revision 1.2  2008/12/05 03:55:26  shaojunwu
*解决页面cancel图片对齐的问题
*
*
*/
#ifndef _WS_MODULE_CONTAINER_H
#define _WS_MODULE_CONTAINER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*****************************************************************
define of function
*****************************************************************/

typedef struct st_label {
	char *name;
	char *url;
	
	struct st_label *next;
}STLabel;

// end define of function



/*****************************************************************
define of module_container
*****************************************************************/
//please don't change any of the ENKeyOfDomain  element  value.
//if you add new element, you must add before KEY_DOMAIN_MUN and do not set the new element special value!!!!!  
typedef enum en_key_of_domain{
	PAGE_TITLE,
	MODULE_TITLE,
	
	FORM_ACTION,
	FORM_ENCTYPE,
	FORM_ONSIBMIT,
	FORM_METHOD,
	
	BTN_OK_IMG,
	BTN_OK_URL,
	BTN_OK_SUBMIT_NAME,
	BTN_OK_SUBMIT_VALUE,

	BTN_CANCEL_IMG,	
	BTN_CANCEL_URL,
	
	LABLE_TOP_HIGHT,
	LABLE_BOTTOM_HIGHT,
	
	PUBLIC_INPUT_ENCRY,
	
	KEY_DOMAIN_MUN
}ENKeyOfDomain;


typedef int (*MC_CALLBACK)(void *);

typedef struct st_module_container{
	FILE *outputFileHandle;
	char *domains[KEY_DOMAIN_MUN];
	
	//functions
	unsigned int ulAllLabelNum;
	unsigned int ulActiveLabelIndex;
	STLabel *pstLabelRoot;
	
	//call back
	MC_CALLBACK prefix_of_page;
	void *prefix_callback_param;
	
	MC_CALLBACK content_of_page;
	void *content_callback_param;
}STModuleContainer;

//end define of module_container

//function define
STLabel *LB_create_label();

int LB_setLabelName( STLabel *me, char *name );

int LB_setLabelUrl( STLabel *me, char *url );

int LB_changelabelName_Byindex( STModuleContainer *me, char * LabelName, int index);

int LB_changelabelUrl_Byindex( STModuleContainer *me , char * url, int index);

int LB_destroy_label( STLabel **me );



STModuleContainer *MC_create_module_container();

int MC_destroy_module_container( STModuleContainer **me );

int MC_setOutPutFileHandle( STModuleContainer* me, FILE *outputFileHandle );

int MC_setModuleContainerDomainValue( STModuleContainer* me, ENKeyOfDomain key,char *value );

int MC_setActiveLabel( STModuleContainer* me, unsigned int ulIndex );

STLabel *MC_getLabelByIndex( STModuleContainer *me, unsigned int ulIndex );

int MC_addLabel( STModuleContainer *me, STLabel *pstLabel );

int MC_delLabelByIndex( STModuleContainer *me, unsigned int ulIndex );

int MC_setPrefixOfPageCallBack( STModuleContainer *me, MC_CALLBACK callback, void *callback_param );

int MC_setContentOfPageCallBack( STModuleContainer *me, MC_CALLBACK callback, void *callback_param );

int MC_writeHtml( STModuleContainer *me );




#endif

