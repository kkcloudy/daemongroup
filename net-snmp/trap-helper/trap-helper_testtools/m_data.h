#ifndef _M_DATA_H_
#define _M_DATA_H_

#define RCV_IP "192.168.7.192"
#define RCV_PORT 162
#define TYPE 8

#define GET_GSIGLIST_MEM_ERR NULL


typedef struct {
	int num;
	char * signal_name;
	int func_param;
	char *trap_oid;
	int flag;
}SIGLIST;

typedef int (* TrapSignalSendFunc)( const char *signal_name, int first_arg_type,...);

inline SIGLIST *get_gsiglist_member(int signal_number, int operator);


#endif
