#ifndef __ASD_CONFIG__H__
#define __ASD_CONFIG__H__
#include "cert_info.h"

struct ap_config
{
	
	char cert_name[256];
	char ca_cert_name[256]; //ÈýÖ¤Êé
	unsigned short used_cert;
	unsigned short pad;
};
struct _apcert_info {
	struct cert_obj_st_t *ap_cert_obj;
	struct ap_config config;
};
void unregister_certificate();
#endif
