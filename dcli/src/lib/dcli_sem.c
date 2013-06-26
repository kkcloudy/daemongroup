/******************************************************************************
Copyright (C) Autelan Technology
This software file is owned and distributed by Autelan Technology 
*******************************************************************************
* dcli_wsm.c
*
*
* DESCRIPTION:
*  SEM module Dbus implement.
*
* DATE:
*  2011-04-07
*
* CREATOR:
*  zhangdx@autelan.com
*
*
* CHANGE LOG:
*  2011-04-07 <zhangdx> Create file.
*
******************************************************************************/
#ifndef _D_SEM_
#define _D_SEM_

#include <string.h>
#include <sys/socket.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <dbus/sem/sem_dbus_def.h>
#include <sys/wait.h>

#include "command.h"
#include "dcli_main.h"
#include "dcli_sem.h"
#include "dcli_system.h"
#include "board/board_define.h"
#include "bsd/bsdpub.h"

#define MAX_MASTER_SLOT_NUM 2
#define PRODUCT_NAME_LEN 30

extern char BSD_DBUS_BUSNAME[PATH_LEN];
extern char BSD_DBUS_OBJPATH[PATH_LEN];
extern char BSD_DBUS_INTERFACE[PATH_LEN];

extern char BSD_COPY_FILES_BETEWEEN_BORADS[PATH_LEN];
extern char BSD_SYNCHRONIZE_FILES_TO_OTHER_BOARDS[PATH_LEN];

extern int is_distributed;

/*
 * get slot_id from /proc/board/slot_id
 * return : -1, get slot_id failed
 * add by caojia
 */
int dcli_slot_id_get(void)
{
	FILE *fd;
	int slot_id = -1;
	char temp_buf[4];

	fd = fopen("/proc/product_info/board_slot_id", "r");
	if (fd == NULL)
	{
		return -1;
	}
	fread(temp_buf, sizeof(char), 4, fd);
	if ((temp_buf[0] > '0') && (temp_buf[0] <= '9'))
	{
		slot_id = temp_buf[0] - '0';
		if (temp_buf[1] == '\0')
		{
			fclose(fd);
			return slot_id;
		}
		else if ((temp_buf[1] > '0') && (temp_buf[1] <= '9') && (temp_buf[2] == '\0'))
		{
			slot_id = (slot_id * 10) + (temp_buf[1] - '\0');
			fclose(fd);
			return slot_id;
		}
	}
	fclose(fd);

	return slot_id;
}

int dcli_master_slot_id_get(int *master_slot_id)
{
    FILE *fd;
	char buff[8][8];
	int i;
	fd = fopen("/dbm/product/master_slot_id", "r");
	if (fd == NULL)
	{
		return -1;
	}
	
	for(i = 0; i < 2; i++)
	{
		if(!fgets(buff[i], 8, fd))
			printf("read error no value\n");
		master_slot_id[i] = strtoul(buff[i], NULL, 10);
	}
	
	fclose(fd);
    
	return 0;	
}
int get_product_info(char *filename)
{
	int fd;
	char buff[16] = {0};
	unsigned int data;

	if(filename == NULL)
	{
		return -1;
	}

	fd = open(filename, O_RDONLY, 0);
	if (fd >= 0) 
	{
		if(read(fd, buff, 16) < 0) 
			printf("Read error : no value\n");
		/* for bug: AXSSZFI-292, "popen failed for pager: Too many open files" */
		close(fd);
	}
	else
	{        
		printf("Open file:%s error!\n",filename);
		return -1;
	}

	data = strtoul(buff, NULL, 10);

	return data;
}

int get_str_from_file(char *filename, char *buff)
{
	int fd;

	if((filename == NULL) || (buff == NULL))
	{
		return -1;
	}

	fd = fopen(filename, "r");
	if (fd == NULL)
	{
        printf("Open file:%s error!\n",filename);
		return -1;
	}

    fscanf(fd, "%s", buff);
    fclose(fd);
    return 0;
}

int get_int_from_file(char *filename)
{
	int fd;
	unsigned int data;

	if(filename == NULL)
	{
		return -1;
	}

	fd = fopen(filename, "r");
	if (fd == NULL)
	{
        printf("Open file:%s error!\n",filename);
		return -1;
	}
	
	fscanf(fd, "%d", &data);
	fclose(fd);

	return data;
}


DBusMessage *dbus_sem_msg_new_method_call(unsigned char *dbus_name, \
                                      unsigned char *obj_name,  \
                                      unsigned char *if_name,   \
                                      unsigned char *cmd_name);
/*clx*/
int atoi(const char *str)
{
    unsigned int value=0;
    int b_plus=1;

    switch(*str)
    {
    case '+':
        str++;
        break;
    case '-':
        b_plus=0;
        str++;
        break;
    default:
        break;
    }

    while('\0' != *str)
    {
        value = (value*10)+(*str-'0');
        str++;
    }
    if(!b_plus)
        value=-value;
    return value;
}

