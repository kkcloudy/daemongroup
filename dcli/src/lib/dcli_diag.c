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
* dcli_diag.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for ASIC diagnosis configuration.
*
* DATE:
*		03/24/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.23 $	
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <util/npd_if.h>

#include "sysdef/npd_sysdef.h"
#include "sysdef/portal_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"

#include "command.h"
#include "if.h"

#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include <sys/mman.h> 
#include "dcli_system.h"
#include "dcli_diag.h"
#include "dcli_main.h"   /* for dbus_connection_dcli[] */
#include "dcli_sem.h"
#include "sysdef/returncode.h"

extern DBusConnection *dcli_dbus_connection;
extern int is_distributed;

unsigned char *dcli_mac_regtype[] = {	\
/*   0 */	"command_config",
/*   1 */	"gport_config",
/*   2 */	"mac_0",
/*   3 */	"mac_1",
/*   4 */	"frm_length",
/*   5 */	"pause_quant",
/*   6 */	"sfd_offset",
/*   7 */	"mac_mode",
/*   8 */	"ag_0",
/*	9 */	"tag_1",
/* 10 */	"tx_ipg_length",
/* 11 */	"pause_control",
/* 12 */	"ipg_hd_bkp_cntl",
/* 13 */	"flush_control",
/* 14 */	"rxfifo_stat",
/* 15 */	"txfifo_stat",
/* 16 */	"gport_rsv_mask",
/* 17 */	"gport_stat_update_mask",
/* 18 */	"gport_tpid",
/* 19 */	"gport_sop_s1",
/* 20 */	"gport_sop_s0",
/* 21 */	"gport_sop_s3",
/* 22 */	"gport_sop_s4",
/* 23 */	"port_mac_crs_sel",
/* 24 */	"none"
};

