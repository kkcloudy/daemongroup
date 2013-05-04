#ifdef _D_WCPSS_
#ifndef _ASD_AC_ROAMING_H_
#define _ASD_AC_ROAMING_H_

struct Dcli_Info_Inter_AC_R_Group_T{
	unsigned char GroupID;
	unsigned char WLANID;
	unsigned char *ESSID;
	unsigned char *name;
	unsigned int host_ip;
	Mobility_AC_Info_T *Mobility_AC[G_AC_NUM];
	struct Dcli_Info_Inter_AC_R_Group_T *next;
};


#endif
#endif