DEFUN(config_tipc_cmd_func,
		config_tipc_cmd,
		"send tipc to slot SLOT_ID NUM USLEEP_TM",
		"send tipc to slot SLOT_ID NUM USLEEP_TM\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	
	int slot_id, num, usleep_tm;
	if (argc != 3)
	{
		vty_out(vty, "input:send tipc to slot SLOT_ID NUM USLEEP_TM BUF\n");
		return CMD_FAILURE;
	}

	if (parse_slot_no(argv[0], &slot_id))
	{
		vty_out(vty, "wrong slot id\n");
		return CMD_FAILURE;
	}
	num = atoi(argv[1]);
	usleep_tm = atoi(argv[2]);
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_CONF_TIPC);
	if (!query)
	{
		vty_out(vty, "show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_INT32,&slot_id,
							 DBUS_TYPE_INT32,&num,
							 DBUS_TYPE_INT32,&usleep_tm,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		vty_out(vty, "<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(config_send_file_func,
		config_send_file,
		"send file",
		"send file")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	system("sor.sh imgmd5 /home/admin/testfile 120 > md5");
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_SEND_FILE);
	if (!query)
	{
		vty_out(vty, "show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		vty_out(vty, "<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_slot_n_func,
		config_slot_n_cmd,
		"config slot N",
		"config slot n")
{
	int slot_id, slotcount;
	int fd;
	char c;
	char str[25]={0};
	
	fd = open("/dbm/product/product_type", O_RDONLY);
	if(fd < 0)
	{
		vty_out(vty, "file /dbm/product/product_type open failed\n");
		return -1;
	}
	read(fd, &c, 1);
	close(fd);
	switch (c-'0')
	{
		case 0x1:/*7605i*/
		case 0x6:/*8603*/
			slotcount = 3;break;
		case 0x4:/*8610*/
			slotcount = 10;break;
		case 0x5:/*8606*/
			slotcount = 6;break;
		case 0x7:/*8800*/
			slotcount = 14;break;
		default:
			break;
	}

	if (argc != 1)
	{
		vty_out(vty, "not appoint slot id\n");
		return CMD_FAILURE;
	}

	if (parse_slot_no(argv[0], &slot_id))
	{
		vty_out(vty, "wrong slot id\n");
		return CMD_FAILURE;
	}

	if (slot_id < 1 || slot_id > slotcount)
	{
		vty_out(vty, "wrong slot id, no such slot\n");
		return CMD_FAILURE;
	}

	sprintf(str, "cli_tunnel_client %d", slot_id);
	system(str);

	return CMD_SUCCESS;
}

DEFUN(config_48GE_cmd_func,
		config_48GE_cmd,
		"config 48GE",
		"config 48GE")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	char* name;

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_CONF_48GE);
	if (!query)
	{
		vty_out(vty, "show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		vty_out(vty, "<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&name);

	if (!strcmp(name, "AS6648"))
	{
		system("cli_tunnel_client 3");
	}
	else
	{
		vty_out(vty, "Error:device in slot 3 is not AS6648 or not work normal\n");
		return CMD_FAILURE;
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_sem_cmd_func,
	  config_sem_cmd,
	  "config sem-tunnel",
	  CONFIG_STR
	  "SEM tunnel information\n"
	  "config SEM tunnel\n"
	 )
{
	if (vty->node == CONFIG_NODE)
                vty->node = CONFIG_NODE;
	else
		return CMD_FAILURE;
		
	return CMD_SUCCESS;
}

DEFUN(show_slot_id_func,
	show_slot_id_cmd,
	"show slot id",
	"show slot id"
	"config node\n"
	"slot id\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	int slot_id;
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_SLOT_ID);
	if (!query)
	{
		vty_out(vty, "show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		vty_out(vty, "<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&slot_id);
	
	vty_out(vty, "%d\n", slot_id);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
__inline__ int parse_slot_no(char *str, int *slot_no)
{
	int i = 0, len = 0;
	
	if(str)
		len=strlen(str);
	else 
		return -1;
	
	if ((len == 0) || (len > 2))
	{
		return -1;
	}

	for (i=0; i<len; i++)
	{
		if (!(*(str+i)<='9' && *(str+i)>='0'))
		{
			return -1;
		}
	}

	if (len == 1)
	{
		*slot_no = *(str+0) - '0';
	}
	else
	{
		*slot_no = (*(str+0) - '0') * 10 + *(str+1) - '0';
	}
	return 0;
}

DEFUN(show_slot_n_info_func,
	show_slot_n_info_cmd,
	"show slot SLOT_NO info",
	"show slot n info\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	int board_code;
	int is_master, is_active_master;
	unsigned int function_type;
	char *name;
	
	int slot_id;

	if (argc != 1)
	{
		vty_out(vty, "not appoint slot id\n");
		return CMD_FAILURE;
	}

	if (parse_slot_no(argv[0], &slot_id))
	{
		vty_out(vty, "wrong slot id\n");
		return CMD_FAILURE;
	}

	// TODO: the slot id should be smaller than some value

	//vty_out(vty, "slot id %d\n", slot_id);
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SME_DBUS_SHOW_SLOT_N_INFO);
	if (!query)
	{
		vty_out(vty, "show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_INT32,&slot_id,
							 DBUS_TYPE_INVALID);

	//vty_out(vty, "slot id %d\n", slot_id);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		vty_out(vty, "<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter, &board_code);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &function_type);
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_get_basic(&iter, &is_master);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &is_active_master);
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_get_basic(&iter, &name);

	vty_out(vty, "BOARD_CODE:\t\t0x%x\n", board_code);
	vty_out(vty, "FUNCTION_TYPE:\t\t0x%x\n", function_type);
	vty_out(vty, "IS_MASTER:\t\t%s\n", is_master ? "YES" : "NO");
	vty_out(vty, "IS_ACTIVE_MASTER:\t%s\n", is_active_master ? "YES" : "NO");
	vty_out(vty, "BOARD_NAME:\t\t%s\n", name);
	
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

DEFUN(show_board_info_func,
	show_board_info_cmd,
	"show board info",
	"show board info"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	int board_code;
	char *name;
	int slot_id;
	int is_master, is_active_master;
	unsigned int port_num_on_panel, function_type, obc_port_num, cscd_port_num;
	
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
											 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_BOARD_INFO);
	if (!query)
	{
		vty_out(vty, "show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		vty_out(vty, "<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter, &board_code);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &cscd_port_num);
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_get_basic(&iter, &function_type);
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_get_basic(&iter, &is_active_master);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &is_master);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &name);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &obc_port_num);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &port_num_on_panel);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &slot_id);

	vty_out(vty, "BOARD_CODE:\t\t%d\n", board_code);
	vty_out(vty, "CSCD_PORT_NUM:\t\t0x%x\n", cscd_port_num);
	vty_out(vty, "FUNCTION_TYPE:\t\t0x%x\n", function_type);
	vty_out(vty, "IS_ACTIVE_MASTER:\t%s\n", is_active_master ? "yes" : "no");
	vty_out(vty, "IS_MASTER:\t\t%s\n", is_master ? "yes" : "no");
	vty_out(vty, "NAME:\t\t\t%s\n", name);
	vty_out(vty, "OBC_PORT_NUM:\t\t0x%x\n", obc_port_num);
	vty_out(vty, "PORT_NUM_ON_PANEL:\t0x%x\n", port_num_on_panel);
	vty_out(vty, "SLOT_ID:\t\t%d\n", slot_id);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(show_product_info_func,
	show_product_info_cmd,
	"show product info",
	"show product info"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	int product_type, slotcount, master_slot_count, default_master_slot_id;
	int more_than_one_master_board_on, is_distributed;
	int fan_num, master_slot_id[MAX_MASTER_SLOT_NUM];
	unsigned int board_on_mask;
	char *name;
	int i;

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
											 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_PRODUCT_INFO);
	if (!query)
	{
		vty_out(vty, "show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		vty_out(vty, "<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter, &board_on_mask);
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_get_basic(&iter, &default_master_slot_id);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &fan_num);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &is_distributed);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &master_slot_count);
	dbus_message_iter_next(&iter);

	for (i=0; i<MAX_MASTER_SLOT_NUM; i++)
	{
		dbus_message_iter_get_basic(&iter, &master_slot_id[i]);
		dbus_message_iter_next(&iter);
	}
	
	dbus_message_iter_get_basic(&iter, &more_than_one_master_board_on);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &name);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &product_type);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &slotcount);
	dbus_message_iter_next(&iter);

	vty_out(vty, "BOARD_ON_MASK:\t\t\t0x%x\n", board_on_mask);
	vty_out(vty, "DEFAULT_MASTER_SLOT_ID:\t\t0x%x\n", default_master_slot_id);
	vty_out(vty, "FAN_NUM:\t\t\t0x%x\n", fan_num);
	vty_out(vty, "IS_DISTRIBUTED:\t\t\t%s\n", is_distributed ? "distributed" : "not distributed");
	vty_out(vty, "MASTER_SLOT_COUNT:\t\t0x%x\n", master_slot_count);
	vty_out(vty, "MASTER_SLOT_ID:\t\t\t");
	for (i=0; i<MAX_MASTER_SLOT_NUM; i++)
	{
		vty_out(vty, "0x%x ", master_slot_id[i]);
	}
	vty_out(vty, "\n");
	vty_out(vty, "MORE_THAN_ONE_MSATER_BOARD_ON:\t%s\n", more_than_one_master_board_on ? "yes" : "no");
	vty_out(vty, "BOARD_NAME:\t\t\t%s\n", name);
	vty_out(vty, "PRODUCT_TYPE:\t\t\t0x%x\n", product_type);
	vty_out(vty, "SLOT_COUNT:\t\t\t0x%x\n", slotcount);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(show_slot_info_func,
	show_slot_info_cmd,
	"show slot info",
	"show slot info"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	int i;
	int board_code;
	int is_master, is_active_master;
	unsigned int function_type;
	char *name;
	int slot_count;
	unsigned int board_on_mask;
	int board_state;
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_SLOT_INFO);
	if (!query)
	{
		vty_out(vty, "show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		vty_out(vty, "<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter, &slot_count);	
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &board_on_mask);	
	dbus_message_iter_next(&iter);
	
	for (i=0; i<slot_count; i++)
	{	
		dbus_message_iter_get_basic(&iter, &board_state);	
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &board_code);
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &function_type);
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter, &is_master);
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &is_active_master);
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter, &name);
		if (i < slot_count-1)
		{
			dbus_message_iter_next(&iter);
		}
		
		if (board_on_mask & (0x1<<i))
		{
			
			if (board_state <= 1)
			{
				vty_out(vty, "slot %d:not work normal\n", i+1);
				continue;
			}
			vty_out(vty, "slot %d:\n", i+1);
			vty_out(vty, "\tBOARD_CODE:\t\t0x%x\n", board_code);
			vty_out(vty, "\tFUNCTION_TYPE:\t\t0x%x\n", function_type);
			vty_out(vty, "\tIS_MASTER:\t\t%s\n", is_master ? "YES" : "NO");
			vty_out(vty, "\tIS_ACTIVE_MASTER:\t%s\n", is_active_master ? "YES" : "NO");
			vty_out(vty, "\tBOARD_NAME:\t\t%s\n", name);
		}
		else
		{
			vty_out(vty, "slot %d is empty\n", i+1);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(show_slot_info_history_func,
	show_slot_info_history_cmd,
	"show slot info history",
	"show slot info history"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	int i;
	int board_code;
	int is_master, is_active_master;
	unsigned int function_type;
	char *name;
	int slot_count;
	unsigned int board_on_mask;
	int board_state;
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_SLOT_INFO);
	if (!query)
	{
		vty_out(vty, "show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		vty_out(vty, "<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter, &slot_count);	
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &board_on_mask);	
	dbus_message_iter_next(&iter);
	
	for (i=0; i<slot_count; i++)
	{	
		dbus_message_iter_get_basic(&iter, &board_state);	
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &board_code);
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &function_type);
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter, &is_master);
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &is_active_master);
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter, &name);
		if (i < slot_count-1)
		{
			dbus_message_iter_next(&iter);
		}
			
		if (board_on_mask & (0x1<<i))
		{
			
			if (board_state <= 1)
			{
				vty_out(vty, "slot %d:not work normal\n", i+1);
				continue;
			}
			vty_out(vty, "slot %d:\n", i+1);
			vty_out(vty, "\tBOARD_CODE:\t\t0x%x\n", board_code);
			vty_out(vty, "\tFUNCTION_TYPE:\t\t0x%x\n", function_type);
			vty_out(vty, "\tIS_MASTER:\t\t%s\n", is_master ? "YES" : "NO");
			vty_out(vty, "\tIS_ACTIVE_MASTER:\t%s\n", is_active_master ? "YES" : "NO");
			vty_out(vty, "\tBOARD_NAME:\t\t%s\n", name);
		}
		else
		{
			if (board_state == BOARD_INSERTED_AND_REMOVED)
			{
				vty_out(vty, "slot %d:\t\thistory board\n", i+1);
    			vty_out(vty, "\tBOARD_CODE:\t\t0x%x\n", board_code);
    			vty_out(vty, "\tFUNCTION_TYPE:\t\t0x%x\n", function_type);
    			vty_out(vty, "\tIS_MASTER:\t\t%s\n", is_master ? "YES" : "NO");
    			vty_out(vty, "\tIS_ACTIVE_MASTER:\t%s\n", is_active_master ? "YES" : "NO");
    			vty_out(vty, "\tBOARD_NAME:\t\t%s\n", name);
			}
			else{
			    vty_out(vty, "slot %d is empty\n", i+1);
			}
		}

	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN (system_sync_version_file_to_slot,
       system_sync_version_file_to_slot_cmd,
       "sync_version VERSION_FILE_NAME slot SLOT_ID",
       "sync_version AWX.X.XX.XXXXX.IMG slot 2\n"
       "sync_version AWX.X.XX.XXXXX.IMG slot 2\n")
{
	char file_path_temp[64] = "/blk/";
	char *src_file_path = file_path_temp;
	char *dst_file_path = file_path_temp;
	char version_file_temp[64];
	char *version_file = version_file_temp;
	int file_path_len = 0;
	DBusMessage *query, *reply, *query_sem, *reply_sem;
	DBusError err, err_sem;
	unsigned int ret;
	int slot_id = 0;
	int local_slot_id = 0;
	int tar_switch = 0;
	int op = BSD_TYPE_BOOT_IMG;
	char *resultMd5 = NULL;

	if(strncasecmp((argv[0]+strlen(argv[0])-4),".IMG",4))
	{
		vty_out(vty,"The boot file should be .img file\n");

		return CMD_WARNING;
	}

	memset(version_file_temp, 0, 64);
	strcpy(version_file_temp, argv[0]);

	strcat(file_path_temp, argv[0]);
	sscanf(argv[1],"%d",&slot_id);

	file_path_len = strlen(src_file_path);
	local_slot_id = dcli_slot_id_get();

	if ((slot_id <= 0) || (slot_id > 10) || (slot_id == local_slot_id))
	{
		vty_out(vty, "SLOT_ID %d illegal\n", slot_id);
		return CMD_SUCCESS;
	}

	if ((local_slot_id <= 0) || (local_slot_id >= 10))
	{
		vty_out(vty, "GET LOCAL SLOT_ID FAILED\n");
		return CMD_SUCCESS;
	}

	if(file_path_len >= 64){		
		vty_out(vty,"<error> file path name is too long,should be less than 64\n");
		return CMD_SUCCESS;
	}
    
    //printf("dbus_name = %s\ndbus_objpath = %s\ndbus_interface = %s\n",BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,BSD_DBUS_INTERFACE);
    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
						BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
    
	dbus_error_init(&err);
    
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_STRING,&src_file_path,
							 DBUS_TYPE_STRING,&dst_file_path,
							 DBUS_TYPE_UINT32,&tar_switch,
							 DBUS_TYPE_UINT32,&op,
							 DBUS_TYPE_INVALID);

    vty_out(vty, "Copying version file to slot %d, please wait ...\n", slot_id);
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, 120000, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_STRING,&resultMd5,
					DBUS_TYPE_INVALID)) {
		vty_out(vty, "File md5 value on dest board is %s\n", resultMd5);
		if(ret == 0){
			vty_out(vty,"Copy version file %s to slot %d done.\n", version_file, slot_id);
		}
		else{
			vty_out(vty,"Copy version file %s to slot %d failed.\n", version_file, slot_id);
			return CMD_WARNING;
		}
	}

	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
		query_sem = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SET_BOOT_IMG);
		dbus_error_init(&err_sem);
	    
		dbus_message_append_args(query_sem,
								 DBUS_TYPE_UINT32,&slot_id,
								 DBUS_TYPE_STRING,&version_file,
								 DBUS_TYPE_INVALID);

		vty_out(vty, "Setting boot img on slot %d, please wait ...\n", slot_id);
		
		reply_sem = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query_sem, 60000, &err_sem);
		
		dbus_message_unref(query_sem);
		
		if (NULL == reply_sem){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err_sem)) {
				vty_out(vty,"%s raised: %s",err_sem.name,err_sem.message);
				dbus_error_free_for_dcli(&err_sem);
			}
			return CMD_SUCCESS;
		}

		if (dbus_message_get_args ( reply_sem, &err_sem,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 0){
				vty_out(vty,"Set boot img %s on slot %d done.\n", version_file, slot_id);
			}
			else{
				vty_out(vty,"Set boot img %s on slot %d failed.\n", version_file, slot_id);
				return CMD_WARNING;
			}
		}
	}
	else
	{
		vty_out(vty, "Dbus Connection With Slot %d not Exist, Can't set boot img.\n", slot_id);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;	
}

