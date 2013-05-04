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
* dcli_dhcp.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for dhcp module.
*
* DATE:
*		11/26/2009
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.5 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/ipc.h>

#include"dcli_dhcp6_conf.h"

FILE *leases_file;
char *path_dhcpv6_conf = "/etc/dhcpd.conf";

struct cmd_node subnet6_node = 
{
	SUBNET6_CONF_NODE,
	"%s(config-subnet6)# ",
	1
};

/********************************************************
 * check_ipv6_address
 *
 * Check the legality of the ipv6 address
 *
 *	INPUT:
 *		ipv6_address
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR	      - Legal address
 *		DHCP_IP6_RET_SUCCESS	- Illegal address
 *
 *********************************************************/
int check_ipv6_address(char *ipv6_address)
{
	char addrptr[128] = {0};
	if(NULL == ipv6_address)
	{
		return DHCP_IP6_RET_ERROR;
	}

	if(inet_pton(AF_INET6, ipv6_address, addrptr) != 1)
	{
		return DHCP_IP6_RET_ERROR;
	}

	return DHCP_IP6_RET_SUCCESS;
}


/********************************************************
 * check_doamin
 *
 * Check the legality of the domain
 *
 *	INPUT:
 *		domain_name
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR	      - Legal domain
 *		DHCP_IP6_RET_SUCCESS	- Illegal domain
 *
 *********************************************************/

int check_doamin(char *domain_name)
{
    char *ptr = NULL;

    ptr = domain_name;
    if((*ptr >= '0' && *ptr <= '9') || (*ptr >= 'a' && *ptr <= 'z')
            || (*ptr >= 'A' && *ptr <= 'Z') ){
        ptr++;

        while(*ptr != '\0'){
            if((*ptr >= '0' && *ptr <= '9') || (*ptr >= 'a' && *ptr <= 'z')
                    || (*ptr >= 'A' && *ptr <= 'Z') || (*ptr == '-')){
                ptr++;
            }
            else{
                return DHCP_IP6_RET_ERROR;
            }
        }
    }
    else{
        return DHCP_IP6_RET_ERROR;
    }
    return DHCP_IP6_RET_SUCCESS;
}


/********************************************************
 * get_subnet6_count
 *
 * get subnet number
 *
 *	INPUT:
 *		void
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		NULL	- get subnet number error
 *		other	- subnet number address
 *
 *********************************************************/
int* get_subnet6_count(void)
{
    int ret = -1;
    char *ptr = NULL;

    int f = shm_open(SUBNET6_COUNT, O_RDWR|O_CREAT, FILE_MODE);
    if (f < 0) {	 
        return NULL;
    }

    if(ret = ftruncate(f, sizeof(int))){	
        return NULL;
    }

    ptr = mmap(0, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, f, 0);
    if (NULL == ptr) {  	 
        return NULL;
    }

    return ((int *)ptr);
}


/********************************************************
 * get_subnet
 *
 * get subnet head of address
 *
 *	INPUT:
 *		void
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		NULL	- get error
 *		other	- subnet head address
 *
 *********************************************************/

struct dhcp_subnet6* get_subnet(void)
{
	int ret = -1;
	char *ptr = NULL;
	int *sub_count = NULL;
	int i;

	int f = shm_open(SUB_NET6, O_RDWR|O_CREAT, FILE_MODE);
	if (f < 0) {		
		return NULL;
	}

	if(ret = ftruncate(f, MAX_SIZE)){		
		return NULL;
	}

	ptr = mmap(0, MAX_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, f, 0);
	if (NULL == ptr) {		
		return NULL;
	}

	sub_count = get_subnet6_count();
	if(sub_count < 0){		
		return NULL;
	}

	/*if no subnet, index init -1*/
	if(0 == *sub_count){	 	
		for(i = 0; i < MAX_SUBNET; ++i){
			((struct dhcp_subnet6*)ptr)[i].index = -1;
		}
	}

	return (struct dhcp_subnet6*)ptr;
}


/********************************************************
 * get_ip6_dhcp_server_enable_flag
 *
 * get dhcp server enable flag address
 *
 *	INPUT:
 *		void
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		NULL	- get error
 *		other	- subnet head address
 *
 *********************************************************/

