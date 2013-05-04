#ifdef _D_WCPSS_
#ifndef _DCLI_QOS_H_
#define _DCLI_QOS_H_
void dcli_wqos_init();


int parse_int_ID(char* str,unsigned int* ID);
int parse_char_ID(char * str,unsigned char * ID);

int parse_dot1p_list(char* ptr,unsigned char * count,unsigned char dot1plist[]);
int parse_radio_ifname(char * ptr,int * wtpid,int * radioid,int * wlanid);
int wid_interface_ifname_wlan(char * ptr,struct vty * vty,char *line);
int wid_interface_ifname_radio(char * ptr,struct vty * vty,char *line);
int wid_no_interface_ifname_wlan(char * ptr,struct vty * vty);
int wid_no_interface_ifname_radio(char * ptr,struct vty * vty);
int wid_interface_ifname_ebr(char * ptr,struct vty * vty,char *line);
int wid_no_interface_ifname_ebr(char *ptr,struct vty *vty);
int dcli_forward_mode_config
(
    struct vty * vty,
    char * ifName,
    unsigned int ismode
);

int dcli_tunnel_mode_config
(
    struct vty * vty,
    char * ifName,
    unsigned int ismode,
    unsigned int modeflag,
    char nodeFlag,
    unsigned int wlanID
);

#endif
#endif
