/* Configuration generator.
   Copyright (C) 2000 Kunihiro Ishiguro

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#include <zebra.h>
#include <sys/mman.h>
#include <unistd.h>

#include "command.h"
#include "linklist.h"
#include "memory.h"

#include "vtysh/vtysh.h"

vector configvec;

extern int vtysh_writeconfig_integrated;
extern void config_add_line_uniq (struct list *config, const char *line);
extern int (*dcli_get_rpa_broadcast_mask)(struct vty *vty, int slot_id, unsigned int *mask, int *is_default_mask);

struct config
{
  /* Configuration node name. */
  char *name;

  /* Configuration string line. */
  struct list *line;

  /* Configuration can be nest. */
  struct config *config;

  /* Index of this config. */
  u_int32_t index;
};

struct list *config_top;

struct apache {
        int Listen_http;
        int Listen_https;
        char ServerName[32];
} ;
/* add for lacp zhangchunlong@autelan.com 2012-12-12 */
int get_product_info(char *filename)
{
	int fd;
	char buff[16] = {0};
	unsigned int data;

	if(NULL == filename) {
		return -1;
	}

    fd = open(filename, O_RDONLY, 0);
    if(fd >= 0) {
        if(read(fd, buff, 16) < 0) {
            syslog(LOG_WARNING, "get_product_info: Read error : no value\n");
            close(fd);
            return 0;
        }    
    }
    else {        
        syslog(LOG_WARNING, "get_product_info: Open file:%s error!\n", filename);
        return 0;
    }
    
    data = strtoul(buff, NULL, 10);

    close(fd);
    
	return data;
}

__inline__ int parse_slotport_tag_no(char *str,unsigned char *slotno,unsigned char *portno,unsigned int * tag1,unsigned int *tag2)
{
	char *endptr = NULL;
	char *endptr2 = NULL;
	char *endptr3 = NULL;
	char *endptr4 = NULL;
	char * tmpstr = str;
	
	if(NULL == str)
	{
		return CMD_FAILURE;
	}
	if((NULL == slotno)||(NULL == portno)||(NULL == tag1))
	{
		return CMD_FAILURE;
	}
	*slotno = 0;
	*portno = 0;
	*tag1 = 0;
	if(NULL == tag2)
	{
		*tag2 = 0;
	}
	if(!strncmp(str,"eth",3) || !strncmp(str,"mng",3))
	{
		tmpstr = str+3;
	}
	if (NULL == tmpstr) {return -1;}
	if(((tmpstr[0] == '0')&&(tmpstr[1] != '-'))|| \
		(tmpstr[0] > '9')||(tmpstr[0] < '0'))
	{
         return CMD_FAILURE;
	}
	*portno = (char)strtoul(tmpstr,&endptr,10);
	if (endptr) 
	{
		if (('-' == endptr[0])&& \
			(('0' < endptr[1])&&('9' >= endptr[1])))
		{
            *slotno = *portno;
			*portno = (char)strtoul((char *)&(endptr[1]),&endptr2,10);
			/*}
			for eth1, eth1.2, eth1.2.3
			else endptr2 = endptr;
			{
			*/
			if(endptr2)
			{	
				if('\0' == endptr2[0])
				{
					*tag1 = 0;
					return CMD_SUCCESS;
				}
				else if(('.' == endptr2[0])&&\
					(('0' < endptr2[1])&&('9' >= endptr2[1])))
				{
					*tag1 = strtoul((char *)&(endptr2[1]),&endptr3,10);
					if((NULL == endptr3)||('\0' == endptr3[0]))
					{
						if(tag2) *tag2 = 0;
						return CMD_SUCCESS;
					}
					if(!tag2) return CMD_FAILURE;
					if((endptr3 != NULL)&&(endptr3[0] == '.'))
					{
						if(('0' >= endptr3[1])||('9' < endptr3[1]))
						{
							return CMD_FAILURE;
						}
						if(tag2)
						{
							*tag2 = strtoul((char *)&(endptr3[1]),&endptr4,10);
							if((endptr4 != NULL)&&(endptr4[0] != '\0'))
							{
								return CMD_FAILURE;
							}
						}
						return CMD_SUCCESS;
					}
					return CMD_FAILURE;
					
				}
				else
				{
					*tag1 = 0;
					return CMD_FAILURE;
				}
			}
			
			*tag1 = 0;
			if(tag2) *tag2 = 0;
			return CMD_SUCCESS;
		}
	}
	*slotno = 0;
	*tag1 = 0;
	if(tag2) *tag2 = 0;
	/* return 0;*/
	return CMD_FAILURE;	
}
static int intf_lacp_process_for_ospf(char *name)
{
	unsigned char slot = 0,port = 0;
	unsigned int vid1 = 0,vid2 = 0;
	int function_type = -1;
	char section[128] = {0};
	char file_path[64] = {0};
	char *eth_name = NULL;
	char *token = NULL;
	int ret = CMD_FAILURE;
	int fd = CMD_FAILURE;
	int i=0;

	if(name)
	{
		/*because the first char is '\n'*/
		sprintf(section,"%s",name+1);
	}
	else
	{
		return CMD_FAILURE;
	}
	if(memcmp(section,"!interface eth",14) != 0)
	{
		/*not eth port*/
		return CMD_FAILURE;
	}
	token = strtok(section," ");
	if(token)
	{
		/*get eth name*/
		eth_name=strtok(NULL," ");
	}
	if((eth_name) && (memcmp(eth_name,"eth",3) == 0))
	{
		ret = parse_slotport_tag_no(eth_name,&slot,&port,&vid1,&vid2);
		if(ret != CMD_SUCCESS)
		{
			syslog(LOG_WARNING,"parse %s Failure %s",eth_name,VTY_NEWLINE);
			return CMD_FAILURE;
		}
		if((vid1 == 0) && (vid2 == 0) && (slot != 0) && (port != 0))
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", slot);
			function_type = get_product_info(file_path);
			if(function_type == 0x4)/*switch board*/
			{
				/*if is switch board,do not save interface cmd*/
				return CMD_SUCCESS;
			}
			else
			{
				/*syslog(LOG_WARNING,"not a switch board !%s",VTY_NEWLINE);*/
				return CMD_FAILURE;
			}
		}
		else
		{
			syslog(LOG_WARNING,"param do not match ! %s",VTY_NEWLINE);
			return CMD_FAILURE;
		}
	}
	else
	{
		/*not a eth port */
		return CMD_FAILURE;
	}
}
/* add end */

