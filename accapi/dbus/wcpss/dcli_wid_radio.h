#ifndef _WID_DCLI_RADIO_H
#define _WID_DCLI_RADIO_H

#define RADIO_RATE_LIST_LEN 20

struct RadioList{
	int RadioId;
	char FailReason;

	struct RadioList *next;
	struct RadioList *RadioList_list;
	struct RadioList *RadioList_last;
};

struct RadioRate_list{
	int RadioId;
	int list[RADIO_RATE_LIST_LEN];
	struct RadioRate_list *next;
};


/* book add for 11n rate paras, 2010-10-21 */
struct dcli_n_rate_info{
	int rate;
	int stream_num;
	int mcs;
	int cwmode;
	int guard_interval;
};





























#endif
