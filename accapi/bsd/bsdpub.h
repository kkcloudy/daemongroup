#ifndef _BSD_PUB_H
#define	_BSD_PUB_H


#include <linux/tipc.h>
#include <dbus/dbus.h>


#define MAX_IFNAME_LEN (16)
#define MAX_IFNAME_NUM (16)
#define MAX_INSTANCE (16+1)
#define MAX_SLOT_NUM (16)
#define PATH_LEN    (128)
#define NAME_LEN    (96)
#define BSD_COMMAND_BUF_LEN (256)
#define DEFAULT_LEN	(256)
#define CFDISK_MIN_MEM (1000)
#define SYSTEM_MIN_MEM (200000)


extern int BSDMsgqID;


enum bsdDbusReturnValue {
    BSD_SUCCESS = 0,
    BSD_GET_SLOT_INFORMATION_ERROR= 1,
    BSD_INIT_SOCKET_ERROR = 2,
    BSD_ESTABLISH_CONNECTION_FAILED = 3,
    BSD_ILLEGAL_SOURCE_FILE_PATH= 4,
    BSD_ILLEGAL_DESTINATION_FILE_PATH = 5,
    BSD_GET_FILE_SIZE_ERROR = 6,
    BSD_NOT_ENOUGH_MEMERY = 7,
    BSD_SEND_MESSAGE_ERROR = 8,
    BSD_RECEIVE_MESSAGE_ERROR = 9,
    BSD_PEER_SAVE_FILE_ERROR = 10,
    BSD_EVENTID_NOT_MATCH = 11,
    BSD_SERVER_NOT_CATCH = 12,
    BSD_MALLOC_MEMERY_ERROR = 13,
    BSD_MD5_ERROR = 14,
    BSD_WAIT_THREAD_CONDITION_TIMEOUT = 15,
    BSD_ADD_TO_MESSAGE_QUEUE_ERROR = 16,
    BSD_UNKNOWN_ERROR = 17,
    BSD_MD5_ERROR_THIRD = 18
};

/* boo modify , 2012-8-3 */
#if 0
typedef enum bsd_file_type{
    BSD_TYPE_NORMAL = 0,
    BSD_TYPE_BOOT_IMG = 1,
    BSD_TYPE_SINGLE = 2,
    BSD_TYPE_PROC = 3
}bsd_file_type_t;
#else
typedef enum bsd_file_type {
    BSD_TYPE_NORMAL = 0,
    BSD_TYPE_BOOT_IMG = 1,	  	//ac img file
    BSD_TYPE_WTP = 2,			//ap img or configuration files
    BSD_TYPE_PATCH = 3,			//patch
    BSD_TYPE_CORE = 4,			//core file in /proc
	BSD_TYPE_FOLDER = 5,		//normal folder
	BSD_TYPE_SINGLE = 6,		//normal single file
	BSD_TYPE_CMD = 7,			//command
	BSD_TYPE_BLK = 8,
	BSD_TYPE_COMPRESS = 9,
	BSD_TYPE_WTP_FOLDER = 10
}bsd_file_type_t;
#endif


struct BsdMsg{
    bsd_file_type_t op;
    unsigned int tar_slot_id;
    char src_path[PATH_LEN];
    char tar_path[PATH_LEN];
};


struct BsdMsgQ{
	long mqid;
	struct BsdMsg mqinfo;
};


enum BsdSynType{
    BSD_SYNC_AC_VERSION = 1,
    BSD_SYNC_WTP = 2
};


extern struct Bsd_Board_Info *HOST_BOARD;
extern struct Bsd_Board_Info *BSD_BOARD[MAX_SLOT_NUM];


#endif

