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
* dcli_rpa.c
*
*
* CREATOR:
*		caojia@autelan.com
*
* DESCRIPTION:
*		CLI definition for rpa module.
*
* DATE:
*		4/13 2012
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.33 $	
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif
#include <string.h>
#include <sys/socket.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <dbus/rpa/rpa_dbus_def.h>
#include <sys/wait.h>

#include "command.h"
#include "dcli_main.h"
#include "dcli_rpa.h"

unsigned char str2hexnum(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return 0; /* foo */
}

unsigned long str2hex(unsigned char *str)
{
	int value = 0;

	while (*str) {
		value = value << 4;
		value |= str2hexnum(*str++);
	}

	return value;
}


DEFUN (rpa_set_broadcast_mask,
       rpa_set_broadcast_mask_cmd,
       "rpa set broadcast-mask MASK slot SLOT",
       "Rpa module command\n"
       "Set action\n"
       "Set the mask which rpa module broadcast specific packet according to\n"
       "Mask value\n"
       "Slot\n"
       "Slot id in product\n")
{
	DBusMessage *query, *reply;
	DBusError err;

	int ret;
	FILE *fd;
	unsigned int mask = 0x0;
	unsigned int max_mask = 0x0;
	int slot_id;
	int slot_count;
	int i;

	fd = fopen("/dbm/product/slotcount", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [2] error\n");
		return -1;
	}
	fscanf(fd, "%d", &slot_count);
	fclose(fd);

	for (i = 0; i < slot_count; i++) {
		max_mask |= (1 << i);
	}
	
	if (!strncmp(argv[0], "0x", 2)) {
		mask = str2hex((unsigned char *)(argv[0] + 2));
	}
	else {
		sscanf(argv[0], "%d", &mask);
	}

	if ((mask & (~max_mask)) > 0) {
		vty_out(vty, "Input mask value is illegal. Max mask value on current product is %#x .\n", max_mask);
		return CMD_WARNING;
	}

	if (((strlen(argv[1]) == 1) && (argv[1][0] <= '9') && (argv[1][0] > '0')) || \
		(strlen(argv[1]) == 2) && (argv[1][0] == '1') && (argv[1][1] == '0'))
	{
		sscanf(argv[1],"%d", &slot_id);
		//vty_out(vty, "Slot_id : %d\n", slot_id);

		if (slot_id > slot_count)
		{
			vty_out(vty, "slot number range on this product : 1 ~ %d\n", slot_count);

			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty, "error slot number : %s\n", argv[1]);

		return CMD_WARNING;
	}

	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		char *slot_num[10] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
		char confirm[64] = {0};
		int flag = 0;
		
		vty_out(vty, "+----------------");
		for (i = 0; i < slot_count; i++) {
			vty_out(vty, "------");
		}
		vty_out(vty, "+\n");
		vty_out(vty, "|    %-4s     |", "To Slot");
		for (i = 0; i < slot_count; i++) {
			vty_out(vty, "  %-2s |", slot_num[i]);
		}
		vty_out(vty, "\n");
		vty_out(vty, "+----------------");
		for (i = 0; i < slot_count; i++) {
			vty_out(vty, "------");
		}
		vty_out(vty, "+\n");
		vty_out(vty, "|%s|", "Broadcast Enable");
		for (i = 0; i < slot_count; i++) {
			vty_out(vty, "  %-2s |", ((mask >> i) & 0x1) > 0?"Y":"N");
		}
		vty_out(vty, "\n");
		vty_out(vty, "+----------------");
		for (i = 0; i < slot_count; i++) {
			vty_out(vty, "------");
		}
		vty_out(vty, "+\n");
		vty_out(vty, "\n");

		#if 0
		vty_out(vty, "Set the new broadcast-mask to Slot %d Rpa-module. Confirm ? [yes/no]:\n", slot_id);
		fscanf(stdin, "%s", confirm);

		while (1) {
			if (!strncasecmp("yes", confirm, strlen(confirm))){
				vty_out(vty,"Start setting, please wait...\n");
				flag = 1;
				break;
			}
			else if (!strncasecmp("no", confirm, strlen(confirm))) {
				break;
			}
			else{
				vty_out(vty,"% Please answer 'yes' or 'no'.\n");
				vty_out(vty,"Confirm ? [yes/no]:\n");
				memset(confirm, 0, 64);
				fscanf(stdin, "%s", confirm);
			}
		}
		#endif
		flag = 1;

		if (flag) {
			query = dbus_message_new_method_call(RPA_DAEMON_DBUS_BUSNAME, RPA_DAEMON_DBUS_OBJPATH,
												RPA_DAEMON_DBUS_INTERFACE, RPA_DAEMON_DBUS_SET_BROADCAST_MASK);
			dbus_error_init(&err);

			dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mask,
									DBUS_TYPE_INVALID);

			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
			
			dbus_message_unref(query);

			if (NULL == reply){
				vty_out(vty,"<error> failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}

			if (dbus_message_get_args (reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
				if(ret == 0){
					vty_out(vty,"Successful\n");
				}
				else
				{
					vty_out(vty,"Failed\n");
					return CMD_WARNING;
				}
			}
		}
	}
	else {
		vty_out(vty, "Dbus connection to Slot %d is not available.\n", slot_id);
	}

	return CMD_SUCCESS;
}

int get_rpa_broadcast_mask(struct vty *vty, int slot_id, unsigned int *mask, int *is_default_mask)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int slot_mask;
	int is_default;
	int ret;
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		query = dbus_message_new_method_call(RPA_DAEMON_DBUS_BUSNAME, RPA_DAEMON_DBUS_OBJPATH,
													 RPA_DAEMON_DBUS_INTERFACE, RPA_DAEMON_DBUS_SHOW_BROADCAST_MASK);
		dbus_error_init(&err);

		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
		
		dbus_message_unref(query);

		if (NULL == reply) {
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32, &ret,
						DBUS_TYPE_UINT32, &slot_mask,
						DBUS_TYPE_UINT32, &is_default,
						DBUS_TYPE_INVALID)) {
			if (ret == 0) {
				*mask = slot_mask;
				*is_default_mask = is_default;
				return CMD_SUCCESS;
			}
			else {
				vty_out(vty,"Failed\n");
				return CMD_WARNING;
			}
		}
	}
	else {
		vty_out(vty, "Dbus connection to Slot %d is not available.\n", slot_id);
		return CMD_WARNING;
	}

	return CMD_WARNING;
}