DEFUN(Diagnosis_hw_phy_read_reg_cmd_func,
		Diagnosis_hw_phy_read_reg_cmd,
		"diagnosis-hardware phy read <0-1> <0-30> REGADDR",
		DIAGNOSIS_STR
		"Diagnosis hardware phy information\n"
		"Read register\n"
		"Device range <0-1> valid\n"
		"Port range <0-30> valid\n"
		"Register address\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned short opPort = 0;
	unsigned short opRegaddr = 0;
	unsigned short regValue = 0;

	/* get hwTpye*/
    hwType = 1;
	
	/* get opTpye*/
	opType = 0;

	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))){
	   opDevice= 0;
	}
	else if(0 == strncmp("1", argv[0], strlen(argv[0]))){
	   opDevice = 1;
	}
	else{
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get port no*/
    opPort = strtoul((char*)argv[1], NULL, 0);
	if (opPort > 30) {
	   vty_out(vty,"%% Bad parameter %s!\n", argv[1]);
	   return CMD_WARNING;
	}

	/* get reg address*/
    opRegaddr = strtoul((char*)argv[2], NULL, 0);

   query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_PHY_HW_RW_REG);
   dbus_error_init(&err);

   dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT16, &opPort,
							DBUS_TYPE_UINT16, &opRegaddr,
							DBUS_TYPE_UINT16, &regValue,
							DBUS_TYPE_INVALID);
   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
   
	dbus_message_unref(query);
	if (NULL == reply) {
	   vty_out(vty, "failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   vty_out(vty, "%s raised: %s", err.name, err.message);
		   dbus_error_free_for_dcli(&err);
	   }
	   return CMD_WARNING ;
	}
   
	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_UINT16, &regValue,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	vty_out(vty, "hardware diagnosis phy device read %svalid\r\n", ret ? "in":"");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device",opDevice);
	vty_out(vty, "%-10s:%-4d\r\n", "  port",opPort);
	vty_out(vty, "%-10s:%#-4x\r\n","  register",opRegaddr);
	vty_out(vty, "%-10s:%#-4x\r\n","  value",regValue);
	vty_out(vty, "--------------------------------------\r\n");
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(Diagnosis_hw_phy_write_reg_cmd_func,
		Diagnosis_hw_phy_write_reg_cmd,
		"diagnosis-hardware phy write <0-1> <0-30> REGADDR REGVALUE",
		DIAGNOSIS_STR
		"Diagnosis hardware phy information\n"
		"Read register\n"
		"Device range <0-1> valid\n"
		"Port range <0-30> valid\n"
		"Register address\n"
		"Register value\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned short opPort = 0;
	unsigned short opRegaddr = 0;
	unsigned short regValue = 0;

	/* get hwTpye*/
    hwType = 1;

	/* get opTpye*/
	opType = 1;

	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))){
	   opDevice= 0;
	}
	else if(0 == strncmp("1", argv[0], strlen(argv[0]))){
	   opDevice = 1;
	}
	else{
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get port no*/
    opPort = strtoul((char*)argv[1], NULL, 0);
	if (opPort > 30) {
	   vty_out(vty,"%% Bad parameter %s!\n", argv[1]);
	   return CMD_WARNING;
	}

	/* get reg address*/
    opRegaddr = strtoul((char*)argv[2], NULL, 0);

	/* get reg value*/
    regValue = strtoul((char*)argv[3], NULL, 0);

	vty_out(vty, "hardware diagnosis phy device write\r\n");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device",opDevice);
	vty_out(vty, "%-10s:%-4d\r\n", "  port",opPort);
	vty_out(vty, "%-10s:%#-4x\r\n","  register",opRegaddr);
	vty_out(vty, "%-10s:%#-4x\r\n","  value",regValue);
	vty_out(vty, "--------------------------------------\r\n");
		
   query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_PHY_HW_RW_REG);
   dbus_error_init(&err);

   dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT16, &opPort,
							DBUS_TYPE_UINT16, &opRegaddr,
							DBUS_TYPE_UINT16, &regValue,
							DBUS_TYPE_INVALID);
   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
   
	dbus_message_unref(query);
	if (NULL == reply) {
	   vty_out(vty, "failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   vty_out(vty, "%s raised: %s", err.name, err.message);
		   dbus_error_free_for_dcli(&err);
	   }
	   return CMD_WARNING;
	}
   
   if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT16, &regValue,
								   DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(ret) {
			vty_out(vty, "write operation failed\n");
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(Diagnosis_hw_pci_read_reg_cmd_func,
		Diagnosis_hw_pci_read_reg_cmd,
		"diagnosis-hardware pci read <0-1> REGADDR",
		DIAGNOSIS_STR
		"Diagnosis hardware pci information\n"
		"Read register\n"
		"Device range <0-1> valid\n"
		"Register address\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned int opPort = 0;
	unsigned int opRegaddr = 0;
	unsigned int regValue = 0;

	/* get hwTpye*/
	hwType = 1;

	/* get opTpye*/
	opType = 0;

	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice = 0;
	}
	else if(0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get reg address*/
	opRegaddr = strtoul((char*)argv[1], NULL, 0);

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_PCI_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opRegaddr,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING ;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_UINT32, &regValue,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	vty_out(vty, "hardware diagnosis pci device read\r\n");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device",opDevice);
	vty_out(vty, "%-10s:%#-8x\r\n","  register",opRegaddr);
	vty_out(vty, "%-10s:%#-8x\r\n","  value",regValue);
	vty_out(vty, "--------------------------------------\r\n");

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(Diagnosis_hw_pci_write_reg_cmd_func,
		Diagnosis_hw_pci_write_reg_cmd,
		"diagnosis-hardware pci write <0-1> REGADDR REGVALUE",
		DIAGNOSIS_STR
		"Diagnosis hardware pci information\n"
		"Read register\n"
		"Device range <0-1> valid\n"
		"Register address\n"
		"Register value\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned int opRegaddr = 0;
	unsigned int regValue = 0;

	/* get hwTpye*/
	hwType = 1;

	/* get opTpye*/
	opType = 1;

	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get reg address*/
	opRegaddr = strtoul((char*)argv[1], NULL, 0);

	/* get reg value*/
	regValue = strtoul((char*)argv[2], NULL, 0);

	vty_out(vty, "hardware diagnosis pci device write\r\n");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device",opDevice);
	vty_out(vty, "%-10s:%#-8x\r\n","  register",opRegaddr);
	vty_out(vty, "%-10s:%#-8x\r\n","  value",regValue);
	vty_out(vty, "--------------------------------------\r\n");
		
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_PCI_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opRegaddr,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT32, &regValue,
								   DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(diagnosis_hw_cpu_read_reg_func,
		diagnosis_hw_cpu_read_reg_cmd,
		"diagnosis-hardware cpu read REGADDR",
		DIAGNOSIS_STR
		"Diagnosis hardware CPU register information\n"
		"Read CPU register\n"
		"CPU register address\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	unsigned int opType = 0;
	unsigned int opRegaddr = 0;
	unsigned int regValue = 0;

	/* get opTpye*/
	opType = 0;

	/* get reg address*/
	opRegaddr = strtoul((char*)argv[0], NULL, 0);

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_CPU_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opRegaddr,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_UINT32, &regValue,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	vty_out(vty, "hardware diagnosis cpu register read\r\n");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%#-8x\r\n","  register",opRegaddr);
	vty_out(vty, "%-10s:%#-8x\r\n","  value",regValue);
	vty_out(vty, "%-10s:%#-8s\r\n","  status", ret ? "fail":"ok");
	vty_out(vty, "--------------------------------------\r\n");

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(diagnosis_hw_cpu_write_reg_func,
		diagnosis_hw_cpu_write_reg_cmd,
		"diagnosis-hardware cpu write REGADDR REGVALUE",
		DIAGNOSIS_STR
		"Diagnosis hardware CPU register information\n"
		"Write CPU register\n"
		"CPU register address\n"
		"CPU register value\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	unsigned int opType = 0;
	unsigned int opRegaddr = 0;
	unsigned int regValue = 0;

	/* get opTpye*/
	opType = 1;

	/* get reg address*/
	opRegaddr = strtoul((char*)argv[0], NULL, 0);

	/* get reg value*/
	regValue = strtoul((char*)argv[1], NULL, 0);
		
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_CPU_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opRegaddr,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT32, &regValue,
								   DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	vty_out(vty, "hardware diagnosis cpu register write\r\n");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%#-8x\r\n","  register",opRegaddr);
	vty_out(vty, "%-10s:%#-8x\r\n","  value",regValue);
	vty_out(vty, "%-10s:%#-8s\r\n","  status", ret ? "fail":"ok");
	vty_out(vty, "--------------------------------------\r\n");
	return CMD_SUCCESS;  
}

DEFUN(Diagnosis_hw_cpld_read_reg_cmd_func,
		Diagnosis_hw_cpld_read_reg_cmd,
		"diagnosis-hardware cpld read <0-1> REGADDR",
		DIAGNOSIS_STR
		"Diagnosis hardware cpld information\n"
		"Read register\n"
		"Device range <0-1> valid\n"
		"Register address\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned int opRegaddr = 0;
	unsigned int regValue = 0;

	/* get hwTpye*/
	hwType = 1;

	/* get opTpye*/
	opType = 0;

	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice = 0;
	}
	else if(0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get reg address*/
	opRegaddr = strtoul((char*)argv[1], NULL, 0);

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_CPLD_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opRegaddr,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_UINT32, &regValue,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (CPLD_RETURN_CODE_OPEN_FAIL == ret) {
			vty_out(vty, "open fd fail !\r\n");
		}
		else if (CPLD_RETURN_CODE_IOCTL_FAIL == ret) {
			vty_out(vty, "ioctl fd fail !\r\n");
		}
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	vty_out(vty, "hardware diagnosis cpld device read\r\n");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device",opDevice);
	vty_out(vty, "%-10s:%#-8x\r\n","  register",opRegaddr);
	vty_out(vty, "%-10s:%#-8x\r\n","  value",regValue);
	vty_out(vty, "--------------------------------------\r\n");

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(Diagnosis_hw_cpld_write_reg_cmd_func,
		Diagnosis_hw_cpld_write_reg_cmd,
		"diagnosis-hardware cpld write <0-1> REGADDR REGVALUE",
		DIAGNOSIS_STR
		"Diagnosis hardware cpld information\n"
		"Read register\n"
		"Device range <0-1> valid\n"
		"Register address\n"
		"Register value\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned int opRegaddr = 0;
	unsigned int regValue = 0;

	/* get hwTpye*/
	hwType = 1;

	/* get opTpye*/
	opType = 1;

	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get reg address*/
	opRegaddr = strtoul((char*)argv[1], NULL, 0);

	/* get reg value*/
	regValue = strtoul((char*)argv[2], NULL, 0);

	vty_out(vty, "hardware diagnosis cpld device write\r\n");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device",opDevice);
	vty_out(vty, "%-10s:%#-8x\r\n","  register",opRegaddr);
	vty_out(vty, "%-10s:%#-8x\r\n","  value",regValue);
	vty_out(vty, "--------------------------------------\r\n");
		
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_CPLD_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opRegaddr,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT32, &regValue,
								   DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}
/*|mac_0|mac_1|frm_length|pause_quant| \
//		sfd_offset|mac_mode|tag_0|tag_1|tx_ipg_length|pause_control|ipg_hd_bkp_cntl|flush_control|rxfifo_stat|txfifo_stat|  \
//		gport_rsv_mask|gport_stat_update_mask|gport_tpid|gport_sop_s1|gport_sop_s0|gport_sop_s3|gport_sop_s4|gport_mac_crs_sel)
// REGTYPE*/
DEFUN(Diagnosis_hw_mac_read_reg_cmd_func,
		Diagnosis_hw_mac_read_reg_cmd,
		"diagnosis-hardware mac read <0-1> <0-30> (command_config|gport_config|mac_0|mac_1|frm_length|pause_quant| \
		sfd_offset|mac_mode|tag_0|tag_1|tx_ipg_length|pause_control|ipg_hd_bkp_cntl|flush_control|rxfifo_stat|txfifo_stat| \
		gport_rsv_mask|gport_stat_update_mask|gport_tpid|gport_sop_s1|gport_sop_s0|gport_sop_s3|gport_sop_s4|gport_mac_crs_sel)",
		DIAGNOSIS_STR
		"Diagnosis hardware strncmpistrncmpimac information\n"
		"Read register\n"
		"Device range <0-1> valid\n"
		"Port range <0-30> valid\n"
		"Used by the host processor to control and configure the core\n"
		"GPORT configuration Register\n"
		"MAC Address 32-Bit Word 0\n"
		"MAC Address 32-Bit Word 1\n"
		"Maximum Frame Length\n"
		"Receive Pause Quanta\n"
		"EFM Preamble Length\n"
		"MAC Mode\n"
		"Programmable vlan outer tag\n"
		"Programmable vlan inner tag\n"
		"Programmable Inter-Packet-Gap\n"
		"PAUSE frame timer control register\n"
		"The control register for HD-BackPressure\n"
		"Flush enable control register\n"
		"RXFIFO status register\n"
		"TXFIFO status register\n"
		"GPORT RSV MASK register\n"
		"GPORT STAT_UPDATE MASK register\n"
		"GPORT VLAN Tag Protocol ID\n"
		"GPORT K.SOP used in stacking port FPORT-S1\n"
		"GPORT K.SOP used in stacking port FPORT-S0\n"
		"GPORT K.SOP used in stacking port FPORT-S3\n"
		"GPORT K.SOP used in stacking port FPORT-S4\n"
		"Register for MAC_CRS_SEL signals\n"	
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned int opPort = 0;
	unsigned int opRegtype = 0;
	unsigned int regValue = 0;
	int i = 0;

	/* get hwTpye*/
	hwType = 1;

	/* get opTpye*/
	opType = 0;

	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice = 0;
	}
	else if(0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get reg port*/
	opPort = strtoul((char*)argv[1], NULL, 0);

	for (i=0; i < DCLI_MAC_REGTYPE_NUM; i++) {
		if(0 == strncasecmp(dcli_mac_regtype[i], argv[2], strlen(argv[2]))) {
			opRegtype = i;
			break;
		}
	}
	if (DCLI_MAC_REGTYPE_NUM == i) {
		vty_out(vty,"%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}
	/*
	if(0 == strncmp("command_config", argv[2], strlen(argv[2]))) {
		opRegtype = 0;
	}
	else if(0 == strncmp("gport_config", argv[2], strlen(argv[2]))) {
		opRegtype = 1;
	}
	else if(0 == strncmp("mac_0", argv[2], strlen(argv[2]))) {
		opRegtype = 2;
	}
	else if(0 == strncmp("mac_1", argv[2], strlen(argv[2]))) {
		opRegtype = 3;
	}
	else if(0 == strncmp("frm_length", argv[2], strlen(argv[2]))) {
		opRegtype = 4;
	}
	else if(0 == strncmp("pause_quant", argv[2], strlen(argv[2]))) {
		opRegtype = 5;
	}
	else if(0 == strncmp("sfd_offset", argv[2], strlen(argv[2]))) {
		opRegtype = 6;
	}else if(0 == strncmp("mac_mode", argv[2], strlen(argv[2]))) {
		opRegtype = 7;
	}
	else if(0 == strncmp("tag_0", argv[2], strlen(argv[2]))) {
		opRegtype = 8;
	}
	else if(0 == strncmp("tag_1", argv[2], strlen(argv[2]))) {
		opRegtype = 9;
	}
	else if(0 == strncmp("tx_ipg_length", argv[2], strlen(argv[2]))) {
		opRegtype = 10;
	}
	else if(0 == strncmp("pause_control", argv[2], strlen(argv[2]))) {
		opRegtype = 11;
	}
	else if(0 == strncmp("ipg_hd_bkp_cntl", argv[2], strlen(argv[2]))) {
		opRegtype = 12;
	}
	else if(0 == strncmp("flush_control", argv[2], strlen(argv[2]))) {
		opRegtype = 13;
	}
	else if(0 == strncmp("rxfifo_stat", argv[2], strlen(argv[2]))) {
		opRegtype = 14;
	}
	else if(0 == strncmp("txfifo_stat", argv[2], strlen(argv[2]))) {
		opRegtype = 15;
	}
	else if(0 == strncmp("gport_rsv_mask", argv[2], strlen(argv[2]))) {
		opRegtype = 16;
	}
	else if(0 == strncmp("gport_stat_update_mask", argv[2], strlen(argv[2]))) {
		opRegtype = 17;
	}
	else if(0 == strncmp("gport_tpid", argv[2], strlen(argv[2]))) {
		opRegtype = 18;
	}
	else if(0 == strncmp("gport_sop_s1", argv[2], strlen(argv[2]))) {
		opRegtype = 19;
	}
	else if(0 == strncmp("gport_sop_s0", argv[2], strlen(argv[2]))) {
		opRegtype = 20;
	}else if(0 == strncmp("gport_sop_s3", argv[2], strlen(argv[2]))) {
		opRegtype = 21;
	}else if(0 == strncmp("gport_sop_s4", argv[2], strlen(argv[2]))) {
		opRegtype = 22;
	}
	else if(0 == strncmp("gport_mac_crs_sel", argv[2], strlen(argv[2]))) {
		opRegtype = 23;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}
	*/

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_MAC_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opPort,
							DBUS_TYPE_UINT32, &opRegtype,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_UINT32, &regValue,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	vty_out(vty, "hardware diagnosis mac read %s\r\n", dcli_soc_reg[opRegtype]);
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device",opDevice);
	vty_out(vty, "%-10s:%#-4d\r\n","  port",opPort);
	vty_out(vty, "%-10s:%#-8x\r\n","  value",regValue);
	vty_out(vty, "--------------------------------------\r\n");

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(Diagnosis_hw_mac_write_reg_cmd_func,
		Diagnosis_hw_mac_write_reg_cmd,
		"diagnosis-hardware mac write <0-1> <0-30> (command_config|gport_config|mac_0|mac_1|frm_length|pause_quant| \
		sfd_offset|mac_mode|tag_0|tag_1|tx_ipg_length|pause_control|ipg_hd_bkp_cntl|flush_control|rxfifo_stat|txfifo_stat| \
		gport_rsv_mask|gport_stat_update_mask|gport_tpid|gport_sop_s1|gport_sop_s0|gport_sop_s3|gport_sop_s4|gport_mac_crs_sel) REGVALUE",
		DIAGNOSIS_STR
		"Diagnosis hardware mac information\n"
		"Write register\n"
		"Device range <0-1> valid\n"
		"Port range <0-30> valid\n"
		"Used by the host processor to control and configure the core\n"
		"GPORT configuration Register\n"
		"MAC Address 32-Bit Word 0\n"
		"MAC Address 32-Bit Word 1\n"
		"Maximum Frame Length\n"
		"Receive Pause Quanta\n"
		"EFM Preamble Length\n"
		"MAC Mode\n"
		"Programmable vlan outer tag\n"
		"Programmable vlan inner tag\n"
		"Programmable Inter-Packet-Gap\n"
		"PAUSE frame timer control register\n"
		"The control register for HD-BackPressure\n"
		"Flush enable control register\n"
		"RXFIFO status register\n"
		"TXFIFO status register\n"
		"GPORT RSV MASK register\n"
		"GPORT STAT_UPDATE MASK register\n"
		"GPORT VLAN Tag Protocol ID\n"
		"GPORT K.SOP used in stacking port FPORT-S1\n"
		"GPORT K.SOP used in stacking port FPORT-S0\n"
		"GPORT K.SOP used in stacking port FPORT-S3\n"
		"GPORT K.SOP used in stacking port FPORT-S4\n"
		"Register for MAC_CRS_SEL signals\n"	
		"Register value\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned int opPort = 0;
	unsigned int opRegtype = 0;
	unsigned int regValue = 0;
	int i = 0;
	
	/* get hwTpye*/
	hwType = 1;

	/* get opTpye*/
	opType = 1;

	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get reg port*/
	opPort = strtoul((char*)argv[1], NULL, 0);

	/* get reg value*/
	for (i=0; i < DCLI_MAC_REGTYPE_NUM; i++) {
		if(0 == strncmp(dcli_mac_regtype[i], argv[2], strlen(argv[2]))) {
			opRegtype = i;
			break;
		}
	}
	if (DCLI_MAC_REGTYPE_NUM == i) {
		vty_out(vty,"%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}
	
	regValue = strtoul((char*)argv[3], NULL, 0);

	vty_out(vty, "hardware diagnosis mac write %s\r\n", dcli_mac_regtype[opRegtype]);
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device",opDevice);
	vty_out(vty, "%-10s:%#-4d\r\n","  port",opPort);
	vty_out(vty, "%-10s:%#-8x\r\n","  value",regValue);
	vty_out(vty, "--------------------------------------\r\n");
		
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_MAC_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opPort,
							DBUS_TYPE_UINT32, &opRegtype,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT32, &regValue,
								   DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}
/*
DEFUN(Diagnosis_hw_read_reg_cmd_func,
		Diagnosis_hw_read_reg_cmd,
		"diagnosis-hardware read <0-1> (any|<0-30>) REGNAME",
		DIAGNOSIS_STR
		"Diagnosis hardware information read register\n"
		"Device range <0-1> valid\n"
		"Port range any\n"
		"Port range <0-30> valid\n"
		"Register name\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned int opPort = 0;
	unsigned int opRegtype = 0;
	unsigned int regValue = 0;
	int i = 0;

	// get hwTpye
	hwType = 1;

	// get opTpye
	opType = 0;

	// get device
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice = 0;
	}
	else if(0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	// get reg port
	if(strncmp("any",argv[1],strlen(argv[1])) == 0) {
		opPort = 65535;
	}
	 else {
	 	dcli_str2ulong((char *)argv[1],&opPort);
	}

	for (i=0; i < DCLI_REGTYPE_NUM; i++) {
		if((0 == strcasecmp(dcli_soc_reg[i], argv[2]))&&((i < 166)||(i > 521))) {
			opRegtype = i;
			break;
		}
	}
	if (DCLI_REGTYPE_NUM == i) {
		vty_out(vty,"%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opPort,
							DBUS_TYPE_UINT32, &opRegtype,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING ;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_UINT32, &regValue,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		vty_out(vty, "Register read fail !\n");
	}
	else {
		vty_out(vty, "hardware diagnosis read %s\r\n", dcli_soc_reg[opRegtype]);
		vty_out(vty, "--------------------------------------\r\n");
		vty_out(vty, "%-10s:%-4d\r\n", "  device",opDevice);
		if (65535 == opPort) {
			vty_out(vty, "%-10s:%s\r\n","  port","any");
		}
		else {
			vty_out(vty, "%-10s:%#-4d\r\n","  port",opPort);
		}
		vty_out(vty, "%-10s:%#-8x\r\n","  value",regValue);
		vty_out(vty, "--------------------------------------\r\n");
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}
*/
	
DEFUN(Diagnosis_hw_read_reg_cmd_func,
		Diagnosis_hw_read_reg_cmd,
		"diagnosis-hardware asic read <0-1> <0-3> REGADDR",
		DIAGNOSIS_STR
		"Diagnosis hardware asic register read\n"
		"Diagnosis hardware asic register read\n"
		"Device range <0-1> valid\n"
		"GroupId range <0-3> valid\n"
		"Register address\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned int opGroupID = 0;
	unsigned int opPort = 0;
	unsigned int regaddr = 0;
	unsigned int regValue = 0;
	int i = 0;

	/* get hwTpye*/
	hwType = 1;

	/* get opTpye*/
	opType = 0;

	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice = 0;
	}
	else if(0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get port group id */
	if(0 == strncmp("0", argv[1], strlen(argv[1]))) {
		opGroupID = 0;
	}
	else if(0 == strncmp("1", argv[1], strlen(argv[1]))) {
		opGroupID = 1;
	}
	else if(0 == strncmp("2", argv[1], strlen(argv[1]))) {
		opGroupID = 2;
	}
	else if(0 == strncmp("3", argv[1], strlen(argv[1]))) {
		opGroupID = 3;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[1]);
		return CMD_WARNING;
	}


	regaddr = strtoul((char*)argv[2], NULL, 0);

	
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opGroupID,
							DBUS_TYPE_UINT32, &opPort,
							DBUS_TYPE_UINT32, &regaddr,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_UINT32, &regValue,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		vty_out(vty, "Register read fail !\n");
	}
	else {
		vty_out(vty, "---------------read regsiter------------\r\n");
		vty_out(vty, "%-10s:%-4d\r\n", "  device", opDevice);
		vty_out(vty, "%-10s:%-4d\r\n", "  GroupID", opGroupID);
		vty_out(vty, "%-10s:%#-8x\r\n", "  address", regaddr);
		vty_out(vty, "%-10s:%#-8x\r\n","  value", regValue);
		vty_out(vty, "--------------------------------------\r\n");
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

/*
DEFUN(Diagnosis_hw_write_reg_cmd_func,
		Diagnosis_hw_write_reg_cmd,
		"diagnosis-hardware write <0-1> (any|<0-30>) REGNAME REGVALUE",
		DIAGNOSIS_STR
		"Diagnosis hardware information write register\n"
		"Device range <0-1> valid\n"
		"Port range any\n"
		"Port range <0-30> valid\n"
		"Register name\n"
		"Register value\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned int opPort = 0;
	unsigned int opRegtype = 0;
	unsigned int regValue = 0;
	int i = 0;
	
	// get hwTpye
	hwType = 1;

	// get opTpye
	opType = 1;

	// get device
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	// get reg port
	if(strncmp("any",argv[1],strlen(argv[1])) == 0) {
		opPort = 65535;
	}
	 else {
	 	dcli_str2ulong((char *)argv[1],&opPort);
	}

	// get reg value
	for (i=0; i < DCLI_REGTYPE_NUM; i++) {
		if((0 == strcasecmp(dcli_soc_reg[i], argv[2]))&&((i < 166)||(i > 521))) {
			opRegtype = i;
			break;
		}
	}
	if (DCLI_REGTYPE_NUM == i) {
		vty_out(vty,"%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}
	
	regValue = strtoul((char*)argv[3], NULL, 0);

	vty_out(vty, "hardware diagnosis write %s\r\n", dcli_soc_reg[opRegtype]);
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device",opDevice);
	if (65535 == opPort) {
		vty_out(vty, "%-10s:%s\r\n","  port","any");
	}
	else {
		vty_out(vty, "%-10s:%#-4d\r\n","  port",opPort);
	}
	vty_out(vty, "%-10s:%#-8x\r\n","  value",regValue);
	vty_out(vty, "--------------------------------------\r\n");
		
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opPort,
							DBUS_TYPE_UINT32, &opRegtype,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT32, &regValue,
								   DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		vty_out(vty, "Register write fail !\n");
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}*/

DEFUN(Diagnosis_hw_write_reg_cmd_func,
		Diagnosis_hw_write_reg_cmd,
		"diagnosis-hardware asic write <0-1> <0-3> REGADDR REGVALUE",
		DIAGNOSIS_STR
		"Diagnosis hardware asic register write\n"	
		"Diagnosis hardware asic register write\n"
		"Device range <0-1> valid\n"
		"GroupId range <0-3> valid\n"
		"Register address\n"
		"Register value\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int hwType = 0;
	unsigned int opType = 0;
	unsigned int opDevice = 0;
	unsigned int opGroupID = 0;
	unsigned int opPort = 0;
	unsigned int regaddr = 0;
	unsigned int regValue = 0;
	int i = 0;
	
	/* get hwTpye*/
	hwType = 1;

	/* get opTpye*/
	opType = 1;

	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get port group id */
	if(0 == strncmp("0", argv[1], strlen(argv[1]))) {
		opGroupID = 0;
	}
	else if(0 == strncmp("1", argv[1], strlen(argv[1]))) {
		opGroupID = 1;
	}
	else if(0 == strncmp("2", argv[1], strlen(argv[1]))) {
		opGroupID = 2;
	}
	else if(0 == strncmp("3", argv[1], strlen(argv[1]))) {
		opGroupID = 3;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[1]);
		return CMD_WARNING;
	}


	regaddr = strtoul((char*)argv[2], NULL, 0);

	regValue = strtoul((char*)argv[3], NULL, 0);

	vty_out(vty, "--------------write register-------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device",opDevice);
	vty_out(vty, "%-10s:%-4d\r\n", "  GroupID", opGroupID);
	vty_out(vty, "%-10s:%#-8x\r\n", "  address", regaddr);
	vty_out(vty, "%-10s:%#-8x\r\n","  value",regValue);
	vty_out(vty, "--------------------------------------\r\n");
		
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &hwType,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opGroupID,
							DBUS_TYPE_UINT32, &opPort,
							DBUS_TYPE_UINT32, &regaddr,
							DBUS_TYPE_UINT32, &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT32, &regValue,
								   DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		vty_out(vty, "Register write fail !\n");
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(Diagnosis_hw_asic_cscd_port_prbs_test_cmd_func,
		Diagnosis_hw_asic_cscd_port_prbs_test_cmd,
		"diagnosis-hardware asic cscd_port prbs <0-1>",
		DIAGNOSIS_STR
		"Diagnosis hardware asic cscd_port prbs test\n"	
		"Diagnosis hardware asic cscd_port prbs test\n"	
        "Diagnosis hardware asic cscd_port prbs test\n"
        "Device range <0-1> valid\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int gpnum = 0, gpmemnum = 0;
	unsigned int ret = 0;
	unsigned int opDevice = 0;
	unsigned int opPort = 0;
	unsigned int result = 0;
	unsigned int Gpmem[4] = {0, 0, 0, 0};   /*group member exist(bitmap),0:inexistence,1:inexistence*/
	unsigned int Gpres[4] = {0, 0, 0, 0};  /*group member result(bitmap),0:prbs failed,1:prbs succeed*/

	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}
	vty_out(vty,"Please wait 10sec.\n");
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_HW_CSCD_PORT_PRBS_TEST);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT32, &Gpmem[0],
								   DBUS_TYPE_UINT32, &Gpmem[1],
								   DBUS_TYPE_UINT32, &Gpmem[2],
								   DBUS_TYPE_UINT32, &Gpmem[3],
								   DBUS_TYPE_UINT32, &Gpres[0],
								   DBUS_TYPE_UINT32, &Gpres[1],
								   DBUS_TYPE_UINT32, &Gpres[2],
								   DBUS_TYPE_UINT32, &Gpres[3],
								   DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		vty_out(vty, "asic cscd_port prbs test fail !\n",opPort);
	}
    else{
		for(gpnum = 0; gpnum < 4; gpnum++)
		{
			for(gpmemnum = 0; gpmemnum < 16; gpmemnum++)
			{
				if((Gpmem[gpnum] & (1 << gpmemnum)) != 0)
				{
					opPort = gpnum * 16 + gpmemnum;
					result = Gpres[gpnum] & (1 << gpmemnum);
		            vty_out(vty, "device %d port %d status: %s!\r\n", opDevice, opPort,(result == 0)?"OK":"Error");
				}
			}
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(Diagnosis_hw_asic_prbs_test_cmd_func,
		Diagnosis_hw_asic_prbs_test_cmd,
		"diagnosis-hardware asic-prbs <0-1> <0-59>",
		DIAGNOSIS_STR
		"Diagnosis hardware asic-prbs test\n"	
		"Diagnosis hardware asic-prbs test\n"
		"Device range <0-1> valid\n"
		"Port range <0-59> valid\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int opDevice = 0;
	unsigned int opPort = 0;
	unsigned int result = 0;
	
	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}
	
	/* get port*/
	opPort = strtoul((char*)argv[1], NULL, 0);

	vty_out(vty,"Please wait 10sec.\n");
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_HW_PRBS_TEST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opPort,
							DBUS_TYPE_UINT32, &result,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT32, &result,
								   DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		vty_out(vty, "port %d prbs test fail !\n",opPort);
	}
    else{
		vty_out(vty, "port %d status: %s!\r\n", opPort,(result == 0)?"OK":"Error");
    }
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}
DEFUN(Diagnosis_hw_asic_port_mode_set_cmd_func,
    	Diagnosis_hw_asic_port_mode_set_cmd,
    	"set asic-port (xaui|rxaui|dxaui) <0-1> <0-59>",
    	DIAGNOSIS_STR
    	"Set asic port mode\n"
    	"Set asic port mode xaui\n"
    	"Set asic port mode rxaui\n"
    	"Set asic port mode dxaui\n"
		"Device range <0-1> valid\n"
		"Port range <0-59> valid\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int portMode = 0;	
	unsigned int opDevice = 0;
	unsigned int opPort = 0;

	/* get port mode */
	if (0 == strncmp("xaui", argv[0], strlen(argv[0]))) {
	   portMode = 0;
	}
	else if (0 == strncmp("rxaui", argv[0], strlen(argv[0]))) {
	   portMode = 1;
	}
	else if (0 == strncmp("dxaui", argv[0], strlen(argv[0]))) {
	   portMode = 2;
	}
	else{
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get device*/
	if (0 == strncmp("0", argv[1], strlen(argv[1]))) {
	   opDevice= 0;
	}
	else if (0 == strncmp("1", argv[1], strlen(argv[1]))) {
	   opDevice = 1;
	}
	else{
		vty_out(vty, "%% Bad parameter %s!\n", argv[1]);
		return CMD_WARNING;
	}

	/* get port no*/
    opPort = strtoul((char*)argv[2], NULL, 0);
	if (opPort > 59) {
	   vty_out(vty,"%% Bad parameter %s!\n", argv[2]);
	   return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAGNOSIS_HW_PORT_MODE_SET);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opPort,
                            DBUS_TYPE_UINT32, &portMode,
							DBUS_TYPE_INVALID);
   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
	   vty_out(vty, "failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   vty_out(vty, "%s raised: %s", err.name, err.message);
		   dbus_error_free_for_dcli(&err);
	   }
	   return CMD_WARNING ;
	}
   
	if (!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	if(ret == 0)
	{
	    vty_out(vty, "set dev %d port %d mode %s OK!\n",opDevice,opPort,(portMode == 0)?"xaui":((portMode == 1)?"rxaui":"rdxaui"));
	}
	else
	{
	    vty_out(vty, "set dev %d port %d mode %s Failed!\n",opDevice,opPort,(portMode == 0)?"xaui":((portMode == 1)?"rxaui":"rdxaui"));	
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}
DEFUN(Diagnosis_hw_dump_tab_cmd_func,
		Diagnosis_hw_dump_tab_cmd,
		"diagnosis-hardware dump any <0-1> (any|<0-3>) TABINDEX TABNAME",
		DIAGNOSIS_STR
		"Diagnosis hardware dump table\n"
		"Dump any register table\n"
		"Device range <0-1> valid\n"
		"Block range any\n"
		"Block range <0-3> valid\n"
		"Table index\n"
		"Table name\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int opDevice = 0;
	unsigned int opBlock = 0;
	unsigned int opRegtype = 0;
	unsigned int tabIndex = 0;
	int i = 0;
	unsigned int regdate = 0;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;

	/* get device*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice = 0;
	}
	else if(0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get tab block*/
	if(strncmp("any",argv[1],strlen(argv[1])) == 0) {
		opBlock = 65535;
	}
	 else {
	 	dcli_str2ulong((char *)argv[1],&opBlock);
	}
	 
	/* get tab index*/
	dcli_str2ulong((char *)argv[2],&tabIndex);

	for (i=0; i < DCLI_TABTYPE_NUM; i++) {
		if (0 == strcasecmp(dcli_soc_mem[i], argv[3])) {
			opRegtype = i;
			break;
		}
	}
	if (DCLI_TABTYPE_NUM == i) {
		vty_out(vty,"%% Bad parameter %s!\n", argv[3]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_HW_DUMP_TAB);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &opBlock,
							DBUS_TYPE_UINT32, &tabIndex,
							DBUS_TYPE_UINT32, &opRegtype,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	vty_out(vty, "hardware diagnosis dump %s\r\n", dcli_soc_mem[opRegtype]);
	
	 dbus_message_iter_init(reply,&iter);
	 dbus_message_iter_get_basic(&iter,&ret);
	 
	 dbus_message_iter_next(&iter); 
	 dbus_message_iter_recurse(&iter,&iter_array);
	 
	for (i = 0; i < 18; i++) {
		DBusMessageIter iter_struct;
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&regdate);

		vty_out(vty,"%08x  ",regdate);
		if (0 == (i+1)%6) { vty_out(vty,"\n"); }

		dbus_message_iter_next(&iter_array);
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(Diagnosis_hw_field_dump_cmd_func,
		Diagnosis_hw_field_dump_cmd,
		"diagnosis-hardware dump field <0-1> <1-1024>",
		DIAGNOSIS_STR
		"Diagnosis hardware dump field table\n"
		"Register type\n"
		"Device range <0-1> valid\n"
		"Group range <1-1024> valid\n"
) 
{
	unsigned int opDevice = 0, gnum = 0;
	char *showStr = NULL,*cursor = NULL,ch = 0;
	DBusMessage *query, *reply;
	DBusError err;
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get group num*/
	gnum = strtoul((char*)argv[1], NULL, 0);
	
	query = dbus_message_new_method_call(  
										NPD_DBUS_BUSNAME,		\
										NPD_DBUS_OBJPATH,			\
										NPD_DBUS_INTERFACE, 		\
										NPD_DBUS_SYSTEM_DIAGNOSIS_FIELD_HW_DUMP_TAB);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &opDevice,
						DBUS_TYPE_UINT32, &gnum,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
		/*vtysh_add_show_string(showStr);*/
		vty_out(vty, "%s ", showStr);
	} 
	else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS; 
}

DEFUN(Diagnosis_hw_cosq_dump_cmd_func,
		Diagnosis_hw_cosq_dump_cmd,
		"diagnosis-hardware dump cosq <0-1> <1-29>",
		DIAGNOSIS_STR
		"Diagnosis hardware dump cosq tables\n"
		"Register type\n"
		"Device range <0-1> valid\n"
		"Port range <1-29> valid\n"
) 
{
	unsigned int opDevice = 0, portnum = 0;
	char *showStr = NULL,*cursor = NULL,ch = 0;
	DBusMessage *query, *reply;
	DBusError err;
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get port num*/
	portnum = strtoul((char*)argv[1], NULL, 0);
	
	query = dbus_message_new_method_call(  
										NPD_DBUS_BUSNAME,		\
										NPD_DBUS_OBJPATH,			\
										NPD_DBUS_INTERFACE, 		\
										NPD_DBUS_SYSTEM_DIAGNOSIS_COSQ_HW_DUMP_TAB);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &opDevice,
						DBUS_TYPE_UINT32, &portnum,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
		/*vtysh_add_show_string(showStr);*/
		vty_out(vty, "%s ", showStr);
	} 
	else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS; 
}

DEFUN(Diagnosis_hw_acl_show_cmd_func,
		Diagnosis_hw_acl_show_cmd,
		"diagnosis-hardware acl show <0-1> <1-1024>",
		DIAGNOSIS_STR
		"Diagnosis hardware acl information\n"
		"Show acl rule\n"
		"Rule size range <0-1> valid\n"
		"Acl range <1-1024> valid\n"
) 
{
	unsigned int rulesize = 0, aclindex = 0;
	char *showStr = NULL,*cursor = NULL,ch = 0;
	DBusMessage *query, *reply;
	DBusError err;
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		rulesize= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		rulesize = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get acl index*/
	aclindex = strtoul((char*)argv[1], NULL, 0);

	aclindex -= 1;
	query = dbus_message_new_method_call(  
										NPD_DBUS_BUSNAME,		\
										NPD_DBUS_OBJPATH,			\
										NPD_DBUS_INTERFACE, 		\
										NPD_DBUS_SYSTEM_DIAGNOSIS_ACL_HW_SHOW);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &rulesize,
						DBUS_TYPE_UINT32, &aclindex,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		/*vtysh_add_show_string(showStr);*/
		vty_out(vty, "%s ", showStr);
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS; 
}

DEFUN(diagnosis_hw_xaui_read_reg_cmd_func,
	diagnosis_hw_xaui_read_reg_cmd,
	"diagnosis-hardware xaui-phy read <0-1> <0-30> (external|internal) <0-31> <0-31> REGADDR",
	DIAGNOSIS_STR
	"Diagnosis hardware XAUI PHY information\n"
	"Read register\n"
	"Asic device number, device range <0-1> valid\n"
	"Asic port number, port range <0-30> valid\n"
	"External 10G PHY\n"
	"Internal 10G PHY\n"
	"Phy address used by xsmi interface which valid in external phy mode\n"
	"PHY device to read from (value of 0..31)\n"
	"Register address to read\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int opType = 0;		/* read */
	unsigned int opDevice = 0;
	unsigned short opPort = 0;
	unsigned char phyID = 0;
	unsigned char isExtPhy = 0;
	unsigned short opRegaddr = 0;
	unsigned char phyDve = 0;
	unsigned short regValue = 0;

	/* get device*/
	if (0 == strncmp("0", argv[0], strlen(argv[0]))) {
	   opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
	   opDevice = 1;
	}
	else{
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get port no*/
    opPort = strtoul((char*)argv[1], NULL, 0);
	if (opPort > 30) {
	   vty_out(vty,"%% Bad parameter %s!\n", argv[1]);
	   return CMD_WARNING;
	}

	/* get type of 10G PHY */
	if (0 == strncmp("external", argv[2], strlen(argv[2]))) {
	   isExtPhy = 1;
	}
	else if (0 == strncmp("internal", argv[2], strlen(argv[2]))) {
	   isExtPhy = 0;
	}
	else{
		vty_out(vty, "%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}

	/* get PHY ID*/
    phyID = strtoul((char*)argv[3], NULL, 0);
	if (phyID > 31) {
	   vty_out(vty, "%% Bad parameter %s!\n", argv[3]);
	   return CMD_WARNING;
	}

	/* get phy device */
	phyDve = strtoul((char*)argv[4], NULL, 0);
	if (phyDve > 31) {
	   vty_out(vty, "%% Bad parameter %s!\n", argv[4]);
	   return CMD_WARNING;
	}

	/* get reg address */
	opRegaddr = strtoul((char*)argv[5], NULL, 0);

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAGNOSIS_XAUI_PHY_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT16, &opPort,
							DBUS_TYPE_BYTE,  &phyID,
							DBUS_TYPE_BYTE,  &isExtPhy,
							DBUS_TYPE_UINT16, &opRegaddr,
							DBUS_TYPE_BYTE,  &phyDve,
							DBUS_TYPE_UINT16, &regValue,
							DBUS_TYPE_INVALID);
   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
	   vty_out(vty, "failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   vty_out(vty, "%s raised: %s", err.name, err.message);
		   dbus_error_free_for_dcli(&err);
	   }
	   return CMD_WARNING ;
	}
   
	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_UINT16, &regValue,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	vty_out(vty, "hardware diagnosis xaui-phy device read %s valid\n", ret ? "in":"");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device", opDevice);
	vty_out(vty, "%-10s:%-4d\r\n", "  port", opPort);
	vty_out(vty, "%-10s:%-8s\r\n", "  isExtPhy", isExtPhy ? "external" : "internal");
	vty_out(vty, "%-10s:%-4d\r\n", "  phy ID", phyID);
	vty_out(vty, "%-10s:%#-4x\r\n","  phy device", phyDve);
	vty_out(vty, "%-10s:%#-4x\r\n","  register", opRegaddr);
	vty_out(vty, "%-10s:%#-4x\r\n","  value", regValue);
	vty_out(vty, "--------------------------------------\r\n");

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(diagnosis_hw_xaui_write_reg_cmd_func,
	diagnosis_hw_xaui_write_reg_cmd,
	"diagnosis-hardware xaui-phy write <0-1> <0-30> (external|internal) <0-31> <0-31> REGADDR REGVALUE",
	DIAGNOSIS_STR
	"Diagnosis hardware XAUI PHY information\n"
	"Write register\n"
	"Asic device number, device range <0-1> valid\n"
	"Asic port number, port range <0-30> valid\n"
	"External 10G PHY\n"
	"Internal 10G PHY\n"
	"Phy address used by xsmi interface which valid in external phy mode\n"
	"PHY device to read from (value of 0..31)\n"
	"Register address to write\n"
	"Register value\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int opType = 1;		/* write */
	unsigned int opDevice = 0;
	unsigned short opPort = 0;
	unsigned char phyID = 0;
	unsigned char isExtPhy = 0;
	unsigned short opRegaddr = 0;
	unsigned char phyDve = 0;
	unsigned short regValue = 0;

	/* get device*/
	if (0 == strncmp("0", argv[0], strlen(argv[0]))) {
	   opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
	   opDevice = 1;
	}
	else{
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get port no*/
    opPort = strtoul((char*)argv[1], NULL, 0);
	if (opPort > 30) {
	   vty_out(vty,"%% Bad parameter %s!\n", argv[1]);
	   return CMD_WARNING;
	}

	/* get type of 10G PHY */
	if (0 == strncmp("external", argv[2], strlen(argv[2]))) {
	   isExtPhy = 1;
	}
	else if (0 == strncmp("internal", argv[2], strlen(argv[2]))) {
	   isExtPhy = 0;
	}
	else{
		vty_out(vty, "%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}

	/* get PHY ID*/
    phyID = strtoul((char*)argv[3], NULL, 0);
	if (phyID > 31) {
	   vty_out(vty, "%% Bad parameter %s!\n", argv[3]);
	   return CMD_WARNING;
	}

	/* get phy device */
	phyDve = strtoul((char*)argv[4], NULL, 0);
	if (phyDve > 31) {
	   vty_out(vty, "%% Bad parameter %s!\n", argv[4]);
	   return CMD_WARNING;
	}

	/* get reg address */
	opRegaddr = strtoul((char*)argv[5], NULL, 0);

	/* get reg value*/
    regValue = strtoul((char*)argv[6], NULL, 0);

	vty_out(vty, "hardware diagnosis xaui-phy device write\n");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-10s:%-4d\r\n", "  device", opDevice);
	vty_out(vty, "%-10s:%-4d\r\n", "  port", opPort);
	vty_out(vty, "%-10s:%-8s\r\n", "  isExtPhy", isExtPhy ? "external" : "internal");
	vty_out(vty, "%-10s:%-4d\r\n", "  phy ID", phyID);
	vty_out(vty, "%-10s:%#-4x\r\n","  phy device", phyDve);
	vty_out(vty, "%-10s:%#-4x\r\n","  register", opRegaddr);
	vty_out(vty, "%-10s:%#-4x\r\n","  value", regValue);
	vty_out(vty, "--------------------------------------\r\n");

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAGNOSIS_XAUI_PHY_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT16, &opPort,
							DBUS_TYPE_BYTE,  &phyID,
							DBUS_TYPE_BYTE,  &isExtPhy,
							DBUS_TYPE_UINT16, &opRegaddr,
							DBUS_TYPE_BYTE,  &phyDve,
							DBUS_TYPE_UINT16, &regValue,
							DBUS_TYPE_INVALID);
   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
	   vty_out(vty, "failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   vty_out(vty, "%s raised: %s", err.name, err.message);
		   dbus_error_free_for_dcli(&err);
	   }
	   return CMD_WARNING;
	}

   if ((!(dbus_message_get_args(reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT16, &regValue,
								   DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(ret) {
			vty_out(vty, "write operation failed\n");
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}


DEFUN(show_mib_ge_xg_cmd_func,
		show_mib_ge_xg_cmd,
		"show mib (ge|xg)",
		SHOW_STR
		"Mib information\n"
		"Show mib of GE ports, need to open debug of asic\n"
		"Show mib of XG ports, need to open debug of asic\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int portType = 0;

	/* get port type */
	if(0 == strncmp("ge", argv[0], strlen(argv[0]))) {
		portType = 0;
	}
	else if(0 == strncmp("xg", argv[0], strlen(argv[0]))) {
		portType = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_SHOW_MIB_GE_XG_PORT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &portType,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(diagnosis_hw_eeprom_read_reg_cmd_func,
		diagnosis_hw_eeprom_read_reg_cmd,
		"diagnosis-hardware eeprom read <0-1> <0-255> <0-1> (true|false) (true|false) REGADDR",
		DIAGNOSIS_STR
		"Diagnosis hardware eeprom information\n"
		"Read register\n"
		"TWSI channel range <0-1> valid\n"
		"Eeprom address range <0-255> valid\n"
		"Eeprom type range <0-1> valid\n"
		"Offset is valid\n"
		"Offset is in valid\n"
		"Register address is more than 256\n"
		"Register address is less than 256\n"
		"Register address\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int opType = 0;
	unsigned char twsi_channel = 0;	/* TWSI channel */
	unsigned int eeprom_addr = 0;		/* eeprom address  */
	unsigned int eeprom_type = 0;		/* eeprom type */
	unsigned int validOffset = 0;		/* whether the slave has offset (i.e. Eeprom  etc.), true: valid false: in valid */
	unsigned int moreThan256 = 0;		/* whether the ofset is bigger than 256, true: valid false: in valid */
	unsigned int regAddr = 0;			/* address of eeprom's register */
	unsigned char regValue = 0;		/* value of eeprom's register */

	/* get opTpye*/
	opType = 0;			/* read */

	/* get TWSI channel number */
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		twsi_channel = 0;
	}
	else if(0 == strncmp("1", argv[0], strlen(argv[0]))) {
		twsi_channel = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get eeprom address */
	eeprom_addr = strtoul((char*)argv[1], NULL, 0);

	/* get eeprom type */
	if(0 == strncmp("0", argv[2], strlen(argv[2]))) {
		eeprom_type = 0;
	}
	else if(0 == strncmp("1", argv[2], strlen(argv[2]))) {
		eeprom_type = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}

	/* get offset valid */
	if(0 == strncmp("true", argv[3], strlen(argv[3]))) {
		validOffset = 1;
	}
	else if(0 == strncmp("false", argv[3], strlen(argv[3]))) {
		validOffset = 0;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[3]);
		return CMD_WARNING;
	}

	/* get more than256 */
	if(0 == strncmp("true", argv[4], strlen(argv[4]))) {
		moreThan256 = 1;
	}
	else if(0 == strncmp("false", argv[4], strlen(argv[4]))) {
		moreThan256 = 0;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[4]);
		return CMD_WARNING;
	}

	/* get eeprom reg address */
	regAddr = strtoul((char*)argv[5], NULL, 0);

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAGNOSIS_EEPROM_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_BYTE,   &twsi_channel,
							DBUS_TYPE_UINT32, &eeprom_addr,
							DBUS_TYPE_UINT32, &eeprom_type,
							DBUS_TYPE_UINT32, &validOffset,
							DBUS_TYPE_UINT32, &moreThan256,
							DBUS_TYPE_UINT32, &regAddr,
							DBUS_TYPE_BYTE,   &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING ;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_UINT32, &regValue,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	vty_out(vty, "hardware diagnosis EEPROM device read\r\n");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-14s:%-4d\r\n", "  twsi channel", twsi_channel);
	vty_out(vty, "%-14s:%-4d\r\n", "  eeprom addr", eeprom_addr);
	vty_out(vty, "%-14s:%-8s\r\n", "  eeprom type", eeprom_type ? "16 bit" : "8 bit");
	vty_out(vty, "%-14s:%-5s\r\n", "  valid offset", validOffset ? "TRUE" : "FALSE");
	vty_out(vty, "%-14s:%-5s\r\n", "  moreThan256", moreThan256 ? "TRUE" : "FALSE");
	vty_out(vty, "%-14s:%#-8x\r\n","  register", regAddr);
	vty_out(vty, "%-14s:%#-8x\r\n","  value", regValue);
	vty_out(vty, "--------------------------------------\r\n");

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(diagnosis_hw_eeprom_write_reg_cmd_func,
		diagnosis_hw_eeprom_write_reg_cmd,
		"diagnosis-hardware eeprom write <0-1> <0-255> <0-1> (true|false) (true|false) REGADDR REGVALUE",
		DIAGNOSIS_STR
		"Diagnosis hardware eeprom information\n"
		"Write register\n"
		"Device range <0-1> valid\n"
		"Eeprom address range <0-255> valid\n"
		"Eeprom type range <0-1> valid\n"
		"Offset is valid\n"
		"Offset is in valid\n"
		"Register address is more than 256\n"
		"Register address is less than 256\n"
		"Register address\n"
		"Register value\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int opType = 0;
	unsigned char twsi_channel = 0;	/* TWSI channel */
	unsigned int eeprom_addr = 0;		/* eeprom address  */
	unsigned int eeprom_type = 0;		/* eeprom type */
	unsigned int validOffset = 0;		/* whether the slave has offset (i.e. Eeprom  etc.), true: valid false: in valid */
	unsigned int moreThan256 = 0;		/* whether the ofset is bigger than 256, true: valid false: in valid */
	unsigned int regAddr = 0;			/* address of eeprom's register */
	unsigned char regValue = 0;		/* value of eeprom's register */

	/* get opTpye*/
	opType = 1;					/* read */

	/* get TWSI channel number */
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		twsi_channel = 0;
	}
	else if(0 == strncmp("1", argv[0], strlen(argv[0]))) {
		twsi_channel = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get eeprom address */
	eeprom_addr = strtoul((char*)argv[1], NULL, 0);

	/* get eeprom type */
	if(0 == strncmp("0", argv[2], strlen(argv[2]))) {
		eeprom_type = 0;
	}
	else if(0 == strncmp("1", argv[2], strlen(argv[2]))) {
		eeprom_type = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}

	/* get offset valid */
	if(0 == strncmp("true", argv[3], strlen(argv[3]))) {
		validOffset = 1;
	}
	else if(0 == strncmp("false", argv[3], strlen(argv[3]))) {
		validOffset = 0;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[3]);
		return CMD_WARNING;
	}

	/* get more than256 */
	if(0 == strncmp("true", argv[4], strlen(argv[4]))) {
		moreThan256 = 1;
	}
	else if(0 == strncmp("false", argv[4], strlen(argv[4]))) {
		moreThan256 = 0;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[4]);
		return CMD_WARNING;
	}

	/* get eeprom reg address */
	regAddr = strtoul((char*)argv[5], NULL, 0);

	/* get eeprom reg value */
	regValue = strtoul((char*)argv[6], NULL, 0);

	vty_out(vty, "hardware diagnosis EEPROM device write\r\n");
	vty_out(vty, "--------------------------------------\r\n");
	vty_out(vty, "%-14s:%-4d\r\n", "  twsi channel", twsi_channel);
	vty_out(vty, "%-14s:%-4d\r\n", "  eeprom addr", eeprom_addr);
	vty_out(vty, "%-14s:%-8s\r\n", "  eeprom type", eeprom_type ? "16 bit" : "8 bit");
	vty_out(vty, "%-14s:%-5s\r\n", "  valid offset", validOffset ? "TRUE" : "FALSE");
	vty_out(vty, "%-14s:%-5s\r\n", "  moreThan256", moreThan256 ? "TRUE" : "FALSE");
	vty_out(vty, "%-14s:%#-8x\r\n","  register", regAddr);
	vty_out(vty, "%-14s:%#-8x\r\n","  value", regValue);
	vty_out(vty, "--------------------------------------\r\n");

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAGNOSIS_EEPROM_HW_RW_REG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_BYTE,   &twsi_channel,
							DBUS_TYPE_UINT32, &eeprom_addr,
							DBUS_TYPE_UINT32, &eeprom_type,
							DBUS_TYPE_UINT32, &validOffset,
							DBUS_TYPE_UINT32, &moreThan256,
							DBUS_TYPE_UINT32, &regAddr,
							DBUS_TYPE_BYTE,   &regValue,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING ;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_UINT32, &regValue,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

#if 0
/* add methods of debug for eag */

/**************************************************
 *	command: debug  eag (all|error|warning|debug|event)
 *	
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(config_eag_debug_cmd_func,
	config_eag_debug_cmd,
	"debug eag (all|error|warning|debug|event)",
	DCLI_DEBUG_STR
	MODULE_DEBUG_STR(eag)
	MODULE_DEBUG_LEVEL_STR(eag, all)
	MODULE_DEBUG_LEVEL_STR(eag, error)
	MODULE_DEBUG_LEVEL_STR(eag, warning)
	MODULE_DEBUG_LEVEL_STR(eag, debug)
	MODULE_DEBUG_LEVEL_STR(eag, event)
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty, "%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	if(0 == strncmp(argv[0], "all", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty, "%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										PORTAL_HA_DBUS_BUSNAME,
										PORTAL_HA_DBUS_OBJPATH,
										PORTAL_HA_DBUS_INTERFACE,
										PORTAL_HA_DBUS_METHOD_EAG_DEBUG_ON);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		if (0 != ret) {
			vty_out(vty,"%% Error:open eag's debug information fail!\n");
		}
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

/**************************************************
 *	command: no eag debug (all|error|warning|debug|event)
 *	
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(config_eag_no_debug_cmd_func,
	config_eag_no_debug_cmd,
	"no debug eag (all|error|warning|debug|event)",
	"Disable specific function \n"
	NODEBUG_STR
	DCLI_DEBUG_STR
	MODULE_DEBUG_STR(eag)
	MODULE_DEBUG_LEVEL_STR(eag, all)
	MODULE_DEBUG_LEVEL_STR(eag, error)
	MODULE_DEBUG_LEVEL_STR(eag, warning)
	MODULE_DEBUG_LEVEL_STR(eag, debug)
	MODULE_DEBUG_LEVEL_STR(eag, event)
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int flag = 0;
	
	if (argc > 1) {
		vty_out(vty, "%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	if (0 == strncmp(argv[0], "all", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if (0 == strncmp(argv[0], "error", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0], "warning", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0], "debug", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0], "event", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(
										PORTAL_HA_DBUS_BUSNAME,
										PORTAL_HA_DBUS_OBJPATH,
										PORTAL_HA_DBUS_INTERFACE,
										PORTAL_HA_DBUS_METHOD_EAG_DEBUG_OFF);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		if (0 != ret) {
			vty_out(vty,"%% Error:open eag's debug information fail!\n");
		}
	}else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************************
 *	command: debug eag packet (all|receive|send)
 *	
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(debug_eag_pkt_info,
	config_eag_debug_pkt_info_cmd,
	"debug eag packet (all|receive|send)",
	DCLI_DEBUG_STR
	MODULE_DEBUG_STR(eag)
	MODULE_DEBUG_STR(packet)
	MODULE_DEBUG_LEVEL_STR(packet, all)
	MODULE_DEBUG_LEVEL_STR(packet, receive)
	MODULE_DEBUG_LEVEL_STR(packet, send)
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if (argc > 1) {
		vty_out(vty, "%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if (0 == strncmp(argv[0], "all", strlen(argv[0])))	{
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}else if (0 == strncmp(argv[0], "receive", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_REV;
	}else if (0 == strncmp(argv[0], "send", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										PORTAL_HA_DBUS_BUSNAME,
										PORTAL_HA_DBUS_OBJPATH,
										PORTAL_HA_DBUS_INTERFACE,
										PORTAL_HA_DBUS_METHOD_EAG_DEBUG_ON);
    dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &flag,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		if (0 != ret) {
			vty_out(vty,"%% Error:open eag's debug information fail!\n");
		}
	}else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************************
 *	command: no debug eag packet (all|receive|send)
 *	
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(no_debug_eag_pkt_info,
	config_eag_no_debug_pkt_info_cmd,
	"no debug eag packet (all|receive|send)",
	NODEBUG_STR
	DCLI_DEBUG_STR
	MODULE_DEBUG_STR(eag)
	MODULE_DEBUG_STR(packet)
	MODULE_DEBUG_LEVEL_STR(packet, all)
	MODULE_DEBUG_LEVEL_STR(packet, receive)
	MODULE_DEBUG_LEVEL_STR(packet, send)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int ret  = 0;
	unsigned int flag  = 0;
	
	if (argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	if (0 == strncmp(argv[0], "all", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}else if (0 == strncmp(argv[0], "receive", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_REV;
	}else if (0 == strncmp(argv[0], "send", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										PORTAL_HA_DBUS_BUSNAME,
										PORTAL_HA_DBUS_OBJPATH,
										PORTAL_HA_DBUS_INTERFACE,
										PORTAL_HA_DBUS_METHOD_EAG_DEBUG_OFF);	
    dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &flag,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		if (0 != ret) {
			vty_out(vty,"%% Error:close eag's debug information fail!\n");
		}
	}else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif

void dcli_diag_hardware_watchdog_op
(
	struct vty * vty,
	unsigned int opType,
	unsigned int timeout
)
{
	unsigned int ret = 0, opTimeout = 0, enabled = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err ;

	opTimeout = timeout;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_OBJPATH,			\
							NPD_DBUS_INTERFACE, 		\
							NPD_DBUS_METHOD_SYSTEM_WATCHDOG_TIMEOUT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &opType,
							 DBUS_TYPE_UINT32, &opTimeout,
							 DBUS_TYPE_INVALID);

	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	 dbus_message_unref(query);
	 if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return;
	 }

	 if ((!(dbus_message_get_args ( reply, &err,
									 DBUS_TYPE_UINT32, &ret,
									 DBUS_TYPE_UINT32, &opTimeout,
									 DBUS_TYPE_UINT32, &enabled,
									 DBUS_TYPE_INVALID)))
		 || (0 != ret)) {
		 if (dbus_error_is_set(&err)) {
			 vty_out(vty, "%s raised: %s\n", err.name, err.message);
			 dbus_error_free_for_dcli(&err);
		 }
	 }
	 
	 dbus_message_unref(reply);

	 if(ret) {
		vty_out(vty, "%% Hareware watchdog operation failed %d\n", ret);
		return;
	 }
	 
	 if(SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_OP_GET == opType) {
		vty_out(vty, "Current hardware watchdog %s timeout %d\n",  \
				 (SYSTEM_HARDWARE_WATCHDOG_ENABLE == enabled )? "enabled":"disabled", opTimeout);
	 }
	 else if(SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_OP_SET == opType) {
		 vty_out(vty, "Current hardware watchdog timeout %d\n", opTimeout);
	 }

	 return;
}

DEFUN(diagnosis_hw_watchdog_ctrl_func,
		diagnosis_hw_watchdog_ctrl_cmd,
		"diagnosis-hardware watchdog (enable|disable)",
		DIAGNOSIS_STR
		"Diagnosis hardware watchdog control\n"
		"Enable hardware watchdog\n"
		"Disable hardware watchdog\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err ;
	unsigned int ret = 0;
	unsigned int enabled = SYSTEM_HARDWARE_WATCHDOG_DISABLE;


	/* get device*/
	if(0 == strncmp("enable", argv[0], strlen(argv[0]))) {
		enabled = SYSTEM_HARDWARE_WATCHDOG_ENABLE;
	}
	else if(0 == strncmp("disable", argv[0], strlen(argv[0]))) {
		enabled = SYSTEM_HARDWARE_WATCHDOG_DISABLE;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_METHOD_SYSTEM_WATCHDOG_CONTROL);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enabled,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	if(!ret) {
		vty_out(vty, "Hardware watchdog %s!\n", (SYSTEM_HARDWARE_WATCHDOG_ENABLE == enabled)?"started":"stopped");
	}
	else {
		vty_out(vty, "Start hardware watchdog failed %d\n", ret);
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}


DEFUN(diagnosis_hw_watchdog_timeout_func,
		diagnosis_hw_watchdog_timeout_cmd,
		"diagnosis-hardware watchdog timeout default",
		DIAGNOSIS_STR
		"Diagnosis hardware watchdog control\n"
		"Diagnosis hardware watchdog timeout profile\n"
		"Reset hardware watchdog timeout to default value\n"
) 
{
	unsigned int ret = 0;
	unsigned int timeout = 0, opTimeout = 0;
	
	/* do watchdog timeout setting */
	dcli_diag_hardware_watchdog_op(vty,  \
				SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_OP_SET, SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_DEFAULT);
	
	return CMD_SUCCESS;  
}

DEFUN(diagnosis_hw_watchdog_timeout__dflt_func,
		diagnosis_hw_watchdog_timeout_dflt_cmd,
		"diagnosis-hardware watchdog timeout <1-255>",
		DIAGNOSIS_STR
		"Diagnosis hardware watchdog control\n"
		"Diagnosis hardware watchdog timeout profile\n"
		"Specify hardware watchdog timeout value\n"
) 
{
	unsigned int ret = 0;
	unsigned int timeout = 0, opTimeout = 0;
	
	/* get timeout value */
	timeout = strtoul((char*)argv[0], NULL, 0);
	if (timeout > 255) {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* do watchdog timeout setting */
	dcli_diag_hardware_watchdog_op(vty, SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_OP_SET, timeout);
	return CMD_SUCCESS;  
}

DEFUN(diagnosis_hw_watchdog_timeout_show_func,
		diagnosis_hw_watchdog_timeout_show_cmd,
		"show watchdog configuration",
		SHOW_STR
		"Show hardware watchdog\n"
		"Show hardware watchdog configuration\n"
) 
{	
	/* do watchdog timeout setting */
	dcli_diag_hardware_watchdog_op(vty, SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_OP_GET, 0);
	return CMD_SUCCESS;  
}

/*
 *	opType
 *		0 - show gindex
 *		1 - update gindex
 */
void dcli_diag_gindex_handle
(
	struct vty * vty,
	char *name,
	int *gindex,
	char opType
)
{
#define SIOCGGINDEX		0x8939		/* get global index of the device */
#define SIOCSGINDEX 	0x893A		/* set global index of the device */
#define EGINDEX_ALREADY_INUSE	200 /* consistent with kernel net/core/dev_gop.h */
#define ERRMESG(errno)	\
	(ENODEV == errno) ? "no such device" : 	\
	(EADDRNOTAVAIL == errno) ? "no ip address" : \
	(EGINDEX_ALREADY_INUSE == errno) ? "gindex already in use" : \
	"general error"

	struct ifreq	ifr;
	int fd = -1;
	int	iocmd = 0;
	
	if((!name)||
		((0 != opType) && !gindex)){
		vty_out(vty, "%% Bad parameters: no IFNAME or GINDEX given!\n");
		return;
	}
	memset(&ifr, 0, sizeof(struct ifreq));

	/* init socket */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		vty_out(vty, "%% Init I/O error!\n");
		return ;
	}

	/* build up ioctl arguments */
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	
	if(0 == opType) {
		iocmd = SIOCGGINDEX;
	}
	else if(1 == opType) {
		ifr.ifr_ifindex = *gindex;
		iocmd = SIOCSGINDEX;
	}
	else {
		vty_out(vty, "%% Unsupported I/O command!\n");
		return;
	}

	/* do I/O */
	if (ioctl(fd, iocmd, (char *)&ifr)){
		vty_out(vty, "%% I/O failed: %s", ERRMESG(errno));
	}
	else {
		vty_out(vty, "Interface %s gindex %d\n", ifr.ifr_name, ifr.ifr_ifindex);
	}
	
	close(fd);
	return;
	
}

void dcli_diag_dump_gindex
(	
	struct vty * vty
)
{
	int fd = -1, ret = 0, i = 0;
	char cmd;
	struct if_nameindex *ifin = NULL;
	int gindex = 0;
	
	cmd = 0;
	
	/* dump all interface */
	ifin = if_nameindex();

	if(ifin) {
		vty_out(vty, "%-5s%-16s%-8s%-8s\n","ID","Interface","ifindex", "gindex");
		vty_out(vty, "---- --------------- ------- -------\n");
		while(ifin[i].if_name && ifin[i].if_index) {
			ret = if_name2gindex(ifin[i].if_name, &gindex);
			if(!ret) {
				vty_out(vty, "%-5d%-16s%-8d%-8d\n", \
						i, ifin[i].if_name, ifin[i].if_index, gindex);
			}
			i++;
		}
		if_freenameindex(ifin);
	}
	else {
		vty_out(vty, "%% I/O search interface error %d!\n", errno);
		return;
	}

	return;
}

DEFUN(diagnosis_get_gindex_func,
		diagnosis_get_gindex_cmd,
		"show gindex [IFNAME]",
		SHOW_STR
		"Show interface global index\n"
		"Specify interface name\n"
) 
{	
	char cmd;
	char name[IFNAMSIZ] = {0};

	/* command type : 0-show */
	cmd = 0;
	
	/* parse command line arguments */
	if(argc > 1) {
		vty_out(vty, "%% Bad parameter: too many arguments!\n");
		return CMD_WARNING;
	}
	else if(1 == argc) {
		strncpy(name, argv[0], IFNAMSIZ);
		dcli_diag_gindex_handle(vty, name, NULL,cmd);
	}
	else {
		dcli_diag_dump_gindex(vty);
	}
	
	return CMD_SUCCESS;  
}

DEFUN(diagnosis_set_gindex_func,
		diagnosis_set_gindex_cmd,
		"config gindex IFNAME GINDEX",
		CONFIG_STR
		"Global index configuration\n"
		"Specify interface name\n"
		"Specify interface global index\n"
) 
{	
	char cmd;
	char name[IFNAMSIZ] = {0};
	int gindex = 0;

	/* parse command line arguments */
	if(argc > 2) {
		vty_out(vty, "%% Bad parameter: too many arguments!\n");
		return CMD_WARNING;
	}
	strncpy(name, argv[0], IFNAMSIZ);

	gindex = (int)strtol(argv[1], NULL, 10);
	
	cmd = 1;
	dcli_diag_gindex_handle(vty, name, &gindex,cmd);
	return CMD_SUCCESS;  
}

DEFUN(show_CPU_temperature_cmd_func,
	show_CPU_temperature_cmd,
	"show cpu temperature",
	CONFIG_STR
	"show CPU temperature  \n"
	"show CPU temperature  \n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int opType = 1;
    int master_remote_temp = 0;
    int master_inter_temp = 0;
    int slave_remote_temp = 0;
    int slave_inter_temp = 0;
	
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_CPU_TEMPERATURE_TEST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_INT32,&master_inter_temp,
  							DBUS_TYPE_INT32,&master_remote_temp,
  							DBUS_TYPE_INT32,&slave_inter_temp,
  							DBUS_TYPE_INT32,&slave_remote_temp,									
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
        							DBUS_TYPE_INT32,&master_inter_temp,
          							DBUS_TYPE_INT32,&master_remote_temp,
          							DBUS_TYPE_INT32,&slave_inter_temp,
          							DBUS_TYPE_INT32,&slave_remote_temp,		
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}		
	}
	vty_out(vty,"+---------------+-----------------------------------+\n");
  	vty_out(vty,"|  single board |      %-6s     |      %-5s      |\n","MASTER","SLAVE" );		
  	vty_out(vty,"|     %-4s      |-----------------+-----------------|\n"," ");
  	vty_out(vty,"|  %-11s  |  %-4s  | %-7s|  %-4s  | %-7s|\n","TEMPERATURE", "CORE", "SURFACE", "CORE", "SURFACE");
  	vty_out(vty,"|     %-4s      |--------+--------+--------+--------|\n"," "); 
    vty_out(vty,"|     %-4s      |   %-4d |   %-4d |   %-4d |   %-4d |\n"," ",master_inter_temp,master_remote_temp,
  			                                                                     slave_inter_temp, slave_remote_temp);
    vty_out(vty,"+---------------+-----------------------------------+\n");
	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}


/*wangchong@autelan.com 20121023 for open lion1 for sum,4x,12x for 50G test*/
DEFUN(test_lion1_trunk_cmd_func,
		test_lion1_trunk_cmd,
		"test high-performance (enable|disable)",
		DIAGNOSIS_STR
		"high-performance test for cscd>=30G\n"
		"high-performance test for cscd>=30G\n"
		"high-performance test for cscd>=30G\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int opType = 0;
	if(0 == strncmp("disable", argv[0], strlen(argv[0]))) {
		opType = 0;
	}
	else if(0 == strncmp("enable", argv[0], strlen(argv[0]))) {
		opType = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,
								NPD_DBUS_OBJPATH,
								NPD_DBUS_INTERFACE,
								NPD_DBUS_SYSTEM_LION1_TRUNK_TEST);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opType,				
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}

	}
	dbus_message_unref(reply);
	return CMD_SUCCESS; 
}

DEFUN(test_1x12g12s_trunk_cmd_func,
		test_1x12g12s_trunk_cmd,
		"test trunk_port (0|1)",
		DIAGNOSIS_STR
		"AX81_1x12g12s TRUNK test\n"
		"AX81_1x12g12s TRUNK test\n"
		"AX81_1x12g12s TRUNK test\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int opType = 0;
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opType = 0;
	}
	else if(0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opType = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,
								NPD_DBUS_OBJPATH,
								NPD_DBUS_INTERFACE,
								NPD_DBUS_SYSTEM_TRUNK_PORT_TEST);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS; 
}

DEFUN(test_ge_xg_cmd_func,
		test_ge_xg_cmd,
		"test ge_xg_port (off|on)",
		DIAGNOSIS_STR
		"GE & XG port test\n"
		"Stop test\n"
		"Begin test\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int opType = 0;

	/* get port type */
	if(0 == strncmp("off", argv[0], strlen(argv[0]))) {
		opType = 0;
	}
	else if(0 == strncmp("on", argv[0], strlen(argv[0]))) {
		opType = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_GE_XG_TEST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}


DEFUN(diagnosis_hw_ax8610_prbs_test_cmd_func,
		diagnosis_hw_ax8610_prbs_test_cmd,
		"hwtest prbs ax8610 (Lion0|Lion1)",
		DIAGNOSIS_STR
		"AX8610 PRBS test\n"
		"AX8610 PRBS test\n"
		"AX8610 PRBS test\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
    unsigned int slotBitMap = 0;
	unsigned int opType = 0;
    unsigned int i = 0;

    /* get port type */
	if(0 == strncmp("Lion0", argv[0], strlen(argv[0]))) {
		opType = 0;
	}
	else if(0 == strncmp("Lion1", argv[0], strlen(argv[0]))) {
		opType = 1;
	}
	else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_AX8610_PRBS_TEST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opType,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_UINT32, &slotBitMap,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

    for(i=1; i<=10; i++)
    {
        if((5 == i) || (6 == i))
            continue;
        
        if((slotBitMap>>i) & 0x1)
            vty_out(vty, "slot%d PRBS status error!\n", i);
        else
            vty_out(vty, "slot%d PRBS status OK!\n", i);
    }

	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

/* for test master state change */
DEFUN(test_master_change_cmd_func,
		test_master_change_cmd,
		"master change start",
		DIAGNOSIS_STR
		"test MCB state change\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int opType = 0;

	int i = 0,slot_id =0;	
   	int master_slot_id[2] = {-1, -1};	
	char *master_slot_cnt_file = "/dbm/product/master_slot_count";		
    int master_slot_count = get_product_info(master_slot_cnt_file);
	int local_slot_id = get_product_info("/dbm/local_board/slot_id");

   	ret = dcli_master_slot_id_get(master_slot_id);
	if(ret !=0 )
	{
		vty_out(vty,"get master_slot_id error !\n");
		return CMD_SUCCESS;		
   	}
	if((local_slot_id<0)||(master_slot_count<0))
	{
		vty_out(vty,"get get_product_info return -1 !\n");
		return CMD_SUCCESS;		
   	}
	#if 0
    for(i=0;i<master_slot_count;i++)
    {

		slot_id = master_slot_id[i];
    #else
    for(i=1;i<=10;i++)
    {

		slot_id = i;
	#endif
		
    	query = NULL;
    	reply = NULL;

    	query = dbus_message_new_method_call(
    							   NPD_DBUS_BUSNAME,
    							   NPD_DBUS_OBJPATH,
    							   NPD_DBUS_INTERFACE,
    							   NPD_DBUS_SYSTEM_MCB_STATE_TEST);
    	dbus_error_init(&err);

		/* opType is no use */
    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32, &opType,
    							DBUS_TYPE_INVALID);

        if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
    	{
			if(slot_id == local_slot_id)
			{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
			    /* here do not print "Can not connect to MCB slot:5 " */	
				#if 1  
			   	vty_out(vty,"Can not connect to MCB slot:%d \n",slot_id);	
                #endif    
				continue;   /* for next MCB */
			}
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
    	}

    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"Please check npd on MCB slot %d\n",slot_id);
    		return CMD_WARNING;
    	}

    	if ((!(dbus_message_get_args ( reply, &err,
    									DBUS_TYPE_UINT32, &ret,
    									DBUS_TYPE_INVALID)))
    		|| (0 != ret)) {
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty, "%s raised: %s\n", err.name, err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    	}

    	dbus_message_unref(reply);
    }

    #if 0
	/* delete fdb vlan 1 */
	unsigned short vlanid = 1;
	unsigned int 	op_ret = 0;

    for(i=1;i<=10;i++)
    {

    	query = NULL;
    	reply = NULL;
	
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_VLAN );
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(	query,
    							 	DBUS_TYPE_UINT16,&vlanid,
    							 	DBUS_TYPE_INVALID);

        if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
    	{
		    /* here do not print "Can not connect to MCB slot:5 " */	
			#if 1  
		   	vty_out(vty,"Can not connect to slot:%d \n",i);	
            #endif    
			continue;   /* for next MCB */
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
    	}
    	    	
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply from slot %d.\n",i);
    		return CMD_SUCCESS;
    	}

    	if (!(dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_INVALID))) {
    		vty_out(vty,"Failed get args.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}		
    	} 
    	else {
    		   if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
    				vty_out(vty,"%% Error,the vlan input not exist!\n");
    			}
    			else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
    				vty_out(vty,"%% Error,there is something wrong when deleting.\n");
    			}
    		}
    	dbus_message_unref(reply);
    }
	#endif
	/* end */
	return CMD_SUCCESS;  
}

/* for create vlan on debug node */
DEFUN(diagnosis_create_vlan_cmd_func,
		diagnosis_create_vlan_cmd,
		"vlan create dev <0-1> vid <2-4094>",
		DIAGNOSIS_STR
		"Create vlan of asic chip\n"
		"Dev num \n"
		"Device range <0-1> valid\n"
		"Vlan vid \n"		
		"Vid range <2-4094> valid\n"
)
#if 0
DEFUN(diagnosis_create_vlan_cmd_func,
		diagnosis_create_vlan_cmd,
		"create-vlan <1-10> <0-1> <2-4094>",
		"Create vlan on debug node \n"
		DIAGNOSIS_STR
		"Slot range <1-10> valid\n"		
		"Device range <0-1> valid\n"
		"Vid range <2-4094> valid\n"
)
#endif
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int slot_id = 0;
	unsigned int opDevice = 0;
	unsigned int vlanId = 0;
    #if 0
	/* get slot id*/	
	slot_id = strtoul((char*)argv[0], NULL, 0);
	if((slot_id < 1)||(slot_id > 10))
	{
		vty_out(vty,"%% Bad slot id %s!\n", argv[0]);
		return CMD_WARNING;	
	}
    vty_out(vty,"********%% slot_id: %d!\n", slot_id);
    #endif
    /* get devnum */
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad devnum %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get vid*/
	vlanId = strtoul((char*)argv[1], NULL, 0);
	if((vlanId < 2)||(vlanId > 4094))
    {
		vty_out(vty,"%% Bad vlan id %s!\n", argv[1]);
		return CMD_WARNING;
	}		

	int local_slot_id = get_product_info("/dbm/local_board/slot_id");
    slot_id = local_slot_id;
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAG_CREATE_VLAN);
	dbus_error_init(&err);

	/* opType is no use */
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opDevice,
							DBUS_TYPE_UINT32, &vlanId,
							DBUS_TYPE_INVALID);

    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
	{
		if(slot_id == local_slot_id)
		{
            reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else 
		{	
		    /* here do not print "Can not connect to slot:5 " */	
		   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
    		return CMD_WARNING;
		}
    }
	else
	{
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
	}

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Please check npd on slot %d\n",slot_id);
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	/* end */
	return CMD_SUCCESS;  
}

/* add or delete port to vlan on debug node */
DEFUN(diagnosis_vlan_add_port_cmd_func,
		diagnosis_vlan_add_port_cmd,
    	"vlan (add|delete) dev <0-1> port <0-63> vlan <2-4094> (tag|untag)",
    	DIAGNOSIS_STR
    	"Add port as vlan member\n"
    	"Delete port from vlan\n"
    	"-Dev num \n"
		"Device range <0-1> valid\n"
		"-Port num \n"
		"Port asic range <0-64> valid\n"
		"-Vlan id \n"
		"Vid range <2-4094> valid\n"		
    	"Port member of vlan support 802.1q\n"
    	"Port member of vlan not support 802.1q\n"		
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int slot_id = 0;
	unsigned int opDevice = 0;
	unsigned int vlanId = 0;
	unsigned int portNum = 0;

	boolean isAdd 		= FALSE;
	boolean isTagged 	= FALSE;

	/*fetch the 1st param : add ? delete*/
	if(0 == strncmp(argv[0],"add",strlen(argv[0]))) {
		isAdd = TRUE;
	}
	else if (0 == strncmp(argv[0],"delete",strlen(argv[0]))) {
		isAdd= FALSE;
	}
	else {
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}
	#if 0
	/* get slot */
	slot_id = strtoul((char*)argv[0], NULL, 0);
	if((slot_id < 1)||(slot_id > 10))
	{
		vty_out(vty,"%% Bad slot id %s!\n", argv[0]);
		return CMD_WARNING;	
	}
    vty_out(vty,"********%% slot_id: %d!\n", slot_id);
    #endif

    /* get devnum */
	if(0 == strncmp("0", argv[1], strlen(argv[1]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[1], strlen(argv[1]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad devnum %s!\n", argv[1]);
		return CMD_WARNING;
	}

	/* get port num*/
	portNum = strtoul((char*)argv[2], NULL, 0);
	if((portNum < 0)||(portNum > 64))
	{
		vty_out(vty,"%% Bad port %s!\n", argv[2]);
		return CMD_WARNING;	
	}

	/* get vid*/
	vlanId = strtoul((char*)argv[3], NULL, 0);
	if((vlanId < 2)||(vlanId > 4094))
    {
		vty_out(vty,"%% Bad vlan id %s!\n", argv[3]);
		return CMD_WARNING;
	}	

    /* get tag */
	ret = param_first_char_check((char*)argv[4],1);
	if (NPD_FAIL == ret) {
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}
	else if(1 == ret){
		isTagged = TRUE;
	}
	else if(0 == ret){
		isTagged = FALSE;
	}

	
	int local_slot_id = get_product_info("/dbm/local_board/slot_id");
    slot_id = local_slot_id;
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAG_VLAN_ADD_DEL_PORT);
	dbus_error_init(&err);

	/* opType is no use */
	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&isAdd,
								DBUS_TYPE_UINT32,&opDevice,
								DBUS_TYPE_UINT32,&portNum,								
							 	DBUS_TYPE_UINT32,&vlanId,								
							 	DBUS_TYPE_BYTE,&isTagged,
							 	DBUS_TYPE_INVALID);

    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
	{
		if(slot_id == local_slot_id)
		{
            reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else 
		{	
		    /* here do not print "Can not connect to slot:5 " */	
		   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
    		return CMD_WARNING;
		}
    }
	else
	{
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
	}

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Please check npd on slot %d\n",slot_id);
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	/* end */
	return CMD_SUCCESS;  
}



/* show asic mib on debug node */
DEFUN(diagnosis_read_port_mib_cmd_func,
		diagnosis_read_port_mib_cmd,
    	"show asic-mib dev <0-1> port <0-63> ",
    	DIAGNOSIS_STR
    	"show asic port mib\n"
    	"-Dev num \n"
		"Device range <0-1> valid\n"
		"-Port num \n"
		"Port asic range <0-64> valid\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	
	unsigned int ret = 0, j=0;
	unsigned int slot_id = 0;
	unsigned char opDevice = 0;
	unsigned char portNum = 0;
	unsigned int portStat[70] = {0};

    /* get devnum */
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad devnum %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get port num*/
	portNum = strtoul((char*)argv[1], NULL, 0);
	if((portNum < 0)||(portNum > 64))
	{
		vty_out(vty,"%% Bad port %s!\n", argv[2]);
		return CMD_WARNING;	
	}
	
	int local_slot_id = get_product_info("/dbm/local_board/slot_id");
    slot_id = local_slot_id;
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAG_READ_ASIC_MIB);
	dbus_error_init(&err);


	vty_out(vty,"Show asic mib dev %d port %d\n", opDevice,portNum);

	/* opType is no use */
	dbus_message_append_args(	query,
								DBUS_TYPE_BYTE,&opDevice,
								DBUS_TYPE_BYTE,&portNum,								
							 	DBUS_TYPE_INVALID);

    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
	{
		if(slot_id == local_slot_id)
		{
            reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else 
		{	
		    /* here do not print "Can not connect to slot:5 " */	
		   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
    		return CMD_WARNING;
		}
    }
	else
	{
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
	}


	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"failed get reply.\n"); 
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	dbus_message_iter_init(reply,&iter);	
		
	for(j = 0; j < 70; j++) {
		dbus_message_iter_get_basic(&iter,&(portStat[j]));
		dbus_message_iter_next(&iter);	
	}
	
	vty_out(vty, "Asic detailed info as follow:\n", opDevice,portNum);
	vty_out(vty, "------------------------------------------\n");	
	vty_out(vty, "%-20s %-10s %-10s\n","DESCRIPTION","32BitH","32BitL");	
	vty_out(vty, "%-20s %-10s %-10s\n","--------------------","----------","----------");
	vty_out(vty, "-------------------- RX ------------------\n");	
	
	vty_out(vty, "%-20s %-10u %-10u\n","goodOctetsRcv", portStat[1],portStat[0]);
	vty_out(vty, "%-20s %-10u %-10u\n","badOctetsRcv", portStat[3],portStat[2]);
	vty_out(vty, "%-20s %-10u %-10u\n","goodPktsRcv", portStat[7],portStat[6]);
	vty_out(vty, "%-20s %-10u %-10u\n","badPktsRcv", portStat[9],portStat[8]);
	vty_out(vty, "%-20s %-10u %-10u\n","ucPktsRcv", portStat[63],portStat[62]);
	vty_out(vty, "%-20s %-10u %-10u\n","brdcPktsRcv", portStat[11],portStat[10]);	
	vty_out(vty, "%-20s %-10u %-10u\n","mcPktsRcv", portStat[13],portStat[12]);
	vty_out(vty, "%-20s %-10u %-10u\n","unrecogMacCntrRcv", portStat[37],portStat[36]);
	vty_out(vty, "%-20s %-10u %-10u\n","goodFcRcv", portStat[41],portStat[40]);
	vty_out(vty, "%-20s %-10u %-10u\n","badFcRcv", portStat[61],portStat[60]);
	vty_out(vty, "%-20s %-10u %-10u\n","dropEvents", portStat[43],portStat[42]);
	vty_out(vty, "%-20s %-10u %-10u\n","undersizePkts", portStat[45],portStat[44]);
	vty_out(vty, "%-20s %-10u %-10u\n","fragmentsPkts", portStat[47],portStat[46]);
	vty_out(vty, "%-20s %-10u %-10u\n","oversizePkts", portStat[49],portStat[48]);
	vty_out(vty, "%-20s %-10u %-10u\n","jabberPkts", portStat[51],portStat[50]);
	vty_out(vty, "%-20s %-10u %-10u\n","badCrc", portStat[55],portStat[54]);
	vty_out(vty, "%-20s %-10u %-10u\n","macRcvError", portStat[53],portStat[52]);

	vty_out(vty, "-------------------- TX ------------------\n");	
	vty_out(vty, "%-20s %-10u %-10u\n","goodOctetsSent", portStat[27],portStat[26]);
	vty_out(vty, "%-20s %-10u %-10u\n","goodPktsSent", portStat[29],portStat[28]);
	vty_out(vty, "%-20s %-10u %-10u\n","ucPktsSent", portStat[65],portStat[64]);
	vty_out(vty, "%-20s %-10u %-10u\n","brdcPktsSent", portStat[35],portStat[34]);
	vty_out(vty, "%-20s %-10u %-10u\n","mcPktsSent", portStat[33],portStat[32]);
	vty_out(vty, "%-20s %-10u %-10u\n","fcSent", portStat[39],portStat[38]);
	vty_out(vty, "%-20s %-10u %-10u\n","multiplePktsSent", portStat[67],portStat[66]);
	vty_out(vty, "%-20s %-10u %-10u\n","deferredPktsSent", portStat[69],portStat[68]);
	vty_out(vty, "%-20s %-10u %-10u\n","macTransmitErr", portStat[5],portStat[4]);

	vty_out(vty, "-------------------- Both ----------------\n");	
	vty_out(vty, "%-20s %-10u %-10u\n","pkts64Octets", portStat[15],portStat[14]);
	vty_out(vty, "%-20s %-10u %-10u\n","pkts65to127Octets", portStat[17],portStat[16]);
	vty_out(vty, "%-20s %-10u %-10u\n","pkts128to255Octets", portStat[19],portStat[18]);
	vty_out(vty, "%-20s %-10u %-10u\n","pkts256to511Octets", portStat[21],portStat[20]);
	vty_out(vty, "%-20s %-10u %-10u\n","pkts512to1023Octets", portStat[23],portStat[22]);
	vty_out(vty, "%-20s %-10u %-10u\n","pkts1024tomaxOoctets", portStat[25],portStat[24]);

	vty_out(vty, "%-20s %-10u %-10u\n","collisions", portStat[57],portStat[56]);
	vty_out(vty, "%-20s %-10u %-10u\n","lateCollisions", portStat[59],portStat[58]);
	vty_out(vty, "%-20s %-10u %-10u\n","excessiveCollisions", portStat[31],portStat[30]);
	vty_out(vty, "------------------------------------------\n");
	
    #if 0	
	vty_out(vty, "%-15s %-10u %-10u\n","RxGoodPkts", portStat[1],portStat[0]);
	vty_out(vty, "%-15s %-10u %-10u\n","RxBadPkts", portStat[3],portStat[2]);
	vty_out(vty, "%-15s %-10u %-10u\n","RxGoodBytes", portStat[5],portStat[4]);
	vty_out(vty, "%-15s %-10u %-10u\n","RxBadBytes", portStat[7],portStat[6]);
	vty_out(vty, "%-15s %-10u %-10u\n","RxInternalDrop", portStat[9],portStat[8]);

	vty_out(vty, "%-15s %-10u %-10u\n","Rx_BC_pkts", portStat[11],portStat[10]);
	vty_out(vty, "%-15s %-10u %-10u\n","Rx_UC_pkts", portStat[13],portStat[12]);


	
	vty_out(vty, "%-15s %-10u %-10u\n","TxGoodPkts", portStat[15],portStat[14]);
	vty_out(vty, "%-15s %-10u %-10u\n","TxGoodBytes", portStat[17],portStat[16]);
	vty_out(vty, "%-15s %-10u %-10u\n","TxMacError", portStat[19],portStat[18]);

	vty_out(vty, "%-15s %-10u %-10u\n","Tx_BC_pkts", portStat[21],portStat[20]);
	vty_out(vty, "%-15s %-10u %-10u\n","Tx_UC_pkts", portStat[23],portStat[22]);
	
	vty_out(vty, "----------------------------------------\n");
	#endif
				
	dbus_message_unref(reply);
	/* end */
	return CMD_SUCCESS;  
}

DEFUN(diagnosis_read_asic_speed_cmd_func,
		diagnosis_read_asic_speed_cmd,
		"show asic-speed dev <0-1> port <0-63> ",
		DIAGNOSIS_STR
		"show asic port speed\n"
		"-Dev num \n"
		"Device range <0-1> valid\n"
		"-Port num \n"
		"Port asic range <0-64> valid\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;

	unsigned int slot_id = 0;
	unsigned char opDevice = 0;
	unsigned char portNum = 0;
	unsigned long long bitsumrecv = 0,packetsumrecv = 0;
	unsigned long long bitsumsent = 0,packetsumsent = 0;
	unsigned char type = 0;
	unsigned int value = 0;
	
	int local_slot_id = get_product_info("/dbm/local_board/slot_id");	
	slot_id = local_slot_id;

    /* get devnum */
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		opDevice= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		opDevice = 1;
	}
	else {
		vty_out(vty,"%% Bad devnum %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get port num*/
	portNum = strtoul((char*)argv[1], NULL, 0);
	if((portNum < 0)||(portNum > 64))
	{
		vty_out(vty,"%% Bad port %s!\n", argv[2]);
		return CMD_WARNING;	
	}

		
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_ETHPORTS_INTERFACE_METHOD_GET_PORT_RATE);
	dbus_error_init(&err);



	/* opType is no use */
	dbus_message_append_args(	query,
								DBUS_TYPE_BYTE,&opDevice,
								DBUS_TYPE_BYTE,&portNum,	
								DBUS_TYPE_BYTE,&type,	
								DBUS_TYPE_UINT32,&value,
							 	DBUS_TYPE_INVALID);

    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
	{
		if(slot_id == local_slot_id)
		{
            reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else 
		{	
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_id);
			return CMD_WARNING;
		}
    }
	else
	{
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
	}


	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"failed get reply.\n"); 
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	dbus_message_iter_init(reply,&iter);	
	
	dbus_message_iter_get_basic(&iter,&bitsumrecv);
	dbus_message_iter_next(&iter);	

	
	dbus_message_iter_get_basic(&iter,&packetsumrecv);
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&bitsumsent);
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&packetsumsent);
	dbus_message_iter_next(&iter);	
	
	dbus_message_unref(reply);
		
	vty_out(vty, "-------------------- RX ------------------\n");	
	vty_out(vty, "%-20s %-15lld\n","bitRcv/s", bitsumrecv/3 * 8);
	vty_out(vty, "%-20s %-15lld\n","byteRcv/s", bitsumrecv/3 );
	vty_out(vty, "%-20s %-15lld\n","packRcv/s", packetsumrecv/3);
	vty_out(vty, "-------------------- TX ------------------\n");		
	vty_out(vty, "%-20s %-15lld\n","bitSent/s", bitsumsent/3 * 8);
	vty_out(vty, "%-20s %-15lld\n","byteSent/s", bitsumsent/3 );
	vty_out(vty, "%-20s %-15lld\n","packSent/s", packetsumsent/3);
		
	/* end */
	return CMD_SUCCESS;  
}


DEFUN(diagnosis_endis_asic_cmd_func,
		diagnosis_endis_asic_cmd,
    	"config asic <0-1> (enable|disable) ",
    	DIAGNOSIS_STR
    	"config asic enable or disable\n"
    	"-Dev num \n"
		"Device range <0-1> valid\n"
		"-Enable or Disable \n"
		"Enable or Disable Asic\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	
	unsigned int ret = 0;
	unsigned char slot_id = 0;
	unsigned char asic_num = 0;
	unsigned char endis = 0;

    /* get asic num*/
	if(0 == strncmp("0", argv[0], strlen(argv[0]))) {
		asic_num= 0;
	}
	else if (0 == strncmp("1", argv[0], strlen(argv[0]))) {
		asic_num = 1;
	}
	else 
	{
		vty_out(vty,"%% Bad asic num %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get status of asic --enable or disable*/
	if(0 == strncmp("enable", argv[1], strlen(argv[1]))) {
		endis= 1;
	}
	else if (0 == strncmp("disable", argv[1], strlen(argv[1]))) {
		endis = 0;
	}
	else 
	{
		vty_out(vty,"%% Bad params %s!\n", argv[1]);
		return CMD_WARNING;
	}
	
	
	int local_slot_id = get_product_info("/dbm/local_board/slot_id");
    slot_id = local_slot_id;
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAG_ENDIS_ASIC);
	dbus_error_init(&err);


	vty_out(vty,"config asic %d %s\n", asic_num,endis?"Enable":"Disable");

	/* opType is no use */
	dbus_message_append_args(	query,
								DBUS_TYPE_BYTE,&asic_num,
								DBUS_TYPE_BYTE,&endis,								
							 	DBUS_TYPE_INVALID);

    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
	{
		if(slot_id == local_slot_id)
		{
            reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else 
		{	
		    /* here do not print "Can not connect to slot:5 " */	
		   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
    		return CMD_WARNING;
		}
    }
	else
	{
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
	}


	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"Please check npd on slot %d\n",slot_id);
		return CMD_WARNING;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	/* end */
	return CMD_SUCCESS;  
}

DEFUN(diagnosis_port_cscd_set_cmd_fun, 
	diagnosis_port_cscd_set_cmd, 
	"config port SLOT_PORT to (cscd|network) mode",
	DIAGNOSIS_STR
	"config a port to cscd mode\n"
	"Layer 2 configure\n"
	"Port that in system\n"
	"Config ethernet port number(e.g 1/1 or 1-1 means slot 1 port 1)\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	boolean isCscd 		= FALSE;
	int 	ret 		= 0;
	unsigned int t_slotno = 0,t_portno = 0;
	unsigned char slot_no = 0,local_port_no = 0;
	unsigned int 	op_ret = 0;
	int local_slot_id = get_product_info("/dbm/local_board/slot_id");
	if(local_slot_id<0)
	{
		vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
		return CMD_WARNING;			
	}

	op_ret = parse_slotno_localport((char *)argv[0],&t_slotno,&t_portno);
	if (NPD_FAIL == op_ret) 
	{
    	vty_out(vty,"% Bad parameter:Unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == op_ret)
	{
		vty_out(vty,"% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;

	if(0 == strncmp(argv[1],"cscd",strlen(argv[1]))) {
		isCscd = TRUE;
	}
	else if (0 == strncmp(argv[1],"network",strlen(argv[1]))) {
		isCscd= FALSE;
	}
	else 
	{
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAG_PORT_CSCD_MODE_SET);

	dbus_error_init(&err);

	dbus_message_append_args(	query,
						 		DBUS_TYPE_BYTE,&isCscd,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&local_port_no,
						 		DBUS_TYPE_INVALID);


	if(local_slot_id == slot_no)
	{
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	}
	else
	{
		if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection)
		{
		   	vty_out(vty,"Can not connect to slot:%d, check the slot please !\n",slot_no);	
    		return CMD_WARNING;
		}
		else
		{
			reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,query,-1, &err);
		}
	}

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (DIAG_RETURN_CODE_ERROR == op_ret) {
				vty_out(vty,"%% set port %d/%d to %s mode faied.\n",slot_no,local_port_no,isCscd?"cscd":"network");
			}
			else if (DIAG_RETURN_CODE_SUCCESS== op_ret) {
				vty_out(vty,"%% set port %d/%d to %s mode successfully.\n",slot_no,local_port_no,isCscd?"cscd":"network"); /*such as add port to trunkNode struct.*/
			}
	}
	else 
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(diagnosis_vlan_fdb_delete_func,
	diagnosis_vlan_fdb_delete_cmd,
	"delete fdb vlan <1-4096>",
	"Delete FDB table\n"
	"Config FDB table \n"
	"FDB table vlan Range <1-4096>\n"
	"Config Fdb vlan value\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vlanid = 0;
	unsigned int 	op_ret = 0;
	int local_slot_id = 0;
	int slot_id = 0;
	op_ret = parse_short_parse((char *)argv[0],&vlanid);
	if (NPD_FAIL == op_ret) 
	{
    	vty_out(vty,"% Bad Parameters,FDB agingtime form erro!\n");
		return CMD_SUCCESS;
	}
	if (vlanid <1||vlanid>4096)
	{
		vty_out(vty,"%% Error, vlan outrang!\n");
		return CMD_SUCCESS;
	}
	local_slot_id = get_product_info("/dbm/local_board/slot_id");
	slot_id = local_slot_id;
	if((local_slot_id < 0))
	{
		vty_out(vty,"read file error ! \n");
		return CMD_WARNING;
	}
	vty_out(vty,"Delete fdb with vlan %d\n",vlanid);
	query = dbus_message_new_method_call(	
											NPD_DBUS_BUSNAME,
							  				NPD_DBUS_OBJPATH,
							  				NPD_DBUS_INTERFACE,
											NPD_DBUS_SYSTEM_DIAG_VLAN_FDB_DELETE );
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanid,
							 DBUS_TYPE_INVALID);
	
	if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
	{
		if(slot_id == local_slot_id)
		{
            reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else 
		{	
		    /* here do not print "Can not connect to slot:5 " */	
		   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
    		return CMD_WARNING;
		}
    }
	else
	{
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
	}

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",local_slot_id);
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) 
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}		
	} 
	else 
	{
		if(DIAG_RETURN_CODE_ERROR == op_ret)
		{
			vty_out(vty,"%% Error,there is something wrong when deleting.\n");
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(diagnosis_show_cscd_port_status_cmd_fun, 
	diagnosis_show_cscd_port_status_cmd, 
	"show cscd port status",
	DIAGNOSIS_STR
	"show cscd port up/down status\n"
	"Layer 2 configure\n"
	"Port that in system\n"
	"Config ethernet port number(e.g 1/1 or 1-1 means slot 1 port 1)\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned char port = 0,devNum = 0;
	unsigned long  portstatus = 0,portendis = 0;
	unsigned int  cscdportcnt = 0;
	unsigned int 	op_ret = 0;
	unsigned short trunkId = 0;
	int i = 0;
	
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAG_SHOW_CSCD_PORT_STATUS);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&cscdportcnt);
	if(cscdportcnt == 0)
	{
		vty_out(vty,"Error get cscd port fail !\n");
		return CMD_SUCCESS;
	}
	vty_out(vty,"CSCD PORT UP/DOWN & ENABLE/DISABLE STATUS\n");
	vty_out(vty,"%-4s  %-4s  %-8s  %-5s  %-9s\n","====","====","========","=====","=========");
	vty_out(vty,"%-4s  %-4s  %-8s  %-5s  %-9s\n","DEV","PORT","TRUNK ID","ENDIS","STATUS");
	vty_out(vty,"%-4s  %-4s  %-8s  %-5s  %-9s\n","====","====","========","=====","=========");
	dbus_message_iter_next(&iter);

	dbus_message_iter_recurse(&iter,&iter_array);
	for(;cscdportcnt>i;i++)
	{
		DBusMessageIter iter_struct;
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&devNum);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&port);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&portstatus);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&portendis);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&trunkId);
		if(trunkId > 0)
		{
			vty_out(vty,"%-4d  %-4d  %-8d  %-5s  %-9s\n",devNum,port,trunkId,portendis?"En":"Dis",portstatus?"UP":"DOWN");
		}
		else
		{
			vty_out(vty,"%-4d  %-4d  %-8s  %-5s  %-9s\n",devNum,port,"   -    ",portendis?"En":"Dis",portstatus?"UP":"DOWN");
		}
		dbus_message_iter_next(&iter_array);
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(diagnosis_board_test_cmd_fun, 
	diagnosis_board_test_cmd, 
	"config system to (test|default) mode",
	DIAGNOSIS_STR
	"config system to special mode\n"
	"Layer 2 configure\n"
	"Port that in system\n"
	"Config ethernet port number(e.g 1/1 or 1-1 means slot 1 port 1)\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned char endis = 0;
	unsigned int 	op_ret = 0;
	int local_slot_id = 0,slotNum = 0,active_master_slot_id = 0,i = 0;

	local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	active_master_slot_id = get_product_info(SEM_ACTIVE_MASTER_SLOT_ID_PATH);
	if((local_slot_id<0) || (slotNum < 0) || (active_master_slot_id < 0))
	{
		vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
		return CMD_WARNING;			
	}
	if(0 == strncmp(argv[0],"test",strlen(argv[0]))) {
		endis = TRUE;
	}
	else if (0 == strncmp(argv[0],"default",strlen(argv[0]))) {
		endis= FALSE;
	}
	else 
	{
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}

	if(local_slot_id != active_master_slot_id)
	{
		query = dbus_message_new_method_call(
								   NPD_DBUS_BUSNAME,
								   NPD_DBUS_OBJPATH,
								   NPD_DBUS_INTERFACE,
								   NPD_DBUS_SYSTEM_DIAG_BOARD_TEST);

		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_BYTE,&endis,
							 		DBUS_TYPE_INVALID);



        reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

		dbus_message_unref(query);
		if (NULL == reply) 
		{
			vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",local_slot_id);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
				if (DIAG_RETURN_CODE_ERROR == op_ret) {
					vty_out(vty,"Set system to %s mode faied.\n",endis?"test":"default");
				}
				else if (DIAG_RETURN_CODE_SUCCESS== op_ret) {
					vty_out(vty,"Set system to %s mode successfully.\n",endis?"test":"default"); /*such as add port to trunkNode struct.*/
				}
		}
		else 
		{
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	for(i=1; i <= slotNum; i++)
	{
		query = dbus_message_new_method_call(
								   NPD_DBUS_BUSNAME,
								   NPD_DBUS_OBJPATH,
								   NPD_DBUS_INTERFACE,
								   NPD_DBUS_SYSTEM_DIAG_BOARD_TEST);

		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_BYTE,&endis,
							 		DBUS_TYPE_INVALID);


		if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
		{
			if(i == local_slot_id)
			{
           		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
		   		vty_out(vty,"Can not connect to slot:%d \n",i);	
				continue;
			}
    	}
		else
		{
        	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
		}

		dbus_message_unref(query);
		if (NULL == reply) 
		{
			vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
				if (DIAG_RETURN_CODE_ERROR == op_ret) {
					vty_out(vty,"Set system to %s mode faied.\n",endis?"test":"default");
				}
				else if (DIAG_RETURN_CODE_SUCCESS== op_ret) {
					vty_out(vty,"Set system to %s mode successfully.\n",endis?"test":"default"); /*such as add port to trunkNode struct.*/
				}
		}
		else 
		{
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
		}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}


DEFUN(Diagnosis_set_ap_fake_singal_strength_cmd_func,
		Diagnosis_set_ap_fake_singal_strength_cmd,
		"diagnosis-hardware set ap fake singal strength <80-120>",
		DIAGNOSIS_STR
		"Diagnosis set fake singal strength for AP\n"
		"the value between -VALUE~-80 will be up to -70~-80 and -120~-VALUE will be -120~-80,0 is default\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int fake_singal_strengh = 0;


	if(argc < 1)
	{
	    vty_out(vty,"Command incompleted!\n");
	    return CMD_WARNING;
	}
	if(argc == 1)
	{
		fake_singal_strengh = strtoul((char*)argv[0], NULL, 0);
	}

	vty_out(vty,"fake_singal_strengh  = %d\n",fake_singal_strengh);	
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_SET_AP_FAKE_SINGAL_STRENGTH);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &fake_singal_strengh,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING ;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) 
	{
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(Diagnosis_set_ap_fake_singal_strength_default_cmd_func,
		Diagnosis_set_ap_fake_singal_strength_default_cmd,
		"diagnosis-hardware set ap fake singal strength default",
		DIAGNOSIS_STR
		"Diagnosis set fake singal strength for AP\n"
		"set it to default\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int fake_singal_strengh = 0;

	vty_out(vty,"fake_singal_strengh  = %d\n",fake_singal_strengh);	
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_DIAGNOSIS_SET_AP_FAKE_SINGAL_STRENGTH);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &fake_singal_strengh,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING ;
	}

	if ((!(dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32, &ret,
									DBUS_TYPE_INVALID)))
		|| (0 != ret)) 
	{
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;  
}

DEFUN(diagnosis_get_conntable_func,
		diagnosis_get_conntable_cmd,
		"show connect_table list",
		SHOW_STR
		"Show connect_table list\n"
		"Show connect_table list\n"
) 
{

    int slot_id,i,ret=-1;
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {

        int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
		
		int fd = -1;
		struct stat sb;
		asic_board_cscd_bport_t * mem_gbports_list = NULL;
    	char* file_path = "/dbm/shm/connect_table/shm_conntable";
		
    	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
        int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
        if((local_slot_id<0)||(slotNum<0))
        {
        	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
    		return CMD_WARNING;			
        }
        if(slotNum<6)
        {
        	vty_out(vty,"can`t run in 3 slot product !\n");
    		return CMD_WARNING;			
        }		
		
		/* only read file */
	    fd = open(file_path, O_RDONLY);
    	if(fd < 0)
        {
            vty_out(vty,"Failed to open! %s\n", strerror(errno));
            return NULL;
        }
		fstat(fd,&sb);
		/* map not share */	
        mem_gbports_list = (asic_board_cscd_bport_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0 );
        if(MAP_FAILED == mem_gbports_list)
        {
            vty_out(vty,"Failed to mmap for mem_gbports_list[]! %s\n", strerror(errno));
			close(fd);
            return NULL;
        }	    
    	vty_out(vty,"%-12s  %-25s  %-40s\n","CSCD_PORT","SUME_PORT","TRUNK_ID");
    	vty_out(vty,"============      ======================  ========\n");			
        for(slot_id=0;slot_id<slotNum;slot_id++)
        {
            if(mem_gbports_list[slot_id].board_type == -1)
				continue;
			for(i=0;i<(mem_gbports_list[slot_id].asic_cscd_port_cnt+mem_gbports_list[slot_id].asic_to_cpu_ports);i++)
			{
                if(mem_gbports_list[slot_id].asic_cscd_bports[i].master<3)
    			{
                	vty_out(vty, "slot%d-port%-3d===> ve%02d%s%d--------\n",\
                				            mem_gbports_list[slot_id].slot_id,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].cscd_port,\
                				            mem_gbports_list[slot_id].slot_id,\
                				            (mem_gbports_list[slot_id].asic_cscd_bports[i].master==1)?"f":"s",\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].bport); 
				}
				else
				{
        			vty_out(vty,"slot%d-port%-3d===> SUM_slot%d-lion%d_port%d    trunk%d\n",\
        				            mem_gbports_list[slot_id].slot_id,\
        				            mem_gbports_list[slot_id].asic_cscd_bports[i].cscd_port,\
        				            mem_gbports_list[slot_id].asic_cscd_bports[i].master,\
        				            mem_gbports_list[slot_id].asic_cscd_bports[i].asic_id,\
        				            mem_gbports_list[slot_id].asic_cscd_bports[i].bport,\
        				            mem_gbports_list[slot_id].asic_cscd_bports[i].trunk_id);				           
				}
			}
		}
		vty_out(vty,"======================================================\n");

        ret = munmap(mem_gbports_list,sb.st_size);
        if( ret != 0 )
        {
            vty_out(vty,"Failed to munmap for mem_gbports_list[]! %s\n", strerror(errno));			
        }	
		ret = close(fd);
		if( ret != 0 )
        {
            vty_out(vty,"close shm_conntable failed \n" );   
        }
        /*free(vlanName);*/
        return CMD_SUCCESS;		
    }
}





