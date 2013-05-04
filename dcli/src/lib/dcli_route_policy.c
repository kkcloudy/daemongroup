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
* dcli_route_policy.c
*
* MODIFY:
*		by <shanchx@autelan.com> on 02/24/2010 revision <0.1>
*
* CREATOR:
*		shanchx@autelan.com
*
* DESCRIPTION:
*		ac route policy config
*
* DATE:
*		02/24/2010	
*
*  FILE REVISION NUMBER:
*	$Revision: 1.8 $
*  			
*******************************************************************************/
#include <stdlib.h>
#include <zebra.h>
#include "command.h"
#include "dcli_route_policy.h"
	
static char *dcli_rp_table[256] ;
static char * dcli_rtdsfield_table[256] ;
static struct rp_rule_node * g_rp_rule = NULL;
static struct rp_ip_route_node * g_rp_ip_route = NULL;

static void rtdsfield_initialize()
{
	char buf[512];
	FILE *fp;
	int size = 256;

	fp = fopen(RTFIELD_DATABASE_FILE, "r");
	if (!fp)
		return;
	while (fgets(buf, sizeof(buf), fp)) {
		char *p = buf;
		int id;
		char namebuf[512];

		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == '#' || *p == '\n' || *p == 0)
			continue;
		if (sscanf(p, "0x%x %s\n", &id, namebuf) != 2 &&
		    sscanf(p, "0x%x %s #", &id, namebuf) != 2 &&
		    sscanf(p, "%d %s\n", &id, namebuf) != 2 &&
		    sscanf(p, "%d %s #", &id, namebuf) != 2) {
			return;
		}
		
		if (id<0 || id>size)
			continue;

		dcli_rtdsfield_table[id] = strdup(namebuf);
	}
	fclose(fp);
}

static void rt_tables_initialize()
{
	char buf[512];
	FILE *fp;
	int size = 256;
	int i;

	for(i=0;i<size;i++)
	{
		if(dcli_rp_table[i]){
			free(dcli_rp_table[i]);
			dcli_rp_table[i] = NULL;
		}
	}
	fp = fopen(RT_TABLE_DATABASE_FILE, "r");
	if (!fp)
		return;
	
	while (fgets(buf, sizeof(buf), fp)) {
		char *p = buf;
		int id;
		char namebuf[512];

		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == '#' || *p == '\n' || *p == 0)
			continue;
		if (sscanf(p, "0x%x %s\n", &id, namebuf) != 2 &&
		    sscanf(p, "0x%x %s #", &id, namebuf) != 2 &&
		    sscanf(p, "%d %s\n", &id, namebuf) != 2 &&
		    sscanf(p, "%d %s #", &id, namebuf) != 2) {
		    fclose(fp);
			return;
		}
		
		if (id<0 || id>size)
			continue;

		dcli_rp_table[id] = strdup(namebuf);
	}
	fclose(fp);
}
static void rt_tables_write()
{
	FILE *fp;
	int size = 256;

	fp = fopen(RT_TABLE_DATABASE_FILE, "w");
	if (!fp)
		return;
	fprintf(fp,
			"#\n"
			"# reserved values\n"
			"#\n"
			"255 local\n"
			"254 main\n"
			"253 default\n"
			"0 unspec\n"
			"#\n"
			"# local\n"
			"#\n"
			"#1 inr.ruhep\n");
	for(size=1;size<253;size++)
	{
		if(rt_tables_entry_name(size)!=NULL)
			fprintf(fp,"%d %s\n",size,dcli_rp_table[size]);
	}
	fclose(fp);
}

struct rp_ip_route_node * rp_ip_route_pares(char *buf)
{
	char *p = buf;
	int sn,id;
	char namebuf[512];
	struct rp_ip_route_node *result;

	while (*p == ' ' || *p == '\t')
		p++;
	
	if (*p == '#' || *p == '\n' || *p == 0)
		return NULL;

