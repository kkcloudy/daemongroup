#ifndef _MAPI_HNB_H
#define _MAPI_HNB_H

struct ue_info{
    int ue_id;
    int hnb_id;
    int ue_state;
    int reg_cause;
};

typedef struct ue_list{
    unsigned int uecount;
    struct ue_info **UE_LIST;
}UELIST;

struct hnb_info{
    int hnb_id;
    char *hnb_name;
    char *hnb_ip;
    int hnb_state;
};

typedef struct hnb_list{
    unsigned int hnbcount;
    struct hnb_info **HNB_LIST;
}HNBLIST;
#endif