int get_asic_mib(struct vty *vty,int slot_id,unsigned char opDevice,unsigned char portNum, unsigned int *portStat)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	
	unsigned int ret = 0, j=0;

    /* get devnum */
	if((0!=opDevice)&&(1!=opDevice)) 
	{
		vty_out(vty,"%% Bad devnum %s!\n", opDevice);
		return CMD_WARNING;
	}

	/* get port num*/
	if((portNum < 0)||(portNum > 64))
	{
		vty_out(vty,"%% Bad port %s!\n", portNum);
		return CMD_WARNING;	
	}
	int local_slot_id = get_product_info("/dbm/local_board/slot_id");
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_SYSTEM_DIAG_READ_ASIC_MIB);
	dbus_error_init(&err);


	/* opType is no use */
	dbus_message_append_args(	query,
								DBUS_TYPE_BYTE,&opDevice,
								DBUS_TYPE_BYTE,&portNum,								
							 	DBUS_TYPE_INVALID);

    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
	{
		if(slot_id == local_slot_id)
		{
            reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else 
		{	
		    /* here do not print "Can not connect to slot:5 " */	
		   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
    		return CMD_WARNING;
		}
    }
	else
	{
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
	}


	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"failed get reply.\n"); 
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_WARNING;
	}
	
	dbus_message_iter_init(reply,&iter);	
		
	for(j = 0; j < 70; j++) {
		dbus_message_iter_get_basic(&iter,&(portStat[j]));
		dbus_message_iter_next(&iter);	
	}
