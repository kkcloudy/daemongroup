#ifndef __DCLI_COMMON_DLDP_H__
#define __DCLI_COMMON_DLDP_H__

/*********************************************************
*	head files														*
**********************************************************/
#include <zebra.h>
#include <dbus/dbus.h>
#include "command.h"
			
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <util/npd_list.h>

#include "dcli_system.h"
#include "dcli_main.h"
#include "dcli_dldp.h"


/*********************************************************
*	macro define													*
**********************************************************/

/*********************************************************
*	struct define													*
**********************************************************/


/*********************************************************
*	function declare												*
**********************************************************/


/**********************************************************************************
 * dcli_dldp_check_global_status()
 *	DESCRIPTION:
 *		check  dldp enable/disable global status 
 *
 *	INPUTS:
 *		NULL
 *
 *	OUTPUTS:
 *		unsigned char* stats		- check result
 *
 *	RETURN VALUE:
 *		DLDP_RETURN_CODE_OK	- success
 *		DLDP_RETURN_CODE_ERROR	- fail
***********************************************************************************/
int dcli_dldp_check_global_status
(
	unsigned char* stats,
	unsigned char* devchk
);


/**********************************************************************************
 * dcli_dldp_enable_disable_one_vlan()
 *	DESCRIPTION:
 *		enable/disable DLDP on vlan and its ports by vlanid, base vlan
 *
 *	INPUTS:
 *		unsigned short vlanid
 *		unsigned char enable
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DCLI_NPD_DLDP_RETURN_CODE_OK				- success
 *		DCLI_NPD_DLDP_RETURN_CODE_ERROR			- fail
 * 		DCLI_NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST	- L2 vlan not exist
 *		DCLI_NPD_DLDP_RETURN_CODE_NOT_ENABLE_GBL	- not enable DLDP global
 *		DCLI_NPD_DLDP_RETURN_CODE_NOTENABLE_VLAN	- L2 vlan not enable DLDP
 *		DCLI_NPD_DLDP_RETURN_CODE_HASENABLE_VLAN	- L2 vlan has enabled DLDP
***********************************************************************************/
unsigned int dcli_dldp_enable_disable_one_vlan
(
	unsigned short vlanid,
	unsigned char enable
);


/**********************************************************************************
 *dcli_dldp_vlan_show_running_config()
 *
 *	DESCRIPTION:
 *		show DLDP vlan running config
 *
 *	INPUTS:
 *		NULL
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
int dcli_dldp_vlan_show_running_config
(
	void
);

/**********************************************************************************
 *dcli_dldp_time_show_running_config()
 *
 *	DESCRIPTION:
 *		show DLDP time running config
 *
 *	INPUTS:
 *		NULL
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
int dcli_dldp_time_show_running_config
(
	void
);

/**********************************************************************************
 *  dcli_dldp_get_product_support_function
 *
 *	DESCRIPTION:
 * 		get broad type
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   disable
 *		1   -   enable
 *
 **********************************************************************************/

int dcli_dldp_get_product_support_function
(
	struct vty* vty,
	unsigned char *IsSupport
);

/*********************************************************
*	extern Functions												*
**********************************************************/

#endif