int * get_ip6_dhcp_server_enable_flag(void)
{
	int ret = -1;
	char *ptr = NULL;

	int f = shm_open(DHCP_SERVER_ENABLE, O_RDWR|O_CREAT, FILE_MODE);
	if (f < 0) {		
		return NULL;
	}

	if(ret = ftruncate(f, MAX_SIZE)){		
		return NULL;
	}

	ptr = mmap(0, MAX_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, f, 0);
	if (NULL == ptr) {		
		return NULL;
	}

	return ((int *)ptr);

}


/********************************************************
 * dhcp_find_subnet6_by_name
 *
 * find the subnet by name
 *
 *	INPUT:
 *		sub_ip
 *		
 *	OUTPUT:
 *		subnode
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR	      - not find
 *		DHCP_IP6_RET_SUCCESS	- find success
 *
 *********************************************************/
int dhcp_find_subnet6_by_name( char* sub_ip, struct dhcp_subnet6** subnode )
{
	int i;
	
	struct dhcp_subnet6* subnet = NULL;

	if (!sub_ip) {		
		return DHCP_IP6_RET_ERROR;
	}
	
	subnet = get_subnet();
	if(NULL == subnet){
	     	return DHCP_IP6_RET_ERROR;
	}
	
	for (i = 0; i < MAX_SUBNET; ++i) {
		if(subnet[i].index != -1){
			if (!strcmp(subnet[i].sub_ip, sub_ip)) {	//compare subnet name
				*subnode = &(subnet[i]);				
				return DHCP_IP6_RET_SUCCESS;
			}
		}		
	}

	return DHCP_IP6_RET_ERROR;
}



/********************************************************
 * dhcp_find_subnet6_by_index
 *
 * find the subnet by subnet's index
 *
 *	INPUT:
 *		sub_index
 *		
 *	OUTPUT:
 *		subnode
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR		  - not find
 *		DHCP_IP6_RET_SUCCESS	  - find success
 *
 *********************************************************/

int dhcp_find_subnet6_by_index(int sub_index, struct dhcp_subnet6** subnode )
{
	int i;
	
	struct dhcp_subnet6* subnet = NULL;

	if (sub_index < 0) {		
		return DHCP_IP6_RET_ERROR;
	}
	
	subnet = get_subnet();
	if(NULL == subnet){	    
		return DHCP_IP6_RET_ERROR;
	}

	for (i = 0; i < MAX_SUBNET; ++i) {
		if(subnet[i].index != -1){
			if (sub_index == subnet[i].index) {	//compare subnet index
				*subnode = &(subnet[i]);
				return DHCP_IP6_RET_SUCCESS;
			}
		}
		
	}

	return DHCP_IP6_RET_ERROR;
}



/********************************************************
 * dhcp_server_write_conf
 *
 * save subnet information to /etc/dhcpd.conf
 *
 *	INPUT:
 *		void
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR		  - funtion error
 *		DHCP_IP6_RET_SUCCESS	  - funtion success
 *
 *********************************************************/


int  dhcp_server_write_conf(void)
{
    struct dhcp_subnet6 *subnet = NULL;
    unsigned int i = 0, j = 0, k = 0;
    FILE *fp6 = NULL;

    subnet = get_subnet();
    if(NULL == subnet){    	 
        return DHCP_IP6_RET_ERROR;
    }

    fp6 = fopen(path_dhcpv6_conf, "w+");
    if(NULL == fp6){	 
        return DHCP_IP6_RET_ERROR;
    }

    for(i = 0; i < MAX_SUBNET; ++i)
    {
        if(subnet[i].index != -1){
            if ((subnet[i].sub_ip) && (subnet[i].low_ip) && (subnet[i].high_ip)) {
                fprintf(fp6, "subnet6 %s {\n", subnet[i].sub_ip);
                fprintf(fp6, " range6 %s ", subnet[i].low_ip);
                fprintf(fp6, "%s;\n", subnet[i].high_ip);
            }
            else{
                continue;
            }
            for(j = 0; j < subnet[i].domainnum; ++j){
                if(0 == j){

                    fprintf(fp6, " option dhcp6.domain-search \"%s\",", subnet[i].domainname[j]);
                }
                else{
                    fprintf(fp6, " \"%s\"", subnet[i].domainname[j]);
			 if (j < (subnet[i].domainnum - 1)) {
                        fprintf(fp6, ",");
                    }
                }
		   if(j == (subnet[i].domainnum -1)){
		   	 fprintf(fp6, ";\n");
		   }
            }           

            for(j = 0; j < subnet[i].dnsnum; ++j){
                if(0 == j){
                    fprintf(fp6, " option dhcp6.name-servers %s,", subnet[i].dns[j]);
                }
                else{
                    fprintf(fp6, "%s", subnet[i].dns[j]);
                    if (j < (subnet[i].dnsnum - 1)) {
                        fprintf(fp6, ",");
                    }
                }
		   if(j == (subnet[i].dnsnum -1)){
		   	 fprintf(fp6, ";\n");
		   }
            }
			
            if(subnet[i].defaulttime) {
            	    fprintf(fp6, " default-lease-time %d;\n", subnet[i].defaulttime);
            }
            if(subnet[i].maxtime) {
                 fprintf(fp6, " max-lease-time %d;\n", subnet[i].maxtime);
            }
            fprintf(fp6, "}\n");
        }
       
    }

    fclose(fp6);

    return DHCP_IP6_RET_SUCCESS;
}



