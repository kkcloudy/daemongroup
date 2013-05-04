#ifndef _FWD_DEBUG_H_
#define _FWD_DEBUG_H_

extern CVMX_SHARED  int fwd_debug_log_enable;
extern CVMX_SHARED  int fwd_debug_logtime_enable;


int fwd_debug_dump_packet(cvmx_wqe_t *work);
int fwd_debug_printf(const char * fmt,...);
void fwd_debug_init_global(void);
int fwd_debug_init_local(void);
int kes_fastfwd_print(const char * fmt,...);
int fwd_debug_agent_printf(const char * fmt,...);


/* fast-fwd group add for logsave */
#endif