DEFUN (sem_mcb_active_standby_switch,
       sem_mcb_active_standby_switch_cmd,
       "mcb_active_standby_switch",
       "Master Control Board active and standby function switch\n")
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_MCB_ACTIVE_STANDBY_SWITCH);
	dbus_error_init(&err);
    
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, 60000, &err);
	
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
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		if(ret == 0){
			vty_out(vty,"MCB active standby switch success.\n");
		}
		else{
			vty_out(vty,"MCB active standby switch failed.\n");
			return CMD_WARNING;
		}
	}
	
	return CMD_SUCCESS;
}

DEFUN (sem_disable_keep_alive_temporarily,
       sem_disable_keep_alive_temporarily_cmd,
       "disable sem keep_alive TIME",
       "Disable function\n"
       "Disable sem module function\n"
       "Disable sem keep_alive function temporarily\n"
       "Time(second) for disabling sem keep_alive function\n")
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	FILE *fd;
	int time;

	/* after mcb active standby switched, the is_active_master need get value again , caojia added*/
	fd = fopen("/dbm/local_board/is_active_master", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [1] error\n");
		return -1;
	}
	fscanf(fd, "%d", &is_active_master);
	fclose(fd);

	sscanf(argv[0],"%d",&time);
	vty_out(vty, "Disable Time : %d\n", time);

	if ((time > 300) || (time < 0))
	{
		vty_out(vty, "Max time value is 300 (5 min).\n");
		vty_out(vty, "Min time value is 0.\n");

		return CMD_SUCCESS;
	}

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_DISABLE_KEEP_ALIVE_TEMPORARILY);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &time,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
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
			vty_out(vty,"Disable sem keep_alive function for %d seconds successfully\n", time);
		}
		else{
			vty_out(vty,"Disable sem keep_alive function for %d seconds failed\n", time);
			return CMD_WARNING;
		}
	}

	return CMD_SUCCESS;
}

