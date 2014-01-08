/* Virtual terminal interface shell.
 * Copyright (C) 2000 Kunihiro Ishiguro
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
#include <sys/types.h>
#include <unistd.h>

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <pwd.h>
#include <dlfcn.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <lib/version.h>
#include <syslog.h>
#include "getopt.h"
#include "command.h"
#include "memory.h"

#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include <lib/version.h>
#include <termios.h>



/* VTY shell program name. */
char *progname;

/* Configuration file name and directory. */
char config_default[] = SYSCONFDIR VTYSH_DEFAULT_CONFIG;

/* Flag for indicate executing child command. */
int execute_flag = 0;

/* For sigsetjmp() & siglongjmp(). */
static sigjmp_buf jmpbuf;
static sigjmp_buf jmpbuffer;

/* Flag for avoid recursive siglongjmp() call. */
static int jmpflag = 0;

/* A static variable for holding the line. */
static char *line_read;

/* Master of threads. */
struct thread_master *master;

/*gjd: add for Distribute System*/
int vtysh_enable_flags = 0;

/* SIGTSTP handler.  This function care user's ^Z input. */
void
sigtstp (int sig)
{
  /* Execute "end" command. */
  vtysh_execute ("end");
  
  /* Initialize readline. */
  rl_initialize ();
  printf ("\n");

  /* Check jmpflag for duplicate siglongjmp(). */
  if (! jmpflag)
    return;
  /* vtysh_kill_more();*/

  jmpflag = 0;

  /* Back to main command loop. */
  siglongjmp (jmpbuf, 1);
  
}

/* SIGINT handler.  This function care user's ^Z input.  */


void
sigint (int sig)
{
  /* Check this process is not child process. */
 if (! execute_flag)
    {
      show_input_echo();
	  rl_initialize();
      printf ("\n");
  if (! jmpflag)
    return;
  siglongjmp (jmpbuffer, 1);
    }

}

/* Signale wrapper for vtysh. We don't use sigevent because
 * vtysh doesn't use threads. TODO */
RETSIGTYPE *
vtysh_signal_set (int signo, void (*func)(int))
{
  int ret;
  struct sigaction sig;
  struct sigaction osig;

  sig.sa_handler = func;
  sigemptyset (&sig.sa_mask);
  sig.sa_flags = 0;
#ifdef SA_RESTART
  sig.sa_flags |= SA_RESTART;
#endif /* SA_RESTART */

  ret = sigaction (signo, &sig, &osig);

  if (ret < 0) 
    return (SIG_ERR);
  else
    return (osig.sa_handler);
}

/* Initialization of signal handles. */
void
vtysh_signal_init ()
{
  vtysh_signal_set (SIGINT, sigint);
  vtysh_signal_set (SIGTSTP, sigtstp);
  vtysh_signal_set (SIGPIPE, SIG_IGN);
}

/* Help information display. */
static void
usage (int status)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", progname);
  else
    printf ("Usage : %s [OPTION...]\n\n" \
	    "Integrated shell for Quagga routing software suite. \n\n" \
	    "-b, --boot               Execute boot startup configuration\n" \
	    "-c, --command            Execute argument as command\n" \
	    "-d, --daemon             Connect only to the specified daemon\n" \
	  	"-E, --echo				  Echo prompt and command in -c mode\n" \
	  	"-f, --config_file  	  Set configuration file name\n"\
	    "-h, --help               Display this help and exit\n\n" \
	    "Note that multiple commands may be executed from the command\n" \
	    "line by passing multiple -c args, or by embedding linefeed\n" \
	    "characters in one or more of the commands.\n\n" , progname);

  exit (status);
}