int
line_cmp (char *c1, char *c2)
{
  return strcmp (c1, c2);
}

void
line_del (char *line)
{
  XFREE (MTYPE_VTYSH_CONFIG_LINE, line);
}

struct config *
config_new ()
{
  struct config *config;
  config = XCALLOC (MTYPE_VTYSH_CONFIG, sizeof (struct config));
  return config;
}

int
config_cmp (struct config *c1, struct config *c2)
{
  return strcmp (c1->name, c2->name);
}

void
config_del (struct config* config)
{
  list_delete (config->line);
  if (config->name)
    XFREE (MTYPE_VTYSH_CONFIG_LINE, config->name);
  XFREE (MTYPE_VTYSH_CONFIG, config);
}

struct config *
config_get (int index, const char *line,char * name)
{
  struct config *config;
  struct config *config_loop;
  struct list *master;
  struct listnode *node, *nnode;

  config = config_loop = NULL;

  master = vector_lookup_ensure (configvec, index);

  if (! master)
	{
	  master = list_new ();
	  master->del = (void (*) (void *))config_del;
	  master->cmp = (int (*)(void *, void *)) config_cmp;
	  vector_set_index (configvec, index, master);
	}
  
  for (ALL_LIST_ELEMENTS (master, node, nnode, config_loop))
    {
      if (strncmp (config_loop->name+2, line,strlen(line)) == 0  )
      {
      	config = config_loop;
      	break;
      }			
    }

  if (! config)
    {
      config = config_new ();
      config->line = list_new ();
      config->line->del = (void (*) (void *))line_del;
      config->line->cmp = (int (*)(void *, void *)) line_cmp;
#if 1	
			if(name)
      {

				char _tmpstr[128];
				memset(_tmpstr,0,128);
				if(strlen(name)>=64-strlen(BUILDING_MOUDLE))
					name[64-strlen(BUILDING_MOUDLE)-1] = '\0';
				else
					sprintf(_tmpstr,BUILDING_MOUDLE,name);

      	config->name = XSTRDUP (MTYPE_VTYSH_CONFIG_LINE, _tmpstr);
				config_add_line_uniq (config->line, line);
			}
			else
			{
				config->name = XSTRDUP (MTYPE_VTYSH_CONFIG_LINE, line);
			}
#else
			config->name = XSTRDUP (MTYPE_VTYSH_CONFIG_LINE, line);
#endif
			config->index = index;
      listnode_add (master, config);
    }
  return config;
}

void
config_add_line (struct list *config, const char *line)
{
  listnode_add (config, XSTRDUP (MTYPE_VTYSH_CONFIG_LINE, line));
}

