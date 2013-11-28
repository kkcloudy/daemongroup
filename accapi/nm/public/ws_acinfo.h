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
* ws_acinfo.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifndef _WS_ACINFO_H
#define _WS_ACINFO_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define ACBACKUPFILE "/var/run/redundancy_backup_info"
#define ACBACKUPIDEN "/var/run/identity_backup_info"
#define ACBACKUPMODE "/var/run/mode_backup_info"


#define AC_IDENTITY	"identity"
#define AC_MODE		"mode"
#define AC_STATUS	"status"
#define AC_NETIP	"netip"
#define N_VALEE		"value"
#define N_CONTENT	"content"
#define AC_NETIP_IPV6	"netip_ipv6"

#define IDEN_TYPE	"iden_t"
#define MODE_TYPE	"mode_t"
#define STATUS_TYPE "status_t"
#define NETIP_TYPE  "netip_t"
#define NETIP_TYPE_IPV6  "netip_t_ipv6"

#define LOC_TYPE	"loc_t"
#define CON_TYPE	"con_t"
#define NET_TYPE 	"net_t"


struct netipbk_st
{
	char netip[128];
	char slotid[10];
	char content[50];
	struct netipbk_st *next;
};

struct acbackup_st
{
	char identity[20];
	char mode[20];
	char status[20];
	struct netipbk_st netipst;
	struct netipbk_st netipst_ipv6;
};

struct bkacinfo_st
{
	char key[20];
	char insid[10];
	char netip[128];
};
extern void Free_read_acinfo_xml(struct acbackup_st *head);
extern int read_acinfo_xml(struct acbackup_st *chead,int *confnum);
extern int mod_insbk_xmlnode(char *xmlpath,char *node_name,char *slotid,char *netip);
extern int get_ip_by_active_instance(char *instance,unsigned long *ip);

#endif
