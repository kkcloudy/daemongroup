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
* wp_eag_conf.c
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
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_module_container.h"
#include "ws_portal_container.h"
#include "cgic.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_conf_engine.h"

#include "ws_eag_conf.h"
#include "eag/eag_errcode.h"
#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vrrp.h"
#include <netinet/in.h>
#include "ws_dbus_list_interface.h"

#define _DEBUG	0

#if _DEBUG
#define debug_printf(a...) fprintf(a)
#else
#define debug_printf(a...)
#endif


/***************************************************************
定义页面要用到的结构体
****************************************************************/
//#define MAX_URL_LEN         256


typedef struct{
//	STModuleContainer *pstModuleContainer;
	STPortalContainer *pstPortalContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lauth;
	char encry[BUF_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *fp;
	int formProcessError;
	char plotid[10];
} STPageInfo;

char *radiusType[] = {
	"general",
	"rj_sam",
};

/*****************************************
item  call back function  public    output
*********************************************/
static dbus_parameter parameter;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};

 
/*****************************************
item  call back function  public    get value
*********************************************/
//可以用于普通的input passwd select 等类型的value的获取
static int eag_ins_running_state(DBusConnection * conn, int hansitype, int insid)
{
	int ret = 0;
	struct eag_base_conf baseconf;		
	memset(&baseconf, 0, sizeof(baseconf));
	
	ret = eag_get_base_conf(conn, hansitype, insid, &baseconf);
	if ((EAG_RETURN_OK == ret) && (1 == baseconf.status))
		ret = 1;
	else
		ret = 0;
	return ret;
}


#define SUBMIT_NAME		"submit_save_conf"
#define CONF_FILE_PATH	"/opt/services/conf/eag_conf.conf"




/***************************************************************
申明回调函数
****************************************************************/
static int s_eagconf_prefix_of_page( STPageInfo *pstPageInfo );
static int s_eagconf_content_of_page( STPageInfo *pstPageInfo );
static int doUserCommand( STPageInfo *pstPageInfo );
static int doRedir( STPageInfo *pstPageInfo ,char *plot_id );

/***************************************************************
*USEAGE:	主函数
*Param:		
*Return:	
*			
*Auther:shao jun wu
*Date:2008-12-30 14:12:46
*Modify:(include modifyer,for what resease,date)
****************************************************************/
int cgiMain()
{
	int ret1 = 0;
	STPageInfo stPageInfo;

	DcliWInit();
	ccgi_dbus_init();   
	
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );
	ret1 = init_portal_container(&(stPageInfo.pstPortalContainer));
	stPageInfo.lpublic = stPageInfo.pstPortalContainer->lpublic;
	
	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	stPageInfo.lauth=stPageInfo.pstPortalContainer->llocal;//get_chain_head("../htdocs/text/authentication.txt");

	//stPageInfo.username_encry=dcryption(stPageInfo.encry);
    if( WS_ERR_PORTAL_ILLEGAL_USER == ret1 )
    {
	    ShowErrorPage(search(stPageInfo.lpublic,"ill_user")); 	  /*用户非法*/
		release_portal_container(&(stPageInfo.pstPortalContainer));
		return 0;
	}
	//stPageInfo.iUserGroup = checkuser_group( stPageInfo.username_encry );
	stPageInfo.iUserGroup = stPageInfo.pstPortalContainer->iUserGroup;

	//stPageInfo.pstModuleContainer = MC_create_module_container();
	if( NULL == stPageInfo.pstPortalContainer )
	{
		release_portal_container(&(stPageInfo.pstPortalContainer));
		return 0;
	}
	
	stPageInfo.fp = cgiOut;

	memset(plotid,0,sizeof(plotid));
	cgiFormStringNoNewlines("plotid", plotid, sizeof(plotid)); 
	list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB);
	if (NULL == paraHead1) {
		return 0;
	}
	if(strcmp(plotid, "") == 0)
	{
		parameter.instance_id = paraHead1->parameter.instance_id;
		parameter.local_id = paraHead1->parameter.local_id;
		parameter.slot_id = paraHead1->parameter.slot_id;
		snprintf(plotid,sizeof(plotid)-1,"%d-%d-%d",parameter.slot_id, parameter.local_id, parameter.instance_id);
	}
	else
	{
		get_slotID_localID_instanceID(plotid, &parameter);
	}
	ccgi_ReInitDbusConnection(&ccgi_connection, parameter.slot_id, DISTRIBUTFAG);