void
config_add_line_uniq (struct list *config, const char *line)
{
  struct listnode *node, *nnode;
  char *pnt;

  for (ALL_LIST_ELEMENTS (config, node, nnode, pnt))
    {
      if (strcmp (pnt, line) == 0)
	{
		  listnode_delete(config,pnt);
		  config_add_line(config,line);
		  return;
	}

  }
  config_add_line(config, XSTRDUP (MTYPE_VTYSH_CONFIG_LINE, line));/*CID 13525 (#1 of 1): leaked_storage: Ignoring storage allocated by "z_strdup(181, line)" leaks it*/
}

void
vtysh_config_parse_line (const char *line)
{
  char c;
  static struct config *config = NULL;

  if (! line)
    return;

  c = line[0];

  if (c == '\0')
    return;
/*
  fprintf (stdout,"[%s]\n", line); 
*/
  switch (c)
    {
/*    
    case '!':
*/
    case '#':
    	break;
    case ' ':
      /* Store line to current configuration. */
      if (config)
			{
			  if (strncmp (line, " address-family vpnv4",
			      strlen (" address-family vpnv4")) == 0)
			    config = config_get (BGP_VPNV4_NODE, line,"router bgp");
			  else if (strncmp (line, " address-family ipv4 multicast",
				   strlen (" address-family ipv4 multicast")) == 0)
			    config = config_get (BGP_IPV4M_NODE, line,NULL);
			  else if (strncmp (line, " address-family ipv6",
				   strlen (" address-family ipv6")) == 0)
			    config = config_get (BGP_IPV6_NODE, line,NULL);
			  else if (config->index == RMAP_NODE ||
			           config->index == INTERFACE_NODE ||
				   config->index == VTY_NODE)
			    config_add_line_uniq (config->line, line);
			  else
			    config_add_line (config->line, line);
			}
      else  
				config_add_line (config_top, line);
      break;
    default:
      if (strncmp (line, "interface", strlen ("interface")) == 0)
			{


				config = config_get (INTERFACE_NODE, line,line);
			}
      else if (strncmp (line, "router-id", strlen ("router-id")) == 0)
					config = config_get (ZEBRA_NODE, line,NULL);
      else if (strncmp (line, "router rip", strlen ("router rip")) == 0)
					config = config_get (RIP_NODE, line, "router rip");
      else if (strncmp (line, "router ripng", strlen ("router ripng")) == 0)
					config = config_get (RIPNG_NODE, line, "router ripng");
      else if (strncmp (line, "router ospf", strlen ("router ospf")) == 0)
					config = config_get (OSPF_NODE, line, "router ospf");
      else if (strncmp (line, "router ospf6", strlen ("router ospf6")) == 0)
					config = config_get (OSPF6_NODE, line, "router ospf6");
      else if (strncmp (line, "router bgp", strlen ("router bgp")) == 0)
					config = config_get (BGP_NODE, line, "router bgp");
      else if (strncmp (line, "router isis", strlen ("router isis")) == 0)
  				config = config_get (ISIS_NODE, line, "router isis");
      else if (strncmp (line, "router bgp", strlen ("router bgp")) == 0)
					config = config_get (BGP_NODE, line, "router bgp");
      else if (strncmp (line, "route-map", strlen ("route-map")) == 0)
					config = config_get (RMAP_NODE, line, "route-map");
      else if (strncmp (line, "access-list", strlen ("access-list")) == 0)
					config = config_get (ACCESS_NODE, line, "access-list");
      else if (strncmp (line, "ipv6 access-list",strlen ("ipv6 access-list")) == 0)
					config = config_get (ACCESS_IPV6_NODE, line, "ipv6 access-list");
      else if (strncmp (line, "ip prefix-list",
	       strlen ("ip prefix-list")) == 0)
					config = config_get (PREFIX_NODE, line, "ip prefix-list");
      else if (strncmp (line, "ipv6 prefix-list",
	       strlen ("ipv6 prefix-list")) == 0)
				config = config_get (PREFIX_IPV6_NODE, line, "ipv6 prefix-list");
      else if (strncmp (line, "ip as-path access-list",
	       strlen ("ip as-path access-list")) == 0)
					config = config_get (AS_LIST_NODE, line, "ip as-path access-list");
      else if (strncmp (line, "ip community-list",
	       strlen ("ip community-list")) == 0)
				config = config_get (COMMUNITY_LIST_NODE, line, "ip community-list");

			else if (strncmp (line, "ip route", strlen ("ip route")) == 0)
				config = config_get (IP_NODE, line, NULL);
      else if (strncmp (line, "ipv6 route", strlen ("ipv6 route")) == 0)
   			config = config_get (IP_NODE, line, NULL);

      else if (strncmp (line, "key", strlen ("key")) == 0)
				config = config_get (KEYCHAIN_NODE, line, "key");
      else if (strncmp (line, "line", strlen ("line")) == 0)
				config = config_get (VTY_NODE, line, "line");
      else if ( (strncmp (line, "ipv6 forwarding",
		 strlen ("ipv6 forwarding")) == 0)
	       || (strncmp (line, "ip forwarding",
		   strlen ("ip forwarding")) == 0) )
				config = config_get (FORWARDING_NODE, line, "ip forwarding");
/*
			else if (strncmp (line, "service", strlen ("service")) == 0)
				config = config_get (SERVICE_NODE, line, "service");
*/
			else if (strncmp (line, "debug", strlen ("debug")) == 0)
				config = config_get (DEBUG_NODE, line, NULL);
#if 0
			else if (strncmp (line, "config ebr", strlen ("config ebr")) == 0)
				config = config_get (EBR_NODE, line, line);
			else if (strncmp (line, "config security ", strlen ("config security ")) == 0)
				config = config_get (SECURITY_NODE, line, line );
			else if (strncmp (line, "create wlan", strlen ("create wlan")) == 0)
				config = config_get (WLAN_NODE, line, line);
			else if (strncmp (line, "config wtp", strlen ("config wtp")) == 0)
				config = config_get (WTP_NODE, line, line);
			else if (strncmp (line, "config radio", strlen ("config radio")) == 0)
				config = config_get (RADIO_NODE, line, line);
			else if (strncmp (line, "config wireless qos", strlen ("config wireless qos")) == 0)
				config = config_get (WQOS_NODE, line, line);

			else if (strncmp (line, "debug", strlen ("debug")) == 0)
				config = config_get (DEBUG_NODE, line, NULL);

#endif

			else if (strncmp (line, "password", strlen ("password")) == 0
	       || strncmp (line, "enable password",
			   strlen ("enable password")) == 0)
					config = config_get (AAA_NODE, line, NULL);
      else
			{
			  if (strncmp (line, "log", strlen ("log")) == 0
						|| strncmp (line, "no log", strlen ("no log")) == 0
			      || strncmp (line, "hostname", strlen ("hostname")) == 0
			      
			     )
			    config_add_line_uniq (config_top, line);
			  else
			    config_add_line (config_top, line);
			  config = NULL;
			}
      break;
    }
}