DEFUN(create_ip6_subnet_name_cmd_func,
        create_ip6_subnet_name_cmd,
        "ip6 subnet SUBNET",
        "Config ipv6 command\n"
        "Config subnet command\n"
        "Create ip6 subnet entity\n"        
     )
{
    char* subip = NULL;
    unsigned int nameSize = 0, ret = 0;
    struct dhcp_subnet6  *tempsub = NULL, *sub_net = NULL ;
    int *sub_count = NULL;   
    int i;
	
    int semid;
    struct sembuf sem[2]; /* sembuf defined in sys/sem.h */

    semid = semget(ftok("/tmp/mutex", 'S'), 1, 0666 | IPC_CREAT);
    if (semid < 0) {
	vty_out(vty, "%% get semaphore resource fail\n");
	return CMD_WARNING;
    }
    /* These never change so leave them outside the loop */
    sem[0].sem_num = 0;
    sem[1].sem_num = 0;
    sem[0].sem_flg = SEM_UNDO; /* Release semaphore on exit */
    sem[1].sem_flg = SEM_UNDO; /* Release semaphore on exit */
	
    nameSize = strlen(argv[0]);
    subip = (char*)malloc(nameSize + 1);
    if (NULL == subip) {
	  vty_out(vty, "%% get memory resource fail\n");
        return CMD_WARNING;
    }   
    memset(subip, 0, (nameSize + 1));
    memcpy(subip, argv[0], nameSize);
    
    ret = dhcp_find_subnet6_by_name(subip, &tempsub);   
    if (ret) {						//not find the subnet
        sub_net = get_subnet();
        if(NULL == sub_net){	
            vty_out(vty, "%% access subnet fail\n");
            return CMD_WARNING;
        }
        sub_count = get_subnet6_count();
        if(NULL == sub_count ){
	      vty_out(vty, "%% access subnet  fail\n");
            return CMD_WARNING;
        }       
		
	sem[0].sem_op = 0; /* Wait for zero */
	sem[1].sem_op = 1; /* Add 1 to lock it*/				
	semop(semid, sem, 2);

        for(i = 0; i < MAX_SUBNET; ++i)
        {
            if(-1 == sub_net[i].index)		//save information to share memory where the location is not data (index = -1)
            {
                strncpy(sub_net[i].sub_ip , subip, nameSize);
		   sub_net[i].sub_ip[nameSize] = '\0';
                sub_net[i].index = *sub_count;
                break;
            }
        }		
	
        (*sub_count)++;	       

	 sem[0].sem_op = -1; /*	to unlock */			
	 semop(semid, sem, 1);

        vty->node = SUBNET6_CONF_NODE;
        vty->index = sub_net[i].index;
	  
    }
    else { 
        vty->node = SUBNET6_CONF_NODE;
        vty->index = tempsub->index;
    }	
 
    if(subip != NULL){
    	free(subip);
    }
}



