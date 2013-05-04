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
* wp_authentication.c
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
#include "ws_sndr_cfg.h"
#include "ws_list_container.h"
#include "ws_secondary_container.h"
#include "ws_user_manage.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"


int getActiveUserNum();
static int ShowContRlPage(struct secondary_module_container *p );
int portal_fill_summary( STPubInfoForItem *p_pubinfo, STSndrItem *p_item )
{	
	STUserManagePkg *pstReq;
	STUserManagePkg *pstRsp;

	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	int ret;
	int instRun = DCLI_VRRP_INSTANCE_NO_CREATED;
	char num[10];
	memset(num,0,10);

	char tmp_name[60];
	memset(tmp_name,0,60);
	sprintf(tmp_name,"Portal %s",search(p_pubinfo->local,"word_auth"));
	
	SI_set_label_name( p_item,tmp_name); 
	SI_set_label_img( p_item,"/images/PortalCer.jpg");
	sprintf(temp_url,"wp_user_portal.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	SI_set_summary_title( p_item, search(p_pubinfo->local ,"portal_title"));		
	SI_set_summary_keyinfo( p_item, search(p_pubinfo->local ,"portal_user_num"));	
	SI_set_summary_keyvalue( p_item, search(p_pubinfo->public,"total") );
	#if 1
        char plotid[10];
	memset(plotid,0,10);       
         cgiFormStringNoNewlines("plotid", plotid, 10); 
	int vrrp_id_input;	
	vrrp_id_input=strtoul(plotid,0,10);                                                /*the vrrp_id which user input*/
	if(vrrp_id_input>0)
	{      
		instRun = ccgi_vrrp_hansi_is_running(vrrp_id_input);
		if(instRun != DCLI_VRRP_INSTANCE_CREATED)
		{		  
		  vrrp_id_input = 0;
		}
	}
	
	pstReq = createRequirePkg( REQ_GET_ONLINE_NUM, NULL, NULL );
	if( pstReq == NULL )
	{
		ret = 0;
		fprintf(p_item->fp , "createRequirePkg error!!");
		goto SHOW_USER_NUM;
	}
          pstReq->vrrp_id=vrrp_id_input;
	 pstReq->eagins_id = 0;
	ret = doRequire(pstReq, ntohl(inet_addr("127.0.0.1")), 2001, 5, &(pstRsp));
	if(pstRsp == NULL || -1 == ret )
	{
		ret = 0;
		fprintf(stderr , "doRequire ret response NULL!!");
	}
	else
	{
		ret = pstRsp->all_user_num;
		fprintf(stderr,  "doRequire get response USER_NUM=%d!!",ret);
	}
	#else
	ret=getActiveUserNum();
	#endif
SHOW_USER_NUM:	
	sprintf(num,"%d",ret);
	SI_set_summary_key( p_item, num);	
	SI_set_summary_font(p_item,search(p_pubinfo->public,"menu_summary"));

	return 0;
}

STSCCreateHelper pstControlSCCreateHelper[] = {	
	#if SNDR_PORTAL_ITEM
	{portal_fill_summary}
	#endif 
};

#define HELPER_ITEM_NUM  sizeof(pstControlSCCreateHelper)/sizeof(pstControlSCCreateHelper[0])
# if 0
int cgiMain()
{
    char *encry=(char *)malloc(BUF_LEN);              
    char *str;    

	
	STPubInfoForItem stPubInfoForItem;
	memset( &stPubInfoForItem, 0, sizeof(stPubInfoForItem));  
    struct list *lpublic;  
    lpublic=get_chain_head("../htdocs/text/public.txt");
    struct list *local;   
    local=get_chain_head("../htdocs/text/authentication.txt"); 

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
	return 0;
}
#endif

int cgiMain()
{
        int instRun = DCLI_VRRP_INSTANCE_NO_CREATED;
        char *encry=(char *)malloc(BUF_LEN);              
        char *str;    

	
    	char plotid[10];
    	memset(plotid,0,10);
        cgiFormStringNoNewlines("plotid", plotid, 10); 
        int p_id;
	p_id=strtoul(plotid,0,10);
//	DcliWInit();
	ccgi_dbus_init();   
	if(p_id>0)
	{
            	  instRun = ccgi_vrrp_hansi_is_running(p_id);
            	  if(instRun != DCLI_VRRP_INSTANCE_CREATED)
            	  {
            	  	p_id = 0;
            	  }
	} 	
    	STPubInfoForItem stPubInfoForItem;
    	memset( &stPubInfoForItem, 0, sizeof(stPubInfoForItem));  
        struct list *lpublic;  
        lpublic=get_chain_head("../htdocs/text/public.txt");
        struct list *local;   
        local=get_chain_head("../htdocs/text/authentication.txt"); 

	stPubInfoForItem.public = lpublic;
	stPubInfoForItem.local = local;
	stPubInfoForItem.plotid = p_id;
	
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
            MC_setPageCallBack_z(pstControlSndrContainer, (EX_SHOW_CALL_BACK_NZ) ShowContRlPage,NULL);
            SC_writeHtml(pstControlSndrContainer);  
            release_sndr_module_container( pstControlSndrContainer );  
    	}
    }
	
    free(encry);
	release(lpublic);  
	release(local);	
	return 0;
}

int getActiveUserNum()
{
#define CP_RECORD_DB "/var/run/cpp/cp_user_record.db"
	FILE *fp;
	char line[64];
	int ret = 0;
	
	fp = fopen( CP_RECORD_DB, "r" );
	if( NULL != fp )
	{
		memset( line, 0, sizeof(line) );
		fgets( line, sizeof(line), fp );
		
		if( strlen( line ) > 0 ) 
		{
			sscanf( line, "%d", &ret );
		}
		
		fclose( fp );
	}
	
	return ret;
}

static int ShowContRlPage(struct secondary_module_container *p )
{  	
	int instRun = DCLI_VRRP_INSTANCE_NO_CREATED;
	struct list *lpublic = p->lpublic;
	int i;
	char encryz[50];
	memset(encryz,0,50);
	char plotid[10];
	memset(plotid,0,10);
         cgiFormStringNoNewlines("UN", encryz, 50); 
         cgiFormStringNoNewlines("plotid", plotid, 10); 
	int p_id;
	
	p_id=strtoul(plotid,0,10);
	
	if(p_id>0)
	{
		instRun = ccgi_vrrp_hansi_is_running(p_id);
		if(instRun != DCLI_VRRP_INSTANCE_CREATED)
		{
		  ShowAlert(search(lpublic,"instance_not_exist"));
		  p_id = 0;
		}
	}
	/****************************end Fillet rectangular top******************************/
	fprintf( cgiOut, "<tr><td>\n");
	fprintf( cgiOut, "%s ID:&nbsp;&nbsp;",search(lpublic,"instance"));
	fprintf( cgiOut, "<select name=insid onchange=plotid_change(this)>");
        for(i=0;i<=INSTANCE_NUM;i++)
        {
           if(p_id == i)
    		   fprintf(cgiOut,"<option value='%d' selected>%d</option>\n",i,i);
    	   else
    	            fprintf(cgiOut,"<option value='%d'>%d</option>\n",i,i);
        }
	fprintf( cgiOut, "</select>\n");
	fprintf( cgiOut, "</td></tr>\n");	
	fprintf(cgiOut,"<script type=text/javascript>\n");
   	fprintf(cgiOut,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_authentication.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", encryz);
         fprintf(cgiOut,"</script>\n" );
	/******************************Fillet rectangular bottom********************************/	  
	return 0;	
}