/* VTY shell options, we use GNU getopt library. */
struct option longopts[] = 
{
  { "boot",                 no_argument,             NULL, 'b'},
  /* For compatibility with older zebra/quagga versions */
  { "eval",                 required_argument,       NULL, 'e'},
  { "command",              required_argument,       NULL, 'c'},
  { "daemon",               required_argument,       NULL, 'd'},
  { "echo",                 no_argument,             NULL, 'E'},
  { "config_file",   		required_argument, 	NULL, 'f'},
  { "help",                 no_argument,             NULL, 'h'},
  { 0 }
};

/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
char *
vtysh_rl_gets ()
{
  HIST_ENTRY *last;
  /* If the buffer has already been allocated, return the memory
   * to the free pool. */
  if (line_read)
    {
      free (line_read);
      line_read = NULL;
    }
     
  /* Get a line from the user.  Change prompt according to node.  XXX. */
  line_read = readline (vtysh_prompt ());
/*	set_idle_time_init();*/
     
  /* If the line has any text in it, save it on the history. But only if
   * last command in history isn't the same one. */
  if (line_read && *line_read)
    {
      using_history();
      last = previous_history();
      if (!last || strcmp (last->line, line_read) != 0)
				add_history (line_read);
    }
     
  return (line_read);
}


void *dcli_dl_handle;
void *dcli_dl_handle_sem;
void *dcli_dl_handle_npd;
void *dcli_dl_handle_asd;
void *dcli_dl_handle_wid;

void (*dcli_send_dbus_signal_func)(const char* name,const char* str);
void (*dcli_sync_file)(const char* file, int syn_to_blk) = NULL;
int (*dcli_get_rpa_broadcast_mask)(struct vty *vty, int slot_id, unsigned int *mask, int *is_default_mask) = NULL;


#ifdef DISTRIBUT
int dl_dcli_init(boot_flag)
{

	void (*dcli_init_func)(int); /*dcli_main*/
	/***************sem*************/ 
	void (*dcli_sem_init_func)(void);
	void (*dcli_fpga_init_func)(void);
	/***************npd*************/
	void (*dcli_diag_init_func)(void);
	void (*dcli_vlan_init_func)(void);
	void (*dcli_trunk_init_func)(void);
	void (*dcli_dynamic_trunk_init_func)(void);
	void (*dcli_fdb_init_func)(void);
	void (*dcli_qos_init_func)(void);
	void (*dcli_acl_init_func)(void);
	void (*dcli_stp_element_init_func)(void);
	void (*dcli_drv_routesyn_init_func)(void);
	void (*dcli_tunnel_init_func)(void);
	void (*dcli_pvlan_init_func)(void);
	void (*dcli_prot_vlan_element_init_func)(void);
	void (*dcli_igmp_snp_element_init_func)(void);
	void (*dcli_mld_snp_element_init_func)(void);
	void (*dcli_mirror_init_func)(void);
	/*************asd****************/
	void (*dcli_sta_init_func)(void);
	void (*dcli_security_init_func)(void);
	/************wid******************/
	void (*dcli_wtp_init_func)(void);
	void (*dcli_radio_init_func)(void);
	void (*dcli_ac_init_func)(void);
	void (*dcli_wlan_init_func)(void);
	void (*dcli_ebr_init_func)(void); 
	void (*dcli_aciplist_init_func)(void);
	void (*dcli_ac_group_init_func)(void);
    void (*dcli_ap_group_init_func)(void);
	void (*dcli_bsd_init_func)(void);
	void (*dcli_license_init_func)(void);
	void (*dcli_wbridge_init_func)(void);

	char *error;
	int temp=boot_flag;

/***********************dcli_main********************/
	dcli_dl_handle = dlopen("libdcli.so.0",RTLD_NOW);
	if (!dcli_dl_handle) {
		fputs (dlerror(),stderr);
		printf(" Run without /opt/lib/libdcli.so.0\n");
		EXIT(1);

	}
	dcli_init_func = dlsym(dcli_dl_handle,"dcli_init");
	if ((error = dlerror()) != NULL) {
		printf(" Run without dcli_init be called.\n");
		fputs(error,stderr);
		EXIT(1);
	}
	(*dcli_init_func)(temp);


/***********************dcli_sem**********************/

	if (dcli_dl_handle_sem = dlopen("libdcli_sem.so",RTLD_NOW)){

		dcli_sem_init_func = dlsym(dcli_dl_handle_sem,"dcli_sem_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_sem_init be called.\n");
			fputs(error,stderr);
			//EXIT(1);
		} 
		else 
		{
			(*dcli_sem_init_func)(); 
		}

		dcli_fpga_init_func = dlsym(dcli_dl_handle_sem,"dcli_fpga_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_fpga_init be called.\n");
			fputs(error,stderr);
			//EXIT(1);
		} 
		else 
		{
			(*dcli_fpga_init_func)(); 
		}

	}
	else 
	{
		fputs (dlerror(),stderr);
		printf(" Run without /opt/lib/libdcli_sem.so\n");
	}
