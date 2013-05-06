/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* ws_init_dbus.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#ifdef __cplusplus
extern "C"
{
#endif
#include <syslog.h>
#include <dlfcn.h>
#include <pthread.h>

#include "wcpss/waw.h"
#include "dbus/wcpss/ACDBusPath.h"
#include "dbus/asd/ASDDbusPath.h"
#include "wcpss/waw.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "board/board_define.h"
#include "ws_dcli_wlans.h"
#include "dbus/npd/npd_dbus_def.h"
#include "ws_dcli_vrrp.h"
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"



DBusConnection *ccgi_dbus_connection = NULL;

void *ccgi_dl_handle = NULL;


int WTP_NUM = 4096;		
int G_RADIO_NUM;
int BSS_NUM;
int CBAT_NUM = 0;
int glicensecount = 0;

LICENSE_TYPE **g_wtp_count = NULL;	/*xiaodawei add, 20101029*/
LICENSE_TYPE **g_wtp_binding_count = NULL; /*xiaodawei add, 20101029*/



int dbus_connection_init(void);
int dl_dcli_init(void);
int snmpd_dbus_connection_init(void);


int 
ccgi_local_dbus_init(void) {
	dbus_connection_init();
	dl_dcli_init();
}

int ccgi_dbus_init(void)
{
    init_dbus_connection_list();
}

void destroy_ccgi_dbus(void)
{
    return uninit_dbus_connection_list();
}


int snmpd_dbus_init(void)
{
 	snmpd_collect_mode_init();
 	init_dbus_connection_list();

	return 0;
}

/*
 * lixiang edit at 2012-01-16
 * start
 */
int dbus_connection_init(void) {

	DBusError dbus_error;
	dbus_error_init(&dbus_error);
	
	if(NULL == ccgi_dbus_connection) {
		ccgi_dbus_connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &dbus_error);
		if (ccgi_dbus_connection == NULL) {		    
            if (dbus_error_is_set(&dbus_error)) {
			    syslog(LOG_WARNING, "dbus_connection_init: dbus_bus_get(): %s", dbus_error.message);
                dbus_error_free(&dbus_error);
			}    
		    return CCGI_FAIL;
		}
	}
	return CCGI_SUCCESS;
}

int 
snmpd_dbus_connection_init(void) {
	return dbus_connection_init();
}
/*
 * lixiang edit at 2012-01-16
 * end
 */


int dl_dcli_init(void)
{
	if(!ccgi_dl_handle) {
		ccgi_dl_handle = dlopen("/opt/lib/libdclipub.so.0",RTLD_NOW);
		if (!ccgi_dl_handle) {
			syslog(LOG_WARNING, "dl_dcli_init: Run without /opt/lib/libdcli.so.0\n");
			return -1;
		}
	}	
	return 1;
}

void wireless_ReInitDbusPath(int index, char * path, char * newpath)
{
	int len;
	sprintf(newpath,"%s%d",path,index); 
	if(path == ASD_DBUS_SECURITY_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","security");
	}
	else if(path == ASD_DBUS_SECURITY_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","security");			
	}
	else if(path == ASD_DBUS_STA_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","sta");
	}
	else if(path == ASD_DBUS_STA_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","sta");			
	}
	else if(path == WID_DBUS_WLAN_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wlan");
	}
	else if(path == WID_DBUS_WLAN_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wlan");			
	}
	else if(path == WID_DBUS_WTP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wtp");
	}
	else if(path == WID_DBUS_WTP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wtp");			
	}
	else if(path == WID_DBUS_RADIO_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","radio");
	}
	else if(path == WID_DBUS_RADIO_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","radio"); 		
	}
	else if(path == WID_DBUS_QOS_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","qos");
	}
	else if(path == WID_DBUS_QOS_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","qos");			
	}
	else if(path == WID_DBUS_EBR_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","ebr");
	}
	else if(path == WID_DBUS_EBR_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","ebr");			
	}	
	else if(path == WID_BAK_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","bak");
	}
	else if(path == WID_BAK_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","bak");			
	}
	else if(path == WID_DBUS_ACIPLIST_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","aciplist");
	}
	else if(path == WID_DBUS_ACIPLIST_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","aciplist");			
	}
	else if(path == ASD_DBUS_AC_GROUP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","acgroup");
	}
	else if(path == ASD_DBUS_AC_GROUP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","acgroup");			
	}
	else if(path == WID_DBUS_AP_GROUP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","apgroup");
	}
	else if(path == WID_DBUS_AP_GROUP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","apgroup");			
	}
}