DEFUN(sem_execute_system_command,
	sem_execute_system_command_cmd,
	"sem_exec slot (all|SLOT_NUM) .LINE",
	"sem execute system command\n"
	"sem execute system command on target slots\n"
	"sem execute system command on all slots running normally\n"
	"sem execute system command on one appointed slot\n"
	"right system command to execute\n")
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;

	FILE *fd;
	int slot_count;
	int slot_id;
	char* cmd;

	/* after mcb active standby switched, the is_active_master need get value again , caojia added*/
	fd = fopen("/dbm/local_board/is_active_master", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [1] error\n");
		return -1;
	}
	fscanf(fd, "%d", &is_active_master);
	fclose(fd);

	if ((is_active_master != 1) || (is_distributed == NON_DISTRIBUTED_SYSTEM))
	{
		vty_out(vty, "This command is only surpported by distributed system and only on active master board\n");

		return CMD_SUCCESS;
	}

	fd = fopen("/dbm/product/slotcount", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [2] error\n");
		return -1;
	}
	fscanf(fd, "%d", &slot_count);
	fclose(fd);
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		slot_id = 0;
	}
	else if (((strlen(argv[0]) == 1) && (argv[0][0] <= '9') && (argv[0][0] > '0')) || \
		(strlen(argv[0]) == 2) && (argv[0][0] == '1') && (argv[0][1] == '0'))
	{
		sscanf(argv[0],"%d", &slot_id);
		vty_out(vty, "Slot_id : %d\n", slot_id);

		if (slot_id > slot_count)
		{
			vty_out(vty, "slot number range on this product : 1 ~ %d\n", slot_count);

			return CMD_SUCCESS;
		}
	}
	else
	{
		vty_out(vty, "error slot number : %s\n", argv[0]);
		vty_out(vty, "correct slot number option : all or 1 ~ %d\n", slot_count);

		return CMD_SUCCESS;
	}

	cmd = argv_concat(&argv[1], argc - 1, 0);
	if(strlen(cmd) > SPACIAL_CONFIG_COMMAND_MAX)
	{
		vty_out(vty,"command is too long, please do not more than %d\n",SPACIAL_CONFIG_COMMAND_MAX);
		free(cmd);
		return CMD_WARNING;
	}
	vty_out(vty, "CMD : %s\n", cmd);

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_EXECUTE_SYSTEM_COMMAND);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &slot_id,
							DBUS_TYPE_STRING, &cmd,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	free(cmd);

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
			vty_out(vty,"Successful\n", time);
		}
		else if (ret == 1)
		{
			vty_out(vty,"Sem send exec message failed\n");
		}
		else
		{
			vty_out(vty,"Failed\n", time);
			return CMD_WARNING;
		}
	}

	return CMD_SUCCESS;
}


int parse_slot_id_from_ifname(const char *if_name, int *slot_id)
{
	char *ptr;
	int i=0;
	*slot_id = 0;
	
	if (!if_name) {
		printf("NULL if_name\n");
		return 1;
	}

	ptr = (char *)if_name;
	for (i=0; i<strlen(if_name); i++) {
		if (*(ptr+i) >= '1' && *(ptr+i) <= '9') {
			while (*(ptr+i) >= '0' && *(ptr+i) <= '9') {
				*slot_id = *slot_id * 10 + *(ptr+i) - '0';
				i++;
			}
			break;
		}
	}

	if (i == strlen(if_name)) {
		return 1;
	} else {
		return 0;
	}
	
}

int judge_ve_sub_interface(const char *name)
{
	if((strncmp(name,"ve",2)==0)&&(strlen(name) >= (strlen("ve1.1"))))
		return 1;
	else
		return 0;
}

DEFUN(wan_out_set,
	interface_wan_out_cmd,
	"wan-out (enable|disable)",
	"config wan-out mode\n"
	"from wan out\n"
	"not from wan out\n"
	)
{
	int ret;
	int wan_state = 1;
	char *dev_name = NULL;
	int arglen;
	int dest_slot_id;
	FILE *fd;
	int slot_count;
	int slot;
	char file_path[64] = {0};
	int i=0,k=0;
	int function_type = -1;
	
	dev_name = (char *)vty->index;

	fd = fopen("/dbm/product/slotcount", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [2] error\n");
		return -1;
	}
	fscanf(fd, "%d", &slot_count);
	fclose(fd);

	if(1 != argc){
		vty_out(vty,"%% Bad parameter number!\n");
	}
	arglen = strlen((char*)argv[0]);
	if(!strncmp((char *)argv[0], "enable", arglen)){
		wan_state = 1;
	}
	else if(!strncmp((char *)argv[0], "disable", arglen)){
		wan_state = 0;
	}
	else{
		vty_out(vty,"%% Bad parameter %s!",(char *)argv[0]);
		return CMD_WARNING;
	}

	vty_out(vty, "set the FPGA CFI,wait...\n");
	if((strncmp(dev_name,"ve",2)==0))
	{
		DBusMessage *query, *reply;
		DBusError err;
		int ret;
        #if 0
		ret = parse_slot_id_from_ifname(dev_name, &dest_slot_id);
        vty_out(vty, ". ");
		if (ret) {
			vty_out(vty, "parse slot id failed\n");
			return CMD_WARNING;
		} else {
			if ((dest_slot_id <= 0) || (dest_slot_id > 10)) {
				vty_out(vty, "SLOT_ID %d illegal\n", dest_slot_id);
				return CMD_WARNING;
			} 
		}
		#endif
		
		for (slot = 1; slot < (slot_count+1); slot++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", slot);
			function_type = get_product_info(file_path);
			
			if (function_type != SWITCH_BOARD)
		    {
        		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
        													 SEM_DBUS_INTERFACE, SEM_DBUS_SET_IF_WAN_STATE);
        		dbus_error_init(&err);

        		dbus_message_append_args(query,
        							DBUS_TYPE_UINT32, &wan_state,
        							DBUS_TYPE_STRING, &dev_name,
        							DBUS_TYPE_INVALID);

        		if (dbus_connection_dcli[slot]->dcli_dbus_connection) 
				{
        			reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot]->dcli_dbus_connection,\
        				query, 60000, &err);
        			dbus_message_unref(query);

        			if (NULL == reply){
        				vty_out(vty,"<error> failed get reply.\n");
        				if (dbus_error_is_set(&err)) {
        					vty_out(vty,"%s raised: %s",err.name,err.message);
        					dbus_error_free_for_dcli(&err);
        				}
        				continue;
        			}

        			if (dbus_message_get_args (reply, &err,
        							DBUS_TYPE_UINT32,&ret,
        							DBUS_TYPE_INVALID)) 
        			{
        				if(ret == 0){
        				}
        				else{
        					vty_out(vty,"mode CFI set failed.\n");
        					continue;
        				}
        			}
			    }
	            else 
			    {
    			    vty_out(vty, "no connection to slot %d\n", slot);
					continue;
		        }
		    }
			else
			{
                vty_out(vty,"%% Unsupport sub-interface setting on slot%d(switch-board).\n",slot);
				continue;
			}
	    }
		vty_out(vty, "set the CFI OK!\n");
    }
	return CMD_SUCCESS;
}

