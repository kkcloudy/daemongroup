#include "pppoe_snooping.h"
#include "netlink.h"
#include "log.h"


int pppoe_snp_log_level = PPPOE_SNP_LOG_DEFAULT;

/*
static int pppoe_snp_log(int loglevel, char *format, ...)
{
	va_list list;
	char pformat[1024];
	if(likely(!(pppoe_snp_log_level & loglevel))) {
		return 0;
	}

	memset(pformat, 0, sizeof(pformat));

	va_start(list, format);
	switch(loglevel) {
		case PPPOE_SNP_LOG_DEBUG:
			strcpy(pformat, KERN_DEBUG);
			break;
		
		case PPPOE_SNP_LOG_INFO:
			strcpy(pformat, KERN_INFO);
			break;
		case PPPOE_SNP_LOG_ERROR:
			strcpy(pformat, KERN_ERR);
			break;
		default:
			break;
	}
	sprintf(pformat, "%s pppoe_snp %s", pformat, format);
	vprintk(pformat, list);
	va_end(list);

	return 0;
}
*/


int log_debug(const char *format, ...)
{
	va_list list;
	char pformat[1024];
	if(likely(!(pppoe_snp_log_level & PPPOE_SNP_LOG_DEBUG))) {
		return 0;
	}

	va_start(list, format);

	memset(pformat, 0, sizeof(pformat));
	strcpy(pformat, KERN_DEBUG);

	sprintf(pformat, "%s<pppoe_snp>%s", pformat, format);
	vprintk(pformat, list);
	va_end(list);

	return 0;
}

int log_info(const char *format, ...)
{
	va_list list;
	char pformat[1024] = {0};

	if(likely(!(pppoe_snp_log_level & PPPOE_SNP_LOG_INFO))) {
		return 0;
	}

	va_start(list, format);

	strcpy(pformat, KERN_INFO);

	sprintf(pformat, "%s<pppoe_snp>%s", pformat, format);
	vprintk(pformat, list);
	va_end(list);

	return 0;
}

int log_error(const char *format, ...)
{
	va_list list;
	char pformat[1024] = {0};
	
	if(likely(!(pppoe_snp_log_level & PPPOE_SNP_LOG_ERROR))) {
		return 0;
	}

	va_start(list, format);

	strcpy(pformat, KERN_ERR);

	sprintf(pformat, "%s<pppoe_snp>%s", pformat, format);
	vprintk(pformat, list);
	va_end(list);

	return 0;
}