DEFUN (rpa_show_broadcast_mask,
       rpa_show_broadcast_mask_cmd,
       "show rpa broadcast-mask slot SLOT",
       "\n"
       "Rpa module command\n"
       "Show the mask which rpa module broadcast specific packet according to\n"
       "Slot\n"
       "Slot id in product\n")
{
	DBusMessage *query, *reply;
	DBusError err;

	int ret;
	FILE *fd;
	unsigned int mask = 0x0;
	int is_default;
	int slot_id;
	int slot_count;
	int i;

	fd = fopen("/dbm/product/slotcount", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [2] error\n");
		return -1;
	}
	fscanf(fd, "%d", &slot_count);
	fclose(fd);

	if (((strlen(argv[0]) == 1) && (argv[0][0] <= '9') && (argv[0][0] > '0')) || \
		(strlen(argv[0]) == 2) && (argv[0][0] == '1') && (argv[0][1] == '0'))
	{
		sscanf(argv[0],"%d", &slot_id);
		//vty_out(vty, "Slot_id : %d\n", slot_id);

		if (slot_id > slot_count)
		{
			vty_out(vty, "slot number range on this product : 1 ~ %d\n", slot_count);

			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty, "error slot number : %s\n", argv[0]);

		return CMD_WARNING;
	}

	ret = get_rpa_broadcast_mask(vty, slot_id, &mask, &is_default);
	if (ret == CMD_SUCCESS) {
		char *slot_num[10] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
		char confirm[64] = {0};

		vty_out(vty, "Broadcast mask on Slot %d is %#x\n", slot_id, mask);
		vty_out(vty, "+----------------");
		for (i = 0; i < slot_count; i++) {
			vty_out(vty, "------");
		}
		vty_out(vty, "+\n");
		vty_out(vty, "|    %-4s     |", "To Slot");
		for (i = 0; i < slot_count; i++) {
			vty_out(vty, "  %-2s |", slot_num[i]);
		}
		vty_out(vty, "\n");
		vty_out(vty, "+----------------");
		for (i = 0; i < slot_count; i++) {
			vty_out(vty, "------");
		}
		vty_out(vty, "+\n");
		vty_out(vty, "|%s|", "Broadcast Enable");
		for (i = 0; i < slot_count; i++) {
			vty_out(vty, "  %-2s |", ((mask >> i) & 0x1) > 0?"Y":"N");
		}
		vty_out(vty, "\n");
		vty_out(vty, "+----------------");
		for (i = 0; i < slot_count; i++) {
			vty_out(vty, "------");
		}
		vty_out(vty, "+\n");
	}
	else {
		return ret;
	}

	return CMD_SUCCESS;
}

DEFUN (rpa_show_dev_index_table,
       rpa_show_dev_index_table_cmd,
       "show rpa dev-index-table",
       "\n"
       "Rpa module command\n"
       "Show dev-index-table in RPA module\n"
       )
{
	DBusMessage *query, *reply;
	DBusError err;

	int ret;

	if (dcli_dbus_connection) {
		query = dbus_message_new_method_call(RPA_DAEMON_DBUS_BUSNAME, RPA_DAEMON_DBUS_OBJPATH,
													 RPA_DAEMON_DBUS_INTERFACE, RPA_DAEMON_DBUS_SHOW_DEV_INDEX_TABLE);
		dbus_error_init(&err);

		reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
		
		dbus_message_unref(query);

		if (NULL == reply) {
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32, &ret,
						DBUS_TYPE_INVALID)) {
			if (ret == 0) {
				vty_out(vty, "Success\n");
				return CMD_SUCCESS;
			}
			else {
				vty_out(vty,"Failed\n");
				return CMD_WARNING;
			}
		}
	}
	else {
		vty_out(vty, "Dbus connection is not available.\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

void dcli_rpa_init(void)
{
	//install_element(ENABLE_NODE, &rpa_set_broadcast_mask_cmd);
	//install_element(ENABLE_NODE, &rpa_show_broadcast_mask_cmd);
  
	install_element(CONFIG_NODE, &rpa_set_broadcast_mask_cmd);
	install_element(CONFIG_NODE, &rpa_show_broadcast_mask_cmd);
	install_element(HIDDENDEBUG_NODE, &rpa_show_dev_index_table_cmd);
}

#ifdef __cplusplus
}
#endif
