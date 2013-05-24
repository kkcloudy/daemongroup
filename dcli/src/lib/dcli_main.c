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
* dcli_main.c
*
*
* CREATOR:
*		chenb@autelan.com
*
* DESCRIPTION:
*		CLI module main routine
*
* DATE:
*		12/21/2007	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.115 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <syslog.h>
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h> 
#include <sys/types.h>   
#include <asm/types.h> 
#include <errno.h>
#include <dirent.h>

#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"

#include "command.h"

#include "dcli_main.h"
#include "dcli_acl.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "dcli_boot.h"
#include "dcli_pdc.h"
#include "dcli_rdc.h"
#include "dcli_domain.h"
#ifdef _D_WCPSS_
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
//#include "dbus/wcpss/ACDBusPath.h"
//#include "dbus/asd/ASDDbusPath.h"
#include "dbus/hmd/HmdDbusPath.h"
#include "wid_ac.h"
#include "wid_wtp.h"
#include "dcli_wireless/dcli_wlan.h"
#include "dcli_wireless/dcli_radio.h"
#include "dcli_wireless/dcli_sta.h"
#include "dcli_wireless/dcli_wsm.h"
#include "dcli_wireless/dcli_security.h"
#include "dcli_wireless/dcli_license.h"
#include "dcli_iu.h"
#include "dcli_hnb.h"
#include "dcli_sem.h"
#include "dcli_wireless/dcli_bsd.h"
#include "dcli_strict_access.h"
#include "dcli_pppoe.h"

#endif

DBusConnection *dcli_dbus_connection = NULL;
DBusConnection *config_trap_dbus_connection = NULL;
DBusConnection *stp_dbus_connection = NULL;
int is_distributed = 0; /*add by caojia*/
int is_active_master = 0;
int active_master_slot = 0;
int is_master = 0;
#ifdef DISTRIBUT
DBusConnection *dcli_dbus_connection_local = NULL;
DBusConnection *dcli_dbus_connection_remote = NULL;
dbus_connection *dbus_connection_dcli[MAX_SLOT];

#endif
#ifdef _D_WCPSS_
int WTP_NUM = 4096;		
int gMaxWTPs = 0;

int WTP_NUM_AUTELAN = 0;	
int WTP_NUM_OEM  = 0;	
int glicensecount = 0;
int G_RADIO_NUM;
int BSS_NUM;
int distributFag = 0;/*set it to 1 for test*/
int HostSlotId = 0;
#endif

#ifdef _D_WCPSS_
LICENSE_TYPE **g_wtp_count = NULL;	/*xiaodawei add, 20101029*/
LICENSE_TYPE **g_wtp_binding_count = NULL; /*xiaodawei add, 20101029*/
#elif _D_CC_
int *gmax_wtp_count = NULL;
int *gcurrent_wtp_count = NULL;
#endif

struct global_ethport_s *start_fp[MAX_SLOT_COUNT];
struct global_ethport_s **global_ethport = NULL;

void
dbus_error_free_for_dcli(DBusError *error)
{
	if (dbus_error_is_set(error)) {
		syslog(LOG_NOTICE,"dbus connection of dcli error ,reinit it\n");
		
		syslog(LOG_NOTICE,"%s raised: %s\n",error->name,error->message);
		dcli_dbus_init_remote();
		
	}
	dbus_error_free(error);
	


	
	}



