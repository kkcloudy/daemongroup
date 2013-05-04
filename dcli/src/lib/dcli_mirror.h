#ifndef __DCLI_MIRROR_H__
#define __DCLI_MIRROR_H__

#define MIRROR_STR "Mirror Configuration\n"


/* mirror error message corresponding to error code */
extern unsigned char * dcli_mirror_err_msg[];

void dcli_mirror_init(void);
unsigned int  dcli_show_mirror
(
    struct vty* vty,
    unsigned int profile
);


#endif