/*	
	vty_out(vty, "Asic detailed info as follow:\n", opDevice,portNum);
	vty_out(vty, "------------------------------------------\n");	
	vty_out(vty, "%-20s %-10s %-10s\n","DESCRIPTION","32BitH","32BitL");	
	vty_out(vty, "%-20s %-10s %-10s\n","--------------------","----------","----------");
	vty_out(vty, "-------------------- RX ------------------\n");	
	
	vty_out(vty, "%-20s %-10u %-10u\n","goodOctetsRcv", portStat[1],portStat[0]);
	vty_out(vty, "%-20s %-10u %-10u\n","badOctetsRcv", portStat[3],portStat[2]);
	vty_out(vty, "%-20s %-10u %-10u\n","goodPktsRcv", portStat[7],portStat[6]);
	vty_out(vty, "%-20s %-10u %-10u\n","badPktsRcv", portStat[9],portStat[8]);
	vty_out(vty, "%-20s %-10u %-10u\n","ucPktsRcv", portStat[63],portStat[62]);
	vty_out(vty, "%-20s %-10u %-10u\n","brdcPktsRcv", portStat[11],portStat[10]);	
	vty_out(vty, "%-20s %-10u %-10u\n","mcPktsRcv", portStat[13],portStat[12]);
	vty_out(vty, "%-20s %-10u %-10u\n","unrecogMacCntrRcv", portStat[37],portStat[36]);
	vty_out(vty, "%-20s %-10u %-10u\n","goodFcRcv", portStat[41],portStat[40]);
	vty_out(vty, "%-20s %-10u %-10u\n","badFcRcv", portStat[61],portStat[60]);
	vty_out(vty, "%-20s %-10u %-10u\n","dropEvents", portStat[43],portStat[42]);
	vty_out(vty, "%-20s %-10u %-10u\n","undersizePkts", portStat[45],portStat[44]);
	vty_out(vty, "%-20s %-10u %-10u\n","fragmentsPkts", portStat[47],portStat[46]);
	vty_out(vty, "%-20s %-10u %-10u\n","oversizePkts", portStat[49],portStat[48]);
	vty_out(vty, "%-20s %-10u %-10u\n","jabberPkts", portStat[51],portStat[50]);
	vty_out(vty, "%-20s %-10u %-10u\n","badCrc", portStat[55],portStat[54]);
	vty_out(vty, "%-20s %-10u %-10u\n","macRcvError", portStat[53],portStat[52]);

	vty_out(vty, "-------------------- TX ------------------\n");	
	vty_out(vty, "%-20s %-10u %-10u\n","goodOctetsSent", portStat[27],portStat[26]);
	vty_out(vty, "%-20s %-10u %-10u\n","goodPktsSent", portStat[29],portStat[28]);
	vty_out(vty, "%-20s %-10u %-10u\n","ucPktsSent", portStat[65],portStat[64]);
	vty_out(vty, "%-20s %-10u %-10u\n","brdcPktsSent", portStat[35],portStat[34]);
	vty_out(vty, "%-20s %-10u %-10u\n","mcPktsSent", portStat[33],portStat[32]);
	vty_out(vty, "%-20s %-10u %-10u\n","fcSent", portStat[39],portStat[38]);
	vty_out(vty, "%-20s %-10u %-10u\n","multiplePktsSent", portStat[67],portStat[66]);
	vty_out(vty, "%-20s %-10u %-10u\n","deferredPktsSent", portStat[69],portStat[68]);
	vty_out(vty, "%-20s %-10u %-10u\n","macTransmitErr", portStat[5],portStat[4]);

	vty_out(vty, "-------------------- Both ----------------\n");	
	vty_out(vty, "%-20s %-10u %-10u\n","pkts64Octets", portStat[15],portStat[14]);
	vty_out(vty, "%-20s %-10u %-10u\n","pkts65to127Octets", portStat[17],portStat[16]);
	vty_out(vty, "%-20s %-10u %-10u\n","pkts128to255Octets", portStat[19],portStat[18]);
	vty_out(vty, "%-20s %-10u %-10u\n","pkts256to511Octets", portStat[21],portStat[20]);
	vty_out(vty, "%-20s %-10u %-10u\n","pkts512to1023Octets", portStat[23],portStat[22]);
	vty_out(vty, "%-20s %-10u %-10u\n","pkts1024tomaxOoctets", portStat[25],portStat[24]);

	vty_out(vty, "%-20s %-10u %-10u\n","collisions", portStat[57],portStat[56]);
	vty_out(vty, "%-20s %-10u %-10u\n","lateCollisions", portStat[59],portStat[58]);
	vty_out(vty, "%-20s %-10u %-10u\n","excessiveCollisions", portStat[31],portStat[30]);
	vty_out(vty, "------------------------------------------\n");
*/	
    #if 0	
	vty_out(vty, "%-15s %-10u %-10u\n","RxGoodPkts", portStat[1],portStat[0]);
	vty_out(vty, "%-15s %-10u %-10u\n","RxBadPkts", portStat[3],portStat[2]);
	vty_out(vty, "%-15s %-10u %-10u\n","RxGoodBytes", portStat[5],portStat[4]);
	vty_out(vty, "%-15s %-10u %-10u\n","RxBadBytes", portStat[7],portStat[6]);
	vty_out(vty, "%-15s %-10u %-10u\n","RxInternalDrop", portStat[9],portStat[8]);

	vty_out(vty, "%-15s %-10u %-10u\n","Rx_BC_pkts", portStat[11],portStat[10]);
	vty_out(vty, "%-15s %-10u %-10u\n","Rx_UC_pkts", portStat[13],portStat[12]);


	
	vty_out(vty, "%-15s %-10u %-10u\n","TxGoodPkts", portStat[15],portStat[14]);
	vty_out(vty, "%-15s %-10u %-10u\n","TxGoodBytes", portStat[17],portStat[16]);
	vty_out(vty, "%-15s %-10u %-10u\n","TxMacError", portStat[19],portStat[18]);

	vty_out(vty, "%-15s %-10u %-10u\n","Tx_BC_pkts", portStat[21],portStat[20]);
	vty_out(vty, "%-15s %-10u %-10u\n","Tx_UC_pkts", portStat[23],portStat[22]);
	
	vty_out(vty, "----------------------------------------\n");
	#endif
				
	dbus_message_unref(reply);
	/* end */
	return CMD_SUCCESS;  
}






