/***********************license start************************************
 * File name: fwd_main.c 
 * Auther     : lutao
 * 
 * Copyright (c) Autelan . All rights reserved.
 * 
 **********************license end**************************************/
#include "cvmx-config.h"
#include "executive-config.h"

#include "cvmx.h"
#include "cvmx-spinlock.h"
#include "cvmx-wqe.h"
#include "cvmx-fau.h"
#include "cvmx-fpa.h"
#include "cvmx-pow.h"
#include "cvmx-interrupt.h"
#include "cvmx-malloc.h"

#include <stdio.h>
#include <string.h>
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "uart.h"
#include "shell.h"
#ifdef SDK_VERSION_2_2
#include "fastfwd-common-defs.h"
#else
#include "cvm-common-defs.h"
#endif
#include "autelan_product_info.h"

extern int read_cmd(char *buf, int bufsize);

int uart_index = 0;
static char prompt[32];
static char	*argv[MAX_ARGV_ARGS];
static char shell_cmd_buf[STRING_MAX_LEN];  		/*´æ·ÅÃüÁî×Ö»º³åÇø*/
struct cmd 	cmd_list[MAX_CMDS];
extern const char version_string[];
extern CVMX_SHARED product_info_t product_info;


/**
 * execute cmd
 * param @buf cmd to execute
 */
int execute_cmd(char *buf,int len)
{
	const char    *delim = " ";
	int            argc = 0, i = 0;

	/* Parse command line and setup args */
	argv[argc++] = strtok(buf, delim);
	if (!argv[argc-1] ) {
		return CMD_EXEC_ERROR;
	}

	while(1) {
		/* Can't handle more than 64 arguments */
		if (argc >= MAX_ARGV_ARGS) 
			break;
		argv[argc++] = strtok(0, delim);
		if (!argv[argc-1]) {
			argc--;
			break;
		}
	}

	/* Search for the command */
	for (i = 0; i < MAX_CMDS; i++) {
		if (strcmp(cmd_list[i].name, argv[0]) == 0) 
			break;
	}

	if (i >= MAX_CMDS) {
		return CMD_EXEC_ERROR;
	}

	//printf("Core %d execute_cmd: %s oldstr: %s \r\n",cvmx_get_core_num(),buf,tmp_cmd_buf);
	if(cmd_list[i].func != NULL)		
	{
		int32_t ret = (cmd_list[i].func)(argc, argv);
		return ret;
	}
	else
	{
		return CMD_EXEC_ERROR;
	}
}

/**
 *print shell prompt
 */
void print_prompt(void)
{
	uart_write_string(uart_index, prompt);
}

/**
 *run the shell
 */
void shell_run()
{
	int len;
	int32_t ret = -1;

	/*read the command*/
	if ((len = read_cmd(shell_cmd_buf, sizeof(shell_cmd_buf))) > 1)
	{			
		ret = execute_cmd(shell_cmd_buf, len);
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_SHELL,FASTFWD_COMMON_DBG_LVL_DEBUG,
				"shell_run:receive the ret=%d \r\n",ret);

		switch (ret)
		{
			case CMD_EXEC_SUCCESS:
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_SHELL,FASTFWD_COMMON_DBG_LVL_DEBUG,
						"Command success\r\n");
				break;

			case CMD_ERR_TOO_FEW_ARGC:
				print_prompt();
				uart_write_string(uart_index, "Input too few paramater! \r\n");
				break;

			case CMD_ERR_NOT_MATCH:	
				print_prompt();
				uart_write_string(uart_index, "Command input not match \r\n");
				break;

			case CMD_ERR_TOO_MANY_ARGC:
				print_prompt();
				uart_write_string(uart_index, "Input too many paramater! \r\n");
				break;

			case CMD_EXEC_ERROR:
				print_prompt();
				uart_write_string(uart_index, "Command run failed\r\n");
				break;

			default:					
				break;
		}

		memset(shell_cmd_buf, 0, sizeof(shell_cmd_buf));
		print_prompt();
	}
	else if (len == 1)/*command is null*/
	{
		print_prompt();
	}
}

/**
 * register cmd to shell
 *param @name cmd name
 *param @func cmd handler
 *param @help cmd function
 *param @usage cmd usage
 */
int register_shell_cmd(const char *name, int (*func)(int, char *[]), const char *help, const char *usage, unsigned char status)
{
	int    i = 0;
	int    ret = -1;

	if (!name || !name[0]) 
		goto out;
	for (i = 0; i < MAX_CMDS && cmd_list[i].name[0]; i++) 
		;
	if (i >= MAX_CMDS) 
		goto out;

	strncpy(cmd_list[i].name, name, CMD_NAME_SIZE);
	cmd_list[i].func = func;
	strncpy(cmd_list[i].help, help, CMD_HELP);
	strncpy(cmd_list[i].usage, usage, CMD_USAGE);
	cmd_list[i].status= status;

	ret = 0;
out:
	if(ret < 0)
		printf("Failed to register shell cmd [%s]\n", name);
	return ret;
}

/**
 * unregister cmd 
 *param @name cmd to unregister
 */