DEFUN(delete_ip6_subnet_name_cmd_func,
        delete_ip6_subnet_name_cmd,
        "no ip6 subnet SUBNET",
        "delete operating\n"
        "Config ipv6 command\n"
        "Config subnet command\n"
        "delete subnet command\n"
     )
{
    char* subip = NULL;
    unsigned int nameSize = 0, ret = 0;
    struct dhcp_subnet6  *tempsub = NULL;
    int *sub_count = NULL;
    int semid;
    struct sembuf sem[2]; /* sembuf defined in sys/sem.h */

    semid = semget(ftok("/tmp/mutex", 'S'), 1, 0666 | IPC_CREAT);
    if (semid < 0) {
	vty_out(vty, "%% get semaphore resource fail\n");
	return CMD_WARNING;
    }
    /* These never change so leave them outside the loop */
    sem[0].sem_num = 0;
    sem[1].sem_num = 0;
    sem[0].sem_flg = SEM_UNDO; /* Release semaphore on exit */
    sem[1].sem_flg = SEM_UNDO; /* Release semaphore on exit */

    nameSize = strlen(argv[0]);
    subip = (char*)malloc(nameSize + 1);
    if (NULL == subip) {
	  vty_out(vty, "%% get memory resource fail\n");
        return CMD_WARNING;
    }

    memset(subip, 0, (nameSize + 1));
    memcpy(subip, argv[0], nameSize);    

    ret = dhcp_find_subnet6_by_name(subip, &tempsub);
    if (ret) {
        vty_out(vty, "%% Subnet does not exist\n");
    }
    else {

       sem[0].sem_op = 0; /* Wait for zero */
	sem[1].sem_op = 1; /* Add 1 to lock it*/				
	semop(semid, sem, 2);
        memset(tempsub, 0, sizeof(struct dhcp_subnet6));
        tempsub->index = -1;		//delete the subnet operating is set (index = -1)
        sub_count = get_subnet6_count();
        if(NULL == sub_count){
	     vty_out(vty, "%% access subnet fail\n");
            return CMD_WARNING;
        }
        (*sub_count)--;

        sem[0].sem_op = -1; /*  to unlock */			
        semop(semid, sem, 1);
    }
	
    if(subip != NULL){
    		free(subip);
    }
}


DEFUN(add_dhcp_subnet_ipv6_range_cmd_func,
        add_dhcp_subnet_ipv6_range_cmd,
        "range6 X:X::X:X X:X::X:X",
        "Add dhcp ip6 subnet range\n"
        "Add dhcp ip6 subnet range\n"
        "Delete dhcp ip range\n"
        "Range ip"
        "Low ip string\n"
        "High ip strint\n"
     )
{
    unsigned int ret = 0, index = 0, len = 0;
    struct dhcp_subnet6  *tempsub = NULL;

     int semid;
    struct sembuf sem[2]; /* sembuf defined in sys/sem.h */

      if(check_ipv6_address(argv[0])){
         vty_out(vty, "%% address %s  is Illegal\n", argv[0]);
         return CMD_WARNING;
     }
     if(check_ipv6_address(argv[1])){
         vty_out(vty, "%% address %s  is Illegal\n", argv[1]);
         return CMD_WARNING;
     }

    semid = semget(ftok("/tmp/mutex", 'S'), 1, 0666 | IPC_CREAT);
    if (semid < 0) {
	vty_out(vty, "%% get semaphore resource fail\n");
	return CMD_WARNING;
    }
    /* These never change so leave them outside the loop */
    sem[0].sem_num = 0;
    sem[1].sem_num = 0;
    sem[0].sem_flg = SEM_UNDO; /* Release semaphore on exit */
    sem[1].sem_flg = SEM_UNDO; /* Release semaphore on exit */

    ret = dhcp_find_subnet6_by_index(vty->index, &tempsub);
    if (!ret) {
       sem[0].sem_op = 0; /* Wait for zero */
	sem[1].sem_op = 1; /* Add 1 to lock it*/				
	semop(semid, sem, 2);
		
	  len = strlen(argv[0]);
	
        strncpy(tempsub->low_ip, argv[0], len);
        tempsub->low_ip[len] = '\0';

	  len = strlen(argv[1]);
        strncpy(tempsub->high_ip, argv[1], len);
        tempsub->high_ip[len] = '\0';

        sem[0].sem_op = -1; /*  to unlock */			
        semop(semid, sem, 1);
    }
    else {
		vty_out(vty, "%% dose not find the subnet\n");
    }
   
}

