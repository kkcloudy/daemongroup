#ifndef _AC_MANAGE_NTPSYSLOG_H_
#define _AC_MANAGE_NTPSYSLOG_H_

int manage_show_ntp_configure(struct clientz_st **rule_array,u_long *rule_num);
int ntp_config_pfm_table_entry(char *ifName, char *ipstr,unsigned int state);

void  init_ntp_config(void) ;
int manage_show_time(struct timez_st *rule_array) ;
int ntp_show_running_config(struct running_config **configHead, unsigned int type);
int manage_show_syslog_configure(struct syslogrule_st **rule_array,u_long *rule_num);
int syslog_config_pfm_table_entry(char *ifName, char *ipstr,unsigned int srcport,unsigned int state);
#endif

