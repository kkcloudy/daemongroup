/* zebra daemon main routine.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include <zebra.h>

#include <lib/version.h>
#include "getopt.h"
#include "command.h"
#include "thread.h"
#include "filter.h"
#include "memory.h"
#include "prefix.h"
#include "log.h"
#include "privs.h"
#include "sigevent.h"

#include "zebra/rib.h"
#include "zebra/zserv.h"
#include "zebra/debug.h"
#include "zebra/router-id.h"
#include "zebra/irdp.h"
#include "zebra/rtadv.h"

/* Zebra instance */
struct zebra_t zebrad =
{
  .rtm_table_default = 0,
};

product_inf *product;
extern int board_type ;

/* process id. */
pid_t old_pid;
pid_t pid;

/* Pacify zclient.o in libzebra, which expects this variable. */
struct thread_master *master;

/* Route retain mode flag. */
int retain_mode = 1;

/* Don't delete kernel route. */
extern int keep_kernel_mode;

#ifdef HAVE_NETLINK
/* Receive buffer size for netlink socket */
u_int32_t nl_rcvbufsize = 0;
u_int32_t nl_senbufsize = 0;

#endif /* HAVE_NETLINK */

/* Command line options. */
struct option longopts[] = 
{
  { "batch",       no_argument,       NULL, 'b'},
  { "daemon",      no_argument,       NULL, 'd'},
  { "keep_kernel", no_argument,       NULL, 'k'},
  { "log_mode",    no_argument,       NULL, 'l'},
  { "config_file", required_argument, NULL, 'f'},
  { "pid_file",    required_argument, NULL, 'i'},
  { "help",        no_argument,       NULL, 'h'},
  { "vty_addr",    required_argument, NULL, 'A'},
  { "vty_port",    required_argument, NULL, 'P'},
  { "retain",      no_argument,       NULL, 'r'},
#ifdef HAVE_NETLINK
  { "nl-bufsize",  required_argument, NULL, 's'},
#endif /* HAVE_NETLINK */
  { "user",        required_argument, NULL, 'u'},
  { "group",       required_argument, NULL, 'g'},
  { "version",     no_argument,       NULL, 'v'},
  { 0 }
};

zebra_capabilities_t _caps_p [] = 
{
  ZCAP_NET_ADMIN,
  ZCAP_SYS_ADMIN,
  ZCAP_NET_RAW,
};

/* zebra privileges to run with */
struct zebra_privs_t zserv_privs =
{
#if defined(QUAGGA_USER) && defined(QUAGGA_GROUP)
  .user = QUAGGA_USER,
  .group = QUAGGA_GROUP,
#endif
#ifdef VTY_GROUP
  .vty_group = VTY_GROUP,
#endif
  .caps_p = _caps_p,
  .cap_num_p = sizeof(_caps_p)/sizeof(_caps_p[0]),
  .cap_num_i = 0
};

/* Default configuration file path. */
char config_default[] = SYSCONFDIR DEFAULT_CONFIG_FILE;

/* Process ID saved for use by init system */
const char *pid_file = PATH_ZEBRA_PID;

/* Help information display. */
static void
usage (char *progname, int status)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", progname);
  else
    {    
      printf ("Usage : %s [OPTION...]\n\n"\
	      "Daemon which manages kernel routing table management and "\
	      "redistribution between different routing protocols.\n\n"\
	      "-b, --batch        Runs in batch mode\n"\
	      "-d, --daemon       Runs in daemon mode\n"\
	      "-f, --config_file  Set configuration file name\n"\
	      "-i, --pid_file     Set process identifier file name\n"\
	      "-k, --keep_kernel  Don't delete old routes which installed by "\
				  "rtm.\n"\
	      "-l, --log_mode     Set verbose log mode flag\n"\
	      "-A, --vty_addr     Set vty's bind address\n"\
	      "-P, --vty_port     Set vty's port number\n"\
	      "-r, --retain       When program terminates, retain added route "\
				  "by rtm.\n"\
	      "-u, --user         User to run as\n"\
	      "-g, --group	  Group to run as\n", progname);
#ifdef HAVE_NETLINK
      printf ("-s, --nl-bufsize   Set netlink receive buffer size\n");
printf ("-t, --nl-bufsize	Set netlink send buffer size\n");

#endif /* HAVE_NETLINK */
      printf ("-v, --version      Print program version\n"\
	      "-h, --help         Display this help and exit\n"\
	      "\n");
    }

  exit (status);
}