DEFUN(diagnosis_get_conntable_mib_func,
		diagnosis_get_conntable_mib_cmd,
		"show connect_table mib",
		SHOW_STR
		"Show connect_table mib\n"
		"Show connect_table mib\n"
) 
{

    int slot_id,i,ret=-1;
	unsigned int local_portStat[70]={0},sum_portStat[70]={0};
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {

        int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
		
		int fd = -1;
		struct stat sb;
		asic_board_cscd_bport_t * mem_gbports_list = NULL;
    	char* file_path = "/dbm/shm/connect_table/shm_conntable";
		
    	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
        int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
        if((local_slot_id<0)||(slotNum<0))
        {
        	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
    		return CMD_WARNING;			
        }
        if(slotNum<6)
        {
        	vty_out(vty,"can`t run in 3 slot product !\n");
    		return CMD_WARNING;			
        }		
		
		/* only read file */
	    fd = open(file_path, O_RDONLY);
    	if(fd < 0)
        {
            vty_out(vty,"Failed to open! %s\n", strerror(errno));
            return NULL;
        }
		fstat(fd,&sb);
		/* map not share */	
        mem_gbports_list = (asic_board_cscd_bport_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0 );
        if(MAP_FAILED == mem_gbports_list)
        {
            vty_out(vty,"Failed to mmap for mem_gbports_list[]! %s\n", strerror(errno));
			close(fd);
            return NULL;
        }	  
		
        for(slot_id=0;slot_id<slotNum;slot_id++)
        {
			if(mem_gbports_list[slot_id].board_type == -1)
				continue;
			for(i=0;i<(mem_gbports_list[slot_id].asic_cscd_port_cnt+mem_gbports_list[slot_id].asic_to_cpu_ports);i++)
			{   
				if(mem_gbports_list[slot_id].asic_cscd_bports[i].master<3)
    			{
                	vty_out(vty, "-----------------------slot%d-port%d <===> ve%02d%s%d--------\n",\
                				            mem_gbports_list[slot_id].slot_id,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].cscd_port,\
                				            mem_gbports_list[slot_id].slot_id,\
                				            (mem_gbports_list[slot_id].asic_cscd_bports[i].master==1)?"f":"s",\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].bport); 
                	vty_out(vty, "-----------------------the asic-mib will return in next ver--------\n");
				}
			    else
                {   
                    ret=get_asic_mib(vty, mem_gbports_list[slot_id].asic_cscd_bports[i].master, \
    					                mem_gbports_list[slot_id].asic_cscd_bports[i].asic_id, \
                            mem_gbports_list[slot_id].asic_cscd_bports[i].bport, sum_portStat);
            		if( ret != 0 )
                    {
                        vty_out(vty,"get_asic_mib from SUM_slot%d-lion%d_port%d failed \n",
							                mem_gbports_list[slot_id].asic_cscd_bports[i].master,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].asic_id,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].bport);
						continue;
                    }
    				ret=get_asic_mib(vty, mem_gbports_list[slot_id].slot_id, 0, \
                            mem_gbports_list[slot_id].asic_cscd_bports[i].cscd_port, local_portStat);
            		if( ret != 0 )
                    {
                        vty_out(vty,"get_asic_mib local_portStat failed \n" );   
                    }

                	vty_out(vty, "------------------------------------------\n");	
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10s %-10s\n","DESCRIPTION","32BitH","32BitL","32BitH","32BitL");	
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10s %-10s\n","--------------------","----------","----------","----------","----------");
                	vty_out(vty, "                    slot%d-port%d           ===>=>   SUM_slot%d-lion%d_port%d\n",\
                				            mem_gbports_list[slot_id].slot_id,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].cscd_port,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].master,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].asic_id,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].bport);	
    	
                	vty_out(vty, "%-20s %-10u %-10u ===>    %-10u %-10u\n","goodOctetsRcv", local_portStat[27],local_portStat[26], sum_portStat[1],sum_portStat[0]);
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10u %-10u\n","badOctetsRcv", "-","-", sum_portStat[3],sum_portStat[2]);
                	vty_out(vty, "%-20s %-10u %-10u ===>    %-10u %-10u\n","goodPktsRcv", local_portStat[29],local_portStat[28], sum_portStat[7],sum_portStat[6]);
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10u %-10u\n","badPktsRcv", "-","-", sum_portStat[9],sum_portStat[8]);
                	vty_out(vty, "%-20s %-10u %-10u ===>    %-10u %-10u\n","ucPktsRcv", local_portStat[65],local_portStat[64], sum_portStat[63],sum_portStat[62]);
                	vty_out(vty, "%-20s %-10u %-10u ===>    %-10u %-10u\n","brdcPktsRcv", local_portStat[35],local_portStat[34], sum_portStat[11],sum_portStat[10]);	
                	vty_out(vty, "%-20s %-10u %-10u ===>    %-10u %-10u\n","mcPktsRcv", local_portStat[33],local_portStat[32], sum_portStat[13],sum_portStat[12]);
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10u %-10u\n","unrecogMacCntrRcv", "-","-", sum_portStat[37],sum_portStat[36]);
                	vty_out(vty, "%-20s %-10u %-10u ===>    %-10u %-10u\n","goodFcRcv", local_portStat[39],local_portStat[38], sum_portStat[41],sum_portStat[40]);
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10u %-10u\n","badFcRcv", "-","-", sum_portStat[61],sum_portStat[60]);
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10u %-10u\n","dropEvents", "-","-", sum_portStat[43],sum_portStat[42]);
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10u %-10u\n","undersizePkts", "-","-", sum_portStat[45],sum_portStat[44]);
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10u %-10u\n","fragmentsPkts", "-","-", sum_portStat[47],sum_portStat[46]);
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10u %-10u\n","oversizePkts", "-","-", sum_portStat[49],sum_portStat[48]);
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10u %-10u\n","jabberPkts", "-","-", sum_portStat[51],sum_portStat[50]);
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10u %-10u\n","badCrc", "-","-", sum_portStat[55],sum_portStat[54]);
                	vty_out(vty, "%-20s %-10s %-10s ===>    %-10u %-10u\n","macRcvError", "-","-", sum_portStat[53],sum_portStat[52]);

                	vty_out(vty, "                    slot%d-port%d           <=<==   SUM_slot%d-lion%d_port%d \n",\
                				            mem_gbports_list[slot_id].slot_id,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].cscd_port,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].master,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].asic_id,\
                				            mem_gbports_list[slot_id].asic_cscd_bports[i].bport);	
                	
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10u %-10u\n","goodOctetsRcv", local_portStat[1],local_portStat[0], sum_portStat[27],sum_portStat[26]);
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10s %-10s\n","badOctetsRcv", local_portStat[3],local_portStat[2], "-","-");
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10u %-10u\n","goodPktsRcv", local_portStat[7],local_portStat[6], sum_portStat[29],sum_portStat[28]);
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10s %-10s\n","badPktsRcv", local_portStat[9],local_portStat[8], "-","-");
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10u %-10u\n","ucPktsRcv", local_portStat[63],local_portStat[62], sum_portStat[65],sum_portStat[64]);
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10u %-10u\n","brdcPktsRcv", local_portStat[11],local_portStat[10], sum_portStat[35],sum_portStat[34]);	
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10u %-10u\n","mcPktsRcv", local_portStat[13],local_portStat[12], sum_portStat[33],sum_portStat[32]);
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10s %-10s\n","unrecogMacCntrRcv", local_portStat[37],local_portStat[36], "-","-");
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10u %-10u\n","goodFcRcv", local_portStat[41],local_portStat[40], sum_portStat[39],sum_portStat[38]);
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10s %-10s\n","badFcRcv", local_portStat[61],local_portStat[60], "-","-");
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10s %-10s\n","dropEvents", local_portStat[43],local_portStat[42], "-","-");
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10s %-10s\n","undersizePkts", local_portStat[45],local_portStat[44], "-","-");
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10s %-10s\n","fragmentsPkts", local_portStat[47],local_portStat[46], "-","-");
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10s %-10s\n","oversizePkts", local_portStat[49],local_portStat[48], "-","-");
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10s %-10s\n","jabberPkts", local_portStat[51],local_portStat[50], "-","-");
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10s %-10s\n","badCrc", local_portStat[55],local_portStat[54], "-","-");
                	vty_out(vty, "%-20s %-10u %-10u <===    %-10s %-10s\n","macRcvError", local_portStat[53],local_portStat[52], "-","-");





                	vty_out(vty, "-------------------- Both ----------------\n");	
                	vty_out(vty, "%-20s %-10u %-10u <==>    %-10u %-10u\n","pkts64Octets", local_portStat[15],local_portStat[14], sum_portStat[15],sum_portStat[14]);
                	vty_out(vty, "%-20s %-10u %-10u <==>    %-10u %-10u\n","pkts65to127Octets", local_portStat[17],local_portStat[16], sum_portStat[17],sum_portStat[16]);
                	vty_out(vty, "%-20s %-10u %-10u <==>    %-10u %-10u\n","pkts128to255Octets", local_portStat[19],local_portStat[18], sum_portStat[19],sum_portStat[18]);
                	vty_out(vty, "%-20s %-10u %-10u <==>    %-10u %-10u\n","pkts256to511Octets", local_portStat[21],local_portStat[20], sum_portStat[21],sum_portStat[20]);
                	vty_out(vty, "%-20s %-10u %-10u <==>    %-10u %-10u\n","pkts512to1023Octets", local_portStat[23],local_portStat[22], sum_portStat[23],sum_portStat[22]);
                	vty_out(vty, "%-20s %-10u %-10u <==>    %-10u %-10u\n","pkts1024tomaxOoctets", local_portStat[25],local_portStat[24], sum_portStat[25],sum_portStat[24]);

                	vty_out(vty, "%-20s %-10u %-10u <==>    %-10u %-10u\n","collisions", local_portStat[57],local_portStat[56], sum_portStat[57],sum_portStat[56]);
                	vty_out(vty, "%-20s %-10u %-10u <==>    %-10u %-10u\n","lateCollisions", local_portStat[59],local_portStat[58], sum_portStat[59],sum_portStat[58]);
                	vty_out(vty, "%-20s %-10u %-10u <==>    %-10u %-10u\n","excessiveCollisions", local_portStat[31],local_portStat[30], sum_portStat[31],sum_portStat[30]);
                	vty_out(vty, "--------------------------------------------------------------------------------\n");
    			
    			}
			}
	           
        }
		vty_out(vty,"\n=====================================================================\n");

        ret = munmap(mem_gbports_list,sb.st_size);
        if( ret != 0 )
        {
            vty_out(vty,"Failed to munmap for mem_gbports_list[]! %s\n", strerror(errno));			
        }	
		ret = close(fd);
		if( ret != 0 )
        {
            vty_out(vty,"close shm_conntable failed \n" );   
        }
        /*free(vlanName);*/
        return CMD_SUCCESS;		
}
}

