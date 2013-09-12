/******************************************************************************
* Description:
*   WSM module Dbus_PATH macro file.
*
* Date:
*
* Author:
*
* Copyright:
*   Copyright (C) Autelan Technology. All right reserved.
*
* Modify history:
* DATE:
*  2011-04-13
*
* CREATOR:
*  zhangdx@autelan.com
*
* CHANGE LOG:
*  2011-04-13 <zhangdx> Create file.
******************************************************************************/
#ifndef _SEM_DBUS_DEF_H
#define _SEM_DBUS_DEF_H


/* for mib */
#define SEM_TRAP_OBJPATH					"/aw/sem_trap"
#define SEM_TRAP_INTERFACE					"aw.trap"
#define SEM_TRAP_SYSTEM_STATE				"sem_trap_system_state"
#define SEM_TRAP_BOARD_STATE				"sem_trap_board_state"
#define SEM_DBUS_TRAP_POWER_STATE_CHANGE	"wid_dbus_trap_power_state_change"
#define SEM_DBUS_TRAP_FAN_STATE_CHANGE		"wid_dbus_trap_fan_state_change"

#define SEM_DBUS_BUSNAME	"aw.sem"
#define SEM_DBUS_OBJPATH	"/aw/sem"
#define SEM_DBUS_INTERFACE	"aw.sem"
#define SEM_DBUS_ETHPORTS_OBJPATH   "/aw/sem/ethport"
#define SEM_DBUS_ETHPORTS_INTERFACE "aw.sem.ethport"
#define SEM_DBUS_INTF_OBJPATH       "/aw/sem/interface"
#define SEM_DBUS_INTF_INTERFACE     "aw.sem.interface"

#define SEM_DBUS_CONF_TUNNEL		       "sem_conf_tunnel"
#define SEM_DBUS_CONF_SET_TUNNEL               "sem_conf_set_ipfwd_tunnel"
#define SEM_DBUS_SEND_FILE			"sem_send_file"
#define SEM_DBUS_CONF_TIPC			"sem_conf_tipc"
#define SEM_DBUS_CONF_48GE			"sem_conf_48GE"
#define SEM_DBUS_SHOW_SLOT_ID		"sem_dbus_show_slot_id"
#define SEM_DBUS_SHOW_6185		"sem_dbus_show_6185"
#define SEM_DBUS_SET_6185		"sem_dbus_set_6185"
#define SME_DBUS_SHOW_SLOT_N_INFO	"sem_dbus_show_slot_n_info"
#define SEM_DBUS_SHOW_BOARD_INFO	"sem_dbus_show_board_info"
#define SEM_DBUS_SHOW_PRODUCT_INFO	"sem_dbus_show_product_info"
#define SEM_DBUS_SHOW_SLOT_INFO		"sem_dbus_show_slot_info"

#define SEM_DBUS_INTERFACE_METHOD_SYSTEM_DEBUG_STATE         "dbg_sem"
#define SEM_DBUS_INTERFACE_METHOD_SYSTEM_UNDEBUG_STATE       "no_dbg_sem"

#define SEM_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_LIST          "sem_debus_show_ethport_list"
#define SEM_DBUS_CONFIG_ETHPORT 	    "sem_dbus_config_ethport"
#define SEM_DBUS_CONFIG_ETHPORT_ATTR	"sem_dbus_config_ethport_attribute"
#define SEM_DBUS_SHOW_ETHPORT_ATTR		"sem_dbus_show_ethport_attr"
#define SEM_DBUS_CONFIG_MEDIA_PREFERR	"sem_dbus_config_media_preferr"
#define SEM_DBUS_CONFIG_MEDIA_MODE   	"sem_dbus_config_media_mode"
#define SEM_DBUS_ETHPORTS_GET_SLOT_PORT "sem_dbus_get_slotport_from_index"
#define SEM_DBUS_REBOOT					"sem_dbus_reboot"
#define SEM_DBUS_RESET_ALL				"sem_dbus_reset_all"		/*write cpld reg for hard reset*/
#define SEM_DBUS_SYN_FILE				"sem_dbus_syn_file"
#define SEM_DBUS_SET_BOOT_IMG			"sem_dbus_set_boot_img"
#define SEM_DBUS_MCB_ACTIVE_STANDBY_SWITCH	 "sem_dbus_mcb_active_standby_switch"
#define SEM_DBUS_SYNC_MASTER_ETH_PORT_INFO	 "sem_dbus_sync_master_eth_port_info"
#define SEM_DBUS_SHOW_SYSTEM_ENVIRONMENT     "sem_dbus_show_system_environment"
#define SEM_DBUS_SET_SYSTEM_IMG			"sem_dbus_set_system_img"