/* SIGHUP handler. */
static void 
sighup (void)
{
  zlog_info ("SIGHUP received");

  /* Reload of config file. */
  ;
}

/* SIGINT handler. */
static void
sigint (void)
{
	  zlog_info ("SIGINT or SIGTERM received");
  zlog_notice ("Terminating on signal");

  if (!retain_mode)
    rib_close ();
#ifdef HAVE_IRDP
  irdp_finish();
#endif

  EXIT(0);
}

/* SIGUSR1 handler. */
static void
sigusr1 (void)
{
	  zlog_info ("SIGUSR1 received");
  zlog_rotate (NULL);
}

/* SIGPIPE handler. */
static void
sigpipe (void)
{
	  zlog_info ("SIGPIPE received");
  zlog_rotate (NULL);
  /*CID 11364 (#1 of 1): Missing return statement (MISSING_RETURN)
  	1. missing_return: Arriving at the end of a function without returning a value.
  	A bug , so add func statement(static void) before func name.*/
}


struct quagga_signal_t zebra_signals[] =
{
  { 
    .signal = SIGHUP, 
    .handler = &sighup,
  },
  {
    .signal = SIGUSR1,
    .handler = &sigusr1,
  },
  {
    .signal = SIGINT,
    .handler = &sigint,
  },
  {
    .signal = SIGTERM,
    .handler = &sigint,
  },
    {
		.signal = SIGPIPE,
		.handler = &sigpipe,
    }
};

extern 	int tipc_init(void);

board_id_list* board_id_list_new(char board_id)
{
	board_id_list *temp = NULL;
	temp=(board_id_list*)malloc(sizeof(board_id_list));
	if(NULL == temp)
	{
		zlog_err("%s : malloc error\n",__func__);
		return temp;
	}
	
	temp->next			= NULL;
	temp->prv				= NULL;
	temp->board_id	= board_id;
	
	return temp;
}

int add_board_id_to_list(board_id_list* board_id_list_head,char board_id)
{
	board_id_list *temp=board_id_list_head;

	for(;NULL != temp->next;temp=temp->next);

	temp->next = board_id_list_new(board_id);
	if(NULL == temp->next)
	{
		printf("board_id_list_new NULL\n");
		return -1;
	}
	temp->next->prv = temp;
	 
	return 0;
}

void show_all_board_id(board_id_list* board_id_list_head)
{
	board_id_list *temp;
	for(temp = board_id_list_head;NULL != temp->next;temp=temp->next)
	{
		printf("%d\t",temp->board_id);
	}
	printf("%d\n",temp->board_id);
}

int get_board_id(board_id_list *list, int i)
{
	board_id_list* temp;
	
		if(list == NULL)
		{
			printf("ERROR:take_board_id_from_list:board id list is NULL\n");
			return -1;
		}
		
		if(i < 0)
		{
			printf("ERROR:take_board_id_form_list:i < 0\n");
			return -1;
		}
	
		for(temp = list;i > 0;i--)
		{
			if(temp -> next == NULL)
			{
				printf("ERROR:take_board_id_form_list:bad i check it\n");
				return -1;
			}
	
			temp = temp -> next;
		}
		return (int)(temp -> board_id);

	
}

void show_product_information(product_inf* product)
{
	fprintf(stderr,"board id is:\t%d\n",product->board_id);
	fprintf(stderr,"board type is:\t%s\n",((BOARD_IS_VICE==product->board_type)?"VICE":"MASTER"));
	fprintf(stderr,"master number is:\t%d\n",product->master_num);
	fprintf(stderr,"vice number is:\t%d\n",product->vice_num);
	fprintf(stderr,"master board id is:\t");
	show_all_board_id(product->master_board_id_list);
	fprintf(stderr,"\nvice board is:\t");
	show_all_board_id(product->vice_board_id_list);
}

