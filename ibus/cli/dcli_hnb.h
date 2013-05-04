#ifndef _DCLI_HNB_H
#define _DCLI_HNB_H


int parse_int_ID(char* str,unsigned int* ID);
void dcli_hnb_init(void);
typedef enum{
    WRONG_LEN = 0,
    WRONG_CHARACTER = 1,
    CORRECT_FORMAT = 2
}IMSICheckedVal;


#endif

