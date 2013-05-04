#include <stdio.h>
#include <time.h>
#include <syslog.h>
#include <stdarg.h>
#include "trap-util.h"
#include "trap-def.h"
#include "nm_list.h"
#include "trap-list.h"
#include "hashtable.h"
#include "trap-descr.h"
#include "trap-data.h"
#include "trap-hash.h"



void trap_init_hashtable(hashtable **ht, 
					unsigned int hash_num,
			unsigned int (*hash_value_count_func)(const char *buff, unsigned int len,  unsigned int hash_size))
{

	TRAP_TRACE_LOG ( LOG_DEBUG, "entry.\n" );
	
	TRAP_RETURN_IF_FAILED ( TRAP_OK == hashtable_create_table ( ht, hash_num ), LOG_INFO );
	
	if ( NULL == hash_value_count_func )
		return;
	
	( *ht )->hash_value_count_func = hash_value_count_func;

	return;
}