void
vtysh_config_parse (char *line)
{
  char *begin;
  char *pnt;
  
  begin = pnt = line;

  while (*pnt != '\0')
    {
      if (*pnt == '\n')
			{
			  *pnt++ = '\0';
			  vtysh_config_parse_line (begin);
			  begin = pnt;
			}
      else
			{
			  pnt++;
			}
    }
}

/* Macro to check delimiter is needed between each configuration line
 * or not. */
#define NO_DELIMITER(I)  \
  ((I) == ACCESS_NODE || (I) == PREFIX_NODE || (I) == IP_NODE \
   || (I) == AS_LIST_NODE || (I) == COMMUNITY_LIST_NODE || \
   (I) == ACCESS_IPV6_NODE || (I) == PREFIX_IPV6_NODE \
   || (I) == SERVICE_NODE || (I) == FORWARDING_NODE || (I) == DEBUG_NODE \
   || (I) == AAA_NODE)


/* Display configuration to file pointer. */
void
vtysh_config_dump (FILE *fp)
{
  struct listnode *node, *nnode;
  struct listnode *mnode, *mnnode;
  struct config *config;
  struct list *master;
  char *line;
  unsigned int i;

  for (ALL_LIST_ELEMENTS (config_top, node, nnode, line))
    {
      fprintf (fp, "%s\n", line);
      fflush (fp);
    }
/*	
  fprintf (fp, "!aaaaaaaaaa\n");
*/
  for (i = 0; i < vector_active (configvec); i++)
    if ((master = vector_slot (configvec, i)) != NULL)
      {
			for (ALL_LIST_ELEMENTS (master, node, nnode, config))
			{
				/*if section is interface eth*-* do not continue, zhangchunlong@autelan.com 2012-12-12*/
				if(0 != intf_lacp_process_for_ospf(config->name))
				{
					fprintf (fp, "%s\n", config->name);
			    	fflush (fp);
				}
				else
				{
					continue;
				}
			    for (ALL_LIST_ELEMENTS (config->line, mnode, mnnode, line))
			    {
					fprintf  (fp, "%s\n", line);
					fflush (fp);
			    }
			}
/*					
			    if (! NO_DELIMITER (i))
			      {
							fprintf (fp, "!\n");
							fflush (fp);
			      }

			  }
			if (NO_DELIMITER (i))
			  {
			    fprintf (fp, "!\n");
			    fflush (fp);

				}
 */		    
 }

  for (i = 0; i < vector_active (configvec); i++)
    if ((master = vector_slot (configvec, i)) != NULL)
	  {
				list_delete (master);
				vector_slot (configvec, i) = NULL;
	  }
  list_delete_all_node (config_top);
}

