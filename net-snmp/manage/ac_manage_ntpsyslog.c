#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "board/board_define.h"
#include "ws_log_conf.h"
#include "ws_dbus_list_interface.h"

#include "ac_manage_def.h"
#include "ac_manage_public.h"

#include "ac_manage_ntpsyslog.h"



static void
_ntprule_copy(struct clientz_st *new_rule, struct clientz_st *rule) {
	memset(new_rule, 0, sizeof(struct clientz_st));
	
	memcpy(new_rule->clitipz, rule->clitipz,sizeof(new_rule->clitipz));
	memcpy(new_rule->ifper, rule->ifper,sizeof(new_rule->ifper));
	memcpy(new_rule->timeflag, rule->timeflag,sizeof(new_rule->timeflag));
	memcpy(new_rule->slotid, rule->slotid,sizeof(new_rule->slotid));
	return ;
}



static void
_syslogrule_copy(struct syslogrule_st *new_rule, struct dest_st *rule) {
	memset(new_rule, 0, sizeof(struct syslogrule_st));

	memcpy(new_rule->udpstr, rule->proz,sizeof(new_rule->udpstr));
	memcpy(new_rule->ipstr, rule->sysipz,sizeof(new_rule->ipstr));
	memcpy(new_rule->portstr, rule->sysport,sizeof(new_rule->portstr));
	memcpy(new_rule->flevel, rule->flevel,sizeof(new_rule->flevel));
	memcpy(new_rule->id, rule->indexz,sizeof(new_rule->id));
	return ;
}

int
manage_show_ntp_configure(struct clientz_st **rule_array,u_long *rule_num) {

	int i, j;
	u_long total_num = 0, rule_index = 0;
	struct clientz_st *temp_array = NULL;
	struct clientz_st clitst,*cq = NULL;
	memset(&clitst,0,sizeof(clitst));
    int clinum = 0;
	int retu = 0;
	if(NULL == rule_array || NULL == rule_num) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	*rule_array = NULL;
    retu = read_ntp_client(NTP_XML_FPATH, &clitst, &clinum);
	total_num = clinum;
	temp_array = (struct clientz_st *)calloc(total_num, sizeof(struct clientz_st));
	if(NULL == temp_array) {
		manage_log( LOG_INFO,"manage_show_ntp_rule: malloc rule temp_array fail\n");
		return AC_MANAGE_MALLOC_ERROR;
	}
	if(0 == retu)
	{
		cq = clitst.next;
		i = 0;
		while(cq != NULL)
		{
		    
			_ntprule_copy(&temp_array[i], cq);
			i++;
			cq = cq->next;
		}
		#if 0
		for(i = 0,cq = clitst.next;i<total_num && cq;i++,cq = cq->next)
		{
			_ntprule_copy(&temp_array[i], cq);
			
		}
		#endif
	}
	Free_read_ntp_client(&clitst);
	*rule_array = temp_array;
	*rule_num = total_num;
	return AC_MANAGE_SUCCESS;
}