int open_read_file()
{
	FILE* fd = NULL;
	fd = fopen(PRODUCT_FILE,"r");
	if(NULL == fd)
	{
		perror("open file:");
		return -1;
	}
	fscanf(fd,"%d %d",&product->board_type,&product->board_id);
	fscanf(fd,"%d %d",&product->master_num,&product->vice_num);
	if(1)
	{
		int temp;
		int i;
		for(i=product->master_num;i>0;i--)
		{
			fscanf(fd,"%d",&temp);
			if(-1 == add_board_id_to_list(product->master_board_id_list,temp))
			{
				printf("error \n");
				/*CID 13534 (#1 of 4): Resource leak (RESOURCE_LEAK)
				14. leaked_storage: Variable "fd" going out of scope leaks the storage it points to.
				Bug .not fclose fd , so add.*/
				fclose(fd);/*add*/
				return -1;
			}
		}
	}
	if(1)
	{
		int temp;
		int i;
		for(i=product->vice_num;i>0;i--)
		{
			fscanf(fd,"%d",&temp);
			if(-1 == add_board_id_to_list(product->vice_board_id_list,temp))
			{
			 	/*CID 13534 (#2 of 4): Resource leak (RESOURCE_LEAK)
			  	 20. leaked_storage: Variable "fd" going out of scope leaks the storage it points to.
			   	Bug .not fclose fd , so add.*/
			  	fclose(fd);/*add*/
				return -1;
			}
		}
	}
	/*CID 13534 (#3-4 of 4): Resource leak (RESOURCE_LEAK)
	14. leaked_storage: Variable "fd" going out of scope leaks the storage it points to.
	Bug .not fclose fd , so add.*/
	fclose(fd);/*add*/
	
	/*CID 11363 (#1 of 1): Missing return statement (MISSING_RETURN)
	12. missing_return: Arriving at the end of a function without returning a value.
	Add return vlaue.*/
	return 0;/*add*/

}

product_inf* product_inf_new()
{
	product_inf* temp;
	temp = (product_inf*)malloc(sizeof(product_inf));
	if(NULL == temp)
	{
		zlog_err("%s : malloc error\n",__func__);
		return NULL;
	}
	temp->series = -1;
	temp->product_type = -1;
	temp->board_type = -1;
	memset(temp->board_name, 0,32);
	temp->board_id	 = -1;
	temp->board_code = -1;
	temp->is_master  = -1;
	temp->is_active_master = -1;
	temp->active_master_slotid = -1;
/*	temp->id_instance[0] = -1;*/
/*	temp->id_instance[1] = -1;*/
	memset(temp->board_state, -1,(sizeof(int))*32);
	memset(temp->id_instance, -1 ,(sizeof(int))*32);
	temp->master_num = 0;
	temp->vice_num	 = 0;
	temp->amount_num = 0;
	temp->alive_board_slotid = -1;
	
	temp->master_board_id_list = board_id_list_new(0);
	temp->vice_board_id_list	 = board_id_list_new(0);

	return temp;
}

void release_list(board_id_list* list)
{
	if(NULL == list->next)
	{
		free(list);
		return ; 
	}else{
		release_list(list->next);
	}
	free(list);
}

void release_product_inf(product_inf* product)
{
	release_list(product->master_board_id_list);
	release_list(product->vice_board_id_list);
	free(product);
}

#if 0
void
select_master_or_vice_board(void)
{
	product = product_inf_new();  
	if( -1 == open_read_file())
	{
		zlog_debug("open file of /blk/product for tipc fail\n");
	}
	else
	{
		show_product_information(product);
		board_type = product->board_type;
		if(board_type == BOARD_IS_ACTIVE_MASTER)
		  master_board_init(product);
		else
		  if(board_type == BOARD_IS_VICE)
			vice_board_init(product);		
	}

}
#endif

/*gujd: 2012-02-17 , am 11:40 . Add func for judge Product type .*/
int
judge_product_type(char *product_name)
{
	if(!product)
	{
		zlog_err("Error: struct Product is null .\n");
		return -1;
	}
	if(strncmp(product_name, AX7605I_PRODUCT_NAME, sizeof(AX7605I_PRODUCT_NAME))==0)/*7605i*/
	{
		product->product_type = PRODUCT_TYPE_7605I;
	}
	else if(strncmp(product_name, AX8603_PRODUCT_NAME, sizeof(AX8603_PRODUCT_NAME))==0)/*8603*/
	{
		product->product_type = PRODUCT_TYPE_8603;
	}
	else if(strncmp(product_name, AX8606_PRODUCT_NAME, sizeof(AX8606_PRODUCT_NAME))==0)/*8606*/
	{
		product->product_type = PRODUCT_TYPE_8606;
	}
	else if(strncmp(product_name, AX8610_PRODUCT_NAME, sizeof(AX8610_PRODUCT_NAME))==0)/*8610*/
	{
		product->product_type = PRODUCT_TYPE_8610;
	}
	else if(strncmp(product_name, AX8800_PRODUCT_NAME, sizeof(AX8800_PRODUCT_NAME))==0)/*8800*/
	{
		product->product_type = PRODUCT_TYPE_8800;
	}
	else
	{
		zlog_info("Unkown Product Name !\n");
		return -1;
	}
	return 0;
	
}

