#ifdef _D_WCPSS_
#ifndef _DCLI_WTP_H
#define _DCLI_WTP_H

#define DCLI_GROUP 0
/* Huangleilei copy from 1.3.18, 20130610 */
#define AP_SNR_MAX (-10)		
#define AP_SNR_MIN (-120)

#define WTP_WTP "wtp config\n"
#define WTP_SET "set wtp configuration\n"
/* Huangleilei copy end */

typedef struct
{
    unsigned char       arEther[6];
}ETHERADDR;
void dcli_wtp_trace_script_exec
(
	char * ipAddr,
	unsigned int id,
	unsigned char flag
);

void dcli_wtp_init(void);

int dcli_wtp_parse_wtp_list(char* ptr,int* count,int wtpId[]);

int parse_wtpid_list(char* ptr,update_wtp_list **wtplist);
int get_dir_wild_file_count(char *dir, char *wildfile);

int parse_signedint_ID(char* str, int* ID);

#endif
#endif