void dcli_diag_init(void) {
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_phy_read_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_phy_write_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&diagnosis_hw_xaui_read_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&diagnosis_hw_xaui_write_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_pci_read_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_pci_write_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&diagnosis_hw_cpu_read_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&diagnosis_hw_cpu_write_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_cpld_read_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_cpld_write_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_mac_read_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_mac_write_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_read_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_write_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_asic_cscd_port_prbs_test_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_asic_prbs_test_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_asic_port_mode_set_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_dump_tab_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_field_dump_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_cosq_dump_cmd);
	install_element(HIDDENDEBUG_NODE,&Diagnosis_hw_acl_show_cmd);
	install_element(HIDDENDEBUG_NODE,&show_mib_ge_xg_cmd);
	install_element(HIDDENDEBUG_NODE,&diagnosis_hw_eeprom_read_reg_cmd);
	install_element(HIDDENDEBUG_NODE,&diagnosis_hw_eeprom_write_reg_cmd);
#if 0
	/* add methods of debug for eag */
	install_element(HIDDENDEBUG_NODE,&config_eag_debug_cmd);
	install_element(HIDDENDEBUG_NODE,&config_eag_no_debug_cmd);
	install_element(HIDDENDEBUG_NODE,&config_eag_debug_pkt_info_cmd);
	install_element(HIDDENDEBUG_NODE,&config_eag_no_debug_pkt_info_cmd);
