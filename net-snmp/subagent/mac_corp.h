#ifndef _MAC_CORP_H
#define _MAC_CORP_H


int init_mac_corp_data();

/*输入的mac格式是：16进制大写，以'-'分割*/
int get_corp_by_mac( char *mac, char *corp, int buff_len );

int destroy_mac_corp_data();


/*如果只需要得到一个mac的corp，可以用这个宏，如果想连续得到多个，用上面的函数效率更高！*/
#define get_corp_by_mac_ext( mac, corp, len ) \
		if( init_mac_corp_data() == 0 ) {\
			get_corp_by_mac(mac,corp,len);\
			destroy_mac_corp_data();\
		}


#endif
