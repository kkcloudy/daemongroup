#ifdef __cplusplus
extern "C"
{
#endif
/*********************************************************
*	head files														*
**********************************************************/
#include <getopt.h>
#include <sys/errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <time.h>
#include <iptables.h>
#include <linux/netfilter_ipv4/ip_nat.h>
//#include "syserr.h"
//#include "eag_iptables.h"
//#include "eag_log.h"
#include <syslog.h>
#include <pthread.h>
#include <arpa/inet.h>
//#include "eag_mem.h"
//#include "pubfunc.h"
//the function  asd_inet_int2str  declared in pubfunc.h
//#include "appconn.h"
#include "asd_iptables.h"

/*********************************************************
*	global variable define											*
**********************************************************/
//static pthread_mutex_t asd_iptables_glock;

/*********************************************************
*	extern variable												*
**********************************************************/

/*********************************************************
*	functions define												*
**********************************************************/

/*******************************************************************
 *	connect_up
 * 
 *	DESCRIPTION:
 *		Add the ip to the iptables
 *
 *	INPUT:
 *		user_ip		- user ip 
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		ASD_IPTABLES_RETURN_CODE_ERROR		- error
 *		ASD_IPTABLES_RETURN_CODE_OK 		- success
 *
 *********************************************************************/
int asd_connect_up(const unsigned int user_ip)
{
	int entry_num = 0;
	char chain_name[256];
	char chain_name_nat[256];	
	char target_name[256];
	char target_name_nat[256];
	char ip_str[32] = "";
	
	asd_inet_int2str(user_ip, ip_str, sizeof(ip_str));
	
	if (0 == user_ip>>24)
	{
		syslog(LOG_ERR,"[%d-%d]asd_connect_up error.ip range error! ip_addr == %s",slotid,vrrid,ip_str);	
		return ASD_IPTABLES_RETURN_CODE_ERROR;
	}
	
	memset(chain_name,0,sizeof(chain_name));
	memset(chain_name_nat,0,sizeof(chain_name_nat));
	memset(target_name,0,sizeof(target_name));
	memset(target_name_nat,0,sizeof(target_name_nat));
	
	snprintf(chain_name,sizeof(chain_name),"CP_ASD_PRE_AUTH_F");
	snprintf(chain_name_nat,sizeof(chain_name_nat),"CP_ASD_PRE_AUTH_N");
	snprintf(target_name,sizeof(target_name),"FW_FILTER");
	snprintf(target_name_nat,sizeof(target_name_nat),"FW_DNAT");

	/* search if the chain is exist */
	if (ASD_IPTABLES_RETURN_CODE_OK != asd_check_is_chain("filter",chain_name)  
		|| ASD_IPTABLES_RETURN_CODE_OK != asd_check_is_chain("nat",chain_name_nat))
	{
		syslog(LOG_ERR,"[%d-%d]asd connect_up error,one or more chain is not exist,chain:%s,%s",slotid,vrrid,chain_name,chain_name_nat);
		return ASD_IPTABLES_RETURN_CODE_ERROR;
	}

	/* serch if the entry is exist */
	if (ASD_IPTABLES_RETURN_CODE_NOT_FOUND != asd_get_num_of_entry("filter",chain_name,user_ip,ASD_IPTABLES_SOURCE,&entry_num))
	{
		syslog(LOG_ERR,"[%d-%d]asd connect_up error,entry is exist in the chain of table \"filter\":user_ip==%s,chain_name==%s",slotid,vrrid,ip_str,chain_name);
		return ASD_IPTABLES_RETURN_CODE_ERROR;
	}
	
	/* add the entry */
	if	(ASD_IPTABLES_RETURN_CODE_OK != asd_add_and_del_entry("filter",chain_name,user_ip,0,target_name,ASD_IPTABLES_ADD)
		|| ASD_IPTABLES_RETURN_CODE_OK !=  asd_add_and_del_entry("filter",chain_name,0,user_ip,target_name,ASD_IPTABLES_ADD)
		|| ASD_IPTABLES_RETURN_CODE_OK !=  asd_add_and_del_entry("nat",chain_name_nat,user_ip,0,target_name_nat,ASD_IPTABLES_ADD))
	{
		syslog(LOG_ERR,"[%d-%d]asd connect_up error, add entry error",slotid,vrrid);
		return ASD_IPTABLES_RETURN_CODE_ERROR;
	}

	return ASD_IPTABLES_RETURN_CODE_OK;
}

/*******************************************************************
 *	connect_down
 * 
 *	DESCRIPTION:
 *		Delete the ip from the iptables
 *
 *	INPUT:
 *		user_ip		- user ip
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		ASD_IPTABLES_RETURN_CODE_ERROR		- error
 *		ASD_IPTABLES_RETURN_CODE_OK 		- success
 *
 *********************************************************************/
int asd_connect_down(const unsigned int user_ip)
{
//	int entry_num = 0;
//	int return_ret = ASD_IPTABLES_RETURN_CODE_ERROR;
	char chain_name[256];
	char chain_name_nat[256];	
	char target_name[256];
	char target_name_nat[256];
	char ip_str[32] = "";
	
	asd_inet_int2str(user_ip, ip_str, sizeof(ip_str));
	
	/* check input */
	if (0 == user_ip>>24)
	{
		syslog(LOG_ERR,"[%d-%d]asd connect_down error. ip range error! ip_addr == %s",slotid,vrrid,ip_str);	
		return ASD_IPTABLES_RETURN_CODE_ERROR;
	}
	
	memset(chain_name,0,sizeof(chain_name));
	memset(chain_name_nat,0,sizeof(chain_name_nat));
	memset(target_name,0,sizeof(target_name));
	memset(target_name_nat,0,sizeof(target_name_nat));
	
	snprintf(chain_name,sizeof(chain_name),"CP_ASD_PRE_AUTH_F");
	snprintf(chain_name_nat,sizeof(chain_name_nat),"CP_ASD_PRE_AUTH_N");
	snprintf(target_name,sizeof(target_name),"FW_FILTER");
	snprintf(target_name_nat,sizeof(target_name_nat),"FW_DNAT");
	
	/* search if the chain is exist */
	if (ASD_IPTABLES_RETURN_CODE_OK != asd_check_is_chain("filter",chain_name) 
		|| ASD_IPTABLES_RETURN_CODE_OK != asd_check_is_chain("nat",chain_name_nat))
	{
		syslog(LOG_ERR,"[%d-%d]asd connect_down error,one or more chain is not exist,chain:%s,%s",slotid,vrrid,chain_name,chain_name_nat);
		return ASD_IPTABLES_RETURN_CODE_ERROR;
	}
	
	/* del the entry */
	if	(ASD_IPTABLES_RETURN_CODE_OK != asd_add_and_del_entry("filter",chain_name,user_ip,0,target_name,ASD_IPTABLES_DELETE)
		|| ASD_IPTABLES_RETURN_CODE_OK !=  asd_add_and_del_entry("filter",chain_name,0,user_ip,target_name,ASD_IPTABLES_DELETE)
		|| ASD_IPTABLES_RETURN_CODE_OK !=  asd_add_and_del_entry("nat",chain_name_nat,user_ip,0,target_name_nat,ASD_IPTABLES_DELETE) )
	{
		syslog(LOG_ERR,"[%d-%d]asd connect_down error,delete entry error",slotid,vrrid);
		return ASD_IPTABLES_RETURN_CODE_ERROR;
	}

	return ASD_IPTABLES_RETURN_CODE_OK;
}

/*******************************************************************
 *	asd_check_is_chain
 * 
 *	DESCRIPTION:
 *		Check the chain is exist in the table
 *
 *	INPUT:
 *		table_name - table name
 *		chain_name - chain name
 *	
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		ASD_IPTABLES_RETURN_CODE_ERROR	- error or not chain
 *		ASD_IPTABLES_RETURN_CODE_OK 	- is chain
 *
 *********************************************************************/
int  asd_check_is_chain(const char * table_name,const char * chain_name)
{
	int return_ret = ASD_IPTABLES_RETURN_CODE_ERROR;
	iptc_handle_t handle = NULL;
	if (NULL == table_name || NULL == chain_name)
	{
		syslog(LOG_ERR,"[%d-%d]function asd check_is_chain  error,input error",slotid,vrrid);
		goto return_error;
	}
	handle = iptc_init(table_name);
	if (NULL == handle)
	{
		syslog(LOG_ERR,"[%d-%d]function asd check_is_chain  error,can't init iptc handle,table name:%s",slotid,vrrid,table_name);
		goto return_error;
	}
	
	/*check chain exist*/
	if (0 == iptc_is_chain(chain_name, handle))
	{
		syslog(LOG_ERR,"[%d-%d]chain is not exist in the table,chain name:%s,table name:%s",slotid,vrrid,chain_name,table_name);
		goto return_error;
	}
/*	
return_success:
	return_ret = ASD_IPTABLES_RETURN_CODE_OK;
	goto return_line;
*/
return_error:
	return_ret = ASD_IPTABLES_RETURN_CODE_ERROR;
	goto return_line;
	
return_line:
	if (NULL != handle)
	{
		iptc_free(&handle);
		handle = NULL;
	}
	return return_ret;
}

/*******************************************************************
 *	get_num_of_entry
 * 
 *	DESCRIPTION:
 *		Serch is the entry exist
 *
 *	INPUT:
 *		table_name 	- table name
 *		chain_name 	- chain name
 *		ip_addr		- ip address
 *		type			- the input ip is source or destination
 *	
 *	OUTPUT:
 *		num_of_entry - the num of the entry
 *
 *	RETURN:
 *		ASD_IPTABLES_RETURN_CODE_ERROR		- error
 *		ASD_IPTABLES_RETURN_CODE_NOT_FOUND - not exist
 *		ASD_IPTABLES_RETURN_CODE_OK 		- success ,entry is exist
 *
 *********************************************************************/
int asd_get_num_of_entry(	const char * table_name,const char * chain_name,
							const unsigned int ip_addr,const int type,int * num_of_entry)
{	
	const struct ipt_entry *p_entry = NULL;
	const struct ipt_counters *my_counter = NULL;
	iptc_handle_t handle = NULL;
	unsigned int ui_num = 1;
	char ip_str[32] = "";
	int return_ret = ASD_IPTABLES_RETURN_CODE_OK;
	
	asd_inet_int2str( ip_addr, ip_str, sizeof(ip_str) );
	
	/* check input */
	if (ASD_IPTABLES_SOURCE != type && ASD_IPTABLES_DESTINATION != type)
	{
		syslog(LOG_ERR,"[%d-%d]input error,input:%d",slotid,vrrid,type);
		goto return_error;
	}
	if (0 == ip_addr>>24)
	{
		syslog(LOG_ERR,"[%d-%d]ip range error! ip_addr == %s",slotid,vrrid,ip_str);	
		goto return_error;
	}
	if (NULL == table_name || NULL == chain_name)
	{
		syslog(LOG_ERR,"[%d-%d]input counter_info is NULL",slotid,vrrid);
		goto return_error;
	}
	
	/* iptc handle */
	handle = iptc_init(table_name);
	if (NULL == handle)
	{
		syslog(LOG_ERR,"[%d-%d]can't init iptc handle,table name:%s",slotid,vrrid,table_name);
		goto return_error;
	}

	ui_num = 1;
	/* get rules */
	if (ASD_IPTABLES_SOURCE == type)
	{
		for	(p_entry = iptc_first_rule(chain_name, &handle);
			p_entry;
			p_entry = iptc_next_rule(p_entry, &handle))
		{
			my_counter = iptc_read_counter(chain_name,ui_num, &handle);
			//log_dbg("chain_name=%s,ip_addr=%#X,p_entry->ip.src.s_addr=%#X",chain_name,ip_addr,p_entry->ip.src.s_addr);
			if (ip_addr == p_entry->ip.src.s_addr)
			{				
				*num_of_entry = ui_num;
				goto return_success;
			}
			ui_num++;
		}
	}
	else if (ASD_IPTABLES_DESTINATION == type)
	{
		for	(p_entry = iptc_first_rule(chain_name, &handle);
			p_entry;
			p_entry = iptc_next_rule(p_entry, &handle))
		{
			my_counter = iptc_read_counter(chain_name,ui_num, &handle);
			//log_dbg("ip_addr=%#X,p_entry->ip.dst.s_addr=%#X",ip_addr,p_entry->ip.dst.s_addr);			
			if (ip_addr == p_entry->ip.dst.s_addr)
			{
				*num_of_entry = ui_num;
				goto return_success;
			}
			ui_num++;
		}
	}
	else
	{
		syslog(LOG_ERR,"[%d-%d]type error",slotid,vrrid);
		goto return_error;
	}
/*	
return_not_find:
	return_ret = ASD_IPTABLES_RETURN_CODE_NOT_FOUND;
	goto return_line;
*/
return_success:
	return_ret = ASD_IPTABLES_RETURN_CODE_OK;
	goto return_line;
	
return_error:
	return_ret = ASD_IPTABLES_RETURN_CODE_ERROR;
	goto return_line;
	
return_line:
	if (NULL != handle)
	{
		iptc_free(&handle);
		handle = NULL;
	}
	return return_ret;
}

/*******************************************************************
 *	add_and_del_entry
 * 
 *	DESCRIPTION:
 *		Add or delete the enty
 *
 *	INPUT:
 *		table_name 	- table name
 *		chain_name 	- chain name
 *		dest_ip		- destination ip address
 *		source_ip		- source ip address
 *		target_name	- target name
 *		type			- the input ip is source or destination
 *	
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		ASD_IPTABLES_RETURN_CODE_ERROR		- error
 *		ASD_IPTABLES_RETURN_CODE_OK 		- success
 *
 *********************************************************************/
int  asd_add_and_del_entry	(const char *table_name,const char *chain_name,
							const int source_ip,const int dest_ip,
							const char *target_name,const int type)
{
	struct ipt_entry *p_entry = NULL;
	struct ipt_entry_target *p_target  = NULL;
//	struct ipt_entry_match *p_match = NULL;
//	struct ipt_tcp *ptcp = NULL;
	iptc_handle_t handle = NULL;
	size_t entry_size = 0;
	size_t target_size = 0;
	size_t match_size = 0;
	size_t all_size = 0;
//	int i = 0;
	int return_ret = ASD_IPTABLES_RETURN_CODE_OK;
	char dest_ip_str[32] = "";
	char source_ip_str[32] = "";
	
	//syslog(LOG_DEBUG,"add_and_del_entry asd  lock");
	//pthread_mutex_lock( &asd_iptables_glock );
	//log_dbg("add_and_del_entry lock success");
	
	asd_inet_int2str( dest_ip, dest_ip_str, sizeof(dest_ip_str) );
	asd_inet_int2str( source_ip, source_ip_str, sizeof(source_ip_str) );
	
	/* check input */
	if (ASD_IPTABLES_ADD != type && ASD_IPTABLES_DELETE != type)
	{
		syslog(LOG_ERR,"[%d-%d]input error,input:%d",slotid,vrrid,type);
		goto return_error;
	}
	if ((0 == dest_ip>>24) && (0 == source_ip>>24))
	{
		syslog(LOG_ERR,"[%d-%d]ip range error! dest_ip == %s,source_ip == %s",slotid,vrrid,dest_ip_str,source_ip_str);	
		goto return_error;
	}
	if (NULL == table_name || NULL == chain_name || NULL == target_name)
	{
		syslog(LOG_ERR,"[%d-%d]input counter_info is NULL",slotid,vrrid);
		goto return_error;
	}
	
	#if 0//if need NAT,use this
	int is_nat;
	if(!strcmp(table_name,"nat"))
	{
		is_nat = 1;
	}else if(!strcmp(table_name,"filter"))
	{
		is_nat = 0;
	}else
	{
		return 0;
	}
	#endif
	
	handle = iptc_init(table_name);
	if ( NULL == handle)
	{
		syslog(LOG_DEBUG,"[%d-%d]can't init iptc handle,table name:%s",slotid,vrrid,table_name);
		goto return_error;
	}

	entry_size = IPT_ALIGN(sizeof(struct ipt_entry));

	match_size = 0;
	#if 0//if match port ,use this
	match_size = IPT_ALIGN(sizeof(struct ipt_entry_match)) + IPT_ALIGN(sizeof(struct ipt_tcp));
	#endif
	#if 0//if NAT,use this
	target_size = IPT_ALIGN(sizeof(struct ipt_entry_target));
	if(is_nat)
	{
		target_size += IPT_ALIGN(sizeof(struct ip_nat_multi_range));//nat		
	}else
	{
		target_size += IPT_ALIGN(sizeof(int));
	}
	#endif
	target_size = IPT_ALIGN(sizeof(struct ipt_entry_target))+IPT_ALIGN(sizeof(int));
	all_size = target_size + match_size + entry_size;

	p_entry = malloc(all_size);
	memset(p_entry, 0, all_size);

	/* Set tha Entry part of the entry */
	/* Set source and destination IP address */
	p_entry->ip.src.s_addr = source_ip;	
	p_entry->ip.dst.s_addr = dest_ip;	
	if (0 == source_ip)
	{
		p_entry->ip.smsk.s_addr = 0x0;
	}
	else
	{
		p_entry->ip.smsk.s_addr = -1;
		//e->ip.smsk.s_addr = 0xffffffff;
	}
	if(0 == dest_ip)
	{
		p_entry->ip.dmsk.s_addr = 0x0;
	}
	else
	{
		p_entry->ip.dmsk.s_addr = -1;
		//e->ip.smsk.s_addr = 0xffffffff;
	}	
	/* Set the interface */
	#if 0
	if(strcmp(interface_name,"0"))
	{
		strcpy (p_entry->ip.iniface,interface_name);
		//for(i=strlen(interface_name);i>-1;i--)
		for(i=0;i<strlen(interface_name)+1;i++)
		{
			p_entry->ip.iniface_mask[i] = 0xff;
		}
	}
	#endif	
	/* Set the portol num(tcp 6,udp 17,icmp 1,IPv6 41,ALL 0) */
	#if 0
	if(!strcmp(portol_name,"tcp"))
	{
		p_entry->ip.proto = 6;
	}else if(!strcmp(portol_name,"udp"))
	{
		p_entry->ip.proto = 17;
	}else if(!strcmp(portol_name,"icmp"))
	{
		p_entry->ip.proto = 1;
	}else if(!strcmp(portol_name,"ipv6"))
	{
		p_entry->ip.proto = 41;
	}else
	{
		p_entry->ip.proto = 0;
	}
	#endif
	/* Set the proto (it's ALL here) */
	p_entry->ip.proto = 0;
	/* Set the size */
	p_entry->target_offset = entry_size + match_size;
	p_entry->next_offset = all_size;

	/* Set the ipt_entry_match part of the entry */
	#if 0//if match port,use it
	//Get address
	p_match = (struct ipt_entry_match*)p_entry->elems;
	p_match->u.user.match_size = match_size;
	//Set the portol name
	//strcpy(p_match->u.user.name,portol_name);
	//Set the Match Data of Match part----------------
	//Get address
	ptcp = (struct ipt_tcp*)p_match->data;
	//Set the port 	(All the port is match)
	ptcp->spts[0]=0;ptcp->spts[1]=0xffff;
	ptcp->dpts[0]=0;ptcp->dpts[1]=0xffff;
	#endif

	/* Set the ipt_entry_target part of the entry */
	/* Get address */
	p_target = (struct ipt_entry_target*)(p_entry->elems+match_size);
	p_target->u.user.target_size = target_size;
	/* Set the target */
	strcpy(p_target->u.user.name,target_name);
	//strcpy(pt->u.user.name,"SNAT");
	#if 0//if NAT
	struct ip_nat_multi_range *p_nat;
	p_nat = (struct ip_nat_multi_range *) p_target->data;
	p_nat->rangesize = 1;// 该nat不做随机的选择 ,>1为多NAT	
	p_nat->range[0].flags = IP_NAT_RANGE_PROTO_SPECIFIED |
		IP_NAT_RANGE_MAP_IPS;	
	p_nat->range[0].min.tcp.port = p_nat->range[0].max.tcp.port = 0;
	p_nat->range[0].min_ip = p_nat->range[0].max_ip = inet_addr("4.4.4.4");
	#endif
	
	/* add or del */
	if (ASD_IPTABLES_ADD == type)
	{
		//iptc_append_entry(chain_name,e,&h);//---append is insert in to the last
		if (!iptc_insert_entry(chain_name,p_entry,0,&handle))
		{
			syslog(LOG_ERR,"[%d-%d]add iptables error: %d,%s. table==%s,chain==%s,s_ip==%s,d_ip==%s,target==%s,handle=%p",
						slotid,vrrid,errno, iptc_strerror(errno), table_name, chain_name,
						source_ip_str, dest_ip_str, target_name, handle);
			goto return_error;
		}
	}
	else if (ASD_IPTABLES_DELETE == type)
	{
		if (!iptc_delete_entry(chain_name,p_entry,NULL,&handle))
		{
			syslog(LOG_ERR,"[%d-%d]del iptables error: %d,%s table==%s,chain==%s,s_ip==%s,d_ip==%s,target==%s,handle=%p",
						slotid,vrrid,errno, iptc_strerror(errno), table_name, chain_name,
						source_ip_str, dest_ip_str, target_name, handle);
			goto return_error;
		}
	}
	else
	{
		syslog(LOG_ERR,"[%d-%d]type error!",slotid,vrrid);
	}

	if (!iptc_commit(&handle))
	{
		syslog(LOG_ERR,"[%d-%d]commit iptables error: %d,%s.\n table==%s,chain==%s,s_ip==%s,d_ip==%s,target==%s,handle=%p",
						slotid,vrrid,errno, iptc_strerror(errno), table_name, chain_name,
						source_ip_str, dest_ip_str, target_name, handle);
		goto return_error;
	}
/*
return_success:
	return_ret = ASD_IPTABLES_RETURN_CODE_OK;
	goto return_line;
*/	
return_error:
	return_ret = ASD_IPTABLES_RETURN_CODE_ERROR;
	goto return_line;

return_line:
	
	if (NULL != p_entry)
	{
		free(p_entry);
		p_entry = NULL;
	}		
	if (NULL != handle)
	{
		iptc_free(&handle);
		handle = NULL;
	}
	
	//pthread_mutex_unlock( &asd_iptables_glock );
	//syslog(LOG_DEBUG,"add_and_del_entry unlock");

	return return_ret;
}


char * asd_inet_int2str( unsigned int ip_addr, char *ip_str, unsigned int buffsize )
{
	if(ip_str == NULL)
	{
		return "";
	}
	snprintf( ip_str, buffsize, "%u.%u.%u.%u", ip_addr>>24, (ip_addr&0xff0000)>>16,
				(ip_addr&0xff00)>>8, (ip_addr&0xff) );
				
	return ip_str;
}


//#define MAX_EAG_CP_ID 7
/*just for  test code,don't  submit */
#if  0
int main(int argc, char *argv[])
{       
	if(argc ==3)
	{
            printf("input param  are %d  and  ip %s  , %s\n",argc,argv[1],argv[2]);
	}
	//unsigned int  user_ip=inet_addr(argv[2]);
        struct in_addr  ipaddr_input;
        char sip[20] = "20.0.0.45";
        inet_aton(sip, &ipaddr_input);

        //int ret2=asd_connect_up( ipaddr_input.s_addr);
        printf("the number input after convert is %d\n",ipaddr_input.s_addr);
	if( strcmp(argv[1],"add") == 0 )
	{
		int ret=asd_connect_up(ipaddr_input.s_addr);
		if (ret == ASD_IPTABLES_RETURN_CODE_OK)
		{
                        printf("asd connect up result =%d\n",ret);
		}
		
        }
	else if( strcmp(argv[1],"delete") == 0 )
	{
		int ret2=asd_connect_down( ipaddr_input.s_addr);
		if(ret2 ==ASD_IPTABLES_RETURN_CODE_OK)
		{
                        printf("asd connect down result =%d\n",ret2);
		}
		
	}
	else 
	{
                printf(" bad  input param,return!\n");
	}
        return  0;
}
#endif

#ifdef __cplusplus
}
#endif