#ifndef _D_NANOCELL_
/*******************dcli_npd****************************/
	if (dcli_dl_handle_npd = dlopen("libdcli_npd.so",RTLD_NOW)){

		dcli_diag_init_func = dlsym(dcli_dl_handle_npd,"dcli_diag_init");
		if ((error = dlerror()) != NULL) 
		{
			printf(" Run without dcli_diag_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_diag_init_func)(); 
		}


		dcli_vlan_init_func = dlsym(dcli_dl_handle_npd,"dcli_vlan_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_vlan_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_vlan_init_func)(); 
		}


		dcli_trunk_init_func = dlsym(dcli_dl_handle_npd,"dcli_trunk_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_trunk_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_trunk_init_func)(); 
		}


		dcli_dynamic_trunk_init_func = dlsym(dcli_dl_handle_npd,"dcli_dynamic_trunk_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_dynamic_trunk_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_dynamic_trunk_init_func)(); 
		}

		dcli_fdb_init_func = dlsym(dcli_dl_handle_npd,"dcli_fdb_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_fdb_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_fdb_init_func)(); 
		}

		dcli_qos_init_func = dlsym(dcli_dl_handle_npd,"dcli_qos_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_qos_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_qos_init_func)(); 
		}

		dcli_acl_init_func = dlsym(dcli_dl_handle_npd,"dcli_acl_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_acl_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_acl_init_func)(); 
		}

		dcli_stp_element_init_func = dlsym(dcli_dl_handle_npd,"dcli_stp_element_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_stp_element_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_stp_element_init_func)(); 
		}

		dcli_drv_routesyn_init_func = dlsym(dcli_dl_handle_npd,"dcli_drv_routesyn_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_drv_routesyn_init_func be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_drv_routesyn_init_func)(); 
		}

		dcli_tunnel_init_func = dlsym(dcli_dl_handle_npd,"dcli_tunnel_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_tunnel_init_func be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_tunnel_init_func)(); 
		}

		dcli_pvlan_init_func = dlsym(dcli_dl_handle_npd,"dcli_pvlan_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_pvlan_init_func be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_pvlan_init_func)(); 
		}

		dcli_prot_vlan_element_init_func = dlsym(dcli_dl_handle_npd,"dcli_prot_vlan_element_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_prot_vlan_element_init_func be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_prot_vlan_element_init_func)(); 
		}	

		dcli_igmp_snp_element_init_func = dlsym(dcli_dl_handle_npd,"dcli_igmp_snp_element_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_igmp_snp_element_init_func be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_igmp_snp_element_init_func)(); 
		}	


		dcli_mld_snp_element_init_func = dlsym(dcli_dl_handle_npd,"dcli_mld_snp_element_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_mld_snp_element_init_func be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_mld_snp_element_init_func)(); 
		}	

		dcli_mirror_init_func = dlsym(dcli_dl_handle_npd,"dcli_mirror_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_mirror_init_func be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_mirror_init_func)(); 
		}


	}
	else 
	{
		fputs (dlerror(),stderr);
		printf(" Run without /opt/lib/libdcli_npd.so\n");
	}
#endif

/*******************dcli_asd****************************/

	if (dcli_dl_handle_asd = dlopen("libdcli_asd.so",RTLD_NOW)){

		dcli_sta_init_func = dlsym(dcli_dl_handle_asd,"dcli_sta_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_sta_init be called.\n");
			fputs(error,stderr);
			//EXIT(1);
		} 
		else 
		{
			(*dcli_sta_init_func)(); 
		}


		dcli_security_init_func = dlsym(dcli_dl_handle_asd,"dcli_security_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_security_init be called.\n");
			fputs(error,stderr);
			//EXIT(1);
		} 
		else 
		{
			(*dcli_security_init_func)(); 
		}

	}
	else 
	{
		fputs (dlerror(),stderr);
		printf(" Run without /opt/lib/libdcli_sta.so\n");
	}