DEFUN(ip6_dhcp_server_dns_cmd_func,
        ip6_dhcp_server_dns_cmd,
        "ip6 dhcp server dns IPV6 [IPV6] [IPV6]",
        "Config ipv6 command\n"
        "Config dhcp command\n"
        "Config dhcp server command\n"
        "Ip dhcp server dns entity\n"
        "ipv6 dns address\n"
        "ipv6 dns address\n"
        "ipv6 dns address\n"
     )
{
    unsigned int ret = 0, len = 0, ipnum = 0, i = 0;
    struct dhcp_subnet6  *tempsub = NULL;
     int semid;
    struct sembuf sem[2]; /* sembuf defined in sys/sem.h */

    for(i = 0; i < argc; i++){
	   if(check_ipv6_address(argv[i])){
      	         vty_out(vty, "%% dns %s  is Illegal\n", argv[i]);
      	         return CMD_WARNING;
     	     }
    }

    semid = semget(ftok("/tmp/mutex", 'S'), 1, 0666 | IPC_CREAT);
    if (semid < 0) {
	vty_out(vty, "%% get semaphore resource fail\n");
	return CMD_WARNING;
    }
    /* These never change so leave them outside the loop */
    sem[0].sem_num = 0;
    sem[1].sem_num = 0;
    sem[0].sem_flg = SEM_UNDO; /* Release semaphore on exit */
    sem[1].sem_flg = SEM_UNDO; /* Release semaphore on exit */

    if(SUBNET6_CONF_NODE == vty->node){
        ret = dhcp_find_subnet6_by_index(vty->index, &tempsub);
        if (!ret) {

            sem[0].sem_op = 0; /* Wait for zero */
            sem[1].sem_op = 1; /* Add 1 to lock it*/				
            semop(semid, sem, 2);

            ipnum = argc;
            for(;i < ipnum; i++) {
		   len = strlen(argv[i]);		   
                strncpy(tempsub->dns[i], argv[i], len);
                tempsub->dns[i][len] = '\0';		 
            }
            tempsub->dnsnum = ipnum;

            sem[0].sem_op = -1; /*  to unlock */			
            semop(semid, sem, 1);

	      return CMD_SUCCESS;
        }
        else {
		vty_out(vty, "%% dose not find the subnet\n");
        }
    }
    else {
        vty_out(vty, "must sunet6 configure mode \n");
    }    
}


DEFUN(ip6_dhcp_server_domain_name_cmd_func,
        ip6_dhcp_server_domain_name_cmd,
        "ip6 dhcp server domain NAME [NAME] [NAME]",
        "Config ipv6 command\n"
        "Config dhcp command\n"
        "Config dhcp server command\n"
        "Ip dhcp server dns entity\n"
        "ipv6 domain name\n"
        "ipv6 domain name\n"
        "ipv6 domain name\n"        
     )
{
    unsigned int ret = 0, len = 0, i = 0, domainnum = 0;
    struct dhcp_subnet6  *tempsub = NULL;
    int semid;
    struct sembuf sem[2]; /* sembuf defined in sys/sem.h */

    for(i = 0 ; i < argc ; i++){
	 if((len = strlen(argv[i])) > 64){
		vty_out(vty, "%% domain %s  is too long\n", argv[i]);
		return CMD_WARNING;
	        }
	   if((ret = check_doamin(argv[i]))){
	   	vty_out(vty, "%% domain %s  is Illegal\n",argv[i] );
		return CMD_WARNING;
	   }
    }

    semid = semget(ftok("/tmp/mutex", 'S'), 1, 0666 | IPC_CREAT);
    if (semid < 0) {
	vty_out(vty, "%% get semaphore resource fail\n");
	return CMD_WARNING;
    }
    /* These never change so leave them outside the loop */
    sem[0].sem_num = 0;
    sem[1].sem_num = 0;
    sem[0].sem_flg = SEM_UNDO; /* Release semaphore on exit */
    sem[1].sem_flg = SEM_UNDO; /* Release semaphore on exit */

    if(SUBNET6_CONF_NODE == vty->node){
        ret = dhcp_find_subnet6_by_index(vty->index, &tempsub);
        if (!ret) {
            sem[0].sem_op = 0; /* Wait for zero */
            sem[1].sem_op = 1; /* Add 1 to lock it*/				
            semop(semid, sem, 2);

	      domainnum = argc ;
            for(;i < domainnum; i++) {
                len = strlen(argv[i]);			
                strncpy(tempsub->domainname[i], argv[i], len);
                tempsub->domainname[i][len] = '\0';
            }
            tempsub->domainnum = domainnum;
            sem[0].sem_op = -1; /*  to unlock */			
            semop(semid, sem, 1);
            return CMD_SUCCESS;
        }
        else {
		vty_out(vty, "%% dose not find the subnet\n");
        }
    }
    else {
        vty_out(vty, "must sunet6 configure mode \n");
    }
  
}


