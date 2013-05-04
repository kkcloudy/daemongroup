
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"

#include "pppoe_log.h"


#define LOG_MASK_EMERG			(LOG_MASK(LOG_EMERG))
#define LOG_MASK_ALERT			(LOG_MASK(LOG_ALERT))
#define LOG_MASK_CRIT			(LOG_MASK(LOG_CRIT))
#define LOG_MASK_ERR			(LOG_MASK(LOG_ERR))
#define LOG_MASK_WARNING		(LOG_MASK(LOG_WARNING))
#define LOG_MASK_NOTICE			(LOG_MASK(LOG_NOTICE))
#define LOG_MASK_INFO			(LOG_MASK(LOG_INFO))
#define LOG_MASK_DEBUG			(LOG_MASK(LOG_DEBUG))

#define LOG_ON_ALERT			(LOG_MASK_EMERG 	| LOG_MASK_ALERT)
#define LOG_ON_CRIT				(LOG_ON_ALERT 		| LOG_MASK_CRIT)
#define LOG_ON_ERR				(LOG_ON_CRIT 		| LOG_MASK_ERR)
#define LOG_ON_WARNING			(LOG_ON_ERR 		| LOG_MASK_WARNING)
#define LOG_ON_NOTICE			(LOG_ON_WARNING 	| LOG_MASK_NOTICE)
#define LOG_ON_INFO				(LOG_ON_NOTICE 		| LOG_MASK_INFO)
#define LOG_ON_DEBUG			(LOG_ON_INFO 		| LOG_MASK_DEBUG)


#define	LOG_TOKEN_UNREGISTER	0
#define	LOG_TOKEN_REGISTER		1


struct token_info {
	int state;
	char desc[32];
} pppoe_tokens[TOKEN_NUMS] = {
	{LOG_TOKEN_UNREGISTER, "pppoe instance"},
	{LOG_TOKEN_UNREGISTER, "pppoe backup"},		
	{LOG_TOKEN_UNREGISTER, "pppoe netlink"},
	{LOG_TOKEN_UNREGISTER, "pppoe thread"},
	{LOG_TOKEN_UNREGISTER, "pppoe tbus"},
	{LOG_TOKEN_UNREGISTER, "pppoe method"},
	{LOG_TOKEN_UNREGISTER, "pppoe manage"},
	{LOG_TOKEN_UNREGISTER, "pppoe radius"},
	{LOG_TOKEN_UNREGISTER, "pppoe discover"},
	{LOG_TOKEN_UNREGISTER, "pppoe control"},
	{LOG_TOKEN_UNREGISTER, "pppoe LCP"},
	{LOG_TOKEN_UNREGISTER, "pppoe CHAP"},
	{LOG_TOKEN_UNREGISTER, "pppoe CCP"},
	{LOG_TOKEN_UNREGISTER, "pppoe IPCP"},
	{LOG_TOKEN_UNREGISTER, "pppoe RDC"},
};

static unsigned int	pppoe_debug;
static unsigned int tokens_num = sizeof(pppoe_tokens) / sizeof(pppoe_tokens[0]);


int
pppoe_token_is_registered(PPPOELogToken token) {
	if (unlikely(PPPOE_LOG_DEBUG_ON == pppoe_debug 
		&& token < TOKEN_NUMS)) {
		return LOG_TOKEN_REGISTER == pppoe_tokens[token].state;
	}
	return 0;
}

char *
pppoe_token_desc(PPPOELogToken token) {
	if (likely(token < TOKEN_NUMS)) {
		return pppoe_tokens[token].desc;
	}
	return NULL;
}

int
pppoe_token_register(PPPOELogToken token) {
	if (likely(token < tokens_num)) {
		pppoe_tokens[token].state = LOG_TOKEN_REGISTER;
		return PPPOEERR_SUCCESS;
	}

	return PPPOEERR_EINVAL;
}

int
pppoe_token_unregister(PPPOELogToken token) {
	if (likely(token < tokens_num)) {
		pppoe_tokens[token].state = LOG_TOKEN_UNREGISTER;
		return PPPOEERR_SUCCESS;
	}

	return PPPOEERR_EINVAL;
}

void
pppoe_debug_config(unsigned int debug) {
	if (debug) {
		setlogmask(LOG_ON_DEBUG);
		pppoe_debug = PPPOE_LOG_DEBUG_ON;
	} else{
		setlogmask(LOG_ON_INFO);
		pppoe_debug = PPPOE_LOG_DEBUG_OFF;
	}
}


void
pppoe_log_init(char *daemon) {
	if (!daemon)
		return ;

	openlog(daemon, LOG_PID, LOG_DAEMON);
	pppoe_debug_config(PPPOE_LOG_DEBUG_OFF);
};

void
pppoe_log_exit(void) {
	closelog();
}