/* gjd: add for distribute product */
int
open_read_system_file()
{
	FILE *fd = NULL;
	int distribute_sys = -1;
	int product_type = -1;
	int ret = 0;
	
	char board_name[64];
	char product_name[64];

	memset(board_name, 0 ,64);
	memset(product_name, 0, 64);

	/*the switch of distribute system*/
	 fd = fopen(IS_DISTRIBUTE_SYSTEM,"r");
	 if(fd == NULL)
	 {
		 zlog_notice("open file /dbm/product/is_distributed failed.\n");
		 return -1;
	 }
	 fscanf(fd , "%d",&distribute_sys);
	 fclose(fd);
	 if(distribute_sys != 1)
	 	return -1;
	 /*2011-06-27: pm 3:00*/
	 
	#if 0 
	 /**fetch product type**/
	fd = fopen(PRODUCT_TYPE_FILE,"r");
	if(NULL == fd)
	{
		zlog_notice("open file /dbm/product/product_serial failed\n");
		return -1;
	}
	fscanf(fd,"%d",&product_type);
	if(product_type == 7)
		product->product_type = PRODUCT_TYPE_76;/////////////////////////will change
	else if(product_type == 8)
		product->product_type = PRODUCT_TYPE_86;
	else
		{
			zlog_warn("unkown product type !\n");
			fclose(fd);
			return -1;
		}
	
	fclose(fd);
	#else
	/**fetch product name , use to determine which type this product is . gujd : 2012-02-17, am 10:55**/
	fd = fopen(PRODUCT_NAME_FILE,"r");
	if(NULL == fd)
	{
		zlog_notice("open file /dbm/product/name failed\n");		
		fclose(fd);
		return -1;
	}
	fscanf(fd,"%s",product_name);
	zlog_notice("This Product is %s ..\n",product_name);
	fclose(fd);
	ret = judge_product_type(product_name);
	if(ret < 0)
	{
		zlog_info("Init Product type failed .\n");
		return -1;
	}
	#endif
	
	/**fetch board type**/
	fd = fopen(IS_MASTER_FILE,"r");
	if(NULL == fd)
	{
		zlog_notice("open file /dbm/local_board/is_master failed\n");
		return -1;
	}
	fscanf(fd,"%d",&product->is_master);
	fclose(fd);
	
	fd = fopen(IS_ACTIVE_MASTER_FILE,"r");
	if(NULL == fd)
	{
		zlog_notice("open file /dbm/local_board/is_active_master failed\n");
		return -1;
	}
	fscanf(fd,"%d",&product->is_active_master);
	fclose(fd);

	fd = fopen(IS_ACTIVE_MASTER_SLOT_FILE,"r");
	if(NULL == fd)
	{
		zlog_notice("open file /dbm/local_board/active_master_slot_id failed\n");
		return -1;
	}
	fscanf(fd,"%d",&product->active_master_slotid);
	fclose(fd);
	
	/**fetch slot id**/
	fd = fopen(SLOT_ID_FILE,"r");
	if(fd == NULL)
	{
		zlog_notice("open file /dbm/local_board/slot_id failed\n");
		return -1;	
	}
	fscanf(fd, "%d", &product->board_id);
	zlog_debug("My board slot is %d \n",product->board_id);
	fclose(fd);

	/*fetch alive board slot id*/
	fd = fopen(PRODUCT_ALIVE_BOARD_SLOTID,"r");
	 if(fd == NULL)
	 {
		 zlog_notice("open file /dbm/product/board_on_mask failed.\n");
		 return -1;
	 }
	 fscanf(fd , "%d",&product->alive_board_slotid);
	 fclose(fd);

	 /*fetch loacal board name*/
	fd = fopen(BOARD_NAME_FILE,"r");
	 if(fd == NULL)
	 {
		 zlog_notice("open file /dbm/local_board/name failed.\n");
		 return -1;
	 }
	 fscanf(fd , "%s",board_name);
	 zlog_notice("This slot of board name is %s ..\n",board_name);
	 fclose(fd);

	 /*fetch loacal board code,  conect the product serial , to judge the xxx board , above board often change ,so use board code and serial to judge .not use baord name at all .*/
	fd = fopen(BOARD_CODE_FILE,"r");
	 if(fd == NULL)
	 {
		 zlog_notice("open file /dbm/local_board/board_code failed.\n");
		 return -1;
	 }
	 fscanf(fd , "%d", &product->board_code);
	 zlog_notice("This slot of board code is %d ..\n",product->board_code);
	 fclose(fd);
	 
	return 0;

}