DEFUN(ip6_dhcp_server_default_time_cmd_func,
        ip6_dhcp_server_default_time_cmd,
        "ip6 dhcp server lease-default <1-365>",
        "Config ipv6 command\n"
        "Config dhcp command\n"
        "Config dhcp server command\n"
        "Ip dhcp server lease default time entity\n"
        "day express time\n"
     )
{
    unsigned int ret = 0, len = 0, lease_default = 0;
    struct dhcp_subnet6  *tempsub = NULL;
     int semid;
    struct sembuf sem[2]; /* sembuf defined in sys/sem.h */

    semid = semget(ftok("/tmp/mutex", 'S'), 1, 0666 | IPC_CREAT);
    if (semid < 0) {
	vty_out(vty, "%% get semaphore resource fail\n");
	return CMD_WARNING;
    }
    /* These never change so leave them outside the loop */
    sem[0].sem_num = 0;
    sem[1].sem_num = 0;
    sem[0].sem_flg = SEM_UNDO; /* Release semaphore on exit */
    sem[1].sem_flg = SEM_UNDO; /* Release semaphore on exit */
	
    if(SUBNET6_CONF_NODE == vty->node){
        ret = dhcp_find_subnet6_by_index(vty->index, &tempsub);
        if (!ret) {
            sem[0].sem_op = 0; /* Wait for zero */
            sem[1].sem_op = 1; /* Add 1 to lock it*/				
            semop(semid, sem, 2);
            lease_default = atoi((char*)argv[0]);
            tempsub->defaulttime = lease_default*60*60*24;
            sem[0].sem_op = -1; /*  to unlock */			
     	      semop(semid, sem, 1);
        }
        else {
		vty_out(vty, "%% dose not find the subnet\n");
        }
    }
    else {
        vty_out(vty, "must under sunet6 configure mode \n");
    }

}


DEFUN(
        ip6_dhcp_server_max_time_cmd_func,
        ip6_dhcp_server_max_time_cmd,
        "ip6 dhcp server lease-max <1-365>",
        "Config ipv6 command\n"
        "Config dhcp command\n"
        "Config dhcp server command\n"
        "Ip dhcp server lease max time entity\n"
        "day express time\n"
     )
{
    unsigned int ret = 0, len = 0, lease_max = 0;
    struct dhcp_subnet6  *tempsub = NULL;
    int semid;
    struct sembuf sem[2]; /* sembuf defined in sys/sem.h */

    semid = semget(ftok("/tmp/mutex", 'S'), 1, 0666 | IPC_CREAT);
    if (semid < 0) {
	vty_out(vty, "%% get semaphore resource fail\n");
	return CMD_WARNING;
    }
    /* These never change so leave them outside the loop */
    sem[0].sem_num = 0;
    sem[1].sem_num = 0;
    sem[0].sem_flg = SEM_UNDO; /* Release semaphore on exit */
    sem[1].sem_flg = SEM_UNDO; /* Release semaphore on exit */
	
    if(SUBNET6_CONF_NODE == vty->node){
        ret = dhcp_find_subnet6_by_index(vty->index, &tempsub);
        if (!ret) {
            sem[0].sem_op = 0; /* Wait for zero */
            sem[1].sem_op = 1; /* Add 1 to lock it*/				
            semop(semid, sem, 2);

            lease_max = atoi((char*)argv[0]);
            tempsub->maxtime= lease_max*60*60*24;

	      sem[0].sem_op = -1; /*  to unlock */			
            semop(semid, sem, 1);
        }
        else {
		vty_out(vty, "%% dose not find the subnet\n");
        }
    }
    else {
        vty_out(vty, "must under sunet6 configure mode \n");
    }

}


