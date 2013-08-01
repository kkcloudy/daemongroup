#ifndef _AC_MANAGE_NTPSYSLOG_INTERFACE_H_
#define _AC_MANAGE_NTPSYSLOG_INTERFACE_H_

int ac_manage_show_ntp_rule(DBusConnection *connection, u_long *service_status, struct clientz_st **rule_array, u_long *rule_num);
int ac_manage_add_ntpserver_rule(DBusConnection *connection, struct clientz_st *rule,int config_type);
int ac_manage_inside_ntp_rule(DBusConnection *connection);
int ac_manage_set_ntpstatus_rule(DBusConnection *connection,char *status) ;
int ac_manage_clean_ntp_rule(DBusConnection *connection) ;
int ac_manage_set_timezone(DBusConnection *connection,char *area,char *city);
int ac_manage_add_ntpclient_rule(DBusConnection *connection, struct serverz_st *rule,int opt_type);
int ac_manage_show_ntpclient_rule(DBusConnection *connection,struct serverz_st *head,int *servnum);
int ac_manage_show_ntpupserver_rule(DBusConnection *connection,struct clientz_st *head,int *servnum) ;
int ac_manage_show_time(DBusConnection *connection, struct timez_st *rule_array);
int ac_manage_set_time(DBusConnection *connection,char *timestr);
int ac_manage_add_syslog_rule(DBusConnection *connection, struct syslogrule_st *rule,int opt_type);
int ac_manage_set_syslogstatus_rule(DBusConnection *connection,char *status,char *opt_type);
int ac_manage_show_syslog_rule(DBusConnection *connection, u_long *service_status, struct syslogrule_st **rule_array, u_long *rule_num) ;
int ac_manage_save_syslog_rule(DBusConnection *connection,char *status,char *opt_type);
#endif

