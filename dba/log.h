#ifndef __PPPOE_SNP_KERNLOG_H__
#define __PPPOE_SNP_KERNLOG_H__


#define PPPOE_SNP_LOG_DEBUG			0x01
#define PPPOE_SNP_LOG_INFO			0x02
#define PPPOE_SNP_LOG_ERROR			0x04	
#define PPPOE_SNP_LOG_DEFAULT		0x00
#define PPPOE_SNP_LOG_ALL			(PPPOE_SNP_LOG_DEBUG | PPPOE_SNP_LOG_INFO | PPPOE_SNP_LOG_ERROR)


int log_debug(const char *format, ...);
int log_info(const char *format, ...);
int log_error(const char *format, ...);

#endif