DEFUN(ip6_dhcp_server_enable_cmd_func,
        ip6_dhcp_server_enable_cmd,
        "ip6 dhcp server (enable|disable)",
        "Config ipv6 command\n"
        "Config dhcp command\n"
        "Config dhcp server command\n"
        "start dhcp server\n"
        "stop dhcp server\n"
       
     )
{
    unsigned int ret = 0, isEnable = 0, ret2 = 0;
    int *enable_flag = NULL;
    int semid;
    struct sembuf sem[2]; /* sembuf defined in sys/sem.h */

    semid = semget(ftok("/tmp/mutex", 'S'), 1, 0666 | IPC_CREAT);
    if (semid < 0) {
	vty_out(vty, "%% get semaphore resource fail\n");
	return CMD_WARNING;
    }
    /* These never change so leave them outside the loop */
    sem[0].sem_num = 0;
    sem[1].sem_num = 0;
    sem[0].sem_flg = SEM_UNDO; /* Release semaphore on exit */
    sem[1].sem_flg = SEM_UNDO; /* Release semaphore on exit */
	
    enable_flag = get_ip6_dhcp_server_enable_flag();
    if(NULL == enable_flag){
        vty_out(vty, "%% access memory fail\n");
        return CMD_WARNING;
    }

    if(strncmp("enable",argv[0],strlen(argv[0]))==0) {
        isEnable = 1;
    }
    else if (strncmp("disable",argv[0],strlen(argv[0]))==0) {
        isEnable = 0;
    }
    else {
        vty_out(vty,"%% bad command parameter!\n");
        return CMD_WARNING;
    }

    sem[0].sem_op = 0; /* Wait for zero */
    sem[1].sem_op = 1; /* Add 1 to lock it*/				
    semop(semid, sem, 2);
    if (isEnable) {
        dhcp_server_write_conf();
        ret = system("/usr/bin/dhcp6_start.sh");
        if(WEXITSTATUS(ret) == 2) {
            vty_out(vty, "start dhcp ipv6 fail \n");
	     *enable_flag = 0;
        }
        else {
            *enable_flag = 1;
        }
    }
    else {
        system("pkill dhcp");
        *enable_flag = 0;
    }
    sem[0].sem_op = -1; /*  to unlock */			
    semop(semid, sem, 1);

}

DEFUN(ip6_show_dhcp_server_cmd_func,
        ip6_show_dhcp_server_cmd,
        "show ip6 dhcp",
        CONFIG_STR
        "Show ip6 dhcp configure entity\n"
        "Show ip6 dhcp configure entity\n"
     )
{
    unsigned int ret = 0, i = 0, ret2 = 0, j;
    struct dhcp_subnet6 *subnet = NULL;

    int *enable_flag = NULL;

    enable_flag = get_ip6_dhcp_server_enable_flag();
    if(NULL == enable_flag){
	  vty_out(vty, "%% access memory fail\n");
        return CMD_WARNING;
    }

    subnet = get_subnet();
    if(NULL == subnet){
	  vty_out(vty, "%% access memory fail\n");
        return CMD_WARNING;
    }

    for(i = 0; i < MAX_SUBNET; ++i)
    {
        if(subnet[i].index != -1){
            vty_out(vty, "===============================================\n");
            vty_out(vty, "                 dhcp server ip6 configure\n");
            vty_out(vty, "ip6 subnet %s\n", subnet[i].sub_ip);
            vty_out(vty, "range6 %s ", subnet[i].low_ip);
            vty_out(vty, "%s\n", subnet[i].high_ip);

            for(j = 0; j < subnet[i].domainnum; ++j){
                if(0 == j){

                    vty_out(vty, "ip6 dhcp server domain : \"%s\"", subnet[i].domainname[j]);
                }
                else{
                    vty_out(vty, " \"%s\"", subnet[i].domainname[j]);
                }
            }
            vty_out(vty, "\n");

            for(j = 0; j < subnet[i].dnsnum;++j){
                if(0 == j){
                    vty_out(vty, "ip6 dhcp server dns : %s", subnet[i].dns[j]);

                }
                else{
                    vty_out(vty, " %s", subnet[i].dns[j]);
                }
            }
            vty_out(vty, "\n");
            if(subnet[i].defaulttime) {
                vty_out(vty, "ip6 dhcp server lease-default : %d\n", subnet[i].defaulttime/(60*60*24));
            }
            if(subnet[i].maxtime) {
                vty_out(vty, "ip6 dhcp server lease-max : %d\n", subnet[i].maxtime/(60*60*24));
            }
            vty_out(vty, "===============================================\n\n");
        }
    }

    if (*enable_flag) {
        vty_out(vty, "ip6 dhcp server enable\n");
    }
}


/********************************************************
 * dcli_dhcp6_show_running_cfg
 *
 * show dhcp6 config
 *
 *	INPUT:
 *		vty 
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR		  - funtion error
 *		DHCP_IP6_RET_SUCCESS	  - funtion success
 *
 *********************************************************/