#define SEM_DBUS_SHOW_BOOTROM_ENVIRONMENT_VARIABLE "sem_dbus_show_boot_env_var"
#define SEM_DBUS_SET_BOOTCMD "sem_dbus_set_bootcmd"
#define SEM_DBUS_SET_ENVIRONMENT_VARIABLE "sem_dbus_set_boot_env_var"

#define SEM_DBUS_DISABLE_KEEP_ALIVE_TEMPORARILY		"sem_dbus_disable_keep_alive_temporarily"
#define SEM_DBUS_EXECUTE_SYSTEM_COMMAND				"sem_dbus_execute_system_command"
#define SEM_DBUS_SHOW_ALL_SLOT_SYS_INFO				"sem_dbus_show_all_slot_sys_info"
/*added by zhaocg for md5 img command*/
#define SEM_DBUS_MD5_IMG_SLOT                       "sem_dbus_md5_img_slot"
#define SEM_DBUS_MD5_PATCH_SLOT                     "sem_dbus_md5_patch_slot"
/*added by zhaocg for fastfwd command*/
#define SEM_DBUS_IMG_OR_FASTFWD_SLOT                 "sem_dbus_img_or_fastfwd_slot"
#define SEM_DBUS_DEL_IMG_OR_FASTFWD_SLOT             "sem_dbus_delete_img_or_fastfwd_slot"
#define SEM_DBUS_USER_ADD_SLOT                 "sem_dbus_user_add_slot"
#define SEM_DBUS_USER_DEL_SLOT                 "sem_dbus_user_del_slot"
#define SEM_DBUS_USER_ROLE_SLOT                 "sem_dbus_user_role_slot"
#define SEM_DBUS_USER_SHOW_SLOT                 "sem_dbus_user_show_slot"
#define SEM_DBUS_USER_SHOW_RUNNING                 "sem_dbus_user_show_running"
#define SEM_DBUS_USER_PASSWD_SLOT				"sem_dbus_user_passwd_slot"
#define SEM_DBUS_USER_IS_EXSIT_SLOT				"sem_dbus_user_is_exsit_slot"

#define SEM_DBUS_DOWNLOAD_IMG_SLOT				"sem_dbus_download_img_slot"
#define SEM_DBUS_DOWNLOAD_CPY_CONFIG_SLOT				"sem_dbus_download_cpy_config_slot"
#define SEM_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_INTERFACE   "sem_config_eth_mode_interface"
#define SEM_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE   "sem_config_eth_mode"
#define SEM_DBUS_SUB_INTERFACE_CREATE                            "sem_create_sub_inf"
#define SEM_DBUS_SUB_INTERFACE_DELETE                            "sem_delete_sub_inf"
#define SEM_DBUS_SUB_INTERFACE_ETHPORT_QINQ_TYPE_SET                  "sem_qinq_type_set"

#define SEM_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_RUNNING_CONFIG	"show_ethport_runconfig"
#define SEM_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_STAT    "show_ethport_stat"
#define SEM_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_STAT   "clear_ethport_stat"
#define SEM_DBUS_SHOW_INFO "sem_show_info"

#define SEM_DBUS_TUNNEL_SHOW_RUNNING           "sem_tunnel_show_running"

#define SEM_DBUS_SET_IF_WAN_STATE				"sem_set_interface_wan_state"
#define SEM_DBUS_CAM_CORE_WRITE                 "sem_cam_core_write"
#define SEM_DBUS_CAM_CORE_READ                 "sem_cam_core_read"
#define SEM_DBUS_CAM_WRITE                 "sem_cam_write"
#define SEM_DBUS_CAM_READ                "sem_cam_read"
#define SEM_DBUS_CAR_WRITE                 "sem_car_write"
#define SEM_DBUS_CAR_WHITE_WRITE                 "sem_car_white_write"
#define SEM_DBUS_CAR_SUBNET_WRITE                 "sem_car_subnet_write"
#define SEM_DBUS_CAR_READ                 "sem_car_read"
#define SEM_DBUS_SHOW_CAR_LIST                 "sem_show_car_list"
#define SEM_DBUS_SHOW_CAR_WHITE_LIST                 "sem_show_car_white_list"
#define SEM_DBUS_SHOW_HASH_TABLE                 "sem_show_hash_table"
//#define SEM_DBUS_SHOW_CAR_LIST_DEBUG                 "sem_show_car_list_debug"

