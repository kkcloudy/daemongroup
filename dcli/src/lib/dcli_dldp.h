#ifndef __DCLI_DLDP_H__
#define __DCLI_DLDP_H__

/*********************************************************
*	head files														*
**********************************************************/
#include "dcli_common_dldp.h"

/*********************************************************
*	macro define													*
**********************************************************/
#define	DCLI_DEBUG_STR "debug\n"

#define DCLI_DLDP_STR "DLDP information\n"

#define DCLI_DLDP_RETURN_CODE_BASE			 (0x140000)				/* return code base 	*/
#define DCLI_DLDP_RETURN_CODE_OK			 (0x140000)				/* success			*/
#define DCLI_DLDP_RETURN_CODE_ERROR			 (0x140001)				/* error 				*/
#define DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL (0x140004)				/* DLDP not enabled global*/
#define DCLI_DLDP_RETURN_CODE_OUT_RANGE		 (0x140005)				/* timer value outof range */
#define DCLI_DLDP_RETURN_CODE_SAME_VALUE	 (0x140006)				/* set same value to DLDP timers*/
#define DCLI_DLDP_RETURN_CODE_GET_DETECT_E	 (0x140007)				/* get DLDP detect timers error	*/
#define DCLI_DLDP_RETURN_CODE_GET_REDETECT_E (0x140008)				/* get DLDP re-detect timers error*/
#define DCLI_DLDP_RETURN_CODE_VLAN_NOT_EXIST (0x140009)				/* L2 vlan not exist			*/
#define DCLI_DLDP_RETURN_CODE_NOTENABLE_VLAN (0x14000a)				/* L2 vlan not enable DLDP		*/
#define DCLI_DLDP_RETURN_CODE_HASENABLE_VLAN (0x14000b)				/* L2 vlan has enable DLDP		*/



#define DCLI_DLDP_VLAN_ENABLE				(1)						/* enable dldp on vlan	*/
#define DCLI_DLDP_VLAN_DISABLE				(0)						/* dienable dldp on vlan	*/
#define DCLI_DLDP_PORT_ENABLE				(1)						/* enable dldp on port	*/
#define DCLI_DLDP_PORT_DISABLE				(0)						/* dienable dldp on port	*/

#define DCLI_DLDP_MAX_CHASSIS_SLOT_COUNT	(16)
#define DCLI_DLDP_MAX_ETHPORT_PER_BOARD		(64)


#define DCLI_DLDP_INIT_0					(0)						/* init int/short/long variable, 0*/

#define TURE_5612E 1
#define FALSE_5612E 0
/*********************************************************
*	struct define													*
**********************************************************/
/* DLDP timer type			*/
typedef enum {
	DCLI_DLDP_TYPE_DETECTION = 1,
	DCLI_DLDP_TYPE_REDETECTION,
	DCLI_DLDP_TYPE_INVALID
}DCLI_DLDP_TIMER_TYPE;


/*********************************************************
*	function declare												*
**********************************************************/


/**********************************************************************************
 *dcli_dldp_parse_timeout()
 *
 *	DESCRIPTION:
 *		string to unsigned long
 *
 *	INPUTS:
 *		char* str					-time, is string type
 *	
 *	OUTPUTS:
 *		unsigned int* timeout		- time, is unsigned int type

 *
 *	RETURN VALUE:
 *		DLDP_RETURN_CODE_OK	- success
 *		DLDP_RETURN_CODE_ERROR	- error
 *
 ***********************************************************************************/
int dcli_dldp_parse_timeout
(
	char* str,
	unsigned int* timeout
);

/**********************************************************************************
 *dldp_show_dldp_timer()
 *
 *	DESCRIPTION:
 *		show DLDP timer
 *
 *	INPUTS:
 *		struct vty  *vty
 *		unsigned int detection_interval			- detection interval
 *		unsigned int redetection_interval			- redetection interval
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
void dldp_show_dldp_timer
(
	struct vty  *vty,
	unsigned int detection_interval,
	unsigned int redetection_interval
);

/**********************************************************************************
 *dcli_dldp_show_running_config()
 *
 *	DESCRIPTION:
 *		show DLDP running config
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
int dcli_dldp_show_running_config
(
	struct vty* vty
);

/*********************************************************
*	extern Functions												*
**********************************************************/

#endif