int dcli_dhcp6_show_running_cfg(struct vty *vty)
{
    char *cursor = NULL;
    char _tmpstr[64] = {0};
    int ret = 1, totalLen = 0, i = 0, j;
    struct dhcp_subnet6 *subnet = NULL;

    int *enable_flag = NULL;

    char showStr[10240] = {0};

    cursor = showStr;

    enable_flag = get_ip6_dhcp_server_enable_flag();
    if(NULL == enable_flag){
        return DHCP_IP6_RET_ERROR;
    }

    subnet = get_subnet();
    if(NULL == subnet){        
        return DHCP_IP6_RET_ERROR;
    }
    for(i = 0; i < MAX_SUBNET; ++i){
        if(subnet[i].index != -1){
            if ((subnet[i].sub_ip) && (subnet[i].low_ip) && (subnet[i].high_ip)){
                totalLen += sprintf(cursor, "ip6 subnet %s\n", subnet[i].sub_ip);
                cursor = showStr + totalLen;
                totalLen += sprintf(cursor, " range6 %s ", subnet[i].low_ip);
                cursor = showStr + totalLen;
                totalLen += sprintf(cursor, "%s\n", subnet[i].high_ip);
                cursor = showStr + totalLen;
            }
            else{
		   vty_out(vty, "%% the dhcp not Configuration\n");
                return DHCP_IP6_RET_ERROR;
            }
            for(j = 0; j < subnet[i].domainnum; ++j){
                if(0 == j){
                    totalLen += sprintf(cursor, " ip6 dhcp server domain %s",subnet[i].domainname[i]);
                    cursor = showStr + totalLen;
                }
                else{
                    totalLen += sprintf(cursor, " %s", subnet[i].domainname[i]);
                    cursor = showStr + totalLen;
                }
            }
            totalLen += sprintf(cursor, "\n");
            cursor = showStr + totalLen;

            for(j = 0; j < subnet[i].dnsnum; ++j){
                if(0 == j){
                    totalLen += sprintf(cursor, " ip6 dhcp server dns %s",subnet[i].dns[j] );
                    cursor = showStr + totalLen;

                }
                else{
                    totalLen += sprintf(cursor, " %s", subnet[i].dns[j]);
                    cursor = showStr + totalLen;
                }
            }
            totalLen += sprintf(cursor, "\n");
            cursor = showStr + totalLen;

            if(subnet[i].defaulttime) {
                totalLen += sprintf(cursor, " ip6 dhcp server lease-default %d\n", subnet[i].defaulttime/(60*60*24));
                cursor = showStr + totalLen;
            }
            if(subnet[i].maxtime) {
                totalLen += sprintf(cursor, " ip6 dhcp server lease-max %d\n", subnet[i].maxtime/(60*60*24));
                cursor = showStr + totalLen;
            }
            totalLen += sprintf(cursor, "exit \n\n");
            cursor = showStr + totalLen;
        }


    }

    if (*enable_flag) {
        totalLen += sprintf(cursor, "ip6 dhcp server enable\n");
        cursor = showStr + totalLen;
    }
    sprintf(_tmpstr, BUILDING_MOUDLE, "DHCP6 SERVER");
    vtysh_add_show_string(_tmpstr);
    vtysh_add_show_string(showStr);
    ret = 0;

    return DHCP_IP6_RET_SUCCESS;
}


void 
dcli_dhcp6_init
(
	void
)  
{
	leases_file = fopen("/etc/dhcpd6.leases", "a");

	/*install_node (&subnet6_node, dcli_dhcp6_show_running_cfg, "SUBNET6_CONF_NODE");*/
	install_node (&subnet6_node, NULL, "SUBNET6_CONF_NODE");
	install_default(SUBNET6_CONF_NODE);	
	/*
	install_element(CONFIG_NODE, &create_ip6_subnet_name_cmd);	
	install_element(CONFIG_NODE, &delete_ip6_subnet_name_cmd);
	install_element(SUBNET6_CONF_NODE, &add_dhcp_subnet_ipv6_range_cmd);	
	install_element(SUBNET6_CONF_NODE, &ip6_dhcp_server_dns_cmd);	
	install_element(SUBNET6_CONF_NODE, &ip6_dhcp_server_domain_name_cmd);
	install_element(SUBNET6_CONF_NODE, &ip6_dhcp_server_max_time_cmd);
	install_element(SUBNET6_CONF_NODE, &ip6_dhcp_server_default_time_cmd);
	install_element(CONFIG_NODE, &ip6_show_dhcp_server_cmd);
	install_element(CONFIG_NODE, &ip6_dhcp_server_enable_cmd);	
	*/
}

#ifdef __cplusplus
}
#endif
