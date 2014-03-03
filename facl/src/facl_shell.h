#ifndef _FACL_SHELL_H
#define _FACL_SHELL_H

#include <stdint.h>

int
facl_shell_add_policy(uint32_t facl_tag);

int
facl_shell_del_policy(uint32_t facl_tag);

int
facl_shell_add_rule(facl_rule_t *rule);

int
facl_shell_del_rule(facl_rule_t *rule);

#endif