	result = malloc(sizeof(*result));
	if(result == NULL)
		return NULL;
	memset(result,0,sizeof(*result));
	
	/*32756:  from all iif eth1-9 lookup _rp100*/
	if (sscanf(p, "%s via %s dev", result->prefix,result->dst) == 2)
	{
		if(!strcmp(result->prefix,"default"))
		{
			sprintf(result->prefix,"0.0.0.0/0");
		}
		else if(!strchr(result->prefix,'/'))
		{
			strcat(result->prefix,"/32");
		}
		result->type = 1;
	}
	/*32765:  from 100.0.0.0/8 lookup _rp100*/	
	else if(sscanf(p, "%s dev %s scope link", result->prefix,result->dst) == 2)
	{
		if(!strcmp(result->prefix,"default"))
		{
			sprintf(result->prefix,"0.0.0.0/0");
		}
		else if(!strchr(result->prefix,'/'))
		{
			strcat(result->prefix,"/32");
		}
		result->type = 2;
	}

	else
	{
		free(result);
		return NULL;
	}
	return result;
}

struct rp_rule_node * rp_rule_pares(char *buf)
{
	char *p = buf;
	int sn,id;
	char namebuf[32];
	struct rp_rule_node *result;

	while (*p == ' ' || *p == '\t')
		p++;
	
	if (*p == '#' || *p == '\n' || *p == 0)
		return NULL;

	result = malloc(sizeof(struct rp_rule_node));
	if(result == NULL)
		return NULL;
	memset(result,0,sizeof(struct rp_rule_node));
	memset(namebuf,0,32);
	/*32756:  from all iif eth1-9 lookup _rp100*/
	if (sscanf(p, "%d:	from all iif %s lookup %s", &result->sn,result->ifname, result->tablename) == 3)
	{
		result->type = 1;
	}
	/*32765:  from 100.0.0.0/8 lookup _rp100*/	
	else if(sscanf(p, "%d:	from %s lookup %s", &result->sn, result->prefix,result->tablename) == 3)
	{
			result->type = 2;
			if(!strcmp(result->prefix,"all"))
			{
				sprintf(result->prefix,"0.0.0.0/0");
			}
			else if(!strchr(result->prefix,'/'))
			{
				strcat(result->prefix,"/32");
			}
	}
	/*32757:  from all to 220.0.0.0/24 lookup _rp100*/
	else if(sscanf(p, "%d:	from all to %s lookup %s", &result->sn, result->prefix,result->tablename) == 3)
	{
		result->type = 3;
		if(!strchr(result->prefix,'/'))
		{
			strcat(result->prefix,"/32");
		}
	}
	/*32761:  from all tos 0x06 lookup _rp100 */
	else if(sscanf(p, "%d:	from all tos %s lookup %s", &result->sn,namebuf,result->tablename) == 3)
	{
		int i=0;
		
		for(i=0;i<256;i++)
		{
			if (dcli_rtdsfield_table[i] && (strcmp(dcli_rtdsfield_table[i], namebuf) == 0)) 
			{
				break;
			}
		}
		if(i<256){
			result->tos = i;
			result->type = 4;
		}
		else{
			free(result);
			return NULL;
		}
	}

	else
	{
		free(result);
		return NULL;
	}
	return result;
}

struct rp_ip_route_node *rp_ip_route_get(unsigned char rtb_id)
{
	FILE *fp;
	char cmdstr[CMD_STR_LEN]={0};
	char buf[512];
	struct rp_ip_route_node *ptmp;

	sprintf(cmdstr,"ip route list table %d",rtb_id);
	fp = popen(cmdstr, "r");
	if (!fp)
		return 	NULL;
	
	while (fgets(buf, sizeof(buf), fp)) {
		char *p = buf;

		ptmp = rp_ip_route_pares(p);
		if(!ptmp)
			continue;
		if(!g_rp_ip_route){
			g_rp_ip_route= ptmp;
			g_rp_ip_route->prev=g_rp_ip_route;
		}
		else
		{
			g_rp_ip_route->prev->next=ptmp;
			ptmp->prev=g_rp_ip_route->prev;
			g_rp_ip_route->prev=ptmp;
		}
	}