/*
*functon:set the print level of sem moudle
*/
DEFUN(debug_sem_info ,
		debug_sem_info_cmd,
		"debug sem (all|error|warning|debug|event)",
		DEBUG_STR
		MODULE_DEBUG_STR(sem)
		MODULE_DEBUG_LEVEL_STR(sem,all)
		MODULE_DEBUG_LEVEL_STR(sem,error)
		MODULE_DEBUG_LEVEL_STR(sem,warning)
		MODULE_DEBUG_LEVEL_STR(sem,debug)
		MODULE_DEBUG_LEVEL_STR(sem,event)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
											 SEM_DBUS_INTERFACE, SEM_DBUS_INTERFACE_METHOD_SYSTEM_DEBUG_STATE);
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"failed get reply.\n");	
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&re_value,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		if(ret == 0)
		{
		    vty_out(vty," operation success!\n");
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

/*The no debug sem function*/
DEFUN(no_debug_sem_info ,
		no_debug_sem_info_cmd,
		"no debug sem (all|error|warning|debug|event)",
		NO_STR
		NODEBUG_STR
		MODULE_DEBUG_STR(sem)
		MODULE_DEBUG_LEVEL_STR(sem,all)
		MODULE_DEBUG_LEVEL_STR(sem,error)
		MODULE_DEBUG_LEVEL_STR(sem,warning)
		MODULE_DEBUG_LEVEL_STR(sem,debug)
		MODULE_DEBUG_LEVEL_STR(sem,event)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
											 SEM_DBUS_INTERFACE, SEM_DBUS_INTERFACE_METHOD_SYSTEM_UNDEBUG_STATE);

    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&flag,
									DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"failed get reply.\n");	
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&re_value,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {

		if(ret != 0)
		{
		    vty_out(vty," operation success!\n");
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

DEFUN (sem_apply_patch,
       sem_apply_patch_cmd,
       "sem_apply_patch name PATCH_NAME",
       "sem moudle apply patch\n"
       "patch name\n"
       "patch name string\n")
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusError err;
	DBusMessageIter  iter;
	int ret;
	int slot_id;
	char *endptr = NULL;
	char *patchname = NULL;
	char *patch_result;
	int i=0;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

	patchname= (char *)argv[0];

	ret = strlen(patchname);
	if(ret > 100)
	{
        vty_out(vty, "patch name should less than 100Bytes.\n");
		return CMD_WARNING;
	}

	for(i = 1;i <= slotNum;i++)
	{
		if(i == active_master_slot)
		{
			continue;
		}
		
		if (dbus_connection_dcli[i]->dcli_dbus_connection) 
    	{
        	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
        												 SEM_DBUS_INTERFACE, SEM_DBUS_APPLY_PATCH);
        	dbus_error_init(&err);
        	
        	dbus_message_append_args(query,
        					 		DBUS_TYPE_STRING,&patchname,
        					 		DBUS_TYPE_INVALID);



    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,\
    			query, -1, &err);
    		dbus_message_unref(query);

    		if (NULL == reply)
    		{
    			vty_out(vty,"<error> failed get reply.\n");
    			if (dbus_error_is_set(&err))
    			{
    				vty_out(vty,"%s raised: %s",err.name,err.message);
    				dbus_error_free_for_dcli(&err);
    			}
    		}

        	if (dbus_message_get_args ( reply, &err,
        		                     DBUS_TYPE_INT32,&ret,
        		                     DBUS_TYPE_STRING,&patch_result,
        		                     DBUS_TYPE_INVALID))
        	{
                //vty_out(vty, "SLOT %d retun result %d,look at corresponding slot /blk/patch.log for the patch result!\n",i,ret);
				vty_out(vty, "SLOT %d execute result:\n", i);
				if (ret == 2) {
					vty_out(vty, "patch is not exist\n");
				} else if (ret == 1) {
					vty_out(vty, "patch exec failed\n");
				} else if (ret == 3) {
					vty_out(vty, "failed get execute result,you must check it manually\n");
				} else {
					vty_out(vty, "%s\n", patch_result);
				}
			}
    		else
    		{
    		    vty_out(vty, "SLOT %d get args from replay fail\n",i);
    		}
    		dbus_message_unref(reply);
    	}
    	else 
    	{
        	vty_out(vty, "no connection to slot %d\n", i);
    	}
	}

	if (dbus_connection_dcli[active_master_slot]->dcli_dbus_connection) 
	{
    	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
    												 SEM_DBUS_INTERFACE, SEM_DBUS_APPLY_PATCH);
    	dbus_error_init(&err);
    	dbus_message_append_args(query,
    					 		DBUS_TYPE_STRING,&patchname,
    					 		DBUS_TYPE_INVALID);


		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[active_master_slot]->dcli_dbus_connection,\
			query, -1, &err);
		dbus_message_unref(query);
		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
		}

    	if (dbus_message_get_args ( reply, &err,
    		                     DBUS_TYPE_INT32,&ret,
    		                     DBUS_TYPE_STRING,&patch_result,
    		                     DBUS_TYPE_INVALID))
    	{
            //vty_out(vty, "SLOT %d retun result %d,look at corresponding slot /blk/patch.log for the patch result!\n",i,ret);
			vty_out(vty, "SLOT %d execute result:\n", active_master_slot);
			if (ret == 2) {
				vty_out(vty, "patch is not exist\n");
			} else if (ret == 1) {
				vty_out(vty, "patch exec failed\n");
			} else if (ret == 3) {
				vty_out(vty, "failed get execute result,you must check it manually\n");
			} else {
				vty_out(vty, "%s\n", patch_result);
			}
		}
		else
		{
		    vty_out(vty, "SLOT %d get args from replay fail\n",active_master_slot);
		}
		dbus_message_unref(reply);
	}
	else 
	{
    	vty_out(vty, "no connection to slot %d\n", active_master_slot);
	}
    return CMD_SUCCESS;
}

DEFUN(config_sem_sendto_trap,
	config_sem_sendto_trap_cmd,
	"config sem send-to-trap num NUM",
	"config system parameters\n"
	"config sem parameters\n"
	"config sem send signal to trap\n"
	"send signal number\n"
	"send signal number\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	unsigned int num = 1;
	
	vty_out(vty,"begin to config function,wait .");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    num = strtoul(argv[0], &endptr, 10);

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_SEM_TO_TRAP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_UINT32, &num,
						DBUS_TYPE_INVALID);
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,\
			query, -1, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 0){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"config sem send to trap dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		vty_out(vty, "\nconfig sem send to trap dbus success!\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;
}
DEFUN (sem_apply_patch_single,
       sem_apply_patch_single_cmd,
       "sem_apply_patch_single SLOT_ID PATCH_NAME",
       "sem moudle apply patch\n"
       "slot id\n"
       "patch name string\n")
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	int slot_id;
	char *endptr = NULL;
	char *patchname = NULL;
	char *patch_result;
	int i=0;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	patchname= (char *)argv[1];
	ret = strlen(patchname);
	if(ret > 100)
	{
        vty_out(vty, "patch name should less than 100Bytes.\n");
    	return CMD_WARNING;
	}

	//vty_out(vty, "SLOT %d **********\n",i);
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	{
    	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
    												 SEM_DBUS_INTERFACE, SEM_DBUS_APPLY_PATCH);
    	dbus_error_init(&err);
    	
    	dbus_message_append_args(query,
    					 		DBUS_TYPE_STRING,&patchname,
    					 		DBUS_TYPE_INVALID);



		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, -1, &err);
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
		}
		//vty_out(vty, "SLOT %d !!!!!!!!!!!!!!!!!!!!!!!!!\n",i);
    	if (dbus_message_get_args ( reply, &err,
    		                     DBUS_TYPE_INT32,&ret,
    		                     DBUS_TYPE_STRING,&patch_result,
    		                     DBUS_TYPE_INVALID))
    	{
            //vty_out(vty, "SLOT %d retun result %d,look at corresponding slot /blk/patch.log for the patch result!\n",slot_id,ret);
            if (ret == 2) {
				vty_out(vty, "patch is not exist\n");
			} else if (ret == 1) {
				vty_out(vty, "patch exec failed\n");
			} else if (ret == 3) {
				vty_out(vty, "failed get execute result,you must check it manually\n");
			} else {
				vty_out(vty, "%s\n", patch_result);
			}
		}
		else
		{
		    vty_out(vty, "SLOT %d get args from replay fail\n",slot_id);
		}
		dbus_message_unref(reply);
	}
	else 
	{
    	vty_out(vty, "no connection to slot %d\n", slot_id);
	}
    return CMD_SUCCESS;
}

