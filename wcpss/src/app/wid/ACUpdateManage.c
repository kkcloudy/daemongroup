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
* ACUpdateManage.c
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/

#include "ACUpdateManage.h"

update_wtp_list * updatefailwtplist = NULL;

CWBool insert_uptfail_wtp_list(int id)
{
	//printf("insert_wtp_list\n");

	struct tag_wtpid *wtp_id;
	struct tag_wtpid *wtp_id_next;
	wtp_id = (struct tag_wtpid*)WID_MALLOC(sizeof(struct tag_wtpid));
	if (NULL == wtp_id)
	{
		return CW_FALSE;
	}
	
	wtp_id->wtpid = id;
	wtp_id->next = NULL;
	//printf("*** insert_wtp_list is %d*\n", wtp_id->wtpid);
	
	if(updatefailwtplist == NULL)
	{
		updatefailwtplist = (struct tag_wtpid_list*)WID_MALLOC(sizeof(struct tag_wtpid_list));
		if (NULL == updatefailwtplist)
		{
			CW_FREE_OBJECT_WID(wtp_id);
			return CW_FALSE;
		}
		updatefailwtplist->wtpidlist = wtp_id ;		
		updatefailwtplist->count = 1;
		//printf("*** wtp id:%d insert first  \n",id);
	}
	else
	{
		
		wtp_id_next = updatefailwtplist->wtpidlist;
		while(wtp_id_next->next!= NULL)
		{	
			wtp_id_next = wtp_id_next->next;//insert element int tail
		}
		
		wtp_id_next->next= wtp_id;
		updatefailwtplist->count++;
		
		//printf("*** wtp id:%d insert more  \n",id);
	}
	return CW_TRUE;

}
CWBool delete_uptfail_wtp_list(unsigned int id)
{
	//printf("delete_wtp_list\n");
	if(updatefailwtplist == NULL)
	{
		return CW_FALSE;
	}

	struct tag_wtpid *wtp_id;
	struct tag_wtpid *wtp_id_next;

	wtp_id = updatefailwtplist->wtpidlist;
	wtp_id_next = updatefailwtplist->wtpidlist;

	if(updatefailwtplist->count == 0)
	{
		return CW_FALSE;
	}
	else if(wtp_id_next->wtpid == id)
	{

		updatefailwtplist->wtpidlist = wtp_id_next->next;
		WID_FREE(wtp_id_next);
		wtp_id_next = NULL;
		
		updatefailwtplist->count--;
		
		if(updatefailwtplist->wtpidlist == NULL)
		{
			WID_FREE(updatefailwtplist);
			updatefailwtplist = NULL;
		}
		return CW_TRUE;
	}

	else
	{
		while(wtp_id_next->next != NULL)
		{	
			if(wtp_id_next->next->wtpid== id)
			{

				wtp_id = wtp_id_next->next;
				wtp_id_next->next = wtp_id_next->next->next;
				WID_FREE(wtp_id);
				wtp_id = NULL;
				updatefailwtplist->count--;
				return CW_TRUE;
			}
			wtp_id_next = wtp_id_next->next;
		}
	}

	return CW_FALSE;

}

CWBool find_in_uptfail_wtp_list(int id)
{
	//printf("find_in_wtp_list\n");
	if(updatefailwtplist == NULL)
	{
		return CW_FALSE;
	}
	
	struct tag_wtpid *wtp_id_next;
	
	wtp_id_next = updatefailwtplist->wtpidlist;
	while(wtp_id_next != NULL)
	{	
		if(wtp_id_next->wtpid == id)
		{
			return CW_TRUE;
		}
		wtp_id_next = wtp_id_next->next;
	}

	return CW_FALSE;

}

void destroy_uptfail_wtp_list()
{
	if(updatefailwtplist == NULL)
	{
		return;
	}

	struct tag_wtpid *phead = NULL;
	struct tag_wtpid *pnext = NULL;
	phead = updatefailwtplist->wtpidlist;
	
	WID_FREE(updatefailwtplist);
	updatefailwtplist = NULL;
	
	while(phead != NULL)
	{	
		
		pnext = phead->next;
	
		CW_FREE_OBJECT_WID(phead);

		phead = pnext;

	}	
}

void update_complete_check()
{
	char i = 0;
	if(updatewtplist == NULL)
	{
		update_next_wtp();
	}
	
	if(updatewtplist == NULL)
	{
	
	/*	CWConfigVersionInfo *pnode = gConfigVersionInfo;
		while(pnode != NULL)
		{
			if(strcmp(pnode->str_ap_model,gConfigVersionUpdateInfo->str_ap_model) == 0)
			{
				free(pnode->str_ap_version_name);
				pnode->str_ap_version_name = NULL;
				free(pnode->str_ap_version_path);
				pnode->str_ap_version_path = NULL;
				
				CW_CREATE_STRING_ERR(pnode->str_ap_version_name,strlen(gConfigVersionUpdateInfo->str_ap_version_name),return CW_FALSE;);
				CW_CREATE_STRING_ERR(pnode->str_ap_version_path,strlen(gConfigVersionUpdateInfo->str_ap_version_path),return CW_FALSE;);
				
				strcpy(pnode->str_ap_version_name,gConfigVersionUpdateInfo->str_ap_version_name);
				strcpy(pnode->str_ap_version_path,gConfigVersionUpdateInfo->str_ap_version_path);
			*/	
			
				/*mahz modified to match ap upgrade batchlly*/
				for(i=0;i<BATCH_UPGRADE_AP_NUM;i++){
					CWConfigVersionInfo *tmp_node = gConfigVersionUpdateInfo[i];
					while(tmp_node != NULL){
						CWConfigVersionInfo *free_node = tmp_node;
						tmp_node = tmp_node->next;
						
						CW_FREE_OBJECT_WID(free_node->str_ap_model);		
						CW_FREE_OBJECT_WID(free_node->str_ap_version_name); 	
						CW_FREE_OBJECT_WID(free_node->str_ap_version_path); 	
						CW_FREE_OBJECT_WID(free_node->str_ap_code); 	
						CW_FREE_OBJECT_WID(free_node);		
					}
					gConfigVersionUpdateInfo[i] = NULL;
				}
				
				gupdateControl = 0;
				checkwtpcount =0;
				destroy_wtp_list();
	
	/*			break;
			}
			pnode = pnode->next;
		}
	*/
	}


}



