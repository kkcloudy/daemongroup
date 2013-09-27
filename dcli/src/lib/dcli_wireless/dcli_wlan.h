#ifdef _D_WCPSS_
#ifndef _DCLI_WLAN_H
#define _DCLI_WLAN_H

#define WLAN_IF_NAME_LEN 20

void dcli_wlan_init(void);
int parse_char_ID(char* str,unsigned char* ID);
void CheckWIDIfPolicy(char *whichinterface, unsigned char wlan_if_policy);
int ssid_illegal_character_check(char *str, int len);

#endif
#endif
