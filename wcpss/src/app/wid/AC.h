#ifndef AC_AC_H
#define AC_AC_H


void LISTEN_IF_INIT();
int get_dir_wild_file_count(char *dir, char *wildfile);
int wid_illegal_character_check(char *str , int len, int modify);
#define BIT(x) (1 << (x))
CWBool AsdWsm_WTPOp(unsigned int WtpID,Operate op);
CWBool AsdWsm_bytes_info(unsigned int WTPID, Operate op);
int WIDBAKInfoToASD(unsigned int state,struct sockaddr_in *ipaddr, unsigned int virIP,unsigned int vrrid,char *name, Operate op);
int WIDLocalBAKInfoToASD(unsigned int state,int slotID,unsigned int vrrid, Operate op);
CWBool Asd_neighbor_ap_sta_check_op(unsigned int WtpID,unsigned int N_WtpID,unsigned char mac[6],Operate op);
int wid_del_wtp(struct wtp_con_info * con_info);
CWBool AsdWsm_BssMacOp(unsigned int BSSIndex, Operate op);
struct conflict_wtp_info* wid_add_wtp(struct wtp_con_info * con_info);
struct conflict_wtp_info * wid_del_conflict_wtpinfo(struct wtp_con_info * con_info);
void CWResetWTPProtocolManager(CWWTPProtocolManager *WTPProtocolManager);
int WIDBAKBSSInfoToASD(unsigned int vrrid,unsigned int *bssindex,unsigned int count,Operate op);
int file_check(char *dir);
extern unixAddr toASD_STA;
int wid_notice_asd_switch(unsigned int wtpid,unsigned int policy);

#define WID_CHECK_WTP_STANDARD_RET(_wtpid_, ret)	\
	do {	\
		if (((_wtpid_) >= WTP_NUM) || ((_wtpid_) <= 0) || (NULL == AC_WTP[(_wtpid_)]))	\
		{	\
			wid_syslog_err("%s:%d wtp%d not exist\n", __func__, __LINE__, (_wtpid_));	\
			return (ret);	\
		}	\
	} while (0);
	

#define WID_MALLOC_ERR()	\
	do {	\
			wid_syslog_err("%s:%d malloc failed: %s\n", __func__, __LINE__);	\
	} while (0);


#define WID_CHECK_POINTER_RET(ptr, ret)	\
	do {	\
		if (NULL == (ptr))	\
		{	\
			wid_syslog_err("%s:%d parameter NULL\n", __func__, __LINE__); \
			return (ret); \
		}	\
	} while (0);

	

#endif