//初始化完毕

	
//处理表单
//	stPageInfo.formProcessError = getUserInput( &(stPageInfo.stUserInput) );


	MC_setActiveLabel( stPageInfo.pstPortalContainer->pstModuleContainer, WP_EAG_CONF);

	MC_setPrefixOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_eagconf_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstPortalContainer->pstModuleContainer, (MC_CALLBACK)s_eagconf_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstPortalContainer->pstModuleContainer, cgiOut );

	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ONSIBMIT, "return true;" );
	//可以设置为一个javascript函数,这个js函数的实现放在prefix回调函数中就可以了。
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_METHOD, "post" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, FORM_ACTION, "wp_eag_conf.cgi" );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, BTN_OK_SUBMIT_NAME, SUBMIT_NAME );

	
	MC_setModuleContainerDomainValue( stPageInfo.pstPortalContainer->pstModuleContainer, LABLE_TOP_HIGHT, "25" );

	
	MC_writeHtml( stPageInfo.pstPortalContainer->pstModuleContainer );
	
	free_instance_parameter_list(&paraHead1);
	
	release_portal_container(&(stPageInfo.pstPortalContainer));
	
	
	return 0;
}


static int s_eagconf_prefix_of_page( STPageInfo *pstPageInfo )
{
	int i=0;

	if( NULL == pstPageInfo )
	{
		return -1;
	}
		
 	fprintf( cgiOut, "<style type=text/css>.a3{width:30px;border:0px; text-align:center}</style>" );
 	fprintf( cgiOut, "<script language=javascript src=/ip.js></script>\n" );
 	fprintf( cgiOut, "<script language=javascript src=/fw.js></script>\n" );
    if( cgiFormSubmitClicked(SUBMIT_NAME) == cgiFormSuccess  )
    {
   			doUserCommand( pstPageInfo );   			
			doRedir( pstPageInfo , plotid);
    }

	return 0;		
}
 
