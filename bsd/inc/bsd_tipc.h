#ifndef _BSD_TIPC_H_
#define _BSD_TPIC_H_

#include <linux/tipc.h>
//#include <dbus/dbus.h>

#define BSD_TIPC_SERVER_TYPE (0x6000)
#define BSD_SERVER_BASE_INST (128)

#define TIPC_BSD_TYPE_LOW	(0x6000)
#define TIPC_BSD_TYPE_HIGH	(0x6fff)

#define BSD_TIPC_INSTANCE_BASE	(0x10000)
#define	BSD_TIPC_VERSION_SYN_INSTANCE	(SEM_TIPC_INSTANCE_BASE + 0x20)

struct Bsd_Board_Info{
	int slot_no;
	int tipcfd;
	int state;
	struct sockaddr_tipc tipcaddr;
};

void BSDTipcInit();
void BSDClientTipcInit();
void BSDClientGroupTipcInit();
int BSDServerTipcInit(unsigned int slotid);
int BSDSendWTP(unsigned const int slotid);
int BSDSendACVersion(unsigned const int slotid);
int BSDAddMessageQueue(const unsigned int slotid, const char *src_path, const char *des_path, const int op);
int BSDCopyFile(const unsigned int slotid, const char *source_dir, const char *target_dir, int op);
int BSDSendMemeryCheckRequest(const unsigned int slotid, const char *src_path, const char *des_path, unsigned short event_id);
int BSDSendFile(const unsigned int slotid, const char *src_path, const char *des_path, int op, char *md5Result);
int BSDSendDir(const unsigned int slotid, const char *source_dir, const char *target_dir, const int op);
int BSDReceiveFile(const char *recvbuf, unsigned int recv_len, unsigned int iflag, FILE **fp);
int BSDSynchronizeFilesToBoard(int syn_type, int slot_id);
int BSDSynchronizeFiles(int syn_type);
int BSDSynchronizeFilesV2(const char *src_path, const char *des_path, const int op);
int BSDSendNotify(int slot_no, int stat, unsigned short event_id, char * md5);
int BSDGetAliveSlotIDs(int *ID, int op);
int BSDMemeryCheck(const unsigned int slotid, const char *src_path, const char *des_path, unsigned short event_id);
int BSDSendDesPathCheckRequest(const unsigned int slotid, const char *des_path, unsigned short event_id);
int BSDDesPathCheck(const unsigned int slotid, const char *des_path, unsigned short event_id);
int BSDCopyFilesToBoards(const unsigned int iSlotId, const char *pSrcPath, const char *pDesPath, const int iOption, const int iCompressFlag);
int BSDHandleFileOp(const int slotid, const char *src_path,  const char *des_path, const int op, const int eventId);
int BSDCalculateMd5(const char *src_path, char *resultStr);

#endif
