
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <dbus/dbus.h>

#include "ACDbusDef.h"
#include "ASDDbusDef.h"

#include "npd_dbus_def.h"

#include "m_data.h"
#include "m_dbus.h"
#include "m_log.h"
#include "ac_sample_dbus.h"
#include "old_1.3_snmp_trap_def.h"
#include "old_1.2omc_ws_snmpd_trap_common.h"


static SIGLIST gsiglist[]={
	//signal number         signal name           signal send function
	//Ap
	{1,WID_DBUS_TRAP_WID_AP_CPU_THRESHOLD,1101,WTP_INNER_CPU_USAGE_TOOHIGH_TRAP,1<<24},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_UINT32, &cpu,\
								DBUS_TYPE_UINT32, &cpu_threshold,\
								DBUS_TYPE_BYTE, &flag,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{2,WID_DBUS_TRAP_WID_AP_CPU_THRESHOLD,1101,WTP_INNER_CPU_USAGE_TOOHIGH_CLEAR_TRAP,0<<24},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_UINT32, &cpu,\
								DBUS_TYPE_UINT32, &cpu_threshold,\
								DBUS_TYPE_BYTE, &flag,// =0 or 1 for toohigh Recov or toohigh \
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{3,WID_DBUS_TRAP_WID_AP_MEM_THRESHOLD,1102,WTP_INNER_MEM_USAGE_TOOHIGH_TRAP, 1<<24},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE,   &mem,\
								DBUS_TYPE_UINT32, &mem_threshold,\
								DBUS_TYPE_BYTE, &flag,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{4,WID_DBUS_TRAP_WID_AP_MEM_THRESHOLD,1102,WTP_INNER_MEM_USAGE_TOOHIGH_CLEAR_TRAP, 0<<24},//\DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE,   &mem,\
								DBUS_TYPE_UINT32, &mem_threshold,\
								DBUS_TYPE_BYTE, &flag,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{5,WID_DBUS_TRAP_WID_AP_TEMP_THRESHOLD,1102,WTP_INNER_TEMPER_TOOHIGH_TRAP, 1<<24},//DBUS_TYPE_UINT32, &wtpindex,\
							DBUS_TYPE_STRING, &wtpsn,\
							DBUS_TYPE_BYTE,   &temperature,\
							DBUS_TYPE_UINT32, &temp_threshold,\
							DBUS_TYPE_BYTE, &flag,\
							DBUS_TYPE_BYTE, &wtpmac[0],\
							DBUS_TYPE_BYTE, &wtpmac[1],\
							DBUS_TYPE_BYTE, &wtpmac[2],\
							DBUS_TYPE_BYTE, &wtpmac[3],\
							DBUS_TYPE_BYTE, &wtpmac[4],\
							DBUS_TYPE_BYTE, &wtpmac[5],

	{6,WID_DBUS_TRAP_WID_AP_TEMP_THRESHOLD,1102,WTP_INNER_TEMPER_TOOHIGH_CLEAR_TRAP, 0<<24 },//DBUS_TYPE_UINT32, &wtpindex,\
							DBUS_TYPE_STRING, &wtpsn,\
							DBUS_TYPE_BYTE,   &temperature,\
							DBUS_TYPE_UINT32, &temp_threshold,\
							DBUS_TYPE_BYTE, &flag,\
							DBUS_TYPE_BYTE, &wtpmac[0],\
							DBUS_TYPE_BYTE, &wtpmac[1],\
							DBUS_TYPE_BYTE, &wtpmac[2],\
							DBUS_TYPE_BYTE, &wtpmac[3],\
							DBUS_TYPE_BYTE, &wtpmac[4],\
							DBUS_TYPE_BYTE, &wtpmac[5],

	{7,WID_DBUS_TRAP_WID_AP_RUN_QUIT,902,WTP_INNER_WTP_OFFLINE_TRAP,0},//DBUS_TYPE_UINT32, &wtpindex,\
							DBUS_TYPE_STRING, &wtpsn,\
							DBUS_TYPE_BYTE, &state,// 0-quit /1-run\
							DBUS_TYPE_BYTE, &wtpmac[0],\
							DBUS_TYPE_BYTE, &wtpmac[1],\
							DBUS_TYPE_BYTE, &wtpmac[2],\
							DBUS_TYPE_BYTE, &wtpmac[3],\
							DBUS_TYPE_BYTE, &wtpmac[4],\
							DBUS_TYPE_BYTE, &wtpmac[5],	
	
	{8,WID_DBUS_TRAP_WID_AP_RUN_QUIT,902,WTP_INNER_WTP_ONLINE_TRAP,1<<24},//DBUS_TYPE_UINT32, &wtpindex,\
							DBUS_TYPE_STRING, &wtpsn,\
							DBUS_TYPE_BYTE, &state,// 0-quit /1-run\
							DBUS_TYPE_BYTE, &wtpmac[0],\
							DBUS_TYPE_BYTE, &wtpmac[1],\
							DBUS_TYPE_BYTE, &wtpmac[2],\
							DBUS_TYPE_BYTE, &wtpmac[3],\
							DBUS_TYPE_BYTE, &wtpmac[4],\
							DBUS_TYPE_BYTE, &wtpmac[5],
							
	{9,WID_DBUS_TRAP_WID_AP_RRM_STATE_CHANGE,905,WTP_INNER_MT_WORKMODE_CHANGE_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
							DBUS_TYPE_STRING, &wtpsn,\
							DBUS_TYPE_BYTE, &state,// 0-disable /1-enable\
							DBUS_TYPE_BYTE, &wtpmac[0],\
							DBUS_TYPE_BYTE, &wtpmac[1],\
							DBUS_TYPE_BYTE, &wtpmac[2],\
							DBUS_TYPE_BYTE, &wtpmac[3],\
							DBUS_TYPE_BYTE, &wtpmac[4],\
							DBUS_TYPE_BYTE, &wtpmac[5],

	{10,WID_DBUS_TRAP_WID_WTP_AP_FLASH_WRITE_FAIL,901,WTP_INNER_FLASH_WRITE_FAIL_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
												DBUS_TYPE_STRING, &wtpsn,\
												DBUS_TYPE_BYTE, &wtpmac[0],\
												DBUS_TYPE_BYTE, &wtpmac[1],\
												DBUS_TYPE_BYTE, &wtpmac[2],\
												DBUS_TYPE_BYTE, &wtpmac[3],\
												DBUS_TYPE_BYTE, &wtpmac[4],\
												DBUS_TYPE_BYTE, &wtpmac[5],\
												DBUS_TYPE_UINT32, &flag,//failure--0,successfull -- 1

	{11,ASD_DBUS_SIG_KEY_CONFLICT,303,WTP_INNER_SSID_KEY_CONFLICT_TRAP,55},//DBUS_TYPE_BYTE, &type,\
								DBUS_TYPE_BYTE, &var1,\
								DBUS_TYPE_BYTE, &var2,

	{12,WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE,8,WTP_INNER_COVER_HOLE_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
									DBUS_TYPE_STRING, &wtpsn,\
									DBUS_TYPE_BYTE, &wtpmac[0],\
									DBUS_TYPE_BYTE, &wtpmac[1],\
									DBUS_TYPE_BYTE, &wtpmac[2],\
									DBUS_TYPE_BYTE, &wtpmac[3],\
									DBUS_TYPE_BYTE, &wtpmac[4],\
									DBUS_TYPE_BYTE, &wtpmac[5],
									
	{13,WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE_CLEAR,8,WTP_INNER_COVER_HOLE_CLEAR_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
									DBUS_TYPE_STRING, &wtpsn,\
									DBUS_TYPE_BYTE, &wtpmac[0],\
									DBUS_TYPE_BYTE, &wtpmac[1],\
									DBUS_TYPE_BYTE, &wtpmac[2],\
									DBUS_TYPE_BYTE, &wtpmac[3],\
									DBUS_TYPE_BYTE, &wtpmac[4],\
									DBUS_TYPE_BYTE, &wtpmac[5],

	{14,WID_DBUS_TRAP_WID_WTP_TRANFER_FILE,8,WTP_APP_FILE_TRANSFER_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
										DBUS_TYPE_STRING, &wtpsn,\
										DBUS_TYPE_BYTE, &wtpmac[0],\
										DBUS_TYPE_BYTE, &wtpmac[1],\
										DBUS_TYPE_BYTE, &wtpmac[2],\
										DBUS_TYPE_BYTE, &wtpmac[3],\
										DBUS_TYPE_BYTE, &wtpmac[4],\
										DBUS_TYPE_BYTE, &wtpmac[5],
	
	{15,ASD_DBUS_SIG_STA_VERIFY_FAILED,14,WTP_APP_LINK_VARIFY_FAILED_TRAP,55},//DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],\
								DBUS_TYPE_INT32,  &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],
								
	{16,ASD_DBUS_SIG_STA_VERIFY,14,WTP_APP_LINK_VARIFY_TRAP,55},//DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],\
								DBUS_TYPE_INT32,  &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{17,WID_DBUS_TRAP_WID_WTP_AP_DOWN,8,WTP_INNER_DOWN_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
										DBUS_TYPE_STRING, &wtpsn,\
										DBUS_TYPE_BYTE, &wtpmac[0],\
										DBUS_TYPE_BYTE, &wtpmac[1],\
										DBUS_TYPE_BYTE,	&wtpmac[2],\
										DBUS_TYPE_BYTE,	&wtpmac[3],\
										DBUS_TYPE_BYTE,	&wtpmac[4],\
										DBUS_TYPE_BYTE,	&wtpmac[5],

	{18,WID_DBUS_TRAP_WID_WTP_AP_REBOOT,8,WTP_INNER_SYSTEM_START_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
										DBUS_TYPE_STRING, &wtpsn,\
										DBUS_TYPE_BYTE, &wtpmac[0],\
										DBUS_TYPE_BYTE, &wtpmac[1],\
										DBUS_TYPE_BYTE, &wtpmac[2],\
										DBUS_TYPE_BYTE, &wtpmac[3],\
										DBUS_TYPE_BYTE, &wtpmac[4],\
										DBUS_TYPE_BYTE, &wtpmac[5],

	{19,WID_DBUS_TRAP_WID_WTP_CODE_START,8,WTP_INNER_COLD_START_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
										DBUS_TYPE_STRING, &wtpsn,\
										DBUS_TYPE_BYTE, &wtpmac[0],\
										DBUS_TYPE_BYTE, &wtpmac[1],\
										DBUS_TYPE_BYTE, &wtpmac[2],\
										DBUS_TYPE_BYTE, &wtpmac[3],\
										DBUS_TYPE_BYTE, &wtpmac[4],\
										DBUS_TYPE_BYTE, &wtpmac[5],

	{20,WID_DBUS_TRAP_WID_WLAN_PRESHARED_KEY_CHANGE,0,WTP_INNER_PRESHARE_KEY_CHANGE_TRAP,55},//!!!dbus_message_iter_get_basic(&iter, &wlanid);
	
	{21,WID_DBUS_TRAP_WID_WTP_ELECTRIFY_REGISTER_CIRCLE,9,WTP_INNER_ELEC_REG_CIRCLE_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
											DBUS_TYPE_UINT32, &registercircle,\
											DBUS_TYPE_STRING, &wtpsn,\
											DBUS_TYPE_BYTE, &wtpmac[0],\
											DBUS_TYPE_BYTE, &wtpmac[1],\
											DBUS_TYPE_BYTE, &wtpmac[2],\
											DBUS_TYPE_BYTE, &wtpmac[3],\
											DBUS_TYPE_BYTE, &wtpmac[4],\
											DBUS_TYPE_BYTE, &wtpmac[5],

	{22,WID_DBUS_TRAP_WID_WTP_IP_CHANGE_ALARM,8,WTP_INNER_IP_CHANGE_ALARM,55},//DBUS_TYPE_UINT32, &wtpindex,\
										DBUS_TYPE_STRING, &wtpsn,\
										DBUS_TYPE_BYTE,&wtpmac[0],\
										DBUS_TYPE_BYTE,	&wtpmac[1],\
										DBUS_TYPE_BYTE,	&wtpmac[2],\
										DBUS_TYPE_BYTE,	&wtpmac[3],\
										DBUS_TYPE_BYTE,	&wtpmac[4],\
										DBUS_TYPE_BYTE,	&wtpmac[5],

	{23,WID_DBUS_TRAP_WID_WLAN_ENCRYPTION_TYPE_CHANGE,0,WTP_INNER_AUTH_MODE_CHANGE_TRAP,55},//!!! dbus_message_iter_get_basic(&iter, &wlanid);

	{24,WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE,1502,WTP_APP_AP_INTEFERENCE_DETECTED_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_BYTE,   &chchannel,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],\
								DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],
								
	{25,WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE,1502,WTP_APP_STA_INTEFERENCE_DETECTED_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_BYTE,   &chchannel,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],\
								DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],

	{26,WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE,1502,WTP_APP_DEVICE_INTEFERENCE_DETECTED_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_BYTE,   &chchannel,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],\
								DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],

	{27,WID_DBUS_TRAP_WID_AP_WIFI_IF_ERROR,1103,WTP_APP_MODULE_TROUBLE_TRAP,1<<24},//DBUS_TYPE_UINT32, &wtpindex,\
									DBUS_TYPE_STRING, &wtpsn,\
									DBUS_TYPE_BYTE, &ifindex,\
									DBUS_TYPE_BYTE, &state,\
									DBUS_TYPE_BYTE, &flag,\
									DBUS_TYPE_BYTE, &wtpmac[0],\
									DBUS_TYPE_BYTE, &wtpmac[1],\
									DBUS_TYPE_BYTE, &wtpmac[2],\
									DBUS_TYPE_BYTE, &wtpmac[3],\
									DBUS_TYPE_BYTE, &wtpmac[4],\
									DBUS_TYPE_BYTE, &wtpmac[5],
	{28,WID_DBUS_TRAP_WID_AP_WIFI_IF_ERROR,1103,WTP_APP_MODULE_TROUBLE_CLEAR_TRAP,0},//DBUS_TYPE_UINT32, &wtpindex,\
									DBUS_TYPE_STRING, &wtpsn,\
									DBUS_TYPE_BYTE, &ifindex,\
									DBUS_TYPE_BYTE, &state,\
									DBUS_TYPE_BYTE, &flag,\
									DBUS_TYPE_BYTE, &wtpmac[0],\
									DBUS_TYPE_BYTE, &wtpmac[1],\
									DBUS_TYPE_BYTE, &wtpmac[2],\
									DBUS_TYPE_BYTE, &wtpmac[3],\
									DBUS_TYPE_BYTE, &wtpmac[4],\
									DBUS_TYPE_BYTE, &wtpmac[5],

	{29,WID_DBUS_TRAP_WID_AP_ATH_ERROR,12,WTP_APP_RADIO_DOWN_TRAP,0},//DBUS_TYPE_UINT32, &wtpindex,\
							DBUS_TYPE_STRING, &wtpsn,\
							DBUS_TYPE_BYTE, &radioid,\
							DBUS_TYPE_BYTE, &wlanid,\
							DBUS_TYPE_BYTE, &type,// 1-manual  /2-auto\
							DBUS_TYPE_BYTE, &flag,// 0-disable /1-enable\
							DBUS_TYPE_BYTE, &wtpmac[0],\
							DBUS_TYPE_BYTE, &wtpmac[1],\
							DBUS_TYPE_BYTE, &wtpmac[2],\
							DBUS_TYPE_BYTE, &wtpmac[3],\
							DBUS_TYPE_BYTE, &wtpmac[4],\
							DBUS_TYPE_BYTE, &wtpmac[5],

	{30,WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN,8,WTP_APP_RADIO_DOWN_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
									DBUS_TYPE_STRING, &wtpsn,\
									DBUS_TYPE_BYTE, &wtpmac[0],\
									DBUS_TYPE_BYTE, &wtpmac[1],\
									DBUS_TYPE_BYTE, &wtpmac[2],\
									DBUS_TYPE_BYTE, &wtpmac[3],\
									DBUS_TYPE_BYTE, &wtpmac[4],\
									DBUS_TYPE_BYTE, &wtpmac[5],
									
	{31,WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN_CLEAR,8,WTP_APP_RADIO_DOWN_CLEAR_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
									DBUS_TYPE_STRING, &wtpsn,\
									DBUS_TYPE_BYTE, &wtpmac[0],\
									DBUS_TYPE_BYTE, &wtpmac[1],\
									DBUS_TYPE_BYTE, &wtpmac[2],\
									DBUS_TYPE_BYTE, &wtpmac[3],\
									DBUS_TYPE_BYTE, &wtpmac[4],\
									DBUS_TYPE_BYTE, &wtpmac[5],


	{32,WID_DBUS_TRAP_WID_AP_ATH_ERROR,12,WTP_APP_RADIO_DOWN_CLEAR_TRAP,1<<24},//DBUS_TYPE_UINT32, &wtpindex,\
							DBUS_TYPE_STRING, &wtpsn,\
							DBUS_TYPE_BYTE, &radioid,\
							DBUS_TYPE_BYTE, &wlanid,\
							DBUS_TYPE_BYTE, &type,// 1-manual  /2-auto\
							DBUS_TYPE_BYTE, &flag,// 0-disable /1-enable\
							DBUS_TYPE_BYTE, &wtpmac[0],\
							DBUS_TYPE_BYTE, &wtpmac[1],\
							DBUS_TYPE_BYTE, &wtpmac[2],\
							DBUS_TYPE_BYTE, &wtpmac[3],\
							DBUS_TYPE_BYTE, &wtpmac[4],\
							DBUS_TYPE_BYTE, &wtpmac[5],

	{33,ASD_DBUS_SIG_WTP_DENY_STA,8,WTP_APP_SUB_DATABASE_FULL_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{34,ASD_DBUS_SIG_DE_WTP_DENY_STA,8,WTP_APP_ADD_USR_FAIL_CLEAR_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{35,WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR,8,WTP_APP_DFS_FREECOUNT_BELOW_THRESHOLD_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{36,WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR_CLEAR,8,WTP_APP_CHANNEL_TOO_LOW_CLEAR_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{37,WID_DBUS_TRAP_WID_WTP_CHANNEL_CHANGE,11,WTP_INNER_CHNNNEL_MODIFY_TRAP,55},//DBUS_TYPE_UINT32, &radioID,\
											DBUS_TYPE_BYTE, &chan_past,\
											DBUS_TYPE_BYTE, &chan_curr,\
											DBUS_TYPE_UINT32, &wtpindex,\
											DBUS_TYPE_STRING, &wtpsn,\
											DBUS_TYPE_BYTE, &wtpmac[0],\
											DBUS_TYPE_BYTE, &wtpmac[1],\
											DBUS_TYPE_BYTE, &wtpmac[2],\
											DBUS_TYPE_BYTE, &wtpmac[3],\
											DBUS_TYPE_BYTE, &wtpmac[4],\
											DBUS_TYPE_BYTE, &wtpmac[5],

	{38,ASD_DBUS_SIG_STA_JIANQUAN_FAILED,15,WTP_APP_STATION_AUTH_FAILED_TRAP,55},//DBUS_TYPE_BYTE, &mac[0],\
							DBUS_TYPE_BYTE, &mac[1],\
							DBUS_TYPE_BYTE, &mac[2],\
							DBUS_TYPE_BYTE, &mac[3],\
							DBUS_TYPE_BYTE, &mac[4],\
							DBUS_TYPE_BYTE, &mac[5],\
							DBUS_TYPE_INT32,  &wtpindex,\
							DBUS_TYPE_UINT16, &reason_code,\
							DBUS_TYPE_STRING, &wtpsn,\
							DBUS_TYPE_BYTE, &wtpmac[0],\
							DBUS_TYPE_BYTE, &wtpmac[1],\
							DBUS_TYPE_BYTE, &wtpmac[2],\
							DBUS_TYPE_BYTE, &wtpmac[3],\
							DBUS_TYPE_BYTE, &wtpmac[4],\
							DBUS_TYPE_BYTE, &wtpmac[5],\
							switch(reason_code){\
		case 1:\
			strcpy(str_reason,"802.1X_Authentication_time_out.");\
			break;\
		case 2:\
			strcpy(str_reason,"802.1X_Reauthentication_times_reach_maxium.");\
			break;\
		case 3:\
			strcpy(str_reason,"Invlalid_port.");\
			break;\
		case 4:\
			strcpy(str_reason,"802.1X_Authentication_fail.");\
			break;\
		case 5:\
			strcpy(str_reason,"4way_hand_shake_fail.");\
			break;\
		default:\
			strcpy(str_reason,"Unspecified_failure.");		\
	}\

	{39,ASD_DBUS_SIG_STA_ASSOC_FAILED,15,WTP_APP_STA_ASSIOCIATION_FAILED_TRAP,55},//DBUS_TYPE_BYTE, &mac[0],\
									DBUS_TYPE_BYTE, &mac[1],\
									DBUS_TYPE_BYTE, &mac[2],\
									DBUS_TYPE_BYTE, &mac[3],\
									DBUS_TYPE_BYTE, &mac[4],\
									DBUS_TYPE_BYTE, &mac[5],\
									DBUS_TYPE_INT32,  &wtpindex,\
									DBUS_TYPE_UINT16, &reason_code,\
									DBUS_TYPE_STRING, &wtpsn,\
									DBUS_TYPE_BYTE, &wtpmac[0],\
									DBUS_TYPE_BYTE, &wtpmac[1],\
									DBUS_TYPE_BYTE, &wtpmac[2],\
									DBUS_TYPE_BYTE, &wtpmac[3],\
									DBUS_TYPE_BYTE, &wtpmac[4],\
									DBUS_TYPE_BYTE, &wtpmac[5],\
									switch(reason_code)\
		{\
		case 1:\
			strcpy(str_reason,"Unspecified_failure.");\
			break;\
		case 14:\
			strcpy(str_reason,"Transaction_sequence_number_out_of_expected_sequence.");\
			break;\
		case 17:\
			strcpy(str_reason,"Association_denied_because_AP_is_unable_to_handle_additional_associated_STAs.");\
			break;			\
		case 23:\
			strcpy(str_reason,"Association_request_rejected_because_the_information_in the_Power_Capability_element_is_unacceptable.");\
			break;\
		case 40:\
			strcpy(str_reason,"Invalid_information_element.");\
			break;\
		case 41:\
			strcpy(str_reason,"Invalid_group_cipher.");\
			break;\
		case 42:\
			strcpy(str_reason,"Invalid_pairwise_cipher.");\
			break;\
		case 43:\
			strcpy(str_reason,"Invalid_AKMP.");\
			break;\
		default:\
			strcpy(str_reason,"Unspecified_failure.");	\
			break;\
		}

	{40,ASD_DBUS_SIG_WAPI_TRAP,1501,WTP_APP_USER_WITH_INVALID_CER_BREAK_NETWORK_TRAP,1<<24},//DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],\
								DBUS_TYPE_BYTE,   &reason, reason=1  \
								DBUS_TYPE_INT32,  &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{41,ASD_DBUS_SIG_WAPI_TRAP,1501,WTP_APP_STATION_REPITIVE_ATTACK_TRAP,2<<24},//DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],\
								DBUS_TYPE_BYTE,   &reason, reason=2 \
								DBUS_TYPE_INT32,  &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{42,ASD_DBUS_SIG_WAPI_TRAP,1501,WTP_APP_TAMPER_ATTACK_TRAP, 3<<24},//DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],\
								DBUS_TYPE_BYTE,   &reason, reason=3 \
								DBUS_TYPE_INT32,  &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{43,ASD_DBUS_SIG_WAPI_TRAP,1501,WTP_APP_LOW_SAFE_LEVEL_ATTACK_TRAP,4<<24},//DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],\
								DBUS_TYPE_BYTE,   &reason, reason=4 \
								DBUS_TYPE_INT32,  &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{44,ASD_DBUS_SIG_WAPI_TRAP,1501,WTP_APP_ADDRESS_REDIRECTION_ATTACK_TRAP,5<<24},//DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],\
								DBUS_TYPE_BYTE,   &reason, reason=5 \
								DBUS_TYPE_INT32,  &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{45,WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_DANGER_AP,8,AC_APP_DISCOVER_DANGER_AP_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{46,WID_DBUS_TRAP_WID_WTP_FIND_UNSAFE_ESSID,904,WTP_APP_AP_FIND_UNSAFE_ESSID,55},//DBUS_TYPE_UINT32, &wtpindex,\
							DBUS_TYPE_STRING, &wtpsn,\
							DBUS_TYPE_BYTE, &wtpmac[0],\
							DBUS_TYPE_BYTE, &wtpmac[1],\
							DBUS_TYPE_BYTE, &wtpmac[2],\
							DBUS_TYPE_BYTE, &wtpmac[3],\
							DBUS_TYPE_BYTE, &wtpmac[4],\
							DBUS_TYPE_BYTE, &wtpmac[5],\
							DBUS_TYPE_STRING, &name,

	{47,WID_DBUS_TRAP_WID_WTP_DIVORCE_NETWORK,8,AC_APP_AP_LOST_NET_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
												DBUS_TYPE_STRING, &wtpsn,\
												DBUS_TYPE_BYTE, &wtpmac[0],\
												DBUS_TYPE_BYTE, &wtpmac[1],\
												DBUS_TYPE_BYTE, &wtpmac[2],\
												DBUS_TYPE_BYTE, &wtpmac[3],\
												DBUS_TYPE_BYTE, &wtpmac[4],\
												DBUS_TYPE_BYTE, &wtpmac[5],

	{48,ASD_DBUS_SIG_STA_LEAVE,16,WTP_APP_STATION_OFF_LINE_TRAP,55},//DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],\
								DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_UINT32, &g_bss,\
								DBUS_TYPE_BYTE,   &wlanid,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{49,ASD_DBUS_SIG_STA_COME,16,WTP_APP_STATION_ON_LINE_TRAP,55},//DBUS_TYPE_BYTE, &mac[0],\
								DBUS_TYPE_BYTE, &mac[1],\
								DBUS_TYPE_BYTE, &mac[2],\
								DBUS_TYPE_BYTE, &mac[3],\
								DBUS_TYPE_BYTE, &mac[4],\
								DBUS_TYPE_BYTE, &mac[5],\
								DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_UINT32, &g_bss,\
								DBUS_TYPE_BYTE,   &wlanid,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{50,WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE_CLEAR,903,WTP_APP_OTHER_DEVICE_INTERF_CLEAR_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_BYTE,   &chchannel,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{51,WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE_CLEAR,903,WTP_APP_INTERF_NEARBY_CLEAR_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_BYTE,   &chchannel,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{52,WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE_CLEAR,903,WTP_APP_STA_INTERFERE_CLEAR_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
								DBUS_TYPE_BYTE,   &chchannel,\
								DBUS_TYPE_STRING, &wtpsn,\
								DBUS_TYPE_BYTE, &wtpmac[0],\
								DBUS_TYPE_BYTE, &wtpmac[1],\
								DBUS_TYPE_BYTE, &wtpmac[2],\
								DBUS_TYPE_BYTE, &wtpmac[3],\
								DBUS_TYPE_BYTE, &wtpmac[4],\
								DBUS_TYPE_BYTE, &wtpmac[5],

	{53,WID_DBUS_TRAP_WID_WTP_ENTER_IMAGEDATA_STATE, 9,WTP_INNER_AP_UPDATE_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
											DBUS_TYPE_UINT32, &registercircle,\
											DBUS_TYPE_STRING, &wtpsn,\
											DBUS_TYPE_BYTE, &wtpmac[0],\
											DBUS_TYPE_BYTE, &wtpmac[1],\
											DBUS_TYPE_BYTE, &wtpmac[2],\
											DBUS_TYPE_BYTE, &wtpmac[3],\
											DBUS_TYPE_BYTE, &wtpmac[4],\
											DBUS_TYPE_BYTE, &wtpmac[5],

	{54,WID_DBUS_TRAP_WID_WTP_FIND_WIDS_ATTACK,1401,WTP_APP_AP_FIND_ATTACK,55},//DBUS_TYPE_UINT32, &wtpindex,\
							DBUS_TYPE_STRING, &wtpsn,\
							DBUS_TYPE_BYTE, &wtpmac[0],\
							DBUS_TYPE_BYTE, &wtpmac[1],\
							DBUS_TYPE_BYTE, &wtpmac[2],\
							DBUS_TYPE_BYTE, &wtpmac[3],\
							DBUS_TYPE_BYTE, &wtpmac[4],\
							DBUS_TYPE_BYTE, &wtpmac[5],\
							DBUS_TYPE_BYTE, &attackStamac[0],\
							DBUS_TYPE_BYTE, &attackStamac[1],\
							DBUS_TYPE_BYTE, &attackStamac[2],\
							DBUS_TYPE_BYTE, &attackStamac[3],\
							DBUS_TYPE_BYTE, &attackStamac[4],\
							DBUS_TYPE_BYTE, &attackStamac[5],


	//AC
	{55,"wid_dbus_trap_power_state_change",1,AC_APP_POWER_OFF_TRAP,1},//DBUS_TYPE_UINT32, &power_state, power_state=0 acPowerOffRecovTrap 1 acPowerOffTrap

	{56,"wid_dbus_trap_power_state_change",1,AC_APP_POWER_OFF_CLEAR_TRAP,0},//DBUS_TYPE_UINT32, &power_state, power_state=0 acPowerOffRecovTrap 1 acPowerOffTrap
	
	{57,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_CPU,2,AC_APP_CPU_ULTILIZATION_OVER_THRESHOLD_TRAP,1},//DBUS_TYPE_INT32, &type, \
											DBUS_TYPE_UINT32,&latest,

	{58,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_CPU,2,AC_APP_CPU_USAGE_TOO_HIGH_CLEAR_TRAP,0},//DBUS_TYPE_INT32, &type, \
	
	{59,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MEMUSAGE,2,AC_APP_MEM_ULTILIZATION_OVER_THRESHOLD_TRAP,1},//DBUS_TYPE_INT32, &type, \
											DBUS_TYPE_UINT32,&latest,
											
	{60,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MEMUSAGE,2,AC_APP_MEM_USAGE_TOO_HIGH_CLEAR_TRAP,0},//DBUS_TYPE_INT32, &type, \

	

	{61,NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP,7,AC_INNER_TURN_TO_BACK_DEVICE_TRAP,99},//DBUS_TYPE_UINT32, &state,\
										DBUS_TYPE_BYTE, &sysmac[0],\
										DBUS_TYPE_BYTE, &sysmac[1],\
										DBUS_TYPE_BYTE, &sysmac[2],\
										DBUS_TYPE_BYTE, &sysmac[3],\
										DBUS_TYPE_BYTE, &sysmac[4],\
										DBUS_TYPE_BYTE, &sysmac[5],

	{62,NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP,7,AC_INNER_TURN_TO_BACK_DEVICE_TRAP,3},//DBUS_TYPE_UINT32, &state,\
										DBUS_TYPE_BYTE, &sysmac[0],\
										DBUS_TYPE_BYTE, &sysmac[1],\
										DBUS_TYPE_BYTE, &sysmac[2],\
										DBUS_TYPE_BYTE, &sysmac[3],\
										DBUS_TYPE_BYTE, &sysmac[4],\
										DBUS_TYPE_BYTE, &sysmac[5],

	{63,"dhcp_notify_web_pool_event",4,AC_APP_DHCP_ADDRESS_EXHAUSTED_TRAP,1},//DBUS_TYPE_UINT32, &flags,\
								DBUS_TYPE_UINT32, &total, \
								DBUS_TYPE_UINT32, &not_used, \
								DBUS_TYPE_STRING, &poolname, 

	{64,"dhcp_notify_web_pool_event",4,AC_APP_DHCP_ADDRESS_EXHAUSTED_CLEAR_TRAP,0},//DBUS_TYPE_UINT32, &flags,\
								DBUS_TYPE_UINT32, &total, \
								DBUS_TYPE_UINT32, &not_used, \
								DBUS_TYPE_STRING, &poolname, 

	{65,WID_DBUS_TRAP_WID_WTP_ACTIMESYNCHROFAILURE,8,AC_INNER_AP_TIME_SYNC_FAIL_TRAP,0},//!!!901 DBUS_TYPE_UINT32, &wtpindex,\
													DBUS_TYPE_STRING, &wtpsn,\
													DBUS_TYPE_BYTE, &wtpmac[0],\
													DBUS_TYPE_BYTE, &wtpmac[1],\
													DBUS_TYPE_BYTE, &wtpmac[2],\
													DBUS_TYPE_BYTE, &wtpmac[3],\
													DBUS_TYPE_BYTE, &wtpmac[4],\
													DBUS_TYPE_BYTE, &wtpmac[5],

	{66,NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_IP,302,AC_INNER_CHANGE_IP_TRAP,5555},//DBUS_TYPE_UINT32, &isAdd,\
									DBUS_TYPE_BYTE, &mask,\
									DBUS_TYPE_STRING, &ip,

	{67,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_BANDWITH,3,AC_APP_BAND_ULTILIZATION_OVER_THRESHOLD_TRAP,1},//DBUS_TYPE_INT32, &type,\
											DBUS_TYPE_UINT32, &ifindex,\
											DBUS_TYPE_UINT32, &latest,
											
	{68,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DROP_RATE,3,AC_APP_LOSTPACKET_RATE_OVER_THRESHOLD_TRAP,1},//DBUS_TYPE_INT32, &type,\
											DBUS_TYPE_UINT32, &ifindex,\
											DBUS_TYPE_UINT32, &latest,

	{69,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MAX_USER,2,AC_APP_MAX_USERNUM_OVER_THRESHOLD_TRAP,1},//DBUS_TYPE_INT32, &type,\
											DBUS_TYPE_UINT32,&latest,

	{70,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_REQ,2,AC_APP_AUTH_SUC_RATE_BELOW_THRESHOLD_TRAP,1},//DBUS_TYPE_INT32, &type,	\
											DBUS_TYPE_UINT32,&latest,

	{71,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_IP_POOL,2,AC_APP_AVE_IPPOOL_OVER_THRESHOLD_TRAP,1},//DBUS_TYPE_INT32, &type,\
											DBUS_TYPE_UINT32,&latest,

	{72,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_FIND_SYN_ATTACK,2,AC_APP_FIND_SYN_ATTACK,1},//DBUS_TYPE_INT32, &type,\
														DBUS_TYPE_UINT32,&latest,

	{73,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_TEMPERATURE,2,AC_APP_TEMPER_TOO_HIGH_TRAP,1},//DBUS_TYPE_INT32, &type,\
											DBUS_TYPE_UINT32,&latest,

	{74,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_TEMPERATURE,2,AC_APP_TEMPER_TOO_HIGH_CLEAR_TRAP,0},//DBUS_TYPE_INT32, &type,\
											DBUS_TYPE_UINT32,&latest,
											
	{75,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_AUTH,301,AC_APP_RADIUS_AUTH_SERVER_NOT_REACH_TRAP,1},//DBUS_TYPE_UINT32, &type,\
											DBUS_TYPE_STRING, &ip,\
											DBUS_TYPE_UINT16, &port,

	{76,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_ACC,301,AC_APP_RADIUS_ACCOUNT_SERVER_NOT_REACH_TRAP,1},//DBUS_TYPE_UINT32, &type,\
											DBUS_TYPE_STRING, &ip,\
											DBUS_TYPE_UINT16, &port,

	{77,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_AUTH,301,AC_APP_RADIUS_AUTH_SERVER_NOT_REACH_CLEAR_TRAP,0},//DBUS_TYPE_UINT32, &type,\
											DBUS_TYPE_STRING, &ip,\
											DBUS_TYPE_UINT16, &port,
											
	{78,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_ACC,301,AC_APP_RADIUS_ACCOUNT_SERVER_NOT_REACH_CLEAR_TRAP,0},//DBUS_TYPE_UINT32, &type,\
											DBUS_TYPE_STRING, &ip,\
											DBUS_TYPE_UINT16, &port,

	{79,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_PORTAL_REACH,301,AC_APP_PORTAL_SERVER_NOT_REACH_TRAP,1},//DBUS_TYPE_UINT32,  &type, \
											DBUS_TYPE_STRING, &cur_ip,\
											DBUS_TYPE_UINT16, &port,

	{80,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_PORTAL_REACH,301,AC_APP_PORTER_SERVER_NOT_REACH_CLEAR_TRAP,0},//DBUS_TYPE_UINT32,  &type, \
											DBUS_TYPE_STRING, &cur_ip,\
											DBUS_TYPE_UINT16, &port,	
										
	{81,WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE,1502,WTP_APP_CHANNEL_OBSTRUCTION_TRAP,55},//DBUS_TYPE_UINT32, &wtpindex,\
							DBUS_TYPE_BYTE,   &chchannel,\
							DBUS_TYPE_STRING, &wtpsn,\
							DBUS_TYPE_BYTE, &wtpmac[0],\
							DBUS_TYPE_BYTE, &wtpmac[1],\
							DBUS_TYPE_BYTE, &wtpmac[2],\
							DBUS_TYPE_BYTE, &wtpmac[3],\
							DBUS_TYPE_BYTE, &wtpmac[4],\
							DBUS_TYPE_BYTE, &wtpmac[5],\
							DBUS_TYPE_BYTE, &mac[0],\
							DBUS_TYPE_BYTE, &mac[1],\
							DBUS_TYPE_BYTE, &mac[2],\
							DBUS_TYPE_BYTE, &mac[3],\
							DBUS_TYPE_BYTE, &mac[4],\
							DBUS_TYPE_BYTE, &mac[5],
							
	
											//DBUS_TYPE_UINT32,&latest,
											
	
											//DBUS_TYPE_UINT32,&latest,
											
												
	{82,WID_DBUS_TRAP_WID_WTP_ACTIMESYNCHROFAILURE,8, ".0.22",1<<24},//!!!901 DBUS_TYPE_UINT32, &wtpindex,\
													DBUS_TYPE_STRING, &wtpsn,\
													DBUS_TYPE_BYTE, &wtpmac[0],\
													DBUS_TYPE_BYTE, &wtpmac[1],\
													DBUS_TYPE_BYTE, &wtpmac[2],\
													DBUS_TYPE_BYTE, &wtpmac[3],\
													DBUS_TYPE_BYTE, &wtpmac[4],\
													DBUS_TYPE_BYTE, &wtpmac[5],\
													//DBUS_TYPE_BYTE,	&flag,
	//{83,},//\
	{84,},//\
	{85,},//\
	{86,},//\
	{87,},//\
	{88,},//\
	{89,},//\
	{90,},//\
	{91,},//\
	//{not completed WID_DBUS_TRAP_WID_WTP_NEIGHBOR_CHANNEL_AP_INTERFERENCE\
											ApNeighborChannelInterfTrap\
											ApNeighborChannelInterfTrapClear

};


//exg: {1,AC_SAMPLE_OVER_THRESHOLD_SIGNAL_PORTAL_REACH,3},// DBUS_TYPE_UINT32, &type,  DBUS_TYPE_STRING, &rcv_ip,  DBUS_TYPE_UINT16, &rcv_port,


int len =sizeof(gsiglist)/sizeof(SIGLIST);


inline SIGLIST *get_gsiglist_member(int signal_number, int operator)
{
	signal_number=signal_number-1;

	if(signal_number<0||signal_number>=len)
	{
		return GET_GSIGLIST_MEM_ERR;
	}

	return &gsiglist[signal_number];

	/*

	   SIGLIST sig;

	   sig.num=gsiglist[signal_number].num;
	   sig.signal_name=gsiglist[signal_number].signal_name;
	   sig.func_param=gsiglist[signal_number].func_param;

	   return sig;
	   */
}

/*{1,WID_DBUS_TRAP_WID_AP_CPU_THRESHOLD, (message, &error,
  DBUS_TYPE_UINT32, &wtpindex,
  DBUS_TYPE_STRING, &wtpsn,
  DBUS_TYPE_UINT32, &cpu,
  DBUS_TYPE_UINT32, &cpu_threshold,
  DBUS_TYPE_BYTE, &flag,
  DBUS_TYPE_BYTE, &wtpmac[0],
  DBUS_TYPE_BYTE, &wtpmac[1],
  DBUS_TYPE_BYTE, &wtpmac[2],
  DBUS_TYPE_BYTE, &wtpmac[3],
  DBUS_TYPE_BYTE, &wtpmac[4],
  DBUS_TYPE_BYTE, &wtpmac[5],
  DBUS_TYPE_INVALID)
*/