int dcli_global_ethports_init()
{
    int fd = -1, shmid;
	char file_path[64];	
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	unsigned int flength = 0;
	int i, j;
	int error;

	global_ethport = (struct global_ethport_s **)malloc(sizeof(struct global_ethport_s*)*GLOBAL_ETHPORTS_MAXNUM);
	if(NULL == global_ethport)
	{
		printf("memory alloc error for eth port init!!!\n");
		return;
	}
	memset(global_ethport, 0, sizeof(struct global_ethport_s*)*GLOBAL_ETHPORTS_MAXNUM);
	
    for(i = 0; i < slotNum; i++)
    {
    	sprintf(file_path, "/dbm/shm/ethport/shm%d", i+1);

	 	fd = open(file_path, O_RDWR, 00755);
		
    	if(fd < 0)
        {
            printf("Failed to open %s! %s\n", file_path, strerror(errno));
            return -1;
        }

        start_fp[i] = (struct global_ethport_s *)mmap(NULL, sizeof(struct global_ethport_s)*BOARD_GLOBAL_ETHPORTS_MAXNUM,
    		PROT_READ, MAP_SHARED, fd, 0);
        if(start_fp[i] == MAP_FAILED)
		{
            printf("Failed to mmap for slot%d! %s\n", i+1, strerror(errno));
			return -1;
		}
		
		for(j = 0; j < BOARD_GLOBAL_ETHPORTS_MAXNUM; j++)
		{
	        global_ethport[i*BOARD_GLOBAL_ETHPORTS_MAXNUM + j] = &start_fp[i][j];
		}

		close(fd);
    }	
}


int dcli_distributed_info_get(void)
{
	FILE *fd;

	//fd = fopen("/proc/board/is_distributed", "r");
	fd = fopen("/proc/product_info/distributed_product", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [1] error\n");
		return -1;
	}
	fscanf(fd, "%d", &is_distributed);
	distributFag = is_distributed;
	fclose(fd);

	
	fd = fopen("/dbm/local_board/is_active_master", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [2] error\n");
		return -1;
	}
	fscanf(fd, "%d", &is_active_master);
	fclose(fd);
	
	fd = fopen("/dbm/product/active_master_slot_id", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [2] error\n");
		return -1;
	}
	fscanf(fd, "%d", &active_master_slot);
	fclose(fd);
	return 0;
}
int dcli_host_board_info_get(void)
{
	FILE *fd;

	fd = fopen("/dbm/local_board/slot_id", "r");
	if (fd == NULL)
	{
		return -1;
	}
	fscanf(fd, "%d", &HostSlotId);
	fclose(fd);

	fd = fopen("/dbm/local_board/is_master", "r");
	if (fd == NULL)
	{
		return -1;
	}
	fscanf(fd, "%d", &is_master);
	fclose(fd);
	return 0;
}


#ifdef DISTRIBUT
int dcli_dbus_init_local(void) {
	DBusError dbus_error;

	dbus_error_init (&dbus_error);
	dcli_dbus_connection_local = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (dcli_dbus_connection_local == NULL) {
		printf ("dbus_bus_get(): %s", dbus_error.message);
		return FALSE;
	}

	dbus_bus_request_name(dcli_dbus_connection_local,"aw.cli.local",0,&dbus_error);
	
	if (dbus_error_is_set(&dbus_error)) {
		printf("request name failed: %s", dbus_error.message);
		return FALSE;
	}
	dcli_dbus_connection=dcli_dbus_connection_local;
	
	syslog(LOG_INFO,"dcli_dbus_connection_local OK\n");
	return TRUE;

}/*
int get_remote_listen_num()
{
	int fp;
	int tipc_connect_num;
	fp=fopen("/mnt/slot_num_file","r");
	if(fp <= 0){
		printf("open slot_num_file error\n");
		return -1;
		}
	if(fscanf(fp,"%d",&tipc_connect_num) <=0 ){
		fprintf(stderr,"read slot_num_file error\n");
		return -1;
		}
	printf("get tipc_connect_num is %d\n",tipc_connect_num);
	close(fp);
	return (tipc_connect_num==1?2:1);
}*/
#if 0
int init_tipc_config()
{
	if(!config_tipc_once)
	{
		char temp[256];
		memset(temp,0,256);
		sprintf(temp,"tipc-config -addr=1.1.%d 2> /dev/null",get_remote_slot_num()?2:1);
		if(-1 == system("tipc-config -netid=1111 2> /dev/null"))
		{
			printf("system error,check it\n");
			return FALSE;
			}
		
		if(-1 == system(temp))
		{
			printf("system error,check it\n");
			return FALSE;
			}
		
		if(-1 == system("tipc-config -be=eth:eth1-12 2> /dev/null"))
		{
			printf("system error,check it\n");
			return FALSE;
			}
		config_tipc_once=1;
	}
}
#endif
int dbus_connection_remote_init(dbus_connection** connection)
{
	if(NULL != (*connection))
	{	
		if(NULL != (*connection) -> dcli_dbus_connection)
			dbus_connection_close((*connection) -> dcli_dbus_connection);
		free(*connection);
		(*connection) = NULL;
	}

	(*connection) = (dbus_connection*)malloc(sizeof(dbus_connection));
	
	if(NULL == (*connection))
	{
		syslog(LOG_INFO,"malloc error\n");
		return -1;
	}

	(*connection) -> dcli_dbus_connection	= NULL;
	(*connection) -> slot_id				= -1;
	(*connection) -> board_type				= -1;
	(*connection) -> board_state			= -1;

}