	pclose(fp);
	return g_rp_ip_route;
}

struct rp_rule_node *rp_rule_get(void)
{
	FILE *fp;
	char *cmdstr= "ip rule list";
	char buf[512];
	struct rp_rule_node *ptmp,*tmp;

	
	fp = popen(cmdstr, "r");
	if (!fp)
		return 	NULL;
	memset(buf,0,512);
	while (buf == fgets(buf, 512, fp)) {
		char *p = buf;

		while (*p == ' ' || *p == '\t')
			p++;
		
		if (*p == '#' || *p == '\n' || *p == 0)
			continue;

		
		ptmp = rp_rule_pares(p);
		if(!ptmp)
			continue;
		if(!g_rp_rule){
			g_rp_rule= ptmp;
			g_rp_rule->prev=g_rp_rule;
/*			ptmp->next = g_rp_rule;*/

		}
		else if(ptmp->type == 2)
		{

			u_char isset=1;
			tmp=g_rp_rule;
			while(tmp)
			{
				if( tmp->type == 2 &&
					!(strcmp(tmp->tablename ,ptmp->tablename) 
					||strcmp(tmp->prefix, ptmp->prefix)))
				{	
					free(ptmp);
					isset = 0;
					break;
				}
				tmp=tmp->next;
			}
			
			if(isset)
			{
				g_rp_rule->prev->next=ptmp;
				
				ptmp->prev=g_rp_rule->prev;
				g_rp_rule->prev=ptmp;
/*				ptmp->next = g_rp_rule;*/

			}

		}
		else
		{
			g_rp_rule->prev->next=ptmp;

			ptmp->prev=g_rp_rule->prev;
			g_rp_rule->prev=ptmp;
/*			ptmp->next = g_rp_rule;*/

		}
		memset(buf,0,512);
	}

	pclose(fp);
	return g_rp_rule;
}

static void rp_ip_route_show(struct vty *vty,unsigned char rtb_id,const char* rtb_name)
{
	int i;	
	struct rp_ip_route_node * rtrn,*tmp;

	if(rtb_name)/*show */
	{
		rtrn = rp_ip_route_get(rtb_id);
		if(rtrn == NULL)
		{
			vty_out(vty,"The system policy route rule %d is none.\n",rtb_id);
			return ;
		}
		
		vty_out(vty,"Route policy %d ip route \n",rtb_id);
		while(rtrn )
		{
			switch(rtrn->type){
			case 1: /*32756:  from all iif eth1-9 lookup _rp100*/
				vty_out(vty,"%s via %s \n",rtrn->prefix,rtrn->dst);
				break;
			case 2: /*32765:  from 100.0.0.0/8 lookup _rp100*/	
				vty_out(vty,"%s dev %s scope link \n",rtrn->prefix,rtrn->dst);
				break;
			default:
				break;
			}
			tmp = rtrn;
			rtrn=rtrn->next;
			free(tmp);
		
		}
		g_rp_ip_route = NULL;
	}
	vty_out(vty,"\n");
	return;
}