static int
ntp_create_pfm_request(struct pfmOptParameter *pfmParameter, 
                                    char *ifName, 
                                    char *ipstr,
                                    unsigned int state) {
    syslog(LOG_DEBUG, "enter ntp_create_pfm_request\n");
    
    if(NULL == pfmParameter || NULL == ifName) {
        syslog(LOG_WARNING, "ntp_create_pfm_request: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
        if(0 == local_slotID) {
            syslog(LOG_WARNING, "ntp_create_pfm_request: get local slot id error\n");
            return AC_MANAGE_FILE_OPEN_FAIL;
        }
    }
    
    memset(pfmParameter, 0, sizeof(*pfmParameter));
    if(state) {
        pfmParameter->pfm_opt = 0;				/*state 1:add pfm rule,state 0 :del pfm rule*/
    }
    else {
        pfmParameter->pfm_opt = 1;
    }
    
    pfmParameter->pfm_opt_para = 0;
    pfmParameter->pfm_protocol = 17;			/*TCP protocal =6,UDP =17*/
    
    pfmParameter->ifName = ifName;	  
    syslog(LOG_DEBUG,"ntp_config_pfm_table_entry:ifname=%s",ifName);
    pfmParameter->src_ipaddr = ipstr;
    pfmParameter->src_port = 123;
    
    pfmParameter->dest_ipaddr = "all";
    pfmParameter->dest_port = 0;
    
    pfmParameter->slot_id = local_slotID;


/*
   pfmParameter->pfm_opt_para = 0;
   pfmParameter->pfm_protocol = 17;
   
   pfmParameter->ifName = ifName;	 
   
   pfmParameter->src_ipaddr = "all";
   pfmParameter->src_port = 0;
   
   pfmParameter->dest_ipaddr = "all";
   pfmParameter->dest_port = dest_port;
   
   pfmParameter->slot_id = local_slotID;

   */ 
    syslog(LOG_DEBUG, "exit ntps_create_pfm_request\n");

    return AC_MANAGE_SUCCESS;
}

static int
syslog_create_pfm_request(struct pfmOptParameter *pfmParameter, 
                                    char *ifName, 
                                    char *ipstr,
                                    unsigned int srcport,
                                    unsigned int state) {
    syslog(LOG_DEBUG, "enter ntp_create_pfm_request\n");
    
    if(NULL == pfmParameter || NULL == ifName) {
        syslog(LOG_WARNING, "ntp_create_pfm_request: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
        if(0 == local_slotID) {
            syslog(LOG_WARNING, "ntp_create_pfm_request: get local slot id error\n");
            return AC_MANAGE_FILE_OPEN_FAIL;
        }
    }
    
    memset(pfmParameter, 0, sizeof(*pfmParameter));
    if(state) {
        pfmParameter->pfm_opt = 0;				/*state 1:add pfm rule,state 0 :del pfm rule*/
    }
    else {
        pfmParameter->pfm_opt = 1;
    }
    
    pfmParameter->pfm_opt_para = 0;
    pfmParameter->pfm_protocol = 6;			/*TCP protocal =6,UDP =17*/
    
    pfmParameter->ifName = ifName;	  
    syslog(LOG_DEBUG,"syslog_config_pfm_table_entry:ifname=%s",ifName);
    pfmParameter->src_ipaddr = ipstr;
    pfmParameter->src_port = srcport;
    
    pfmParameter->dest_ipaddr = "all";
    pfmParameter->dest_port = 0;
    
    pfmParameter->slot_id = local_slotID;


/*
   pfmParameter->pfm_opt_para = 0;
   pfmParameter->pfm_protocol = 17;
   
   pfmParameter->ifName = ifName;	 
   
   pfmParameter->src_ipaddr = "all";
   pfmParameter->src_port = 0;
   
   pfmParameter->dest_ipaddr = "all";
   pfmParameter->dest_port = dest_port;
   
   pfmParameter->slot_id = local_slotID;

   */ 
    syslog(LOG_DEBUG, "exit ntps_create_pfm_request\n");

    return AC_MANAGE_SUCCESS;
}

 int
ntp_config_pfm_table_entry(char *ifName, char *ipstr,unsigned int state) {
    syslog(LOG_DEBUG, "enter ntp_config_pfm_table_entry\n");
    
    if(NULL == ifName) {
        syslog(LOG_WARNING, "ntp_config_pfm_table_entry: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

	unsigned int ip_addr_input=0;
    syslog(LOG_DEBUG, "ntp_config_pfm_table_entry: ifname=%s,state=%d\n",ifName,state);
    struct pfmOptParameter pfmParameter = { 0 };
    if(AC_MANAGE_SUCCESS != ntp_create_pfm_request(&pfmParameter, 
                                                      ifName,
                                                      ipstr,
                                                      state)) {
        syslog(LOG_WARNING, "ntp_config_pfm_table_entry: create apache pfm request error\n");
        return AC_MANAGE_CONFIG_FAIL;
    }

    int ret = AC_MANAGE_SUCCESS;
    ret = manage_config_pfm_table_entry(&pfmParameter);
    syslog(LOG_DEBUG, "ntp_config_pfm_table_entry: call manage_config_pfm_table_entry , return %d\n", ret);

    syslog(LOG_DEBUG, "exit ntp_config_pfm_table_entry\n");
    return ret;
}



void  init_ntp_config(void) 
{

    if(0 == active_master_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			active_master_slotID = manage_get_product_info(PRODUCT_ACTIVE_MASTER);
		}
        if(0 == active_master_slotID) {
            syslog(LOG_WARNING, "manage_init_ntp_config: get active master slot id error\n");
            return ;
        }
    }

    if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
        if(0 == local_slotID) {
            syslog(LOG_WARNING, "manage_init_ntp_config: get local slot id error\n");
            return ;
        }
    }
    if(active_master_slotID == local_slotID) 
	{
		set_master_default_server_func();
    }
    else 
	{
		set_default_inside_client_func(active_master_slotID);
    }
	//start_ntp();
	return ;
}


int
manage_show_time(struct timez_st *rule_array) {

	char buff[256] = {0};
	FILE *pp = NULL;
	//pp=popen("date -u +\"%a %b %d %T CST %Y\" -d \"8 hour\"","r");
	pp=popen("date","r");  
	if(pp!=NULL)
	{
		memset(buff,0,sizeof(buff));
		fgets( buff, sizeof(buff), pp );
		memcpy(rule_array->nowstr, buff,sizeof(rule_array->nowstr));
		pclose(pp);
	}

	pp=popen("date -u","r");  
	if(pp!=NULL)
	{
		memset(buff,0,sizeof(buff));
		fgets( buff, sizeof(buff), pp );
		memcpy(rule_array->utcstr, buff,sizeof(rule_array->utcstr));
		pclose(pp);
	}
	return AC_MANAGE_SUCCESS;
}


int 
ntp_show_running_config(struct running_config **configHead, unsigned int type) {
    if(NULL == configHead) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *configHead = NULL;
    struct running_config *configEnd = NULL;

	
    if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
        if(0 == local_slotID) {
            syslog(LOG_WARNING, "ntp_show_running_config: get loal slot id error\n");
            manage_free_running_config(configHead);
            return AC_MANAGE_FILE_OPEN_FAIL;
        }
    }
	
	NTPALL_ST ntpall; 
	memset(&ntpall,0,sizeof(ntpall));
    int cnum=0,snum=0,unum = 0;
    struct clientz_st *cq,*upcq;
	struct serverz_st *sq;

	read_upper_ntp(NTP_XML_FPATH, &(ntpall.upclist),&unum);
	read_ntp_client(NTP_XML_FPATH, &(ntpall.clist),&cnum);	
	read_ntp_server(NTP_XML_FPATH, &(ntpall.serst), &snum);

	if(active_master_slotID == local_slotID)
	{
		sq=ntpall.serst.next;
		while(sq!=NULL)
		{
			if(0 != strncmp(sq->servipz,"169.254.1.",10))
			{
				struct running_config *temp_config = manage_new_running_config();
				if(temp_config) 
				{
					snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " client add %s/%s", sq->servipz,sq->maskz);
				    manage_insert_running_config(configHead, &configEnd, temp_config);
				}
			}
			sq=sq->next;
		}
	}
	else
	{
		sq=ntpall.serst.next;
		while(sq!=NULL)
		{
			struct running_config *temp_config = manage_new_running_config();
			if(temp_config) 
			{
				snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " client add %s/%s %d", sq->servipz,sq->maskz,local_slotID);
			    manage_insert_running_config(configHead, &configEnd, temp_config);
			}

			sq=sq->next;
		}
	}

	upcq=ntpall.upclist.next;
	while(upcq!=NULL)
	{
		struct running_config *temp_config = manage_new_running_config();
		if(temp_config) 
		{
		    snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " upperserver add %s %s %s %d", upcq->clitipz,upcq->ifper,upcq->slotid,local_slotID);
		    manage_insert_running_config(configHead, &configEnd, temp_config);
		}
		upcq=upcq->next;
	}


	if(active_master_slotID == local_slotID)
	{
		cq=ntpall.clist.next;
		while(cq!=NULL)
		{
			struct running_config *temp_config = manage_new_running_config();
			if(temp_config) 
			{
			    snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " server add %s %s", cq->clitipz,cq->ifper);
			    manage_insert_running_config(configHead, &configEnd, temp_config);
			}
			cq=cq->next;
		}
	}
	
	if(cnum>0)
	{
		Free_read_ntp_client(&(ntpall.clist));
	}
	if(snum>0)
	{
		Free_read_ntp_server(&(ntpall.serst));
	}
	if(unum>0)
	{
		Free_read_upper_ntp(&(ntpall.upclist));
	}

	return AC_MANAGE_SUCCESS;
}


