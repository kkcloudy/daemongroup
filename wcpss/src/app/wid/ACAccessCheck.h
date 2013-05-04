#ifndef AC_ACCESS_CHECK_H
#define AC_ACCESS_CHECK_H

struct wtp_access_info * ap_add(WID_ACCESS *AC, struct sockaddr_in * sa, CWWTPVendorInfos *valPtr, CWWTPDescriptor *valPtr1, char * name);
void del_all_ap(WID_ACCESS *AC);
extern CWThreadMutex ACAccessWTP;
struct wtp_access_info * get_ap(WID_ACCESS *AC, unsigned int ipip);
void free_ap(WID_ACCESS *AC, struct wtp_access_info *wtp);

#endif