#endif	
	install_element(HIDDENDEBUG_NODE,&diagnosis_hw_watchdog_ctrl_cmd);
	install_element(HIDDENDEBUG_NODE,&diagnosis_hw_watchdog_timeout_cmd);
	install_element(HIDDENDEBUG_NODE,&diagnosis_hw_watchdog_timeout_dflt_cmd);
	install_element(HIDDENDEBUG_NODE,&diagnosis_hw_watchdog_timeout_show_cmd);

	install_element(HIDDENDEBUG_NODE, &diagnosis_get_gindex_cmd);
	install_element(HIDDENDEBUG_NODE, &diagnosis_set_gindex_cmd);

    /* Hardware test */
	install_element(HIDDENDEBUG_NODE, &test_ge_xg_cmd);	
	install_element(HIDDENDEBUG_NODE, &test_lion1_trunk_cmd);	
	install_element(HIDDENDEBUG_NODE, &test_1x12g12s_trunk_cmd);
	install_element(HIDDENDEBUG_NODE, &show_CPU_temperature_cmd);
	install_element(HIDDENDEBUG_NODE, &test_master_change_cmd);	
    install_element(HIDDENDEBUG_NODE, &diagnosis_hw_ax8610_prbs_test_cmd);	
    install_element(HIDDENDEBUG_NODE, &diagnosis_create_vlan_cmd);	
    install_element(HIDDENDEBUG_NODE, &diagnosis_vlan_add_port_cmd);	
    install_element(HIDDENDEBUG_NODE, &diagnosis_read_port_mib_cmd);	
	install_element(HIDDENDEBUG_NODE, &diagnosis_read_asic_speed_cmd);	
	install_element(HIDDENDEBUG_NODE, &diagnosis_endis_asic_cmd);
	install_element(HIDDENDEBUG_NODE, &diagnosis_port_cscd_set_cmd);
	install_element(HIDDENDEBUG_NODE, &diagnosis_vlan_fdb_delete_cmd);
	install_element(HIDDENDEBUG_NODE, &diagnosis_show_cscd_port_status_cmd);
	install_element(HIDDENDEBUG_NODE, &diagnosis_board_test_cmd);
	install_element(HIDDENDEBUG_NODE, &Diagnosis_set_ap_fake_singal_strength_cmd);
	install_element(HIDDENDEBUG_NODE, &Diagnosis_set_ap_fake_singal_strength_default_cmd);
	
	install_element(HIDDENDEBUG_NODE, &diagnosis_get_conntable_cmd);
	install_element(HIDDENDEBUG_NODE, &diagnosis_get_conntable_mib_cmd);
	
	
}

#ifdef __cplusplus
}
#endif

