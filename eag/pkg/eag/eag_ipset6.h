

#ifndef _EAG_IPSET6_H
#define _EAG_IPSET6_H

#include <sys/types.h>
#include <netdb.h>


int add_user_to_ipset6(const int user_id, const int hansitype,
		const ip_set_ip_t user_ip);

int del_user_from_ipset6(const int user_id, const int hansitype,
        const ip_set_ip_t user_ip);

int add_preauth_user_to_ipset6(const int user_id, const int hansitype,
        const ip_set_ip_t user_ip);

int del_preauth_user_from_ipset6(const int user_id, const int hansitype,
        const ip_set_ip_t user_ip);

int eag_ipset6_init();

int eag_ipset6_exit();

#endif