void
judge_local_board_name(product_inf *product)
{
	if(!product)
	{
		zlog_err("Product is null , not Distribute System. \n");
		return ;
	}

	/*if(product->product_type == PRODUCT_TYPE_76)*//*7605i*/
	if(product->product_type == PRODUCT_TYPE_7605I)/*7605i*/
	{
		if(product->board_code == AX71_CRSMU_BOARD_CODE)/*71_SMU*/
		{
			strncpy(product->board_name,AX71_CRSMU,sizeof(AX71_CRSMU));
			zlog_info("This board is : %s .\n",product->board_name);
			return;
		}
		else if(product->board_code == AX71_2X12G12S_BOARD_CODE)/*71_2X*/
		{
			strncpy(product->board_name,AX71_2X12G12S,sizeof(AX71_2X12G12S));
			zlog_info("This board is : %s .\n",product->board_name);
			return;
		}
		else
		{
			zlog_warn("Unknown 7605i board code !\n");
			return;
		}
		
	}
//	else if(product->product_type == PRODUCT_TYPE_86)/*8610*/
	else if((product->product_type == PRODUCT_TYPE_8603)/*8603*/
			||(product->product_type == PRODUCT_TYPE_8606)/*8606*/
			||(product->product_type == PRODUCT_TYPE_8610)/*8610*/
			||(product->product_type == PRODUCT_TYPE_8800))/*8800*/
	/*can use product series 86XX*/
	 {
		if(product->board_code == AX81_CRSMU_BOARD_CODE)/*81_SMU*/
		{
			strncpy(product->board_name,AX81_SMU,sizeof(AX81_SMU));
			zlog_info("This board is : %s .\n",product->board_name);
			return;
		}
		else if(product->board_code == AX81_2X12G12S_BOARD_CODE)/*81_2X*/
		{
			strncpy(product->board_name,AX81_2X12G12S,sizeof(AX81_2X12G12S));
			zlog_info("This board is : %s .\n",product->board_name);
			return;
		}
		else if(product->board_code == AX81_AC12C_BOARD_CODE)/*81_AC12C*/
		{
			strncpy(product->board_name,AX81_AC12C,sizeof(AX81_AC12C));
			zlog_info("This board is : %s .\n",product->board_name);
			return;
		}
		else if(product->board_code == AX81_AC8C_BOARD_CODE)
		{
			strncpy(product->board_name,AX81_AC8C,sizeof(AX81_AC8C));/*81_AC8C*/
			zlog_info("This board is : %s .\n",product->board_name);
			return;
		}
		else if(product->board_code == AX81_1X12G12S_BOARD_CODE)/*81_1X(FPGA)*/
		{
			strncpy(product->board_name,AX81_1X12G12S,sizeof(AX81_1X12G12S));
			zlog_info("This board is : %s .\n",product->board_name);
			return;
		}/*gujd : 2012-05-10, am 10:40 .Add code for mange FPGA board .*/
		
		else if(product->board_code == AX81_AC4X_BOARD_CODE)/*81_4X*/
		{
			strncpy(product->board_name,AX81_AC4X,sizeof(AX81_AC4X));
			zlog_info("This board is : %s .\n",product->board_name);
			return;
		}/*gujd : 2012-10-31, pm 6:10 .Add code for mange 4X board .*/
		
		else	
		{
			zlog_warn("Unknown  board code !\n");
			return;
		}
		
	}
	else
	 {
	 	zlog_warn("Unkown product type[%d] .\n",product->product_type);
		return ;
		
	    }
}