/*******************dcli_wid************************************/

	if (dcli_dl_handle_wid = dlopen("libdcli_wid.so",RTLD_NOW)){

		dcli_wtp_init_func = dlsym(dcli_dl_handle_wid,"dcli_wtp_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_wtp_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_wtp_init_func)(); 
		}


		dcli_radio_init_func = dlsym(dcli_dl_handle_wid,"dcli_radio_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_radio_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_radio_init_func)(); 
		}

		dcli_ac_init_func = dlsym(dcli_dl_handle_wid,"dcli_ac_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_ac_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_ac_init_func)(); 
		}


		dcli_wlan_init_func = dlsym(dcli_dl_handle_wid,"dcli_wlan_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_wlan_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_wlan_init_func)(); 
		}

		dcli_ebr_init_func = dlsym(dcli_dl_handle_wid,"dcli_ebr_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_ebr_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_ebr_init_func)(); 
		}

		dcli_aciplist_init_func = dlsym(dcli_dl_handle_wid,"dcli_aciplist_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_aciplist_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_aciplist_init_func)(); 
		}

		dcli_ac_group_init_func = dlsym(dcli_dl_handle_wid,"dcli_ac_group_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_ac_group_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_ac_group_init_func)(); 
		}

		dcli_ap_group_init_func = dlsym(dcli_dl_handle_wid,"dcli_ap_group_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_ap_group_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_ap_group_init_func)(); 
		}

	
		dcli_bsd_init_func = dlsym(dcli_dl_handle_wid,"dcli_bsd_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_bsd_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_bsd_init_func)(); 
		}		

		dcli_license_init_func = dlsym(dcli_dl_handle_wid,"dcli_license_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_license_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_license_init_func)(); 
		}	

		dcli_wbridge_init_func = dlsym(dcli_dl_handle_wid,"dcli_wbridge_init");
		if ((error = dlerror()) != NULL) {
			printf(" Run without dcli_wbridge_init be called.\n");
			fputs(error,stderr);
		} 
		else 
		{
			(*dcli_wbridge_init_func)(); 
		}	
				

	}
	else 
	{
		fputs (dlerror(),stderr);
		printf(" Run without /opt/lib/libdcli_wid.so\n");
	}
	vtysh_send_dbus_signal_init();

	vtysh_sync_file_init();

	vtysh_get_rpa_broadcast_mask_init();

	/*
	 * The handle should not be closed before we do not use libdcli any more.
	 */
	return 1;

}
#else
int dl_dcli_init()
{
  void (*dcli_init_func)();
  char *error;

  dcli_dl_handle = dlopen("libdcli.so.0",RTLD_NOW);
  if (!dcli_dl_handle) {
      fputs (dlerror(),stderr);
      printf("Run without /opt/lib/libdcli.so.0\n");
      //exit(1);
      
  }
  dcli_init_func = dlsym(dcli_dl_handle,"dcli_init");
  if ((error = dlerror()) != NULL) {
      printf("Run without dcli_init be called.\n");
      fputs(error,stderr);
      //exit(1);

  }
  (*dcli_init_func)();
  vtysh_send_dbus_signal_init();
  vtysh_sync_file_init();

  /*
   * The handle should not be closed before we do not use libdcli any more.
   */
  return 1;

}
#endif
void vtysh_send_dbus_signal_init(void)
{
	
	char *error;
	dcli_send_dbus_signal_func = dlsym(dcli_dl_handle,"dcli_send_dbus_signal");
	if ((error = dlerror()) != NULL) {
		printf("Run without dcli_send_dbus_signal be called.\n");
		fputs(error,stderr);
	}

}

