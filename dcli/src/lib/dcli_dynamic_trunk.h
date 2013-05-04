#ifndef __DCLI_DYNAMIC_TRUNK_H__
#define __DCLI_DYNAMIC_TRUNK_H__
void dcli_dynamic_trunk_init() ;
int dcli_dynamic_trunk_show_running_config
(
	struct vty * vty
	);
	
int dcli_dynamic_trunk_show_trunk_member_list
(
	struct vty * vty,
	unsigned short trunkId
);

char * dcli_dynamic_trunk_error_info(unsigned int op_ret);

typedef struct {
	unsigned char portNum;
	unsigned char devNum;
}DYNAMIC_TRUNK_MEMBER_STC;



#endif