void wireless_ReInitDbusPath_V2(int local,int index, char * path, char * newpath)
{
	int len;
	sprintf(newpath,"%s%d_%d",path,local,index); 
	if(path == ASD_DBUS_SECURITY_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","security");
	}
	else if(path == ASD_DBUS_SECURITY_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","security");			
	}
	else if(path == ASD_DBUS_STA_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","sta");
	}
	else if(path == ASD_DBUS_STA_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","sta");			
	}
	else if(path == WID_DBUS_WLAN_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wlan");
	}
	else if(path == WID_DBUS_WLAN_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wlan");			
	}
	else if(path == WID_DBUS_WTP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wtp");
	}
	else if(path == WID_DBUS_WTP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wtp");			
	}
	else if(path == WID_DBUS_RADIO_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","radio");
	}
	else if(path == WID_DBUS_RADIO_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","radio");			
	}
	else if(path == WID_DBUS_QOS_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","qos");
	}
	else if(path == WID_DBUS_QOS_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","qos");			
	}
	else if(path == WID_DBUS_EBR_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","ebr");
	}
	else if(path == WID_DBUS_EBR_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","ebr");			
	}	
	else if(path == WID_BAK_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","bak");
	}
	else if(path == WID_BAK_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","bak");			
	}
	else if(path == WID_DBUS_ACIPLIST_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","aciplist");
	}
	else if(path == WID_DBUS_ACIPLIST_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","aciplist");			
	}
	else if(path == ASD_DBUS_AC_GROUP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","acgroup");
	}
	else if(path == ASD_DBUS_AC_GROUP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","acgroup");			
	}
	else if(path == WID_DBUS_AP_GROUP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","apgroup");
	}
	else if(path == WID_DBUS_AP_GROUP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","apgroup");			
	}
}


void ccgi_ReInitDbusPath(int index, char * path, char * newpath)
{
	wireless_ReInitDbusPath(index,path,newpath);
}

void ccgi_ReInitDbusPath_v2(int local,int index, char * path, char * newpath)
{
	wireless_ReInitDbusPath_V2(local, index, path, newpath);
}
void ccgi_ReInitDbusConnection(DBusConnection **dcli_dbus_connection,int slot_id,int distributFag)
{

	#if 0 
	int distributFag_z = 0;
	distributFag_z = get_is_distributed();
	if((distributFag_z)&&(dbus_connection_dcli[slot_id])&&(NULL != dbus_connection_dcli[slot_id]->dcli_dbus_connection))
	{
		*dcli_dbus_connection = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
	}
	else
	{
		*dcli_dbus_connection = dcli_dbus_connection_local;
	}
	#endif
	get_slot_dbus_connection(slot_id, dcli_dbus_connection, SNMPD_INSTANCE_MASTER_V3);

}
int get_is_distributed()
{
	FILE *fd;
	int is_distributed = 0;
	fd = fopen("/proc/board/is_distributed", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [1] error\n");
		return 0;
	}
	fscanf(fd, "%d", &is_distributed);
	fclose(fd);
	return is_distributed;
}

int get_dbm_effective_flag()
{
	FILE *fd = NULL;
	int dbm_effective_flag = 0;
	fd = fopen("/dbm/dbm_effective_flag", "r");
	if (fd == NULL)
	{
		return INVALID_DBM_FLAG;
	}
	fscanf(fd, "%d", &dbm_effective_flag);
	fclose(fd);
	return dbm_effective_flag;
}

#ifdef __cplusplus
}
#endif

