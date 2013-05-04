#ifndef _IUH_LICENSE_H
#define _IUH_LICENSE_H

typedef struct{
	unsigned int gmax_hnb_count;
	unsigned int gcurrent_hnb_count;
	unsigned int flag;
}LICENSE_TYPE;
extern LICENSE_TYPE **g_hnb_count;

int parse_int_ID(char* str,unsigned int* ID);
int read_ac_info(char *FILENAME,char *buff);
int get_dir_wild_file_count(char *dir, char *wildfile);
void Iuh_License_Init();
#endif