static void rp_rule_show(struct vty *vty,unsigned char rtb_id,const char* rtb_name)
{
	int i;	
	struct rp_rule_node * rtrn,*tmp;

	if(rtb_name)/*show */
	{
		rtrn = rp_rule_get();
		if(rtrn == NULL)
		{
			vty_out(vty,"The system route policy rule %d is NULL.\n",rtb_id);
			return ;
		}
		
		vty_out(vty,"Route policy %d ip rule :\n",rtb_id);
		while(rtrn )
		{
			if(strcmp(rtrn->tablename,rtb_name) == 0)
			{
				switch(rtrn->type){
					case 1: /*32756:  from all iif eth1-9 lookup _rp100*/
						vty_out(vty,"%8d: from all IIF %s\n",rtrn->sn,rtrn->ifname);
						break;
					case 2: /*32765:  from 100.0.0.0/8 lookup _rp100*/	
						vty_out(vty,"%8d: from %s \n",rtrn->sn,rtrn->prefix);
						break;
					case 3: /*32757:  from all to 220.0.0.0/24 lookup _rp100*/
						vty_out(vty,"%8d: from all to %s \n",rtrn->sn,rtrn->prefix);
						break;
					case 4: /*32761:  from all tos 0x06 lookup _rp100 */
						vty_out(vty,"%8d: from all tos %d \n",rtrn->sn,rtrn->tos);
						break;
					default:
						break;
					}
			}
			tmp = rtrn;
			rtrn=rtrn->next;
			free(tmp);
		
		}
		g_rp_rule = NULL;
	}
	vty_out(vty,"\n");
	return;
}


/****************************************************
*cmd:
*	0 delete
*	1 add or modify
*
************************************************************/
static int rt_tables_entry_changed(int cmd,unsigned char rtb_id )
{
	char rtb_name[32];
	char *tmp_str;

	if(rtb_id==0 || rtb_id>32 )
		return 0;

	if(dcli_rp_table[rtb_id] != NULL)
	{
		free(dcli_rp_table[rtb_id]);
		dcli_rp_table[rtb_id]=NULL;
	}
	if(cmd == 1)/*add or modify*/
	{
		tmp_str=RTB_NAME(rtb_name,rtb_id);
		dcli_rp_table[rtb_id] = strdup(rtb_name);
	}
	return 1;
}
static unsigned char g_rtb_id ;

DEFUN(set_route_policy_func,
	set_route_policy_func_cmd,
	"route policy <1-32>",
	"Route policy\n"
	"Policy\n"
	"Route policy ID\n"
	)
{
	unsigned char rtb_id = (unsigned char)atoi(argv[0]);
	
	if(!IS_RTB_ID(rtb_id))
	{
		vty_out(vty,"Error policy id %s\n",argv[0]);
		return CMD_WARNING;
	}
	rt_tables_initialize();
	if(rt_tables_entry_name(rtb_id))
	{
		g_rtb_id = (unsigned char)rtb_id;
		vty->node = ROUTE_POLICY_NODE;
		vty->index = &g_rtb_id;
		return CMD_SUCCESS;
	}
	if(rt_tables_entry_changed(1,rtb_id))
	{
		rt_tables_write();
		g_rtb_id = (unsigned char)rtb_id;
		vty->index = &g_rtb_id;
		vty->node = ROUTE_POLICY_NODE;
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"Can't create route policy %s\n",argv[0]);
		return CMD_WARNING;
	}
}
DEFUN(del_route_policy_func,
	del_route_policy_func_cmd,
	"no route policy <1-32>",
	NO_STR
	"Route policy\n"
	"Policy\n"
	"Route policy ID\n"
	)
{
	unsigned char rtb_id = (unsigned char)atoi(argv[0]);
	
	if(!IS_RTB_ID(rtb_id))
	{
		vty_out(vty,"Error policy id %s\n",argv[0]);
		return CMD_WARNING;
	}
	
	rt_tables_initialize();
	
	rp_ip_route_delete(rtb_id);
	rp_rule_delete(rtb_id);
	
	if(!rt_tables_entry_name(rtb_id))
	{
		vty_out(vty,"The policy of id %s isn't exist\n",argv[0]);
		return CMD_SUCCESS;
	}
	if(rt_tables_entry_changed(0,rtb_id))
	{
		rt_tables_write();
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"Can't delete route policy %s\n",argv[0]);
		return CMD_WARNING;
	}
}