void vtysh_sync_file_init(void)
{
	
	char *error;
	dcli_sync_file = dlsym(dcli_dl_handle,"sync_file");
	if ((error = dlerror()) != NULL) {
		printf("Run without print_msg_form_vtysh be called.\n");
		fputs(error,stderr);
	}

}

void vtysh_get_rpa_broadcast_mask_init(void)
{
	char *error;
	dcli_get_rpa_broadcast_mask = dlsym(dcli_dl_handle,"get_rpa_broadcast_mask");
	if ((error = dlerror()) != NULL) {
		printf("Run without dcli_get_rpa_broadcast_mask be called.\n");
		fputs(error,stderr);
	}
}

void get_default_config(void)
{
	FILE* confp=NULL;
	char* buf = NULL;
	char* ptr = NULL;
	char tmp[128];


	memset(tmp,0,128);	
	confp = fopen(VTYSH_DEFAULT_CONFIG_FILE,"r");
	if (!confp )
	  return ;
	/*CID 14560 (#1 of 1): Unused pointer value (UNUSED_VALUE)returned_pointer:
	Pointer "ptr" returned by "fgets(tmp, 128, confp)" is never used*/
	while(ptr = fgets(tmp,128,confp))
	{
		if(strstr(tmp,CONFIG_PRE_MARK))
		{
			buf = (char*)malloc(128);
			memcpy(buf,tmp+strlen(CONFIG_PRE_MARK),strlen(tmp)-strlen(CONFIG_PRE_MARK)-1);
			integrate_default = buf;/*CID 13526 (#1 of 1):  overwrite_var: Overwriting "integrate_default" in "integrate_default = buf" leaks the storage that "integrate_default" points to*/
			continue;
		}
		
		if(strstr(tmp,DEFAULT_ADMIN_USER_MARK))
		{
			buf = (char*)malloc(128);
			memcpy(buf,tmp+strlen(DEFAULT_ADMIN_USER_MARK),strlen(tmp)-strlen(DEFAULT_ADMIN_USER_MARK)-1);
			default_admin_user = buf;
			continue;
		}
	}
	fclose(confp);
	
	
}
/* VTY shell main routine. */
extern char* configfile;

int boot_flag = 0;
int vtysh_config_flag = 0;
int idle_time = IDLE_TIME_DEFAULT;
int idle_time_rem ;
#ifdef DISTRIBUT
int tipc_dbus_flag = 0;
#endif
/* start - added by zhengbo 2011-12-23 */
int cmd_flag = 0;
/* end - added by zhengbo 2011-12-23 */


struct termios bktty;
void sigalrm_idtime_func(int sig)
{

	if(idle_time_rem <=0 && idle_time!=0)
	{
		fprintf(stdout,"\n!\n!\nThe idle time out,user exit!!\n");
		tcsetattr(fileno(stdout),TCSANOW,&bktty);
		exit(0);
	}
	else
		idle_time_rem--;

	alarm(idle_time_sec);
	return;

}

void init_idel_time(void)

{
	set_idle_time_init();
	signal(SIGALRM, sigalrm_idtime_func);
	alarm(idle_time_sec);
	return;
}