static int s_eagconf_content_of_page( STPageInfo *pstPageInfo )
{
	
	if( NULL == pstPageInfo )
	{
		return -1;
	}
	FILE *fp = pstPageInfo->fp;
	
	int i = 0;
	int ret = 0;

	instance_parameter *pq = NULL;
	char temp[10] = { 0 };
	
	
	struct eag_base_conf baseconf;
	memset(&baseconf, 0, sizeof(baseconf));
	ret = eag_get_base_conf(ccgi_connection, parameter.local_id, parameter.instance_id, &baseconf);

	
	fprintf( fp, "<table width=500>\n" );
	////////////////////////////////////////////
	fprintf(fp,"<tr>");
	fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,HS_PLOT_ID));
	fprintf(fp,"<td><select name=plotid onchange=plotid_change(this)>");
	for (pq=paraHead1;(NULL != pq);pq=pq->next)
	{
		memset(temp,0,sizeof(temp));
		snprintf(temp,sizeof(temp)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);
		
		if (strcmp(plotid, temp) == 0)
			fprintf(cgiOut,"<option value='%s' selected>%s</option>\n",temp,temp);
		else	       
			fprintf(cgiOut,"<option value='%s'>%s</option>\n",temp,temp);
	}

	fprintf(fp,"</select></td>");
	fprintf(fp,"</tr>");
	fprintf(fp,"<script type=text/javascript>\n");
   	fprintf(fp,"function plotid_change( obj )\n"\
   	"{\n"\
   	"var plotid = obj.options[obj.selectedIndex].value;\n"\
   	"var url = 'wp_eag_conf.cgi?UN=%s&plotid='+plotid;\n"\
   	"window.location.href = url;\n"\
   	"}\n", pstPageInfo->encry);
    fprintf(fp,"</script>\n" );
	if (EAG_RETURN_OK == ret)
	{

	//status
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,HS_STATUS));
		fprintf(fp,"<td><input type=checkbox name=estatus value=1 %s></td>",(1 == baseconf.status) ? "checked" : "");
		fprintf(fp,"</tr>");

	//pdc distribute
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth, "pdc_distribute"));
		fprintf(fp,"<td><input type=checkbox name=pdc_distribute value=1 %s></td>",(1 == baseconf.pdc_distributed) ? "checked" : "");
		fprintf(fp,"</tr>");

	//rdc distribute
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth, "rdc_distribute"));
		fprintf(fp,"<td><input type=checkbox name=rdc_distribute value=1 %s></td>",(1 == baseconf.rdc_distributed) ? "checked" : "");
		fprintf(fp,"</tr>");

	//pdc hansi
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"pdc_hansi"));
		fprintf(fp,"<td><input type=text name=pdc_hansi value=%d-%d maxLength=5><font color=red>(slotid-insid)</font></td>",baseconf.pdc_slotid, baseconf.pdc_insid);
		fprintf(fp,"</tr>");
		
	//rdc hansi
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"rdc_hansi"));
		fprintf(fp,"<td><input type=text name=rdc_hansi value=%d-%d maxLength=5><font color=red>(slotid-insid)</font></td>",baseconf.rdc_slotid, baseconf.rdc_insid);
		fprintf(fp,"</tr>");
	//nas ip 
		char instr[32] = {0};
		ccgi_ip2str( baseconf.nasip, instr, sizeof(instr) - 1);
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>","NAS IP");
		fprintf(fp,"<td><input type=text name=nasip value=%s></td>",instr);
		fprintf(fp,"</tr>");

	//portal port
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,HS_PPI_PORT));
		fprintf(fp,"<td><input type=text name=pport value=%u maxLength=5><font color=red>(1-65535)</font></td>",baseconf.portal_port);
		fprintf(fp,"</tr>");

	//portal requirement timeout
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"portal_timeout"));
		fprintf(fp,"<td><input type=text name=ptimeout value=%u maxLength=2><font color=red>(1-10)</font></td>",baseconf.portal_retry_interval);
		fprintf(fp,"</tr>");

	//portal retry times
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"portal_retry"));
		fprintf(fp,"<td><input type=text name=pretry value=%d maxLength=2><font color=red>(0-10)</font></td>",baseconf.portal_retry_times);
		fprintf(fp,"</tr>");

	//auto-session
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,HS_AUTO_SESSION));
		fprintf(fp,"<td><input type=checkbox name=autosession value=1 %s></td>",(1 == baseconf.auto_session) ? "checked" : "");
		fprintf(fp,"</tr>");

	//account interval
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,HS_RADACCTINTERVAL));
		fprintf(fp,"<td><input type=text name=accint value=%d maxLength=4><font color=red>(60-3600)</font></td>",baseconf.radius_acct_interval);
		fprintf(fp,"</tr>");

	//radius requirement timeout
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"radius_timeout"));
		fprintf(fp,"<td><input type=text name=rtimeout value=%d maxLength=2><font color=red>(1-10)</font></td>",baseconf.radius_retry_interval);
		fprintf(fp,"</tr>");

	//radius master retry times
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"radius_master_retry"));
		fprintf(fp,"<td><input type=text name=rmretry value=%d maxLength=2><font color=red>(0-10)</font></td>",baseconf.radius_retry_times);
		fprintf(fp,"</tr>");

	//radius backup retry times
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"radius_back_retry"));
		fprintf(fp,"<td><input type=text name=rbretry value=%d maxLength=2><font color=red>(0-10)</font></td>",baseconf.vice_radius_retry_times);
		fprintf(fp,"</tr>");

	//max-http-request per 5s
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,HS_MAX_HTTPRSP));
		fprintf(fp,"<td><input type=text name=maxhttp value=%u maxLength=3><font color=red>(10-100)</font></td>",baseconf.max_redir_times);
		fprintf(fp,"</tr>");

	//force-dhcplease
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"only_dhcp"));
		fprintf(fp,"<td><input type=checkbox name=onlydhcp value=1 %s></td>",(1 == baseconf.force_dhcplease) ? "checked" : "");
		fprintf(fp,"</tr>");

	//idle timeout
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,HS_DEFIDLETIMEOUT));
		fprintf(fp,"<td><input type=text name=itimeout value=%d maxLength=5><font color=red>(60-86400)</font></td>",baseconf.idle_timeout);
		fprintf(fp,"</tr>");

	//idleflow
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,HS_IDLEFLOW));
		fprintf(fp,"<td><input type=text name=idleflow value=%llu maxLength=8><font color=red>(0-10485760)</font></td>", baseconf.idle_flow);
		fprintf(fp,"</tr>");

	//force-wireless
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,HS_FORCE_WIRELESS));
		fprintf(fp,"<td><input type=checkbox name=onlywire value=1 %s></td>",(1 == baseconf.force_wireless)?"checked":"");
		fprintf(fp,"</tr>");

	//flux-from-wireless or other
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"eag_flux_mode"));
		fprintf(fp,"<td><select name=fluxwire>");
		fprintf(fp,"<option value=%s %s>%s</option>","iptables_L2",(FLUX_FROM_IPTABLES_L2 == baseconf.flux_from)?"selected":"","iptables_L2");
		fprintf(fp,"<option value=%s %s>%s</option>","wireless",(FLUX_FROM_WIRELESS == baseconf.flux_from)?"selected":"","wireless");
		fprintf(fp,"<option value=%s %s>%s</option>","fastfwd",(FLUX_FROM_FASTFWD == baseconf.flux_from)?"selected":"","fastfwd");
		fprintf(fp,"<option value=%s %s>%s</option>","fastfwd_iptables",(FLUX_FROM_FASTFWD_IPTABLES == baseconf.flux_from)?"selected":"","fastfwd_iptables");
		fprintf(fp,"<option value=%s %s>%s</option>","iptables",(FLUX_FROM_IPTABLES == baseconf.flux_from)?"selected":"","iptables");
		fprintf(fp,"</select></td>");
		fprintf(fp,"</tr>");

	//flux-interval
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"eag_flux_int"));
		fprintf(fp,"<td><input type=text name=fluxint value=%d maxLength=4><font color=red>(10-3600)</font></td>",baseconf.flux_interval);
		fprintf(fp,"</tr>");

	//ipset-auth
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"eag_ipset"));
		fprintf(fp,"<td><input type=checkbox name=ipset value=1 %s></td>",(1 == baseconf.ipset_auth)?"checked":"");
		fprintf(fp,"</tr>");

	//check-nasportid
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"eag_checknas"));
		fprintf(fp,"<td><input type=checkbox name=checknas value=1 %s></td>",(1 == baseconf.check_nasportid)?"checked":"");
		fprintf(fp,"</tr>");

    #if 0 
	//mac-auth server
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"eag_mac_auth"));
		fprintf(fp,"<td><input type=checkbox name=macauth value=1 %s></td>",(1 == baseconf.macauth_switch)?"checked":"");
		fprintf(fp,"</tr>");

	//mac-auth flux-interval
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"eag_mac_flux"));
		fprintf(fp,"<td><input type=text name=macfluxint value=%d maxLength=4></td>",baseconf.macauth_flux_interval);
		fprintf(fp,"</tr>");
		
	//mac-auth flux-threshold
		fprintf(fp,"<tr>");
		fprintf(fp,"<td width=150>%s</td>",search(pstPageInfo->lauth,"eag_mac_fluxth"));
		fprintf(fp,"<td><input type=text name=macfluxth value=%d maxLength=9></td>",baseconf.macauth_flux_threshold);
		fprintf(fp,"</tr>");
		#endif

		
	}
	////////////////////////////////////////////
	fprintf(pstPageInfo->fp,"<tr><td colspan=2><input type=hidden name=plotid value=\"%s\"></td></tr>\n",plotid);
	fprintf( pstPageInfo->fp, "</table>\n" );
	fprintf(pstPageInfo->fp,"<input type=hidden name=UN value=\"%s\">",pstPageInfo->encry);
	return 0;	
}