DEFUN(sem_clean_messageoftoday_command,
	sem_clean_messageoftoday_command_cmd,
	"clean messageoftoday",
	"clean function\n"
	"clean the messageoftoday file\n")
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;

	char cmdstr[128] = "sudo echo Auteware > /etc/motd";
	pid_t status;

	status = system(cmdstr);
	if (status != -1) {
		if (WIFEXITED(status)) {
			if (0 == WEXITSTATUS(status)) {
				vty_out(vty, "Clear motd(messageoftoday) done.\n");
				return CMD_SUCCESS;
			}
			else {
				vty_out(vty, "Operation failed.\n");
				return CMD_WARNING;
			}
		}
		else {
			vty_out(vty, "Operation Failed\n");
			return CMD_WARNING;
		}
	}
	else {
		vty_out(vty, "Operation failed\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}



/************************************

xu
**************************************/
DEFUN (sem_flash_erase,
		   sem_flash_erase_cmd,
		   "flash_erase SLOT_ID  PATH_NAME",
		   "flash_erase\n"
		   "slod id\n"
		   "partition_name\n")
	{
	DBusMessage *query, *reply;
	DBusError err;
	int slot_id;
	char *endptr = NULL;
	char *patchname = NULL;
	int i=atoi(argv[0]);
	int ret = -1;
	char cmd[256];
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
   	if(2 == argc )
	{ 
		
    
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	  
		if (strcmp(argv[1],"uboot") == 0){
			patchname = "mtd1";
		}
		else if(strcmp(argv[1],"env") == 0){
			patchname = "mtd2";
		}
		else{
			vty_out(vty, " partition name error\n");
			return CMD_WARNING;

		}
		//patchname= (char *)argv[1];
		vty_out(vty, "after erasing , must writing bootrom to flash,or cannot be restarted \n");

		
		vty_out(vty, "starting erase flash for SLOT %d **********\n",i);
		

		if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
		{
	    	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
	    												 SEM_DBUS_INTERFACE, SEM_DBUS_FLASH_ERASE_PARTITIO);
	    	dbus_error_init(&err);
	    	
	    	dbus_message_append_args(query,
	    					 		DBUS_TYPE_STRING,&patchname,
	    					 		DBUS_TYPE_INVALID);



			reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
				query, -1, &err);
			dbus_message_unref(query);

			if (NULL == reply)
			{
				vty_out(vty,"<error> failed get reply.\n");
				if (dbus_error_is_set(&err))
				{
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
			}
	//		vty_out(vty, "SLOT %d !!!!!!!!!!!!!!!!!!!!!!!!!\n",i);
	    	if (dbus_message_get_args ( reply, &err,
	    		                     DBUS_TYPE_INT32,&ret,
	    		                     DBUS_TYPE_INVALID))
	    	{
				vty_out(vty,"remote %d :",slot_id);
	            
				switch ((ret)) { 	
					case 0: 
						
						vty_out(vty,"SUCCESS\n");			
						return CMD_SUCCESS; 	
					case 1: 		
						vty_out(vty,"error.\n");			
						break;		
					case 2: 		
						vty_out(vty,"open device fialed\n");			
						break;		
					case 3: 		
						vty_out(vty,"upable to get MTD device\n");			
						break;		
					case 4: 		
						vty_out(vty,"get bad block failed\n");			
						break;			
					default:			
						vty_out(vty,"erase flash fialed\n");			
						break;		
						}	
				return CMD_WARNING;
	            
			}
			else
			{
			    vty_out(vty, "SLOT %d get args from replay fail\n",slot_id);
			}
			dbus_message_unref(reply);
		}
		else 
		{
	    	vty_out(vty, "no connection to slot %d\n", slot_id);
			return CMD_SUCCESS;
		}
   		}
	
	return CMD_SUCCESS;
}


DEFUN (flash_write_bootrom,
       flash_write_bootrom_cmd,
       "flash_write_boot SLOT_ID BOOT_NAME",
       "update_flash_boot\n"
       "slot id\n"
       "u-boot file name string\n")
{
		DBusMessage *query, *reply;
		DBusError err;
		int slot_id;
		char *endptr = NULL;
		char *patchname = NULL;
		int ret = -1;
		int i=atoi(argv[0]);
		char cmd[256];
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(2 == argc )
		{ 
			
    		 slot_id = strtoul(argv[0], &endptr, 10);
		if(slot_id > slotNum || slot_id <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
       		 return CMD_WARNING;
		}
			patchname= (char *)argv[1];
			if(strncasecmp((patchname + strlen(patchname)-4),".bin",4))
			{
				vty_out(vty,"The uboot file should be .bin file\n");

				return CMD_WARNING;
			}
			vty_out(vty, "start writing bootrom to flash SLOT %d *****Just a minute, please\n",i);
	
			if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
			{
				query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
															 SEM_DBUS_INTERFACE, WRITE_BOOT_TO_FLASH);
				dbus_error_init(&err);
				
				dbus_message_append_args(query,
										DBUS_TYPE_STRING,&patchname,
										DBUS_TYPE_INVALID);
	
	
	
				reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
					query, -1, &err);
				dbus_message_unref(query);
	
				if (NULL == reply)
				{
					vty_out(vty,"<error> failed get reply.\n");
					if (dbus_error_is_set(&err))
					{
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
				}
				//vty_out(vty, "SLOT %d !!!!!!!!!!!!!!!!!!!!!!!!!\n",i);
				if (dbus_message_get_args ( reply, &err,
										 DBUS_TYPE_INT32,&ret,
										 DBUS_TYPE_INVALID))
				{
					vty_out(vty,"remote %d :",slot_id);
					
					switch ((ret)) {	
						case 0: 
							
							vty_out(vty,"WRITE SUCCESS\n");			
							return CMD_SUCCESS; 	
						case 1: 		
							vty_out(vty,"writing uboot fialed\n");			
							break;		
						case 2: 		
							vty_out(vty,"open file failed,Please check the  filename, write BOOTROM again\n");			
							break;		
						case 3: 		
							vty_out(vty,"unable to get MTD device\n");			
							break;		
						case 4: 		
							vty_out(vty,"read fiel failed\n");			
							break;		
						case 5: 		
							vty_out(vty,"write file failed\n");			
							break;
						case -1:
							vty_out(vty," file check failed\n");
							break;
						default:			
							vty_out(vty,"writing uboot failed\n");			
							break;		
							}	
					return CMD_WARNING;
					
				}
				else
				{
					vty_out(vty, "SLOT %d get args from replay fail\n",slot_id);
				}
				dbus_message_unref(reply);
			}
			else 
			{
				vty_out(vty, "no connection to slot %d\n", slot_id);
				return CMD_SUCCESS;
			}
			}else{
				vty_out(vty, " enter error\n");
				return CMD_WARNING;
			}
			return CMD_SUCCESS;
	}	
	