#define SEM_DBUS_GET_CAR_LIST_COUNT                 "sem_get_car_list_count"
#define SEM_DBUS_GET_CAR_WHITE_LIST_COUNT                 "sem_get_car_white_list_count"
#define SEM_DBUS_GET_CAR_TABLE_RESTORE                 "sem_set_car_table_restore"
#define SEM_DBUS_GET_CAR_TABLE_EMPTY                 "sem_set_car_table_empty"
#define SEM_DBUS_SHOW_FPGA_CAR_VALID                "show_fpga_car_valid"
#define SEM_DBUS_SET_HASH_AGING_TIME                 "set_hash_aging_time"
#define SEM_DBUS_SET_HASH_UPDATE_TIME                 "set_hash_update_time"
#define SEM_DBUS_SHOW_HASH_CAPACITY                "show_hash_capacity"
#define SEM_DBUS_SHOW_PORT_COUNTER                "show_port_counter"
#define SEM_DBUS_SHOW_HASH_STATISTICS                "show_hash_statistics"
#define SEM_DBUS_SHOW_FPGA_REG                "show_fpga_reg"
#define SEM_DBUS_CHECK_BOARD_TYPE                "check_board_type"
#define SEM_DBUS_LOCAL_CHECK_BOARD_TYPE                "local_check_board_type"
#define SEM_DBUS_SHOW_FPGA_DDR_QDR                "show_fpga_ddr_qdr"
#define SEM_DBUS_SHOW_FPGA_HASH_VALID                "show_fpga_hash_valid"
#define SEM_DBUS_SHOW_FPGA_REG_ARR                "show_fpga_reg_arr"
#define SEM_DBUS_SHOW_FPGA_WAN_IF                "show_fpga_wan_if"
#define SEM_DBUS_GET_FPGA_WAN_IF_NUM                 "sem_get_fpga_wan_if_num"


#define SEM_DBUS_CONFIG_FPGA_SYSREG_WORKING                "config_fpga_sysreg_working"
#define SEM_DBUS_CONFIG_SEM_TO_TRAP                "config_sem_to_trap"
#define SEM_DBUS_CONFIG_FPGA_SYSREG_QOS                "config_fpga_sysreg_QoS"
#define SEM_DBUS_CONFIG_FPGA_SYSREG_MODE                "config_fpga_sysreg_mode"
#define SEM_DBUS_CONFIG_FPGA_SYSREG_CAR_UPDATE_CFG                "config_fpga_sysreg_car_update_cfg"
#define SEM_DBUS_CONFIG_FPGA_SYSREG_TRUNK_TAG_TYPE                "config_fpga_sysreg_trunk_tag_type"
#define SEM_DBUS_CONFIG_FPGA_SYSREG_WAN_TAG_TYPE                "config_fpga_sysreg_wan_tag_type"
#define SEM_DBUS_CONFIG_FPGA_SYSREG_CAR_LINKUP                "config_fpga_sysreg_car_linkup"
#define SEM_DBUS_CONFIG_FPGA_SYSREG_CAR_LINKDOWN                "config_fpga_sysreg_car_linkdown"
#define SEM_DBUS_SET_FPGA_RESET                "set_fpga_reset"
#define SEM_DBUS_FPGA_SHOW_RUNNING_CONFIG                 "fpga_show_running_config"
#define SEM_DBUS_CONFIG_WRITE_FPGA            "config_fpga_write_fpga"
#define SEM_DBUS_CONFIG_WRITE_CPLD                "config_fpga_write_cpld"
#define SEM_DBUS_CONFIG_READ_CPLD                "config_fpga_read_cpld"

#define SEM_DBUS_APPLY_PATCH		"sem_dbus_apply_patch"
#define SEM_DBUS_DELETE_PATCH		"sem_dbus_delete_patch"
/* added by zhengbo for upload snapshot */
#define SEM_DBUS_UPLOAD_SNAPSHOT	"sem_dbus_upload_snapshot"

/********************************************
xufujun

**********************************************/

#define SEM_DBUS_FLASH_ERASE_PARTITIO  "flash_erase_partition"
#define WRITE_BOOT_TO_FLASH		"write_boot_to_flash"


#define SEM_MAC_LEN 6
 

#define SYN_FILE_SUCCESS	0
#define SYN_FILE_FAILED		-1

enum {
	SEM_COMMAND_SUCCESS,
	SEM_COMMAND_FAILED,
	SEM_WRONG_PARAM,
	SEM_OPERATE_NOT_SUPPORT,
	SEM_COMMAND_NOT_SUPPORT,
	SEM_COMMAND_NOT_PERMIT
};


/*
	this marco indicate config ethport attr type
	and return value
*/
#define DEFAULT 0xff
#define ADMIN (1<<0)
#define SPEED (1<<1)
#define AUTONEGT (1<<2)
#define AUTONEGTS (1<<3)
#define AUTONEGTD (1<<4)
#define AUTONEGTF (1<<5)
#define DUMOD (1<<6)
#define FLOWCTRL (1<<7)
#define BACKPRE (1<<8)
#define LINKS (1<<9)
#define CFGMTU (1<<10)

#define	SLOT_PORT_ANALYSIS_SLOT(combination, slot) 	(slot = (((combination)>>6) & 0x1f) + 1)
#define	SLOT_PORT_COMBINATION(slot, port)	(((((slot) & 0x1f) - 1)<<6) + ((port) - 1))

#endif