int judge_is_active_master()
{
	FILE *fd = NULL;
	int is_distribute = -1;
	int is_master = -1;
	int is_active_master = -1;

	/*the switch of distribute system*/
	 fd = fopen(IS_DISTRIBUTE_SYSTEM,"r");
	 if(fd == NULL)
	 {
		 zlog_notice("open file /dbm/product/is_distributed failed.\n");
		 return -1;
	 }
	 if(1!=fscanf(fd , "%d",&is_distribute))
	{
		fclose(fd);
		zlog_notice("Get product is_distribute error\n");
		return -1;
	}
	 fclose(fd);
	 if(is_distribute != 1)
	 {
	 	zlog_notice("The product is not Distribute System !\n");/*normal system , not Distribute System  */
	 	return 2;
	 	}
	/**fetch board type**/
	fd = fopen(IS_MASTER_FILE,"r");
	if(NULL == fd)
	{
		zlog_notice("open file /dbm/local_board/is_master failed\n");
		return -1;
	}
	
	if(1!=fscanf(fd,"%d",&is_master))
	{
		fclose(fd);
		zlog_notice("Get product is_master error\n");
		return -1;
	}
	fclose(fd);
	
	fd = fopen(IS_ACTIVE_MASTER_FILE,"r");
	if(NULL == fd)
	{
		zlog_notice("open file /dbm/local_board/is_active_master failed\n");
		return -1;
	}
	
	if(1!=fscanf(fd,"%d",&is_active_master))
	{
		fclose(fd);
		zlog_notice("Get product is_active_master error\n");
		return -1;
	}
	fclose(fd);

	if(is_master == 1 && is_active_master == 1)/*Distribute System : active master board */
		return 1;
	else
		return 0;							/*Distribute System : not active master board */
	
}