DEFUN (delete_patch_single,
       delete_patch_single_cmd,
       "delete patch PATCH_NAME [SLOT_ID]",
       "Delete system file\n"
       "Delete moudle patch\n"
       "Patch name string\n"
       "Slot id")
{
	DBusMessage *query, *reply;
	DBusError err;
	int slot_id;
	int fd;
	char c;
	char *endptr = NULL;
	char *patchname = NULL;
	int i=atoi(argv[1]);
	int local_slot_id = get_product_info("/dbm/local_board/slot_id");
	int ret = -1;
	char cmd[256];
	
   	if(2 == argc)
	{ 
		
		//rm **.sp at all slot
	    slot_id = atoi(argv[1]);
		if(slot_id > get_product_info(SEM_SLOT_COUNT_PATH) || slot_id <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
	        return CMD_WARNING;
		}

		if(i == local_slot_id)
		{
			
			//rm /mnt/patch/**.sp
			memset(cmd,0,256);
			sprintf(cmd,"rm /mnt/patch/%s > /dev/null 2> /dev/null",argv[0]);
			ret = system(cmd);
			ret = WEXITSTATUS(ret);
			if(ret == 1)
			{
				vty_out(vty,"local :Not found\n");
	//			return CMD_WARNING;
			}

			// rm /blk/patch/**.sp
			memset(cmd,0,256);
			sprintf(cmd,"sor.sh ls patch/%s 20 | wc -l > /mnt/exist",argv[0]);
			system(cmd);
			fd = open("/mnt/exist", O_RDONLY);
			if (fd < 0)
			{
				vty_out(vty,"open /mnt/exist error\n");
				return CMD_FAILURE;
			}
			read(fd, &c, 1);
			close(fd);
			if (c-'0')
			{
				memset(cmd,0,256);
				sprintf(cmd,"patch/%s",argv[0]);
				if(CMD_SUCCESS != sor_exec(vty,"rm",cmd,20))
					return CMD_WARNING;
				else
					return CMD_SUCCESS;
			}
		}

		patchname= (char *)argv[0];

		//vty_out(vty, "SLOT %d **********\n",i);

		if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
		{
	    	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
	    												 SEM_DBUS_INTERFACE, SEM_DBUS_DELETE_PATCH);
	    	dbus_error_init(&err);
	    	
	    	dbus_message_append_args(query,
	    					 		DBUS_TYPE_STRING,&patchname,
	    					 		DBUS_TYPE_INVALID);



			reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
				query, -1, &err);
			dbus_message_unref(query);

			if (NULL == reply)
			{
				vty_out(vty,"<error> failed get reply.\n");
				if (dbus_error_is_set(&err))
				{
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
			}
			//vty_out(vty, "SLOT %d !!!!!!!!!!!!!!!!!!!!!!!!!\n",i);
	    	if (dbus_message_get_args ( reply, &err,
	    		                     DBUS_TYPE_INT32,&ret,
	    		                     DBUS_TYPE_INVALID))
	    	{
				vty_out(vty,"remote %d :",slot_id);
	            
				switch ((ret)) { 	
					case 0: 
						
						vty_out(vty,"SUCCESS\n");			
						return CMD_SUCCESS; 	
					case 1: 		
						vty_out(vty,"Sysetm internal error (1).\n");			
						break;		
					case 2: 		
						vty_out(vty,"Sysetm internal error (2).\n");			
						break;		
					case 3: 		
						vty_out(vty,"Storage media is busy.\n");			
						break;		
					case 4: 		
						vty_out(vty,"Storage operation time out.\n");			
						break;		
					case 5: 		
						vty_out(vty,"No left space on storage media.\n");			
						break;		
					case 6: 		
						vty_out(vty,"Not found\n");			
						break;		
					default:			
						vty_out(vty,"Sysetm internal error (3).\n");			
						break;		
						}	
				return CMD_WARNING;
	            
			}
			else
			{
			    vty_out(vty, "SLOT %d get args from replay fail\n",slot_id);
			}
			dbus_message_unref(reply);
		}
		else 
		{
	    	vty_out(vty, "no connection to slot %d\n", slot_id);
			return CMD_SUCCESS;
		}
   	}else{
    	memset(cmd,0,256);
    	sprintf(cmd,"rm /mnt/patch/%s > /dev/null 2> /dev/null",argv[0]);
    	ret = system(cmd);
    	ret = WEXITSTATUS(ret);
    	if(ret == 1)
    	{
    		vty_out(vty,"local :Not found\n");
    	}

    	memset(cmd,0,256);
    	sprintf(cmd,"sor.sh ls patch/%s 20 | wc -l > /mnt/exist",argv[0]);
    	system(cmd);
    	fd = open("/mnt/exist", O_RDONLY);
    	if (fd < 0)
    	{
    		vty_out(vty,"open /mnt/exist error\n");
    		return CMD_FAILURE;
    	}
    	read(fd, &c, 1);
    	close(fd);
    	if (c-'0')
    	{
    		memset(cmd,0,256);
    		sprintf(cmd,"patch/%s",argv[0]);
    		if(CMD_SUCCESS != sor_exec(vty,"rm",cmd,20))
    			vty_out(vty,"delete error\n");
    		else
    			vty_out(vty,"Delete local board patch success\n");
    	}

    	patchname= (char *)argv[0];
    	for(i = 1;i < MAX_SLOT ; i++)
    	{
    		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
    		{
    			if (i != HostSlotId)
    			{
    		    	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
    		    												 SEM_DBUS_INTERFACE, SEM_DBUS_DELETE_PATCH);
    		    	dbus_error_init(&err);
    		    	
    		    	dbus_message_append_args(query,
    		    					 		DBUS_TYPE_STRING,&patchname,
    		    					 		DBUS_TYPE_INVALID);



    				reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,\
    					query, -1, &err);

    				if (NULL == reply)
    				{
    					vty_out(vty,"<error> failed get reply.\n");
    					if (dbus_error_is_set(&err))
    					{
    						vty_out(vty,"%s raised: %s",err.name,err.message);
    						dbus_error_free_for_dcli(&err);
    					}
    				}
    				
    		    	if (dbus_message_get_args ( reply, &err,
    		    		                     DBUS_TYPE_INT32,&ret,
    		    		                     DBUS_TYPE_INVALID))
    			   	{
    					vty_out(vty,"remote %d :",i);
    		            
    					switch ((ret)) { 	
    						case 0: 
    							
    							vty_out(vty,"Delete slot %d patch success\n",i);
    							break;
    						case 1: 		
    							vty_out(vty,"Sysetm internal error (1).\n");			
    							break;		
    						case 2: 		
    							vty_out(vty,"Sysetm internal error (2).\n");			
    							break;		
    						case 3: 		
    							vty_out(vty,"Storage media is busy.\n");			
    							break;		
    						case 4: 		
    							vty_out(vty,"Storage operation time out.\n");			
    							break;		
    						case 5: 		
    							vty_out(vty,"No left space on storage media.\n");			
    							break;		
    						case 6: 		
    							vty_out(vty,"Not found\n");			
    							break;		
    						default:			
    							vty_out(vty,"Sysetm internal error (3).\n");			
    							break;		
    						}	
    					//return CMD_WARNING;
    		            
    				}
    				else
    				{
    				    vty_out(vty, "SLOT %d get args from replay fail\n",i);
    				}
    				dbus_message_unref(reply);
    			}
    		}else{
                //vty_out(vty, "NO connection with slot %d\n",i);
			}
    	}
    return CMD_SUCCESS;
    }
}
	
DEFUN (cf_card_partition_func,
	    cf_card_partition_cmd,
	   	"partition_cf_card",
		"CF card partition\n")
{
	int status;

	vty_out(vty,"CF card file backup,Please wait a moment ...\n");
	status = system("/usr/bin/cp_to_memory.sh");

	switch (WEXITSTATUS(status)) 
	{ 	
		case 0: 		
			vty_out(vty,"CF card file backup success.\n");
			break;
		case 1: 		
			vty_out(vty,"Mount /blk failed.\n");			
			return CMD_WARNING;
		case 2: 		
			vty_out(vty,"CF card so many version files,please keep the version file not more than 15.\n");			
			return CMD_WARNING;
		case 3: 		
			vty_out(vty,"Create directory (temp_directory) failed.\n");			
			return CMD_WARNING;
		case 4: 		
			vty_out(vty,"pkill sad.sh failed\n");			
			return CMD_WARNING;
		case 5: 		
			vty_out(vty,"Copy files from storarge to memory failed.\n");			
			return CMD_WARNING;
		case 6: 		
			vty_out(vty,"Free memory is not enough to sort the storage.\n");			
			return CMD_WARNING;		
		default:						
			break;		
	}	

	vty_out(vty,"Start formatting CF card,Please wait a moment ...\n");
	status = system("/usr/bin/setup_format_cf_card.sh /dev/sda");
	
	if(WEXITSTATUS(status) == 0)
	{
        vty_out(vty,"Format CF card success.\n");
	}
	else
	{
		vty_out(vty,"Format CF card failed.\n");
		return CMD_FAILURE;
	}

	vty_out(vty,"Copy the file to CF card,Please wait a moment ...\n");
	status = system("/usr/bin/cp_to_storage.sh");

	switch (WEXITSTATUS(status)) 
	{ 	
		case 0: 		
			vty_out(vty,"CF card file backup success.\n");
			break;
		case 7: 		
			vty_out(vty,"copy file back to the storage failed\n");			
			return CMD_WARNING;		
		case 8: 		
			vty_out(vty,"create directory /tmp_dir failed.\n");			
			return CMD_WARNING;	
		case 9: 		
			vty_out(vty,"mount the second partion to /tmp_dir failed..\n");			
			return CMD_WARNING;	
		default:						
			break;		
	}	

	return CMD_SUCCESS;
}