DEFUN(show_route_policy_func,
	show_route_policy_func_cmd,
	"show route policy <1-32>",
	SHOW_STR
	"Route policy\n"
	"Policy\n"
	"Route policy ID\n"
	)
{
	unsigned char rtb_id;

	
	rt_tables_initialize();
	if(argc == 1)
	{
		rtb_id=atoi(argv[0]);
		if(rtb_id<1 || rtb_id>32)
		{
			vty_out(vty,"Error route policy ID\n");
			return CMD_WARNING;
		}
		if(rt_tables_entry_name(rtb_id))
			rp_rule_show(vty,rtb_id,rt_tables_entry_name(rtb_id));
		else{
			vty_out(vty,"The route policy %d doesn't create\n",rtb_id);
			return CMD_WARNING;
		}
	}
	else if(argc==0)
	{
	
		for(rtb_id = 1; rtb_id<=32;rtb_id++)
		{			
			if(rt_tables_entry_name(rtb_id))
				rp_rule_show(vty,rtb_id,rt_tables_entry_name(rtb_id));
		}
	}
	return CMD_SUCCESS;
}
ALIAS(show_route_policy_func,
	show_route_policy_func_cmd1,
	"show route policy",
	SHOW_STR
	"Show boot img which can be used infomation\n"
	)