/* Read up configuration file from file_name. */
static void
vtysh_read_file (FILE *confp)
{
  int ret;
  struct vty *vty;

  vty = vty_new ();
  vty->fd = 0;			/* stdout */
	vty->fp = stdout;
  vty->type = VTY_TERM;
  vty->node = CONFIG_NODE;
  vtysh_execute_no_pager ("enable");
  vtysh_execute_no_pager ("configure terminal");

  /* Execute configuration file. */
  ret = vtysh_config_from_file (vty, confp);

  vtysh_execute_no_pager ("end");
  vtysh_execute_no_pager ("disable");

  vty_close (vty);

  if (ret != CMD_SUCCESS) 
    {
      switch (ret)
	{
	case CMD_ERR_AMBIGUOUS:
	  fprintf (stderr, "Ambiguous command.\n");
	  break;
	case CMD_ERR_NO_MATCH:
	  fprintf (stderr, "There is no such command.\n");
	  break;
	}
      fprintf (stderr, "Error occured during reading below line.\n%s\n", 
	       vty->buf);
		EXIT(1);
    }
}

/* Read up configuration file from config_default_dir. */
int
vtysh_read_config (char *config_default_dir)
{
  FILE *confp = NULL;

  confp = fopen (config_default_dir, "r");
  if (confp == NULL)
    return (1);

  vtysh_read_file (confp);
  fclose (confp);
  host_config_set (config_default_dir);

  return (0);
}

/* We don't write vtysh specific into file from vtysh. vtysh.conf should
 * be edited by hand. So, we handle only "write terminal" case here and
 * integrate vtysh specific conf with conf from daemons.
 */
/**add for pfm show running**/
int
vtysh_pfm_config_write(struct vty* vty)
{
	int i,m;
	char _tmpstr[64];
	char *temp;
	char *temp_char;
	/*FILE* fp;*/
	/*gujd: 2012-02-09: pm 4:57 . In order to decrease the warning when make img . 
	Delete "FILE *fp", use "int fp", because the return of api "open" is integer .*/	
	int fp;
	void* data;
	char* data_temp;
	char* p_temp;
	struct stat sb;
	
	p_temp=data_temp=malloc(1024);
	memset(data_temp,0,1024);
	memset(_tmpstr,0,64);
	sprintf(_tmpstr,BUILDING_MOUDLE,"PFM CONFIG");
	vtysh_add_show_string(_tmpstr);
	fp=open("/var/run/pfm_setup",O_RDWR);
	#if 0
	if(NULL==fp)
		{
		
		free(data_temp);
		return 0;
		}
	#else
	if(fp < 0)
		{
		
		free(data_temp);
		return 0;
		}

	#endif
	if(-1==fstat(fp,&sb))
	{	
		close(fp);
		return 0;
	}
	data=mmap(NULL,sb.st_size,PROT_READ,MAP_PRIVATE,fp,0);
	if(MAP_FAILED==data)
		{
		
		free(data_temp);
		close(fp);
		return 0;
		}
	for(i=m=0;i<sb.st_size;i++)
		{
		if((*((char*)data+i))=='\n')
			{
			memcpy(p_temp,data+m,(i-m)+1);
			p_temp+=(i-m)+1;
			m=i+1;
			}
		}
	
	temp_char=strrchr(data_temp,'\n');
	*temp_char='\0';
#if 0
	for(i=0;i<sb.st_size;i++)
		{
		if((*((char*)data+i))=='\n')
			{
			vty_out(vty,"insert a space %d\n",i+1);
			insert(data,temp,i+1);
			}
		}
	temp_char=strrchr(data,'\n');
	*temp_char='\0';
#endif
	vtysh_add_show_string(data_temp);
	close(fp);
	free(data_temp);
	munmap(data,sb.st_size);
	return 0;
}
#define PFM_DNS_FILE			"/var/run/pfm_dns"
#define PFM_RADIUS_FILE			"/var/run/pfm_radius"