int dbus_connection_init_all(dbus_connection** dbus_connection)
{
	int i = 0;
	if(NULL == dbus_connection)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_init_all:dbus_connection = NULL\n");
		return -1;
	}

	for(i = 0;i < MAX_SLOT;i++)
	{
		if(0 == dbus_connection_remote_init(&dbus_connection[i]))
		{
			syslog(LOG_INFO,"ERROR:dbus_connection_init_all:init connection %d error\n",i);
			return -1;
		}
	}
	return 0;
}
	

int dbus_connection_free(dbus_connection* connection)
{
	free(connection);
	connection = NULL;
	return 0;
}

int dbus_connection_free_all(dbus_connection** connection)
{
	int i = 0;
	for(i = 0;i < MAX_SLOT;i++)
	{
		dbus_connection_free(connection[i]);
	}

	return 0;
}

int dbus_connection_register(int slot_id,dbus_connection** connection)
{
	
	DBusError dbus_error;	
	dbus_error_init (&dbus_error);
	
	if(slot_id > MAX_SLOT)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_register:error slot_id\n");
		return -1;
	}
	
	if((*connection) == NULL)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_register:connection is NULL\n");
		return -1;
	}

	(*connection) -> slot_id 			= slot_id;
	(*connection) -> board_type			= -1;
	(*connection) -> board_state			= -1;
	(*connection) -> dcli_dbus_connection 	= dbus_bus_get_remote(DBUS_BUS_SYSTEM,slot_id,&dbus_error);

	if((*connection) -> dcli_dbus_connection == NULL)
	{
		syslog(LOG_INFO,"dbus_bus_get(): %s", dbus_error.message);
		return -1;
		
	}
	
	return 0;
	
	
}

int dbus_connection_register_all(dbus_connection** dbus_connection)
{
	int i = 0;	
	int fd=0;	
	int product_serial = 0;	
	int max_number = 0;

	
	fd = fopen("/dbm/product/slotcount", "r");
	if(0==fd)
	{
		max_number=15;
	}else{
		fscanf(fd, "%d", &max_number);
		fclose(fd);
	}
#if 0
	if(8 == product_serial)
	{
		max_number = 15;
	}
	else if(7 == product_serial)
	{
		max_number = 3;
	}
#endif
	
	if(NULL == dbus_connection)
	{
		syslog(LOG_INFO,"ERROR:dbus_connection_register_all:dbus_connection = NULL\n");
		return -1;
	}
	for(i = 1;i <= max_number;i++)
	{
		
		syslog(LOG_INFO,"\n===============connect slot %d ===================\n",i);
		if(-1 == dbus_connection_register(i,&dbus_connection[i]))
		{
			syslog(LOG_INFO,"ERROR:dbus-connection_register_all:connect slot %d error\n",i);
			continue;
		}
	}
	return 0;
}


	
int dcli_dbus_init_remote(void) {
	if(dbus_connection_init_all(dbus_connection_dcli) == -1)
	{
		return FALSE;
	}

	if(dbus_connection_register_all(dbus_connection_dcli) == -1)
	{
		return FALSE;
	}
	return TRUE;
}
	



