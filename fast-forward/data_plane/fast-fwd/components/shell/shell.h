
#ifndef _SHELL_H
#define _SHELL_H


#define CMD_NAME_SIZE 32
#define MAX_ARGV_ARGS 64
#define MAX_CMDS 100
#define CMD_HELP 64
#define CMD_USAGE 128


#define CMD_HIDE_VIEW  0
#define CMD_DISPLAY_VIEW  1

struct cmd {
	char name[CMD_NAME_SIZE];
	int (*func)(int, char *[]);		// call back fuction pointer  
	char help[CMD_HELP];
	char usage[CMD_USAGE];
	unsigned char status; 
};

#define 	DEFAULT_PROMPT  "Shell$ "

#define CMD_EXEC_SUCCESS  0		
#define CMD_EXEC_ERROR     -1 		
#define CMD_ERR_TOO_FEW_ARGC     -2 
#define CMD_ERR_TOO_MANY_ARGC       -3
#define CMD_ERR_NOT_MATCH               -4

#define STRING_MAX_LEN  2048

/* for print ip */
#define IP_FMT(m)	\
				((uint8_t*)&(m))[0], \
				((uint8_t*)&(m))[1], \
				((uint8_t*)&(m))[2], \
				((uint8_t*)&(m))[3]
				
/* for print mac */
#define MAC_FMT(m)  \
				((uint8_t*)(m))[0], \
				((uint8_t*)(m))[1], \
				((uint8_t*)(m))[2], \
				((uint8_t*)(m))[3], \
				((uint8_t*)(m))[4], \
				((uint8_t*)(m))[5]

/* for print protocol */
#define PROTO_STR(t)  ((t) == 0x6 ? "TCP" : ((t) == 0x11 ? "UDP" : ((t) == 0x1 ? "ICMP" : "Unknown")))


extern void 	shell_run(void);
extern void 	print_prompt(void);
int register_shell_cmd(const char *name, int (*func)(int, char *[]), const char *help, const char *usage, unsigned char status);
extern int 	unregister_shell_cmd(const char *name);
extern void 	shell_init(int uart_i);
extern int execute_cmd(char *buf,int len);
void display_banner(void);
extern void shell_common_register(void);
void switch_uart_to_linux(); /*add by zhaohan*/
void show_ciu_stat();

#endif

