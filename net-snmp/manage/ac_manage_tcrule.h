#ifndef _AC_MANAGE_TCRULE_H_
#define _AC_MANAGE_TCRULE_H_

int manage_config_flow_control_service(unsigned int status);

int manage_add_tcrule(TCRule *tcRuleNew);

int manage_set_tcrule_offset(struct tcrule_offset_s *offset);

int manage_delete_tcrule(unsigned int index);

int manage_show_tcrule(TCRule **rule_array, unsigned int *count);

int manage_show_tcrule_offset(struct tcrule_offset_s **offset_array, unsigned int *count);

int manage_show_tcrule_running_config(struct running_config **configHead);

#endif
