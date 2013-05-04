#ifndef _IPFWD_LOG_PROC_H_
#define _IPFWD_LOG_PROC_H_

#define FASTFWD_LOG_MEM_NAME    "fastfwd_log_core%d"   
#define FASTFWD_LOG_PROC_NAME "fastfwd_log_proc"
#define FASTFWD_LOG_CTL_PROC_NAME "fastfwd_log_ctl_proc"

#define FWD_LOG_PAGE_SIZE   1024 /* 1KB */
#define FWD_LOG_MEM_SHOW_LEN (4 * 1024)

/* 
* total size = buf_num * buf_size * FWD_LOG_PAGE_SIZE
*/
typedef struct fwd_log_info_s
{
    uint32_t buf_size;
    uint8_t buf_num;    
    uint8_t rsvd0;
    uint8_t rsvd1;
}fwd_log_info_t;

#define MAX_CORE_NUM 32

int fwd_log_proc_init(void);
int fwd_log_proc_release(void);

#endif
