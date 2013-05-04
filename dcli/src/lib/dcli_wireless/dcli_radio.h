#ifdef _D_WCPSS_
#ifndef _DCLI_RADIO_H
#define _DCLI_RADIO_H

#define RATE_SPLIT_COMMA 	','
#define RADIO_IF_NAME_LEN 20
#define RADIO_RATE_LIST_LEN 20
#define DCLI_GROUP 1


typedef enum{
	check_rate=0,
	check_comma,
	check_fail,
	check_end,
	check_success
}rate_list_state;

/*struct RadioList{
	int RadioId;
	char FailReason;
	struct RadioList *next;
	struct RadioList *RadioList_list;
	struct RadioList *RadioList_last;
};*/
/*struct RadioRate_list{
	int RadioId;
	int list[RADIO_RATE_LIST_LEN];
	struct RadioRate_list *next;
};*/
typedef enum{
	check_wtpid=0,
	check_sub,
	check_fail_,
	check_end_,
	check_success_,
	check_radioid
}radio_id_state;

#define PARSE_RADIO_IFNAME_SUB_ '-'


void dcli_radio_init(void);
int parse_char_ID(char* str,unsigned char* ID);
int parse_int_ID(char* str,unsigned int* ID);
int parse_short_ID(char* str,unsigned short* ID);

void CheckWIDIfPolicy(char *whichinterface, unsigned char wlan_if_policy);

int parse_radio_id(char* ptr,int *wtpid,int *radioid);

#endif
#endif