int
manage_show_syslog_configure(struct syslogrule_st **rule_array,u_long *rule_num) {

	int i, j;
	u_long total_num = 0, rule_index = 0;
	struct syslogrule_st *temp_array = NULL;
	struct dest_st d_head,*dq;
	memset(&d_head,0,sizeof(d_head));
    int clinum = 0;
	int retu = 0;
	if(NULL == rule_array || NULL == rule_num) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}
	if_syslog_exist();
	*rule_array = NULL;
    retu = read_dest_xml(&d_head, &clinum);
	total_num = clinum;
	temp_array = (struct syslogrule_st *)calloc(total_num, sizeof(struct syslogrule_st));
	if(NULL == temp_array) {
		manage_log( LOG_INFO,"manage_show_ntp_rule: malloc rule temp_array fail\n");
		return AC_MANAGE_MALLOC_ERROR;
	}
	if(0 == retu)
	{
		dq=d_head.next;
		i = 0;
		while(dq != NULL)
		{
		    
			_syslogrule_copy(&temp_array[i], dq);
			i++;
			dq = dq->next;
		}
	}
	Free_read_dest_xml(&d_head);
	*rule_array = temp_array;
	*rule_num = total_num;
	return AC_MANAGE_SUCCESS;
}

 int
syslog_config_pfm_table_entry(char *ifName, char *ipstr,unsigned int srcport,unsigned int state) {
    syslog(LOG_DEBUG, "enter ntp_config_pfm_table_entry\n");
    
    if(NULL == ifName) {
        syslog(LOG_WARNING, "ntp_config_pfm_table_entry: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

	unsigned int ip_addr_input=0;
    syslog(LOG_DEBUG, "ntp_config_pfm_table_entry: ifname=%s,state=%d\n",ifName,state);
    struct pfmOptParameter pfmParameter = { 0 };
    if(AC_MANAGE_SUCCESS != syslog_create_pfm_request(&pfmParameter, 
                                                      ifName,
                                                      ipstr,
                                                      srcport,
                                                      state)) {
        syslog(LOG_WARNING, "ntp_config_pfm_table_entry: create apache pfm request error\n");
        return AC_MANAGE_CONFIG_FAIL;
    }

    int ret = AC_MANAGE_SUCCESS;
    ret = manage_config_pfm_table_entry(&pfmParameter);
    syslog(LOG_DEBUG, "ntp_config_pfm_table_entry: call manage_config_pfm_table_entry , return %d\n", ret);

    syslog(LOG_DEBUG, "exit ntp_config_pfm_table_entry\n");
    return ret;
}