int vtysh_rpa_config_write(struct vty* vty)
{
	FILE *fd;
	char cmd_str[64] = {0};
	char temp_str[64] = {0};
	int slot_id;
	int slot_count = 0;
	unsigned int board_on_mask = 0x0;
	unsigned int mask = 0x0;
	int is_default_mask;
	int ret;

	fd = fopen("/dbm/product/slotcount", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [2] error\n");
		return -1;
	}
	if(1!=fscanf(fd, "%d", &slot_count))
	{
		fclose(fd);
		fprintf(stderr,"Get product slot_count error\n");
		return -1;
	}
	fclose(fd);

	fd = fopen("/dbm/product/board_on_mask", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get product board_on_mask error\n");
		return -1;
	}
	if(1!=fscanf(fd, "%d", &board_on_mask))
	{
		fclose(fd);
		fprintf(stderr,"Read product board_on_mask error\n");
		return -1;
	}
	fclose(fd);

	sprintf(temp_str, BUILDING_MOUDLE, "RPA CONFIG");
	vtysh_add_show_string(temp_str);

	for (slot_id = 0; slot_id < slot_count; slot_id++) {
		if (board_on_mask & (1 << slot_id)) {
			ret = dcli_get_rpa_broadcast_mask(vty, (slot_id + 1), &mask, &is_default_mask);
			if (ret == 0) {
				if (!is_default_mask) {
					memset(cmd_str, 0, 64);
					sprintf(cmd_str, "rpa set broadcast-mask %#x slot %d\n", mask, (slot_id + 1));
					vtysh_add_show_string(cmd_str);
				}
			}
		}
	}

	return 0;
}

int
vtysh_ssh_telnet_config_write(struct vty* vty)
{
	unsigned int port=0;
	int ret;
	char _tmpstr[64];
//telnet config write
	/*FILE *fp;*/
	/*gujd: 2012-02-09: pm 4:44 . In order to decrease the warning when make img . 
	Delete "FILE *fp", use "int fp", because the return of api "open" is integer .*/	
	int fp; 
	struct stat sb;
	char *temp_data;
	char *data;
	memset(_tmpstr,0,64);
	sprintf(_tmpstr,BUILDING_MOUDLE,"TELNET INFO");
	vtysh_add_show_string(_tmpstr);
//check out services
	fp=open(SERVICES_PATH,O_RDWR);	
#if 0
	if(NULL==fp)
	{
		vty_out(vty,SYS_ERROR);
		return 1;
	}
#else
	if(fp < 0)
	{
		vty_out(vty,SYS_ERROR);
		return 1;
	}

#endif
	if(-1==fstat(fp,&sb))
	{	
		close(fp);
		return 1;
	}
	data=mmap(NULL,sb.st_size,PROT_READ,MAP_SHARED,fp,0);
	if(MAP_FAILED==data)
	{
		vty_out(vty,SYS_ERROR);
		close(fp);
		return 1;
	}
	for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"telnet"))
	{
		if(NULL==temp_data)
		{
			vty_out(vty,"Can't found telnet port%s",VTY_NEWLINE);
			break;
		}
		if((*(temp_data-1))==10 && (*(temp_data+6))==9) //is real telnet
		{
			port=atoi(temp_data+6);
			break;
		}
	}
	munmap(data,sb.st_size);
	close(fp);

//check out inetd.conf
	fp=open(INETD_PATH,O_RDWR);
#if 0
	if(NULL==fp)
	{
		vty_out(vty,SYS_ERROR);
		return 1;
	}
#else
	if(fp < 0)
	{
		vty_out(vty,SYS_ERROR);
		return 1;
	}
#endif
	if(-1==fstat(fp,&sb))
	{	
		close(fp);
		return 1;
	}
	data=mmap(NULL,sb.st_size,PROT_READ,MAP_SHARED,fp,0);
	if(MAP_FAILED==data)
	{
		vty_out(vty,SYS_ERROR);
		close(fp);
		return 1;
	}
	for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"elnet"))
	{
		if(!temp_data)
		{
			vty_out(vty,"Can not found #elnet\n");
			break;
		}
		
		if((*(temp_data-2))==10 && (*(temp_data+5))==9 && (*(temp_data+5))==9) //is real telnet
		{
			if((*(temp_data-1))=='#')
			{
				memset(_tmpstr,0,64);
				sprintf(_tmpstr,"service telnet disable");
				vtysh_add_show_string(_tmpstr);
			}
			if((*(temp_data-1))=='t')
			{
				if(23!=port && port)
				{
					memset(_tmpstr,0,64);
					sprintf(_tmpstr,"service telnet enable %d", port);
					vtysh_add_show_string(_tmpstr);
				}
			}
			break;
		}
		
	}
	munmap(data,sb.st_size);
	close(fp);


//ssh config write	
	fp=open(SSHD_CONFIG_PATCH,O_RDWR);
#if 0
	if(NULL==fp)
	{
		vty_out(vty,SYS_ERROR);
		return 0;
	}
#else
	if(fp < 0)
	{
		vty_out(vty,SYS_ERROR);
		return 0;
	}