void
product_boot_every_board_state(product_inf *product)
{
	int id_mask = -1;
	int i,slot = -1;
	int alive_count = 0;

	if(product == NULL)/*not Distribute System*/
		return;
	
	id_mask = product->alive_board_slotid;
	for(i = 0; i < 32 ; i++)
	 {
		if( (id_mask >> i) & 1)
		 {
			product->board_state[i] = BOARD_ON_PRODUCT;
		 }
		else
			product->board_state[i] = BOARD_OFF_PRODUCT;

	}
	
	/*	for(i = 0; i < 32; i++) */	/*其实就10个槽位，不用取到32*/
	/*for(i = 0; i < 10; i++)*/
	for(i = 0; i < 32 ; i++) /*enlarge reach to 32 slot*/
	{
		if(product->board_state[i] == BOARD_ON_PRODUCT)
		{
			slot = i + 1;		/*因为i是从0开始的，与slot之间差1*/
			alive_count++;		/*所有在位板子的一个计数器*/
			product->id_instance[i] = slot; 	/*记录slot作为创建socket的实例号*/				
		}
	}
	if(product->product_type == PRODUCT_TYPE_7605I)/*7605i*/
		product->amount_num = 3;
	else if(product->product_type == PRODUCT_TYPE_8603)/*8603*/
		product->amount_num = 3;
	else if(product->product_type == PRODUCT_TYPE_8606)/*8606*/
		product->amount_num = 6;
	else if(product->product_type == PRODUCT_TYPE_8610)/*8610*/
		product->amount_num = 10;
	else if(product->product_type == PRODUCT_TYPE_8800)/*8800*/
		product->amount_num = 14;
	else	/*in future suppor more*/
		zlog_warn("unkown product !\n");
	
	return;
	
}


/* gjd: add for distribute product system or normal system*/
void
select_board_master_or_vice()
{
	product = product_inf_new();
	/**open  every file and fetch data**/
	if(open_read_system_file() < 0)
	{
		zlog_notice("+++++++++++++++Normal System start up++++++++++++++\n");
		release_product_inf(product);
		product = NULL;
		return;
	}
	////////////////////add product series here///////////////////////////
	judge_local_board_name(product);
	product_boot_every_board_state(product);
	
	if(product->is_master == 1)/**board_type : master**/
	{
		if(product->is_active_master == 0)
		{
		  zlog_notice("+++++++++++++++Distributed System :Backup Master board start up++++++++++++++\n");
		  product->board_type = BOARD_IS_BACKUP_MASTER;/**backup master board , master (use as)-->vice**/
		  vice_board_init(product);
//		  return 0;
		}
		else
		{			
			zlog_notice("+++++++++++++++Distributed System : Active Master board start up++++++++++++++\n");
			product->board_type = BOARD_IS_ACTIVE_MASTER;/**active master board**/
			master_board_init(product);
//			return 1;
		}
	}
    else if(product->is_master == 0)
	{
		zlog_notice("+++++++++++++++Distributed System :Vice board start up++++++++++++++\n");
		product->board_type = BOARD_IS_VICE;
		vice_board_init(product);
//		return 0;
	}

		
}