#else
int dcli_dbus_init(void) {
	DBusError dbus_error;

	dbus_error_init (&dbus_error);
	dcli_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (dcli_dbus_connection == NULL) {
		printf ("dbus_bus_get(): %s", dbus_error.message);
		return FALSE;
	}

	dbus_bus_request_name(dcli_dbus_connection,"aw.cli",0,&dbus_error);
	
	if (dbus_error_is_set(&dbus_error)) {
		printf("request name failed: %s", dbus_error.message);
		return FALSE;
	}
}
#endif
int dcli_stp_dbus_init(void) {
	DBusError dbus_error;

	dbus_error_init (&dbus_error);
	stp_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (stp_dbus_connection == NULL) {
		printf ("dbus_bus_get(): %s", dbus_error.message);
		return FALSE;
	}

	dbus_bus_request_name(stp_dbus_connection,"aw.new",0,&dbus_error);
	
	if (dbus_error_is_set(&dbus_error)) {
		printf("request name failed: %s", dbus_error.message);
		return FALSE;
	}

}

void dcli_config_write(char * str, int local,int slotID, int hansiID,int opened, int closed)
{
	FILE *fp = NULL;
	DIR *dir = NULL;
	char newpath[128] = {0};
	char cmd[256] = {0};
	char defaultDir[] = "/var/run/config";
	char defaultPath[] = "/var/run/config/Instconfig";
	if(local)
		sprintf(newpath,"%s%d-1-%d",defaultPath,slotID,hansiID);
	else
		sprintf(newpath,"%s%d-0-%d",defaultPath,slotID,hansiID);
	if(opened){
		dir = opendir(defaultDir);
		if(dir == NULL){
			printf("%s\n",cmd);
			sprintf(cmd,"sudo mkdir %s",defaultDir);
			system(cmd);

			memset(cmd, 0, 128);
			sprintf(cmd,"sudo chmod 755 %s",defaultDir);
			system(cmd);
			memset(cmd, 0, 128);			
		}else{
			closedir(dir);
		}
		sprintf(cmd,"rm %s 2>/dev/null",newpath);
		system(cmd);
	}
	fp = fopen(newpath, "at+");
	if(fp == NULL){
		return ;
	}
	fwrite(str, strlen(str),1, fp);
	fclose(fp);
	return;
}

void dcli_config_writev2(char * str,int slotID, int hansiID,char * filename,int continue_w)
{
	FILE *fp = NULL;
	DIR *dir = NULL;
	DIR *slot_dir = NULL;
	char filepath[128] = {0};
	char cmd[256] = {0};
	char defaultDir[] = "/mnt/config";
	char defaultSlotPath[] = "/mnt/config/slot";
	char newSlotPath[128] = {0};	
	if(slotID != 0){
		sprintf(newSlotPath,"%s%d",defaultSlotPath,slotID);
	}else{
		sprintf(newSlotPath,"%sM",defaultSlotPath);
	}	
	if(hansiID != 0)
		sprintf(filepath,"%s/%s%d",newSlotPath,filename,hansiID);
	else
		sprintf(filepath,"%s/%s",newSlotPath,filename);
	dir = opendir(defaultDir);
	if(dir == NULL){
		sprintf(cmd,"sudo mkdir %s",defaultDir);
		system(cmd);
		memset(cmd, 0, 128);
		sprintf(cmd,"sudo chmod 755 %s",defaultDir);
		system(cmd);
		memset(cmd, 0, 128);
		sprintf(cmd,"sudo mkdir %s",newSlotPath);
		system(cmd);
		memset(cmd, 0, 128);
		sprintf(cmd,"sudo chmod 755 %s",newSlotPath);
		system(cmd);
		memset(cmd, 0, 128);
		
	}else{
		closedir(dir);
		slot_dir = opendir(newSlotPath);
		if(slot_dir == NULL){
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo mkdir %s",newSlotPath);
			system(cmd);
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo chmod 755 %s",newSlotPath);
			system(cmd);
			memset(cmd, 0, 128);
		}else
			closedir(slot_dir);
	}
	if(!continue_w){
		memset(cmd, 0, 128);
		sprintf(cmd,"rm %s 2>/dev/null",filepath);
		system(cmd);
	}
	fp = fopen(filepath, "at+");
	if(fp == NULL){
		return ;
	}
	fwrite(str, strlen(str),1, fp);
	fclose(fp);
	return;
}