#endif	
	if(-1==fstat(fp,&sb))
	{	
		close(fp);
		return 0;
	}
	data=mmap(NULL,sb.st_size,PROT_READ,MAP_SHARED,fp,0);
	if(MAP_FAILED==data)
	{
		vty_out(vty,SYS_ERROR);
		close(fp);
		return 0;
	}
	for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"Port"))
	{
		if(NULL==temp_data)
		{
			vty_out(vty,"Can't found telnet port%s",VTY_NEWLINE);
			break;
		}
		if((*(temp_data-1))==10 && (*(temp_data+4))==' ') //is real telnet
		{
			port=atoi(temp_data+4);
			break;
		}
	}
	munmap(data,sb.st_size);
	close(fp);

	memset(_tmpstr,0,64);
	sprintf(_tmpstr,BUILDING_MOUDLE,"SSH INFO");
	vtysh_add_show_string(_tmpstr);

	memset(_tmpstr,0,64);
	sprintf(_tmpstr,"ps -ef |grep sshd | grep -v \"grep sshd\" > /dev/null");
	ret = system(_tmpstr);
	if(WEXITSTATUS(ret)== 0)
	{
		
		memset(_tmpstr,0,64);
		if(22!=port)
		sprintf(_tmpstr,"service ssh enable %d",port);
		else
		sprintf(_tmpstr,"service ssh enable");
		vtysh_add_show_string(_tmpstr);
	}
	else{
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,"service ssh disable");
		vtysh_add_show_string(_tmpstr);
		}

	{
		FILE *fp;
		char buf[512]={0};
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"DNS INFO");
		vtysh_add_show_string(_tmpstr);

		fp = fopen(PFM_DNS_FILE,"r");
		if(fp!=NULL)
		{
			while(fgets(buf,512,fp))
			{
				memset(_tmpstr,0,64);
				strcpy(_tmpstr,buf);
				vtysh_add_show_string(buf);
			}	
			fclose(fp);
		}
	}
	
	{
		FILE *fp;
		char buf[512]={0};
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"RADUIS INFO");
		vtysh_add_show_string(_tmpstr);

		fp = fopen(PFM_RADIUS_FILE,"r");
		if(fp!=NULL)
		{
			while(fgets(buf,512,fp))
			{
				memset(_tmpstr,0,64);
				strcpy(_tmpstr,buf);
				vtysh_add_show_string(buf);
			}	
			fclose(fp);
		}
	}
#if 0
munmap(data,sb.st_size);
close(fp);
#endif
return 0;
}

int  test_apache_enable() 
{
	char tmpz[128];
	int   sflag=-1;
	FILE *pipe=NULL;
		
	pipe=popen("ps -ef|grep \"/usr/sbin/apache2 -k start\" |grep -v grep|wc -l","r");
	if (pipe != NULL)
	{
        	memset(tmpz,0,sizeof(tmpz));
		fgets( tmpz, sizeof(tmpz), pipe );	
		sflag = strtoul( tmpz, 0, 10);
        	pclose( pipe );
	}

	return (sflag>0)?1:0;
}

int  vtysh_http_config_write(struct vty* vty)
{
     struct apache apache_get_conf;
     int iret = -1;
     int port=0;
     char _tmpstr[64]; 
	 
	FILE *fp = NULL;
       char line[512];	
	fp = fopen("/etc/apache2/ports.conf", "r" );
       if( NULL != fp )
       {
	   	  memset( line, 0, sizeof( line ) );
                fgets( line, sizeof(line), fp );
		  sscanf( line, "Listen %d", &apache_get_conf.Listen_http );

	   	  memset( line, 0, sizeof( line ) );
                fgets( line, sizeof(line), fp );
		  sscanf( line, "Listen %d", &apache_get_conf.Listen_https );

	   	  memset( line, 0, sizeof( line ) );
                fgets( line, sizeof(line), fp );
		  sscanf( line, "ServerName %s", apache_get_conf.ServerName );	
		  fclose( fp );
		  fp = NULL;		  
	}
	else
	{
            vty_out(vty,"open apache conf file failed!\n");                        
	     return 1;
	}
	port=apache_get_conf.Listen_http;
//	printf("apache port= %d",apache_get_conf.Listen_http);
     if(1== test_apache_enable())
     {
         
	   if( port != 80 )
	   {    
//	   	 vty_out(vty,"debug!!apache is enable\n");                             
	   	 memset(_tmpstr,0,64);
		 sprintf(_tmpstr,"service http enable %d",port);
		 vtysh_config_parse_line(_tmpstr);
	   }    
	   else if(port == 80)
	   {
                vtysh_config_parse_line("service http enable ");
	   }
     }
     else
     {      
//	     vty_out(vty,"debug!!apache is NOT  enable\n");                    
	     if(port != 80)
	     {    
		    memset(_tmpstr,0,64);
		    sprintf(_tmpstr,"service http enable %d",port);
		    vtysh_config_parse_line(_tmpstr);
		    vtysh_config_parse_line("service http disable");
	     }	  	
     }
     return 0;
}

