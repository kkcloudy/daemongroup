#ifndef _BSD_MANAGEMENT_H_
#define _BSD_MANAGEMENT_H_

int BSDGetFileName(const char *file_path, char file_name[NAME_LEN]);
int BSDGetFileDir(const char *file_path, char file_dir[PATH_LEN]);
void * bsdTipcManagement();
void * bsdTcpManagement();


#endif
