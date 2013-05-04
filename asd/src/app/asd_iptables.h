#ifndef   ASD_IPTABLES_H
#define  ASD_IPTABLES_H

#define  ASD_IPTABLES_RETURN_CODE_ERROR   0
#define  ASD_IPTABLES_RETURN_CODE_OK          1
#define  ASD_IPTABLES_RETURN_CODE_NOT_FOUND   2

#define ASD_IPTABLES_SOURCE             3
#define ASD_IPTABLES_DESTINATION    4

#define  ASD_IPTABLES_ADD       1
#define  ASD_IPTABLES_DELETE  2
int  asd_check_is_chain(const char * table_name,const char * chain_name);
int asd_get_num_of_entry(	const char * table_name,const char * chain_name,const unsigned int ip_addr,const int type,int * num_of_entry);
int  asd_add_and_del_entry	(const char *table_name,const char *chain_name,
							const int source_ip,const int dest_ip,
							const char *target_name,const int type);
int asd_connect_up(const unsigned int user_ip);
int asd_connect_down(const unsigned int user_ip);
char * asd_inet_int2str( unsigned int ip_addr, char *ip_str, unsigned int buffsize );



#endif
