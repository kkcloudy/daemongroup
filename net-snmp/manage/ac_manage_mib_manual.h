#ifndef _AC_MANAGE_MIB_MANUAL_H_
#define _AC_MANAGE_MIB_MANUAL_H_

struct mib_manual_stats_s {
     
     struct mib_acif_stats *acifstats_head; 
     unsigned int acifstats_num;
};

int mib_manual_set_acif_stats(struct mib_acif_stats *acif_node);
int mib_show_manual_acif_stats(struct mib_acif_stats **acif_array, unsigned int *acif_num);
int mib_accumulate_acif_stats(struct if_stats_list **ifHead, unsigned int *if_num, unsigned int slot_id);


#endif
