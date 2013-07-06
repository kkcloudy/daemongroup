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
* ws_pppoe_snp.h
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



#ifndef _WS_PPPOE_SNP_H
#define _WS_PPPOE_SNP_H

#include "ws_init_dbus.h"

#define IFNAMESIZE 16


/*state为"enable"或"disable"*/
extern int config_pppoe_snp_server_cmd(char *state);/*返回0表示失败，返回1表示成功*/
															/*返回-1表示bad command parameter*/
															/*返回-2表示error*/

/*state为"enable"或"disable"*/
extern int config_pppoe_snooping_enable_cmd(char *if_name,char *state);/*返回0表示失败，返回1表示成功*/
																				/*返回-1表示bad command parameter*/
																				/*返回-2表示Interface name is too long*/
																				/*返回-3表示if_name is not a ve-interface name*/
																				/*返回-4表示get local_slot_id error*/
																				/*返回-5表示error*/

#endif	

