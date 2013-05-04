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
* dhcp_snp_if.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		dhcp snooping interface for NPD module.
*
* DATE:
*		04/16/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.1.1.1 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************
*	head files														*
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
	
#include "util/npd_list.h"
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_bmapi.h"
#include "npd_c_slot.h"
#include "npd_vlan.h"
#include "npd_product.h"
#include "npd_eth_port.h"
#include "npd_log.h"
#include "npd_rstp_common.h"

#include "dhcp_snp_com.h"
#include "dhcp_snp_if.h"


/*********************************************************
*	global variable define											*
**********************************************************/


/*********************************************************
*	extern variable												*
**********************************************************/


/*********************************************************
*	functions define												*
**********************************************************/

/**********************************************************************************
 *dhcp_snp_get_slot_port_from_eth_index ()
 *
 *	DESCRIPTION:
 *
 *	INPUTS:
 *		unsigned int eth_g_index
 *
 *	OUTPUTS:
 *		unsigned int *slot_no,
 *		unsigned int *port_no
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
void dhcp_snp_get_slot_port_from_eth_index
(
	unsigned int eth_g_index,
	unsigned int *slot_no,
	unsigned int *port_no
)
{
	unsigned int slot_index = NPD_DHCP_SNP_INIT_0;
	unsigned int port_index = NPD_DHCP_SNP_INIT_0;
	unsigned int slot_no_tmp = NPD_DHCP_SNP_INIT_0;
	unsigned int port_no_tmp = NPD_DHCP_SNP_INIT_0;

	slot_index = npd_get_slot_index_from_eth_index(eth_g_index);
	slot_no_tmp = npd_get_slot_no_from_index(slot_index);
	port_index = npd_get_port_index_from_eth_index(eth_g_index);
	port_no_tmp = npd_get_port_no_from_index(slot_index, port_index);
	
	*slot_no = slot_no_tmp;
	*port_no = port_no_tmp;

	return ;
}

#ifdef __cplusplus
}
#endif