DEFUN (ip_policy_route, 
       ip_policy_route_cmd,
       "ip route A.B.C.D/M (A.B.C.D|INTERFACE)",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n")
{
	char cmdstr[CMD_STR_LEN]= {0};
	unsigned char rtbid = *(unsigned char*)(vty->index);
	char *rtb_name=NULL;
	int ret;

	if(!IS_RTB_ID(rtbid)|| !(rtb_name = rt_tables_entry_name(rtbid)))
	{
		vty_out(vty,"Get route policy error, pls check it\n");
		return CMD_WARNING;
	}
	
	ret = inet_aton (argv[1],NULL);

	if(ret==1)
		sprintf(cmdstr,"ip route add %s via %s table %s",argv[0],argv[1],rtb_name);
	else
		sprintf(cmdstr,"ip route add %s dev %s table %s",argv[0],argv[1],rtb_name);
	system(cmdstr);
	return CMD_SUCCESS;
	
}
#if 0
DEFUN (ip_route_flags2,
       ip_route_flags2_cmd,
       "ip route A.B.C.D/M (reject|blackhole)",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n")
{

}
#endif

void rp_ip_route_delete(unsigned char rtb_id)
{
	int i;	
	struct rp_ip_route_node * rtrn,*tmp;
	char cmdstr[CMD_STR_LEN]= {0};


	rtrn = rp_ip_route_get(rtb_id);
	if(rtrn == NULL)
	{
		return ;
	}
	
	while(rtrn )
	{
		switch(rtrn->type){
		case 1: /*32756:  from all iif eth1-9 lookup _rp100*/
			sprintf(cmdstr,"ip route del %s via %s table %d",rtrn->prefix,rtrn->dst,rtb_id);
			break;
		case 2: /*32765:  from 100.0.0.0/8 lookup _rp100*/	
			sprintf(cmdstr,"ip route del %s dev %s table %d",rtrn->prefix,rtrn->dst,rtb_id);
			break;
		default:
			break;
		}
		tmp = rtrn;
		rtrn=rtrn->next;
		free(tmp);
		system(cmdstr);
		system("ip route flush cache");
	}
	g_rp_ip_route = NULL;
	return;
}

void rp_rule_delete(unsigned char rtb_id)
{
	int i;	
	struct rp_rule_node * rtrn,*tmp;
	char cmdstr[CMD_STR_LEN]= {0};

	rt_tables_initialize();

	rtrn = rp_rule_get();
	if(rtrn == NULL)
	{
		return ;
	}
	
	while(rtrn )
	{
		if(rt_tables_entry_name(rtb_id)&&(strcmp(rtrn->tablename,rt_tables_entry_name(rtb_id)) == 0))
		{
			switch(rtrn->type){
				case 1: /*32756:  from all iif eth1-9 lookup _rp100*/
					sprintf(cmdstr,"ip rule del dev %s table %s",rtrn->ifname,rtrn->tablename);
					break;
				case 2: /*32765:  from 100.0.0.0/8 lookup _rp100*/	
					sprintf(cmdstr,"ip rule del from %s table %s",rtrn->prefix,rtrn->tablename);
					break;
				case 3: /*32757:  from all to 220.0.0.0/24 lookup _rp100*/
					sprintf(cmdstr,"ip rule del to %s table %s",rtrn->prefix,rtrn->tablename);
					break;
				case 4: /*32761:  from all tos 0x06 lookup _rp100 */
					sprintf(cmdstr,"ip rule del tos %x table %s",rtrn->tos,rtrn->tablename);
					break;
				default:
					break;
				}
			system(cmdstr);
			system("ip route flush cache");
		}
		tmp = rtrn;
		rtrn=rtrn->next;
		free(tmp);
	}

	g_rp_rule = NULL;
	return;
}



DEFUN (no_ip_policy_route, 
       no_ip_policy_route_cmd,
       "no ip route A.B.C.D/M (A.B.C.D|INTERFACE)",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       )
{
	char cmdstr[CMD_STR_LEN]= {0};
	unsigned char rtbid = *(unsigned char*)(vty->index);
	char *rtb_name=NULL;
	int ret;

	if(!IS_RTB_ID(rtbid)|| !(rtb_name = rt_tables_entry_name(rtbid)))
	{
		vty_out(vty,"Get route policy error, pls check it\n");
		return CMD_WARNING;
	}

	
	ret = inet_aton (argv[1],NULL);

	if(ret==1)
		sprintf(cmdstr,"ip route del %s via %s table %s",argv[0],argv[1],rtb_name);
	else
		sprintf(cmdstr,"ip route del %s dev %s table %s",argv[0],argv[1],rtb_name);


	system(cmdstr);
	return CMD_SUCCESS;
	
}

DEFUN (ip_policy_rule, 
		ip_policy_rule_cmd,
		"policy rule (add|delete) (from|to)  A.B.C.D/M",
		"Route policy\n"
		"Policy rule\n"
		"Add policy rule\n"
		"Delete policy rule\n"
		"Route source\n"
		"Route destination\n"
		"IP prefix (e.g. 10.0.0.0/8)\n"
       )
{
	char cmdstr[CMD_STR_LEN]= {0};
	unsigned char rtbid = *(unsigned char*)(vty->index);
	char *rtb_name=NULL;

	if(!IS_RTB_ID(rtbid)|| !(rtb_name = rt_tables_entry_name(rtbid)))
	{
		vty_out(vty,"Get route policy error, pls check it\n");
		return CMD_WARNING;
	}
	sprintf(cmdstr,"ip rule %s %s %s table %s",
		strncmp(argv[0],"a",1)?"del":"add",
		strncmp(argv[1],"f",1)?"to":"from",argv[2],
		rtb_name);

	
	system(cmdstr);
	system("ip route flush cache");
	return CMD_SUCCESS;
	
}

DEFUN (ip_policy_rule1, 
		ip_policy_rule_cmd1,
		"policy rule (add|delete) tos (4|8|16)",
		"Route policy\n"
		"Policy rule\n"
		"Add policy rule\n"
		"Delete policy rule\n"
		"Tos(terms of service)\n"
		"Reliability\n"
		"Throughput\n"
		"Lowdelay\n"
       )
{
	char cmdstr[CMD_STR_LEN]= {0};
	unsigned char rtbid = *(unsigned char*)(vty->index);
	char *rtb_name=NULL;
	int tos = atoi(argv[1]);

	if(!IS_RTB_ID(rtbid)|| !(rtb_name = rt_tables_entry_name(rtbid)))
	{
		vty_out(vty,"Get route policy error, pls check it\n");
		return CMD_WARNING;
	}
	sprintf(cmdstr,"ip rule %s tos %x table %s",
		strncmp(argv[0],"a",1)?"del":"add",tos,rtb_name);

	system(cmdstr);
	system("ip route flush cache");

	return CMD_SUCCESS;
	
}

DEFUN(ip_policy_rule2, 
		ip_policy_rule_cmd2,
		"policy rule (add|delete) interface IFNAME",
		"Route policy\n"
		"Policy rule\n"
		"Add policy rule\n"
		"Delete policy rule\n"
		"Source interface\n"
		"Interface name\n"
       )
{
	char cmdstr[CMD_STR_LEN]= {0};
	unsigned char rtbid = *(unsigned char*)(vty->index);
	char *rtb_name=NULL;

	if(!IS_RTB_ID(rtbid)|| !(rtb_name = rt_tables_entry_name(rtbid)))
	{
		vty_out(vty,"Get route policy error, pls check it\n");
		return CMD_WARNING;
	}
	sprintf(cmdstr,"ip rule %s dev %s table %s",
		strncmp(argv[0],"a",1)?"del":"add",argv[1],rtb_name);

	system(cmdstr);
	system("ip route flush cache");

	return CMD_SUCCESS;
	
}
DEFUN (show_ip_route_table,
       show_ip_route_table_cmd,
       "show ip route policy <1-32>",
       SHOW_STR
       IP_STR
       "IP routing table\n"
       "Route policy\n"
       "Route policy ID")
{
	unsigned char rtb_id;
	
	rt_tables_initialize();
	if(argc == 1)
	{
		rtb_id=atoi(argv[0]);
		if(rtb_id<1 || rtb_id>32)
		{
			vty_out(vty,"Error route policy ID\n");
			return CMD_WARNING;
		}
		if(rt_tables_entry_name(rtb_id))
			rp_ip_route_show(vty,rtb_id,rt_tables_entry_name(rtb_id));
		else{
			vty_out(vty,"The route policy %d doesn't create\n",rtb_id);
			return CMD_WARNING;
		}
	}
	else if(argc==0)
	{
	
		for(rtb_id = 1; rtb_id<=32;rtb_id++)
		{			
			if(rt_tables_entry_name(rtb_id))
				rp_ip_route_show(vty,rtb_id,rt_tables_entry_name(rtb_id));
		}
	}
	return CMD_SUCCESS;
}
ALIAS (show_ip_route_table,
       show_ip_route_table_cmd1,
       "show ip route policy",
       SHOW_STR
       IP_STR
       "IP routing table\n"
       "Route policy ID\n")


int dcli_route_policy_show(struct vty * vty)
{
	unsigned char rtb_id;

	rt_tables_initialize();	
	
	for(rtb_id = 1; rtb_id<=32;rtb_id++)
	{
		if(rt_tables_entry_name(rtb_id) )
		{
			rp_rule_show(vty,rtb_id,rt_tables_entry_name(rtb_id));
			vty_out(vty,"\n");
			rp_ip_route_show(vty,rtb_id,rt_tables_entry_name(rtb_id));
			vty_out(vty,"\n\n");
			
		}
		
	}

	return CMD_SUCCESS;
}
DEFUN (show_route_table_all,
       show_route_table_all_cmd,
       "show route policy all",
       SHOW_STR
       "IP routing table\n"
       "Route policy\n"
       "All\n")
{
	return dcli_route_policy_show(vty);

}


struct cmd_node route_policy_node =
{
	ROUTE_POLICY_NODE,
	"%s(route-policy)# ",
	1
};
int dcli_route_policy_write(struct vty * vty)
{
	char _tmpstr[256];
	unsigned char rtb_id;

	memset(_tmpstr,0,256);
	sprintf(_tmpstr,BUILDING_MOUDLE,"ROUTE POLICY");
	vtysh_add_show_string(_tmpstr);

	rt_tables_initialize();	
	
	for(rtb_id = 1; rtb_id<=32;rtb_id++)
	{			
		if(rt_tables_entry_name(rtb_id)){
			struct rp_ip_route_node * ip_rtrn,*ip_rtrn_tmp;
			struct rp_rule_node * rule_rtrn, *rule_rtrn_tmp;

			sprintf(_tmpstr,"route policy %d",rtb_id);
			vtysh_add_show_string(_tmpstr);

			rule_rtrn = rp_rule_get();
			while(rule_rtrn )
			{
				if(strcmp(rule_rtrn->tablename,rt_tables_entry_name(rtb_id)) == 0)
				{
					switch(rule_rtrn->type){
						case 1: /*32756:  from all iif eth1-9 lookup _rp100*/
							sprintf(_tmpstr," policy rule add interface %s",rule_rtrn->ifname);
							vtysh_add_show_string(_tmpstr);
							break;
						case 2: /*32765:  from 100.0.0.0/8 lookup _rp100*/	
							sprintf(_tmpstr," policy rule add from %s",rule_rtrn->prefix);
							vtysh_add_show_string(_tmpstr);
							break;
						case 3: /*32757:  from all to 220.0.0.0/24 lookup _rp100*/
							sprintf(_tmpstr," policy rule add to %s",rule_rtrn->prefix);
							vtysh_add_show_string(_tmpstr);
							break;
						case 4: /*32761:  from all tos 0x06 lookup _rp100 */
							sprintf(_tmpstr," policy rule add tos %d",rule_rtrn->tos);
							vtysh_add_show_string(_tmpstr);
							break;
						default:
							break;
						}
				}
				rule_rtrn_tmp = rule_rtrn;
				rule_rtrn=rule_rtrn->next;
				free(rule_rtrn_tmp);
			}
			g_rp_rule = NULL;
			

			ip_rtrn = rp_ip_route_get(rtb_id);
			while(ip_rtrn)
			{
				sprintf(_tmpstr," ip route %s %s",ip_rtrn->prefix,ip_rtrn->dst);
				vtysh_add_show_string(_tmpstr);
				
				ip_rtrn_tmp = ip_rtrn;
				ip_rtrn=ip_rtrn->next;
				free(ip_rtrn_tmp);				
			}
			g_rp_ip_route = NULL;
		

			vtysh_add_show_string(" exit");

		}
		
	}


	return CMD_SUCCESS;

}
void dcli_route_policy_init() {

	rtdsfield_initialize();
	rt_tables_initialize();

	
	install_node (&route_policy_node, dcli_route_policy_write, "ROUTE_POLICY_NODE");
	install_default(ROUTE_POLICY_NODE);	

	install_element(ENABLE_NODE, &show_route_policy_func_cmd);
	install_element(CONFIG_NODE, &show_route_policy_func_cmd);
	install_element(ENABLE_NODE, &show_route_policy_func_cmd1);
	install_element(CONFIG_NODE, &show_route_policy_func_cmd1);
	install_element(ENABLE_NODE, &show_route_table_all_cmd);
	install_element(CONFIG_NODE, &show_route_table_all_cmd);
	
	install_element(ENABLE_NODE, &show_ip_route_table_cmd);
	install_element(CONFIG_NODE, &show_ip_route_table_cmd);
	install_element(ENABLE_NODE, &show_ip_route_table_cmd1);
	install_element(CONFIG_NODE, &show_ip_route_table_cmd1);
	
	install_element(CONFIG_NODE, &set_route_policy_func_cmd);
	install_element(CONFIG_NODE, &del_route_policy_func_cmd);

	install_element(ROUTE_POLICY_NODE, &ip_policy_route_cmd);
	install_element(ROUTE_POLICY_NODE, &no_ip_policy_route_cmd);
	install_element(ROUTE_POLICY_NODE, &ip_policy_rule_cmd);
	install_element(ROUTE_POLICY_NODE, &ip_policy_rule_cmd2);
	install_element(ROUTE_POLICY_NODE, &ip_policy_rule_cmd1);

}
	