int get_dns_str(char **name)
{
	FILE *fp=NULL;
	char *ptr=NULL;
	int i=0;

	fp = fopen("/etc/resolv.conf","r");
	
	if(!fp)
		return -1;
	ptr=malloc(128);
	if(!ptr)
		{
		fclose(fp);
		return -1;
	}
	while(fgets(ptr,128,fp))
	{
		if(!strncmp(ptr,"nameserver ",11))
		{
			sprintf(*(name+i),ptr+11);
			i++;
			if (i>=3)
				break;
		}
	}
	free(ptr);
	fclose(fp);
	return i;
	
}

/* start - added by zhengbo 2011.12.13 */
extern int ct_passwd_isenabled(void);
extern int ct_passwd_get(char *passwd, int len);
/* end - added by zhengbo 2011.12.13 */

int 
vtysh_config_write (struct vty* vty)
{
  char line[81];
  extern struct host host;
	char _tmpstr[64];
	int value = -1;
	
	memset(_tmpstr,0,64);
	sprintf(_tmpstr,BUILDING_MOUDLE,"SYSTERM INFO");
	vtysh_add_show_string(_tmpstr);
	
	/* start - added by zhengbo 2011.12.13 */
	char passwd[128] = {0};
	char passwd_buf[160] = {0};
	
	if(ct_passwd_isenabled()) {
		ct_passwd_get(passwd, sizeof(passwd));
		sprintf(passwd_buf, "config passwd %s\n", passwd);
		vtysh_config_parse_line(passwd_buf);
	}
	/* end - added by zhengbo 2011.12.13 */
	
	/*gujd:2013-03-01, am 10:27. Add set hostname independent between every board.*/  
	value = read_int_vlaue_from_file(HOSTNAME_SCOPE_FLAG_FILE);
	if(value == 1)/*global hostname, show run*/
	{
  		if (host.name&&strcmp(host.name,"SYSTEM"))
	    {
	      sprintf (line, "hostname %s", host.name);
	      vtysh_config_parse_line(line);
	    }
	}
	else if(value == 0)/*independent hostname set, show run*/
	{
		if (host.host_name_list)
		{
		  struct listnode *node = NULL, *nextnode = NULL;
		  struct host_name *Hostname = NULL;
		  for(ALL_LIST_ELEMENTS (host.host_name_list, node, nextnode, Hostname))/*search bingo slot*/
		  {
			if(strcmp(Hostname->Host_name,HOSTNAME_DEFAULT))/*not default*/
			{
				sprintf (line, "hostname slot %d %s", Hostname->slotid, Hostname->Host_name);
				vtysh_config_parse_line(line);
			 }
		  
		  }
		}
	}
	else
	{
		syslog(LOG_NOTICE,"Hostname write failed .\n");
		}
	
  if (! vtysh_writeconfig_integrated)
    vtysh_config_parse_line ("no service integrated-vtysh-config");
#if 0/*dongshu for del cli_syslog cmd*/
	if(set_cli_syslog)
	  vtysh_config_parse_line ("set cli-log on");
#endif/*dongshu end*/
  set_idle_time_init();
  if(IDLE_TIME_DEFAULT != idle_time)
  {
	sprintf(line,"idle-timeout %d",idle_time);
  	vtysh_config_parse_line (line);
  }
	idle_time =0;
	/*for dns set*/
  {
	int ret=0,i;
	char *dnsstr[3];
	for(i=0;i<3;i++)
	{	
		dnsstr[i]=malloc(128);
		if(!dnsstr[i])
		{
			ret=-1;
		}
		memset(dnsstr[i],0,128);
	}
	if(ret!=-1)
	{
		ret = get_dns_str(&dnsstr);
		if(ret>0 && ret<=3)
		{		
			for(i=0;i<ret;i++)
			{	
				sprintf(line,"ip dns %s",dnsstr[i]);
				vtysh_config_parse_line (line);
			}	
		}

	}
	
	for(i=0;i<3;i++)
	{	
		if(dnsstr[i])
			free(dnsstr[i]);
	}	
  }
#if 0
	/**gjd :add pfm show running func**/
	vtysh_pfm_config_write(vty);
#endif	
  if(vtysh_ssh_telnet_config_write(vty) )
  {
  	return 1;
  }
  else
  {
  	return 0;
  }
}


void
vtysh_config_init ()
{
  config_top = list_new ();
  config_top->del = (void (*) (void *))line_del;
  configvec = vector_init (1);
}
