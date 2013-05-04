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
 *
 *
 * CREATOR:
 * autelan.software.xxx. team
 *
 * DESCRIPTION:
 * xxx module main routine
 *
 *
 *******************************************************************************/

#ifndef _DRP_OPXML_H
#define _DRP_OPXML_H


#define XML_DOMAIN_D                        "/opt/www/htdocs/dns_cache.xml"
#define XML_DOMAIN_PATH	   	      "/opt/services/option/domain_option"
#define DNS_RESOLVE_DOMAIN_TO_XML "sudo /usr/bin/dns_resolve_xml.sh %s > /dev/null 2>&1"

struct  domain_st
{
	char  *domain_ip;
	char  *domain_name;
	int  ip_index;
	struct domain_st  *next;
};

/*function declaration*/
void free_read_domain_xml(struct domain_st *head,int *free_flag);
int check_and_get_domainname_xml(char * filepath,struct domain_st *chead,
		char * second_node_name,char *second_node_attribute,int *domain_num);
int get_domainname_ip_xml(char * filepath,struct domain_st *chead,
		char * second_node_name,int *domain_num, int *ip_count);
int add_domain_second_node_attr(char *fpath,char * node_name,char *attribute,char *ruler);
int add_domain_third_node(char * fpath,char * node_name,char *attribute,char *ruler,char * third_node_name,char *third_node_content);
int delete_domain_second_xmlnode(char *fpath,char *node_name,char *attribute,char *ruler);
int delete_domain_third_xmlnodel(char *fpath,char *node_name,char *attribute,char *ruler,char *secnode);
int  add_domain_ip_into_cache_xml(const char *domain);


#endif