DEFUN(show_6185_reg,
	show_6185_reg_cmd,
	"show 6185 PHYADDR DEVADDR REGADDR",
	"show 88E6185 register val"
	"show 88E6185 register val\n"
	"88E6185 PHY address\n"
	"88E6185 device address\n"
	"88E6185 register address\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;
    unsigned int dev_addr;
	unsigned int reg_addr;
	unsigned int phy_addr;
	unsigned int val;

	phy_addr = strtol(argv[0],NULL,16);
	dev_addr = strtol(argv[1],NULL,16);
    reg_addr = strtol(argv[2],NULL,16);
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_6185);
	if (!query)
	{
		vty_out(vty, "show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);

	dbus_message_append_args(query,
				 		DBUS_TYPE_UINT32, &dev_addr,
				 		DBUS_TYPE_UINT32, &reg_addr,
				 		DBUS_TYPE_UINT32, &phy_addr,
				 		DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		vty_out(vty, "<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&val);
	vty_out(vty, "dev_addr = 0x%x,reg_addr = 0x%x\n", dev_addr,reg_addr);
	vty_out(vty, "read val = 0x%x\n", val);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}

DEFUN(set_6185_reg,
	set_6185_reg_cmd,
	"set 6185 PHYADDR DEVADDR REGADDR VAL",
	"set 88E6185 register val"
	"set 88E6185 register val\n"
	"88E6185 PHY address\n"
	"88E6185 device address\n"
	"88E6185 register address\n"
	"write val\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;
    unsigned int dev_addr;
	unsigned int reg_addr;
	unsigned int phy_addr;
	unsigned short val;
	int ret;
	
	phy_addr = strtol(argv[0],NULL,16);
	dev_addr = strtol(argv[1],NULL,16);
    reg_addr = strtol(argv[2],NULL,16);
	val = strtol(argv[3],NULL,16);
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_SET_6185);
	if (!query)
	{
		vty_out(vty, "show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);

	dbus_message_append_args(query,
				 		DBUS_TYPE_UINT32, &dev_addr,
				 		DBUS_TYPE_UINT32, &reg_addr,
				 		DBUS_TYPE_UINT16, &val,
				 		DBUS_TYPE_UINT32, &phy_addr,
				 		DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		vty_out(vty, "<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(!ret){
	    vty_out(vty, "dev_addr = 0x%x,reg_addr = 0x%x,val = 0x%x,write done!\n", dev_addr,reg_addr,val);
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}

/*
* Description:
*  Re-encapsulation function dbus_message_new_method_call for Hansi.
*
* Parameter:
*  idx: VRID
*  Other same to function dbus_message_new_method_call
*
* Return:
*  Same to function dbus_message_new_method_call.
*
*/
DBusMessage *dbus_sem_msg_new_method_call(unsigned char *dbus_name, unsigned char *obj_name,
			unsigned char *if_name, unsigned char *cmd_name)
{
	unsigned char dbus_path[PATH_MAX_LEN] = {0};
	unsigned char obj_path[PATH_MAX_LEN] = {0};
	unsigned char if_path[PATH_MAX_LEN] = {0};
	unsigned char cmd_path[PATH_MAX_LEN] = {0};

	if (dbus_name == NULL || obj_name == NULL || if_name == NULL || cmd_name == NULL)
		return NULL;

	sprintf(dbus_path, "%s", dbus_name);
	sprintf(obj_path, "%s", obj_name);
	sprintf(if_path, "%s", if_name);
	sprintf(cmd_path, "%s", cmd_name);

	return dbus_message_new_method_call(dbus_path, obj_path, if_path, cmd_path);
} 

void dcli_sem_init(void)
{
	install_element(ENABLE_NODE,&show_slot_id_cmd);
	install_element(ENABLE_NODE,&show_slot_n_info_cmd);
	install_element(ENABLE_NODE,&show_board_info_cmd);
	install_element(ENABLE_NODE,&show_product_info_cmd);
	install_element(ENABLE_NODE,&show_slot_info_cmd);
	install_element(ENABLE_NODE, &system_sync_version_file_to_slot_cmd);
	install_element(ENABLE_NODE, &sem_disable_keep_alive_temporarily_cmd);
	install_element(ENABLE_NODE, &sem_execute_system_command_cmd);
	install_element(ENABLE_NODE, &sem_clean_messageoftoday_command_cmd);
    install_element(ENABLE_NODE, &cf_card_partition_cmd);
	
    install_element(CONFIG_NODE, &cf_card_partition_cmd);
	install_element(CONFIG_NODE, &config_send_file);
	install_element(CONFIG_NODE, &config_sem_cmd);	
	install_element(CONFIG_NODE, &config_tipc_cmd);/*clx*/
	install_element(CONFIG_NODE, &config_48GE_cmd);
	install_element(CONFIG_NODE, &config_slot_n_cmd);
	install_element(CONFIG_NODE, &show_slot_id_cmd);
	install_element(CONFIG_NODE, &show_slot_n_info_cmd);
	install_element(CONFIG_NODE, &show_board_info_cmd);
	install_element(CONFIG_NODE, &show_product_info_cmd);
	install_element(CONFIG_NODE, &show_slot_info_cmd);
	install_element(CONFIG_NODE, &sem_mcb_active_standby_switch_cmd);
	install_element(CONFIG_NODE, &sem_disable_keep_alive_temporarily_cmd);
	install_element(CONFIG_NODE, &sem_execute_system_command_cmd);
	install_element(ENABLE_NODE, &sem_apply_patch_single_cmd);
	install_element(ENABLE_NODE, &delete_patch_single_cmd);
	install_element(CONFIG_NODE, &debug_sem_info_cmd);//huangjing
	install_element(ENABLE_NODE, &sem_apply_patch_cmd);//huangjing
	install_element(CONFIG_NODE, &no_debug_sem_info_cmd);
	install_element(INTERFACE_NODE, &interface_wan_out_cmd);//huangjing
	//install_element(ENABLE_NODE, &sem_flash_erase_cmd);//xufujun
	install_element(ENABLE_NODE, &flash_write_bootrom_cmd);
    install_element(HIDDENDEBUG_NODE, &config_sem_sendto_trap_cmd);
	install_element(HIDDENDEBUG_NODE, &show_slot_id_cmd);
	install_element(HIDDENDEBUG_NODE, &show_slot_n_info_cmd);
	install_element(HIDDENDEBUG_NODE, &show_board_info_cmd);
	install_element(HIDDENDEBUG_NODE, &show_product_info_cmd);
	install_element(HIDDENDEBUG_NODE, &show_slot_info_cmd);
	install_element(HIDDENDEBUG_NODE, &show_slot_info_history_cmd);
    install_element(HIDDENDEBUG_NODE, &system_sync_version_file_to_slot_cmd);
	install_element(HIDDENDEBUG_NODE, &debug_sem_info_cmd);
	install_element(HIDDENDEBUG_NODE, &no_debug_sem_info_cmd);
	install_element(HIDDENDEBUG_NODE, &show_6185_reg_cmd);
	install_element(HIDDENDEBUG_NODE, &set_6185_reg_cmd);
}

#endif