static int doUserCommand( STPageInfo *pstPageInfo )
{
	char estatus[10] = {0};
	char pdc_distribute[10] = {0};
	char rdc_distribute[10] = {0};
	char pdc_hansi[10] = {0};
	char rdc_hansi[10] = {0};
	char nasip[32] = {0};
	char pport[10] = {0};
	char ptimeout[10] = {0};
	char pretry[10] = {0};
	char autosession[10] = {0};
	char accint[10] = {0};
	char rtimeout[10] = {0};	
	char rmretry[10] = {0};
	char rbretry[10] = {0};
	char maxhttp[10] = {0};
	char onlydhcp[10] = {0};
	char itimeout[10] = {0};
	char idleflow[10] = {0};
	char onlywire[10] = {0};
	char fluxwire[30] = {0};
	char fluxint[10] = {0};
	char ipset[10] = {0};
	char checknas[10] = {0};
	char macauth[10] = {0};
	char macfluxint[10] = {0};
	char macfluxth[10] = {0};
	int ret = 0;
	unsigned long ipaddr;
	struct in_addr inaddr;
	int hs_flag = 0;
	unsigned long port = 0;
	int isrun = 0;
	int pdc_slotid = 0;
	int pdc_insid = 0;
	int rdc_slotid = 0;
	int rdc_insid = 0;
	
	cgiFormStringNoNewlines("estatus", estatus, sizeof(estatus));
	cgiFormStringNoNewlines("pdc_distribute", pdc_distribute, sizeof(pdc_distribute));
	cgiFormStringNoNewlines("rdc_distribute", rdc_distribute, sizeof(rdc_distribute));
	cgiFormStringNoNewlines("pdc_hansi", pdc_hansi, sizeof(pdc_hansi));
	cgiFormStringNoNewlines("rdc_hansi", rdc_hansi, sizeof(rdc_hansi));
	cgiFormStringNoNewlines("nasip", nasip, sizeof(nasip));
	cgiFormStringNoNewlines("pport", pport, sizeof(pport));
	cgiFormStringNoNewlines("ptimeout", ptimeout, sizeof(ptimeout));
	cgiFormStringNoNewlines("pretry", pretry, sizeof(pretry));
	cgiFormStringNoNewlines("autosession", autosession, sizeof(autosession));
	cgiFormStringNoNewlines("accint", accint, sizeof(accint));
	cgiFormStringNoNewlines("rtimeout", rtimeout, sizeof(rtimeout));
	cgiFormStringNoNewlines("rmretry", rmretry, sizeof(rmretry));
	cgiFormStringNoNewlines("rbretry", rbretry, sizeof(rbretry));
	cgiFormStringNoNewlines("maxhttp", maxhttp, sizeof(maxhttp));
	cgiFormStringNoNewlines("onlydhcp", onlydhcp, sizeof(onlydhcp));
	cgiFormStringNoNewlines("itimeout", itimeout, sizeof(itimeout));
	cgiFormStringNoNewlines("idleflow", idleflow, sizeof(idleflow));
	cgiFormStringNoNewlines("onlywire", onlywire, sizeof(onlywire));
	cgiFormStringNoNewlines("fluxwire", fluxwire, sizeof(fluxwire));
	cgiFormStringNoNewlines("fluxint", fluxint, sizeof(fluxint));
	cgiFormStringNoNewlines("ipset", ipset, sizeof(ipset));
	cgiFormStringNoNewlines("checknas", checknas, sizeof(checknas));
	cgiFormStringNoNewlines("macauth", macauth, sizeof(macauth));
	cgiFormStringNoNewlines("macfluxint", macfluxint, sizeof(macfluxint));
	cgiFormStringNoNewlines("macfluxth", macfluxth, sizeof(macfluxth));

	
	int status = 0;
	status = atoi(estatus);

	isrun  = eag_ins_running_state(ccgi_connection, parameter.local_id, parameter.instance_id);
	
	if (1 == isrun) 
	{
		if(0 == status)
		{
			ret = eag_set_services_status(ccgi_connection, parameter.local_id, parameter.instance_id, status );
		}
		else
		{
			ShowAlert("eag is running, please stop it first");
			return 0;
		}
	}
	else
	{
		
		ret = sscanf(pdc_hansi, "%d-%d", &pdc_slotid, &pdc_insid);
		if (pdc_slotid > 0 && pdc_slotid <= 10 && pdc_insid > 0 && pdc_insid <= 16) {
			ret = eag_set_pdc_ins(ccgi_connection, parameter.local_id, parameter.instance_id, 
						pdc_slotid, pdc_insid);
		}
		ret = sscanf(rdc_hansi, "%d-%d", &rdc_slotid, &rdc_insid);
		if (rdc_slotid > 0 && rdc_slotid <= 10 && rdc_insid > 0 && rdc_insid <= 16) {
			ret = eag_set_rdc_ins(ccgi_connection, parameter.local_id, parameter.instance_id, 
						rdc_slotid, rdc_insid);
		}
		inet_aton(nasip, &inaddr);
		ipaddr = ntohl(inaddr.s_addr);

		ret = eag_set_nasip(ccgi_connection, parameter.local_id, parameter.instance_id, ipaddr);
		
		port = atol(pport);
		if (port>=1 && port<=65535)
		{
			ret = eag_set_portal_port(ccgi_connection, parameter.local_id, parameter.instance_id, port);
		}
		int resend_times = -1;
		unsigned long resend_interval = -1;
		resend_interval = atoi(ptimeout);
		resend_times = atoi(pretry);

		if((resend_interval>=1 && resend_interval<=10)&&(resend_times>=0 && resend_times<=10))
		{
			ret = eag_set_portal_retry_params(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									resend_interval,
									resend_times);
		}

		int auto_session = 0;
		auto_session = atoi(autosession);
		ret = eag_set_auto_session(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									auto_session);

		int interval = 0;
		interval = atoi(accint);
		if(interval>=60 && interval<=3600)
		{
			ret = eag_set_acct_interval(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,				
									interval);
		}

		int timeout = -1;
		int master_retry_times = -1;
		int backup_retry_times = -1;
		timeout = atoi(rtimeout);
		master_retry_times = atoi(rmretry);
		backup_retry_times = atoi(rbretry);
		if((timeout>=1 && timeout<=10)&&(master_retry_times>=0 && master_retry_times<=10)&&(backup_retry_times>=0 && backup_retry_times<=10))
		{
			ret = eag_set_radius_retry_params(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,					
									timeout,
									master_retry_times,
									backup_retry_times);
		}

		unsigned long request_times = 0;
		request_times = strtoul(maxhttp, NULL, 10);
		if(request_times>=10 && request_times<=100)
		{
			ret = eag_set_max_redir_times(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									request_times);	
		}

		int force_dhcplease = 0;
		force_dhcplease = atoi(onlydhcp);
		ret = eag_set_force_dhcplease(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									force_dhcplease);

		
		unsigned long long idle_flow = 0;
		unsigned long idle_timeout =0 ;
		
		idle_timeout = atoi(itimeout);
		//idle_flow = strtoul(idleflow, NULL, 10);
		idle_flow = atol(idleflow);

        if((idle_timeout>=60 && idle_timeout<=86400)&&(idle_flow>=0 && idle_flow<=10485760))
    	{
			ret = eag_set_idle_params(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id, 
									idle_timeout,
									idle_flow);
    	}

		int force_wireless = 0;
		force_wireless = atoi(onlywire);
		
		ret = eag_set_force_wireless(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									force_wireless);

		int flux_from = 0;
		if (strncmp(fluxwire, "iptables", strlen(fluxwire)) == 0) 
		{
			flux_from = FLUX_FROM_IPTABLES;
		}
		else if (strncmp(fluxwire, "iptables_L2", strlen(fluxwire)) == 0) 
		{
			flux_from = FLUX_FROM_IPTABLES_L2;
		}
		else if (strncmp(fluxwire, "wireless", strlen(fluxwire)) == 0) 
		{
			flux_from = FLUX_FROM_WIRELESS;
		}
		else if (strncmp(fluxwire, "fastfwd", strlen(fluxwire)) == 0) 
		{
			flux_from = FLUX_FROM_FASTFWD;
		}
		else if (strncmp(fluxwire, "fastfwd_iptables", strlen(fluxwire)) == 0) 
		{
			flux_from = FLUX_FROM_FASTFWD_IPTABLES;
		}
		ret = eag_set_flux_from(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									flux_from);

		int flux_interval = 0;
		flux_interval = atoi(fluxint);
		if(flux_interval>=10 && flux_interval<=3600)
		{
			ret = eag_set_flux_interval(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									flux_interval);
		}

		int ipset_auth = 0;
		ipset_auth = atoi(ipset);

		ret = eag_set_ipset_auth(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									ipset_auth);	

		int check_nasportid = 0;		
		check_nasportid=atoi(checknas);
		ret = eag_set_check_nasportid(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									check_nasportid);	

		#if 0 
		int macauth_switch = 0;
		macauth_switch = atoi(macauth);
		ret = eag_set_macauth_switch(ccgi_dbus_connection, 
											hs_flag, p_id, 
											macauth_switch);

		int macflux_interval = 0;
		macflux_interval = atoi(macfluxint);
		ret = eag_set_macauth_flux_interval(ccgi_dbus_connection, 
												hs_flag, p_id, 
												flux_interval);	
		int flux_threshold = 0;
		flux_threshold = atoi(macfluxth);
		ret = eag_set_macauth_flux_threshold(ccgi_dbus_connection, 
											hs_flag, p_id, 
											flux_threshold);	
		#endif
		if(1 == status)
		{
			ret = eag_set_services_status(ccgi_connection, 
									parameter.local_id,
									parameter.instance_id,
									status);
		}
	}

	return ret;
}


static int doRedir( STPageInfo *pstPageInfo ,char *plot_id )
{

	fprintf(cgiOut, "<script type=text/javascript>\nwindow.location.href='wp_eag_conf.cgi?UN=%s&plotid=%s';\n</script>", 
					pstPageInfo->encry,plot_id );
	exit(0);
		
}