/* Main startup routine. */
int
main (int argc, char **argv)
{
  char *p;
  char *vty_addr = NULL;
  int vty_port = ZEBRA_VTY_PORT;
  int batch_mode = 0;
  int daemon_mode = 0;
  char *config_file = NULL;
  char *progname;
  struct thread thread;
  int ret;

  /* Set umask before anything for security */
  umask (0027);

  /* preserve my name */
  progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);

  zlog_default = openzlog (progname, ZLOG_ZEBRA,
			   LOG_NDELAY|LOG_PID, LOG_DAEMON);

  while (1) 
    {
      int opt;
  
#ifdef HAVE_NETLINK  
      opt = getopt_long (argc, argv, "bdklf:i:hA:P:ru:g:vst:", longopts, 0);
#else
      opt = getopt_long (argc, argv, "bdklf:i:hA:P:ru:g:v", longopts, 0);
#endif /* HAVE_NETLINK */

      if (opt == EOF)
	break;

      switch (opt) 
	{
	case 0:
	  break;
	  /*CID 11166 (#1 of 1): Missing break in switch (MISSING_BREAK)
		unterminated_case: This case (value 98) is not terminated by a 'break' statement.
		Problem. Add a break.*/
	case 'b':
	  batch_mode = 1;
	  break;/*add*/
	case 'd':
	  daemon_mode = 1;
	  break;
	case 'k':
	  keep_kernel_mode = 1;
	  break;
	case 'l':
	  /* log_mode = 1; */
	  break;
	case 'f':
	  config_file = optarg;
	  break;
	case 'A':
	  vty_addr = optarg;
	  break;
        case 'i':
          pid_file = optarg;
          break;
	case 'P':
	  /* Deal with atoi() returning 0 on failure, and zebra not
	     listening on zebra port... */
	  if (strcmp(optarg, "0") == 0) 
	    {
	      vty_port = 0;
	      break;
	    } 
	  vty_port = atoi (optarg);
	  vty_port = (vty_port ? vty_port : ZEBRA_VTY_PORT);
	  break;
	case 'r':
	  retain_mode = 1;
	  break;
#ifdef HAVE_NETLINK
	case 's':
	  nl_rcvbufsize = atoi (optarg);
	  break;
	 case 't':
	  nl_senbufsize = atoi (optarg);
	  break;
#endif /* HAVE_NETLINK */
	case 'u':
	  zserv_privs.user = optarg;
	  break;
	case 'g':
	  zserv_privs.group = optarg;
	  break;
	case 'v':
	  print_version (progname);
	  exit (0);
	  break;
	case 'h':
	  usage (progname, 0);
	  break;
	default:
	  usage (progname, 1);
	  break;
	}
    }

  /* Make master thread emulator. */
  zebrad.master = thread_master_create ();
  master = zebrad.master;
  /* privs initialise */
  zprivs_init (&zserv_privs);

  /* Vty related initialize. */
  /*added by gxd*/
  signal_init (zebrad.master, Q_SIGC(zebra_signals), zebra_signals);
  cmd_init (1);
  vty_init (zebrad.master);
  memory_init ();

  /* Zebra related initialize. */
  zebra_init ();
  rib_init ();
  zebra_if_init ();
  zebra_debug_init ();
  router_id_init();
  zebra_vty_init ();
  access_list_init ();
  rtadv_init ();
#ifdef HAVE_IRDP
  irdp_init();
#endif

  /* For debug purpose. */
  /* SET_FLAG (zebra_debug_event, ZEBRA_DEBUG_EVENT); */

  /* Make kernel routing socket. */
  kernel_init ();
  interface_list ();
  route_read ();

  /* Sort VTY commands. */
  sort_node ();

#ifdef HAVE_SNMP
  zebra_snmp_init ();
#endif /* HAVE_SNMP */

  /* Clean up self inserted route. */
  if (! keep_kernel_mode)
    rib_sweep_route ();

  /* Configuration file read*/
  host_config_set(RTMD_DEFAULT_CONFIG_HOST);
/*  vty_read_config (config_file, config_default);*//*Move it to below.*/

  /* Clean up rib. */
  rib_weed_tables ();

  /* Exit when zebra is working in batch mode. */
  if (batch_mode)
    exit (0);

  /* Needed for BSD routing socket. */
  old_pid = getpid ();

  /* Daemonize. */
  if (daemon_mode)
    daemon (0, 0);

  /* Output pid of zebra. */
  pid_output (pid_file);

  /* Needed for BSD routing socket. */
  pid = getpid ();

  /* Make vty server socket. */
  vty_serv_sock (vty_addr, vty_port, ZEBRA_VTYSH_PATH);
//  zclient_master(zebrad.master);

  /**gjd: for select master board or vice board for distributed product**/
//  select_master_or_vice_board();
  select_board_master_or_vice();
  /*ve_interface_rpa_init();*/
  ve_interface_rpa_init_v2();/*gujd: 2012-06-06, pm 4:00. Add for support muti-core.*/
  eth_interface_rpa_init();
  
  vty_read_config (config_file, config_default);/*gujd : 2012-08-14, Move it from above to here, when rtmd restart because waite Distribute System init.*/

  /* Print banner. */
  zlog_notice ("Rtmd starting: \n");

  while (thread_fetch (zebrad.master, &thread)){
	  zlog_notice ("thread_call %s \n",thread.funcname);
	thread_call (&thread);
  }

  /* Not reached... */
  return 0;
}
