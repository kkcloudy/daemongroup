#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>

#include <unistd.h>
#include <sys/types.h>

#include "manage_log.h"


#define	LOG_TOKEN_UNREGISTER		(0)
#define	LOG_TOKEN_REGISTER		(1)

static u_int	manage_debug = MANAGE_LOG_DEBUG_OFF;
static u_char	manage_debug_tokens[MANAGE_MAX_DEBUG_TOKENS] = { 0 };

static inline u_char
debug_is_token_registered(u_int token) {
 	if(MANAGE_LOG_DEBUG_ON != manage_debug || token >= MANAGE_MAX_DEBUG_TOKENS) {
		return 0;
	}
	
	return manage_debug_tokens[token];
}

static inline void
manage_vlog(int priority, const char *message, va_list args) {
	vsyslog(priority, message, args);

	return ;
}

void
manage_log(u_int priority, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	manage_vlog(priority, format, ap);
	va_end(ap);

	return ;
}

void
manage_token_log(u_int token, const char *format, ...) {
	if(debug_is_token_registered(token)) {		
		va_list ap;
		va_start(ap, format);
		manage_vlog(LOG_DEBUG, format, ap);
		va_end(ap);
	}

	return ;
}

int
manage_token_register(u_int token) {
 	if(token >= MANAGE_MAX_DEBUG_TOKENS) {
		return -1;
	}

	manage_debug_tokens[token] = (u_char)LOG_TOKEN_REGISTER;
	return 0;
}

int
manage_token_unregister(u_int token) {
 	if(token >= MANAGE_MAX_DEBUG_TOKENS) {
		return -1;
	}

	manage_debug_tokens[token] = (u_char)LOG_TOKEN_UNREGISTER;
	return 0;
}

void
manage_debug_config(u_int debug) {
	if(debug) {
		manage_debug = MANAGE_LOG_DEBUG_ON;
	} else{
		manage_debug = MANAGE_LOG_DEBUG_OFF;
	}

	return ;
}