void dcli_if_init()
{
	if_init();
}
#ifdef DISTRIBUT

void dcli_init(int flag)
{
	dcli_distributed_info_get(); /* add by caojia 20110506 */
	dcli_host_board_info_get();
	dcli_dbus_init_local();
	if(flag == 0)
		dcli_dbus_init_remote();
#else
void dcli_init(void)
{
	dcli_dbus_init();
#endif
	dcli_vrrp_element_init();
#if (defined _D_WCPSS_ || defined _D_CC_)	
	dcli_local_hansi_init();
	DcliWInit();
#endif

    /* npdsuit start */
    dcli_global_ethports_init();
	dcli_system_init();
	dcli_diag_init();
	dcli_eth_port_init();
	dcli_vlan_init();

	dcli_trunk_init();
	dcli_dynamic_trunk_init();
	dcli_fdb_init();
	dcli_qos_init();
	dcli_acl_init();
	
	dcli_interface_element_init();
	dcli_stp_element_init();
	dcli_drv_routesyn_init();
    /* can be use as experiment */
	dcli_tunnel_init();
	dcli_pvlan_init();
    dcli_prot_vlan_element_init();	
#if 0
	dcli_dldp_element_init();

#endif

	dcli_igmp_snp_element_init();
	dcli_mld_snp_element_init();	
	

	dcli_mirror_init();
    dcli_qinq_type_init();
    /* npdsuit end */
		
    dcli_system_debug_init();	

#if 1
#ifdef _D_WCPSS_
	dcli_ap_group_init();
#endif	
#if (defined _D_WCPSS_ || defined _D_CC_)
	/*DcliWInit();*/
	dcli_license_init();
	dcli_wlan_init();
#endif	
#endif
#if 0
#ifdef _D_CC_	
	dcli_cbat_init();
	dcli_cnu_init();
	dcli_cbat_radio_init(); 
#endif	
#endif
#if (defined _D_WCPSS_ || defined _D_CC_)
	dcli_wtp_init();
	dcli_radio_init();
	dcli_security_init();
	dcli_sta_init();
	dcli_ac_init();
	dcli_wsm_init();
	dcli_wqos_init();
	dcli_ebr_init();
	dcli_aciplist_init();
#endif	
#ifdef _D_WCPSS_
	dcli_ac_group_init();
#endif
#if 0
    dcli_global_ethports_init();
//    dcli_hnb_init();
	dcli_if_init();
	dcli_system_init();
	dcli_diag_init();
	dcli_tunnel_init();
	dcli_eth_port_init();
	dcli_vlan_init();
	dcli_spacial_config_init();
	dcli_trunk_init();
	dcli_dynamic_trunk_init();
	dcli_fdb_init();
	dcli_pvlan_init();
	dcli_qos_init();
	dcli_acl_init();
    dcli_prot_vlan_element_init();
	dcli_interface_element_init();
	dcli_stp_element_init();
	dcli_drv_routesyn_init();
#endif	
	dcli_dhcp_init();
	//dcli_dhcp6_init();
	dcli_dhcp_ipv6_init();

	dcli_user_init(); 
	dcli_boot_init();
	dcli_sys_manage_init();
	dcli_bsd_init();
	dcli_dnscache_init();
	dcli_dhcp_snp_element_init();
	dcli_captive_init();
	dcli_eag_init();
	dcli_pdc_init();
	dcli_rdc_init();	
	//add for femto iuh & iu
	dcli_hnb_init();
	dcli_iu_init();
#ifndef _VERSION_18SP7_
	dcli_pppoe_init();
#endif /* !_VERSION_18SP7_ */

/* syslog */
	dcli_syslog_init();
	dcli_timezone_init();
	dcli_ntp_init(); 
	dcli_acinfo_init();

/* snmp */
	dcli_acsample_init();
	dcli_snmp_init();
	
/* firewall */
	dcli_firewall_init();
	dcli_tc_init();
	dcli_strict_access_init();
	
#if 0
	dcli_igmp_snp_element_init();
	dcli_mld_snp_element_init();	
	dcli_dldp_element_init();
	dcli_mirror_init();
	dcli_boot_init();
	dcli_sys_manage_init();
	/*dcli_pim_init();*/
    dcli_qinq_type_init();
    dcli_system_debug_init();
/*	dcli_vrrp_element_init();*/
	dcli_hbip_element_init();
	dcli_iptunnel_init();
	
	dcli_dnscache_init();

	dcli_route_policy_init();
	


	/*add sunjc@autelan.com*/
	dcli_hnb_init();
	dcli_iu_init();
	dcli_sigtran2udp_init();
	dcli_sem_init();
	dcli_bsd_init();
#endif
	/*radius*/
	dcli_radius_init();

	dcli_webservice_init();
	dcli_sem_init();
	dcli_rpa_init();
	/*add niehy */
	dcli_fpga_init();
	dcli_pppoe_snp_init();
	dcli_se_agent_init();
	dcli_cvm_ratelimit_element_init();

}   

int dcli_show_running_config(void)
{
/*
	dcli_user_manage_write();
*/
#if 0
	/* show ACL group info*/
	dcli_acl_group_show_running_config();
	/* show ethport config*/
	dcli_eth_port_show_running_config();
	/* trunk config*/
	dcli_trunk_show_running_config();
#ifdef _D_WCPSS_
	dcli_ebr_show_running_config_start();
#endif
	/* ebr config*/
	/* vlan config*/
	dcli_vlan_show_running_config();
	/* igmp snooping config */
	dcli_igmp_snp_show_running_config();
	
	/* static FDB config*/
	dcli_fdb_show_running_config();

	/*stp config*/
	dcli_stp_show_running_cfg();

	/*pvaln config*/
	dcli_pvlan_show_running_cfg();

	/*sys configuration*/
	dcli_sys_global_show_running_cfg();
	/* mirror config*/
	dcli_mirror_show_running_cfg();
/* vlan egress filter*/
dcli_vlan_egress_filter_show_running_config();
/*static-arp
//dcli_ip_arp_show_running_config();
	// DLDP config*/
	dcli_dldp_show_running_config();
	/* DHCP Snooping config */
	dcli_dhcp_snp_show_running_config();
#endif
 	return 1;
}

int dcli_static_arp_show_running_config(struct vty* vty){
    return dcli_ip_arp_show_running_config();
}
#ifdef _D_WCPSS_
int dcli_wcpss_show_running_config_start(void){
	
	/*wtp config
//	dcli_wtp_show_running_config();
	//ebr config*/
	/*dcli_ebr_show_running_config_start();*/
	/*security config	*/
#if 0
	dcli_security_show_running_config();
	/*wl	an config*/
	dcli_wlan_show_running_config_start();
	/*wtp config*/
	dcli_wtp_show_running_config_start();
	/*mac list config*/
	dcli_wlan_list_show_running_config(); 
	dcli_wtp_list_show_running_config();
	dcli_bss_list_show_running_config(); 
#endif
	
 	return 1;
}
int dcli_wcpss_show_running_config_end(void){
	
#if 0
	/*wl	an config*/
	dcli_wlan_show_running_config_end();
	/*wtp config*/
	dcli_wtp_show_running_config_end();
	/*ebr config*/
	dcli_ebr_show_running_config_end();
	/*vrrp	*/
	dcli_vrrp_show_running_cfg();
#endif
	return 1;
}


#endif
#ifdef __cplusplus
}
#endif
/*fengwenchao add cli log 20111031*/
void cli_syslog_info(char *format,...)	
{
	char buf[2048] = {0};

	sprintf(buf,"CLI_INF ");
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_INFO,buf);
}
/*fengwenchao add end*/
