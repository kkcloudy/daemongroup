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
* wp_help.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system help 
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
#include "ws_sndr_cfg.h"
#include "ws_list_container.h"
#include "ws_secondary_container.h"   //引用的新的


static int ShowHelpPage(struct secondary_module_container *p );

int help_fill_summary( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )
{
    struct list *lhelp;     /*解析help.txt文件的链表头*/  
    lhelp=get_chain_head("../htdocs/text/help.txt");
	
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	SI_set_label_name( p_item, search(lhelp,"help")); ///images/SysFun.jpg
	SI_set_label_img( p_item,"/images/Help.jpg");
	sprintf(temp_url,"wp_help.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
    release(lhelp);	
	return 0;
}

STSCCreateHelper pstControlSCCreateHelper[] = {
	#if SNDR_HELP_ITEM
	{help_fill_summary}	
	#endif
};

#define HELPER_ITEM_NUM  sizeof(pstControlSCCreateHelper)/sizeof(pstControlSCCreateHelper[0])

int cgiMain()
{
    char *encry=(char *)malloc(BUF_LEN);              
    char *str;    

	STPubInfoForItem stPubInfoForItem;
	memset( &stPubInfoForItem, 0, sizeof(stPubInfoForItem));  
    struct list *lpublic;  
    lpublic=get_chain_head("../htdocs/text/public.txt");
    struct list *local;   
    local=get_chain_head("../htdocs/text/help.txt"); 

  
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
    	    pstControlSndrContainer->fp=cgiOut;
			pstControlSndrContainer->lpublic=lpublic;
			pstControlSndrContainer->local=local;
 
            MC_setPageCallBack(pstControlSndrContainer, (EX_SHOW_CALL_BACK_N) ShowHelpPage,NULL);
			
			SC_writeHtml(pstControlSndrContainer);  
    		release_sndr_module_container( pstControlSndrContainer );  
    	}
    }
	
    free(encry);
	release(lpublic);  
	release(local);	
	return 0;
}

static int ShowHelpPage(struct secondary_module_container *p )
{  
/****************************end Fillet rectangular top******************************/

  fprintf(cgiOut,"<table align=center border=0  bgcolor=#ffffff cellpadding=0 cellspacing=0>"\
      "<tr><td height=20>&nbsp;</td></tr>");
  fprintf(cgiOut, "<tr align=center height=20>"\
  	                 "<td style=font-size:14px>%s</td></tr>",
  	             "请联系技术支持或者索要产品文档手册");
  fprintf(cgiOut, "<tr align=left height=15><td><td/></tr>");
  fprintf(cgiOut, "<tr align=center height=20>"\
  	                 "<td style=font-size:14px>%s</td></tr>",
  	             "Please contact technical support or ask for product documentation manual");
  fprintf(cgiOut, "<tr align=left height=15><td><td/></tr>");
  fprintf(cgiOut,"</table>");

/******************************Fillet rectangular bottom********************************/	  
return 0;	
}