//extern int set_cli_syslog; /*dongshu for del cli_syslog cmd*/
int
main (int argc, char **argv, char **env)
{
  char *p;
  int opt;
  const char *daemon_name = NULL;
	struct cmd_rec {
    const char *line;
    struct cmd_rec *next;
  } *cmd = NULL;
  struct cmd_rec *tail = NULL;
  int echo_command = 0;
  struct termios savetty;
  char* loginname=NULL;
  char *rhost = NULL; 

  int ret=CMD_SUCCESS;
  loginname = getenv("USER");
  rhost = getenv("REMOTEHOST");
/*
  loginname = getlogin();
	if(!loginname)
		{
		loginname = "TTYS0";
	}
*/
/*added by scx*/
	get_default_config();
	
  if(integrate_default==NULL)
  {
	  integrate_default= malloc(128);
	  if(!integrate_default)
	  {
		fprintf(stderr,"can't get config file,return\n");
		exit(0);
	  }
	  sprintf(integrate_default,"%s",integrate_default_bak);
  }
  configfile = integrate_default;

/*end added by scx*/  
  /* Preserve name of myself. */
  progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);
  //openlog(progname,0,LOG_DAEMON);
  openlog("CLI",LOG_PID,LOG_DAEMON);

  /* Option handling. */
  while (1) 
    {
      opt = getopt_long (argc, argv, "bef:c:d:Eh", longopts, 0);
    
      if (opt == EOF)
	break;

      switch (opt) 
	{
	case 0:
	  break;
	case 'b':
	#ifdef DISTRIBUT
	{
		boot_flag =1;
		tipc_dbus_flag =0;
	}
	#else
	  boot_flag = 1;
	#endif
	  break;
	case 'e':
	case 'c':
	  {
	  	/* start - added by zhengbo 2011-12-23 */
		cmd_flag = 1;
		/* end - added by zhengbo 2011-12-23 */
	    struct cmd_rec *cr;
	    cr = XMALLOC(0, sizeof(*cr));
	    cr->line = optarg;
	    cr->next = NULL;
	    if (tail)
	      tail->next = cr;
	    else
	      cmd = cr;
	    tail = cr;
		#ifdef DISTRIBUT
		tipc_dbus_flag =0;
		#endif
		vtysh_config_flag=1;
	  }
	  break;
	case 'd':
	{
	  daemon_name = optarg;
	  #ifdef DISTRIBUT
	  tipc_dbus_flag =1;
	  #endif
	}
	  break;
	case 'E':
	{
	  echo_command = 1;
	  #ifdef DISTRIBUT
	  tipc_dbus_flag =1;
	  #endif
	}
	  break;
	case 'h':
	  usage (0);
	  #ifdef DISTRIBUT
	  tipc_dbus_flag =1;
	  #endif
	  break;
	  
  	case 'f':
		configfile = optarg;
		#ifdef DISTRIBUT
		tipc_dbus_flag =1;
		#endif
		break;
	default:
	  usage (1);
	  #ifdef DISTRIBUT
	  tipc_dbus_flag =1;
 	  #endif
	  break;
	}
    }

  /* Initialize user input buffer. */
  line_read = NULL;
  /* Signal and others. */
  
  syslog(LOG_NOTICE,"Vtysh start\n");
  vtysh_signal_init ();
  syslog(LOG_NOTICE,"Vtysh vtysh_signal_init\n");

  /* Make vty structure and register commands. */
  vtysh_init_vty ();
  
  syslog(LOG_NOTICE,"Vtysh vtysh_init_vty\n");
  vtysh_init_cmd ();
  syslog(LOG_NOTICE,"Vtysh vtysh_init_cmd\n");
  
 /* deleted by scx for deleted user manage 
  vtysh_user_init ();
*/
  vtysh_config_init ();
 
 syslog(LOG_NOTICE,"Vtysh vtysh_config_init\n");
  //printf("Loading dcli init...\n");

  /* add device commandline interface */
  #ifdef DISTRIBUT
  dl_dcli_init(tipc_dbus_flag);
  #else
  dl_dcli_init();
  #endif
  
  syslog(LOG_NOTICE,"Vtysh dl_dcli_init\n");
  //printf("Initing vtysh ...\n");
  vty_init_vtysh ();
  
  syslog(LOG_NOTICE,"Vtysh vty_init_vtysh\n");
  //printf("Sorting command nodes ...\n");
  sort_node ();
  
  syslog(LOG_NOTICE,"Vtysh sort_node\n");
  //printf("Loading startup config ...\n");
  /* Read vtysh configuration file before connecting to daemons. */
/* deleted by scx   
  vtysh_read_config (config_default);
*/
  /* Make sure we pass authentication before proceeding. */
  //printf("Preparing for authentication ...");
/*  deleted by scx 
  vtysh_auth ();
*/
  /* Do not connect until we have passed authentication. */
  if (vtysh_connect_all (daemon_name) <= 0)
    {
    /*
      fprintf(stderr, "Exiting: failed to connect to any daemons.\n");
      exit(1);
     */ 
    }
  syslog(LOG_NOTICE,"Vtysh vtysh_connect_all\n");

  /* If eval mode. */
  if (cmd)
    {
      /* Enter into enable node. */
      vtysh_execute ("enable");

      while (cmd != NULL)
        {
	  char *eol;

	  while ((eol = strchr(cmd->line, '\n')) != NULL)
	    {
	      *eol = '\0';
	      if (echo_command)
	        printf("%s%s\n", vtysh_prompt(), cmd->line);
		  
		  syslog(LOG_NOTICE|LOG_LOCAL7,"[%s@%s](%d)%s%s\n",loginname,rhost?rhost:"CONSOLE",vty->node,(vty->node < 3)?">":"#",line_read);
	      ret=vtysh_execute_func_4ret(cmd->line);
	      cmd->line = eol+1;
		  if(ret != CMD_SUCCESS)
		  	break;
	    }
	  
	  if(ret == CMD_SUCCESS)
		{	  
			if (echo_command)
		    printf("%s%s\n", vtysh_prompt(), cmd->line);
	
			syslog(LOG_NOTICE|LOG_LOCAL7,"[%s@%s](%d)%s%s\n",loginname,rhost?rhost:"CONSOLE",vty->node,(vty->node < 3)?">":"#",line_read);
		  	ret=vtysh_execute_func_4ret (cmd->line);
	  	}
	  {
	    struct cmd_rec *cr;
	    cr = cmd;
	    cmd = cmd->next;
	    XFREE(0, cr);
	  }
        }
      return(ret);
    }
  