int unregister_shell_cmd(const char *name)
{
	int  i = 0;
	int  ret = -1;

	if (!name || !name[0]) 
		goto out;
	/*find the cmd*/
	for (i = 0; i < MAX_CMDS && strcmp(name, cmd_list[i].name) != 0; i++) 
		;
	if (i >= MAX_CMDS) 
		goto out;

	cmd_list[i].name[0] = 0;
	memset(cmd_list[i].help, 0, CMD_HELP);
	memset(cmd_list[i].usage, 0, CMD_USAGE);

	ret = 0;
out:
	if(ret < 0)
		printf("Failed to unregister shell cmd [%s]\n", name);
	return ret;
}

void display_banner(void)
{
	printf("\n\nCopyright 2010-2012  BEIJING AUTELAN TELECOM CO., LTD.\r\n");
	printf ("%s\n\n", version_string);
}


void show_ciu_stat()
{
	cvmx_ciu_intx0_t irq_control;
	cvmx_ciu_fuse_t fuse;
	int cpu = 0;
	unsigned int fuse_tmp;

	fuse.u64 = cvmx_read_csr(CVMX_CIU_FUSE);
	fuse_tmp = fuse.s.fuse;

	printf("CVMX_CIU_INTX_EN0:\n");
	while(fuse_tmp)
	{ 
		if((fuse_tmp & 0x1) != 0)
		{
			irq_control.u64 = cvmx_read_csr(CVMX_CIU_INTX_EN0(cpu*2));
			printf("core[%2d]: 0x%lx\n", cpu, irq_control.u64);
		}
		cpu++;
		fuse_tmp = fuse_tmp>>1;
	}
}

/* clear se cores uart notifcation */
void clear_se_uart_note()
{
	cvmx_ciu_intx0_t irq_control;
	cvmx_ciu_fuse_t fuse;
	int cpu = 0;
	unsigned int fuse_tmp;
	int se_mask = 0;

	fuse.u64 = cvmx_read_csr(CVMX_CIU_FUSE);
	fuse_tmp = fuse.s.fuse;
	cvmx_sysinfo_t * sysinfo = cvmx_sysinfo_get();
	se_mask = sysinfo->core_mask;

	while(fuse_tmp)
	{
		if((fuse_tmp & 0x1) != 0)
		{
			if(((1<<cpu)&se_mask)!=0)
			{
				irq_control.u64 = cvmx_read_csr(CVMX_CIU_INTX_EN0(cpu*2));
				irq_control.s.uart=0;
				cvmx_write_csr(CVMX_CIU_INTX_EN0(cpu*2), irq_control.u64);
			}
		}
		cpu++;
		fuse_tmp = fuse_tmp>>1;
	}
}


/* switch uart console to linux */
void switch_uart_to_linux()
{
	cvmx_ciu_intx0_t irq_control;
	cvmx_ciu_fuse_t fuse;
	int cpu = 0;
	unsigned int fuse_tmp;
	int linux_first_core = 0;
	int linux_mask = 0;

	fuse.u64 = cvmx_read_csr(CVMX_CIU_FUSE);
	fuse_tmp = fuse.s.fuse;
	cvmx_sysinfo_t * sysinfo = cvmx_sysinfo_get();

	/* find linux first core, it's not a good methord */
	linux_mask = fuse_tmp &  ((~(sysinfo->core_mask)) & 0xffff);
	while(linux_mask)
	{
		if((linux_mask & 0x1) != 0)
		{
			break;
		}
		linux_first_core++;
		linux_mask = linux_mask >> 1;
	}

	/* switch uart interrupt to linux core */
	while(fuse_tmp)
	{
		if((fuse_tmp & 0x1) != 0)
		{
			irq_control.u64 = cvmx_read_csr(CVMX_CIU_INTX_EN0(cpu*2));
			irq_control.s.uart=0;
			if (cpu == linux_first_core)
			{
				irq_control.s.uart=1<<uart_index;
			}
			cvmx_write_csr(CVMX_CIU_INTX_EN0(cpu*2), irq_control.u64);
		}
		cpu++;
		fuse_tmp = fuse_tmp>>1;
	}
}


/**
 * initilize the shell
 * param @uart_i in-and-out to the uart
 */
void shell_init(int uart_i)
{
	int i;
	cvmx_ciu_intx0_t irq_control;	
	uart_index = uart_i;

	/* Setup the interrupt handler */
	cvmx_interrupt_register(CVMX_IRQ_UART0+ uart_index, uart_interrupt, (void*)(unsigned long)uart_index);
	cvmx_interrupt_unmask_irq(CVMX_IRQ_UART0 + uart_index);


	/* Setup the uart */
	uart_setup(uart_index);
	clear_se_uart_note();

	if (product_info.se_mode == SE_MODE_STANDALONE)
	{
		/* Enable the CIU uart interrupt */
		irq_control.u64 = cvmx_read_csr(CVMX_CIU_INTX_EN0(cvmx_get_core_num()*2));
		irq_control.s.uart=(1<<uart_index);
		cvmx_write_csr(CVMX_CIU_INTX_EN0(cvmx_get_core_num()*2), irq_control.u64);
	}

	memset(shell_cmd_buf, 0, strlen(shell_cmd_buf));
	strncpy(prompt, DEFAULT_PROMPT, 32);

	for (i = 0; i < MAX_CMDS; i++)
		cmd_list[i].name[0] = 0;

}