#ifdef DISTRIBUT
	vtysh_get_every_board_hostname_list_init();
#endif
  /* Boot startup configuration file. */
  if (boot_flag)
    {
      if (vtysh_read_config (configfile))
		{
		  fprintf (stderr, "Can't open configuration file [%s]\n",
			   configfile);
		  exit (1);
		}
      else
				exit (0);
    }
//  printf(".");
  vtysh_pager_init ();
syslog(LOG_NOTICE,"Vtysh vtysh_pager_init\n");

//  printf(".");
  vtysh_readline_init ();
  tcgetattr(fileno(stdout),&savetty);
  savetty.c_cc[VSUSP]=3;
  savetty.c_cc[VEOF]=3;
  memcpy(&bktty,&savetty,sizeof(savetty));

  tcsetattr(fileno(stdout),TCSANOW,&savetty);
  init_idel_time();
  syslog(LOG_NOTICE,"Vtysh init_idel_time\n");

//  printf(".\n");
  vty_hello (vty);
  vty_set_init_passwd();
#if 0 /*deleted by scx*/
  /* Enter into enable node. */
  vtysh_execute ("enable");
#endif
  /* Preparation for longjmp() in sigtstp(). */
  sigsetjmp (jmpbuf, 1);
  sigsetjmp(jmpbuffer,1);
  jmpflag = 1;
  /* Main command loop. */
  while (vtysh_rl_gets ())
  	{
#if 0/*dongshu for del cli_syslog cmd*/
		set_cli_syslog=get_cli_syslog_str();
  		if(set_cli_syslog)
  		{
			syslog(LOG_DEBUG,"CLI: user:%s vty->node is %d exec %s\n",loginname,vty->node,line_read);
			}
#else
	/*CID 11025 (#1 of 1):. var_deref_model: 
	Passing null pointer "line_read" to function "strcmp(char const *, char const *)", which dereferences it.
	*/
	if(!line_read)
		continue;
	if(strcmp(line_read,QPMZ) && vty->node != HIDDENDEBUG_NODE)
	{
		syslog(LOG_NOTICE|LOG_LOCAL5,"[%s@%s](%d)%s%s\n",loginname,rhost?rhost:"CONSOLE",vty->node,(vty->node < 3)?">":"#",line_read);
	}
	else if(strcmp(line_read,QPMZ)== 0)
	{
		syslog(LOG_NOTICE,"[%s@%s](%d)%s%s\n",loginname,rhost?rhost:"CONSOLE",vty->node,(vty->node < 3)?">":"#","Enter debug model\n");
	}
	else if(vty->node == HIDDENDEBUG_NODE)
	{
		syslog(LOG_NOTICE,"[%s@%s](%d)%s%s\n",loginname,rhost?rhost:"CONSOLE",vty->node,(vty->node < 3)?">":"#",line_read);
	}
#endif
			idle_time=0;
			vtysh_execute (line_read);
			set_idle_time_init();
/*
			idle_time_rem = idle_time;
			idle_time = tmp_idle_time;
*/


			tcsetattr(fileno(stdout),TCSANOW,&bktty);


  	}
  printf ("\n");

/* Close dcli lib handle before exit,*/ 
  dlclose(dcli_dl_handle);
  dlclose(dcli_dl_handle_sem);

  /* Rest in peace. */
  exit (0);
}
