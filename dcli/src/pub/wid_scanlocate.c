#ifdef _D_WCPSS_
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "wcpss/asd/asd.h"
#include "wid_ac.h"
#include "dbus/wcpss/dcli_wid_wtp.h"

#define BIT(x) (1 << (x))


int strtoint(int index, int lastindex, unsigned char *dvlanlist)
{
	int sum = 0;
	switch(index - lastindex)
	{
		case 2:
			sum = dvlanlist[index-1]-'0';
			break;
		case 3:
			sum = 10*(dvlanlist[index-2]-'0') + (dvlanlist[index-1]-'0');
			break;
		case 4:
			sum = 100*(dvlanlist[index-3]-'0') + 10*(dvlanlist[index-2]-'0') + (dvlanlist[index-1]-'0');
			break;
		case 5:
			sum = 1000*(dvlanlist[index-4]-'0') + 100*(dvlanlist[index-3]-'0') + 10*(dvlanlist[index-2]-'0') + (dvlanlist[index-1]-'0');
			break;
		default:
			printf("%s:  in default branch, error!\n", __func__);
			break;
	}
	return sum;
}

int check_5G_channel(unsigned int number)
{
	printf(" number: %d\n",number);
	//channel_5G[channel] = channel_id 
	unsigned char channel_5G[200] = {0};
	channel_5G[7] = 15; 
	channel_5G[8] = 16; 
	channel_5G[9] = 17; 
	channel_5G[11] = 18; 
	channel_5G[12] = 19; 
	channel_5G[16] = 20; 
	channel_5G[34] = 21; 
	channel_5G[36] = 22; 
	channel_5G[38] = 23; 
	channel_5G[40] = 24; 
	channel_5G[42] = 25; 
	channel_5G[44] = 26; 
	channel_5G[46] = 27; 
	channel_5G[48] = 28; 
	channel_5G[52] = 29; 
	channel_5G[56] = 30; 
	channel_5G[60] = 31; 
	channel_5G[64] = 32; 
	channel_5G[100] = 33; 
	channel_5G[104] = 34; 
	channel_5G[108] = 35; 
	channel_5G[112] = 36; 
	channel_5G[116] = 37; 
	channel_5G[120] = 38; 
	channel_5G[124] = 39; 
	channel_5G[128] = 40; 
	channel_5G[132] = 41; 
	channel_5G[136] = 42; 
	channel_5G[140] = 43; 
	channel_5G[149] = 44; 
	channel_5G[153] = 45; 
	channel_5G[157] = 46; 
	channel_5G[161] = 47; 
	channel_5G[165] = 48; 
	channel_5G[183] = 49; 
	channel_5G[184] = 50; 
	channel_5G[185] = 51; 
	channel_5G[187] = 52; 
	channel_5G[188] = 53; 
	channel_5G[189] = 54; 
	channel_5G[192] = 55; 
	channel_5G[196] = 56; 
	
	//max 5G channel:196
	if(number > 196)
	{
		printf("err3\n");
		return -1;
	}
	
	
	if(channel_5G[number] !=0)
	{
		printf(" return: %d\n",channel_5G[number]+2);
		return channel_5G[number]+2;  //return location in bit map
		
	}
	else
	{
		printf("err4\n");
		return -1;
	}

}

int change_to_5Gchannel(unsigned int location)
{
	//bit_map_to_channel_5G[bit_map_location] = channel
	unsigned char bit_map_to_channel_5G[64] = {0};
	bit_map_to_channel_5G[17] = 7;
	bit_map_to_channel_5G[18] = 8; 
	bit_map_to_channel_5G[19] = 9; 
	bit_map_to_channel_5G[20] = 11; 
	bit_map_to_channel_5G[21] = 12; 
	bit_map_to_channel_5G[22] = 16; 
	bit_map_to_channel_5G[23] = 34; 
	bit_map_to_channel_5G[24] = 36; 
	bit_map_to_channel_5G[25] = 38; 
	bit_map_to_channel_5G[26] = 40; 
	bit_map_to_channel_5G[27] = 42; 
	bit_map_to_channel_5G[28] = 44; 
	bit_map_to_channel_5G[29] = 46; 
	bit_map_to_channel_5G[30] = 48; 
	bit_map_to_channel_5G[31] = 52; 
	bit_map_to_channel_5G[32] = 56; 
	bit_map_to_channel_5G[33] = 60; 
	bit_map_to_channel_5G[34] = 64; 
	bit_map_to_channel_5G[35] = 100; 
	bit_map_to_channel_5G[36] = 104; 
	bit_map_to_channel_5G[37] = 108; 
	bit_map_to_channel_5G[38] = 112; 
	bit_map_to_channel_5G[39] = 116; 
	bit_map_to_channel_5G[40] = 120; 
	bit_map_to_channel_5G[41] = 124; 
	bit_map_to_channel_5G[42] = 128; 
	bit_map_to_channel_5G[43] = 132; 
	bit_map_to_channel_5G[44] = 136; 
	bit_map_to_channel_5G[45] = 140; 
	bit_map_to_channel_5G[46] = 149; 
	bit_map_to_channel_5G[47] = 153; 
	bit_map_to_channel_5G[48] = 157; 
	bit_map_to_channel_5G[49] = 161; 
	bit_map_to_channel_5G[50] = 165; 
	bit_map_to_channel_5G[51] = 183; 
	bit_map_to_channel_5G[52] = 184; 
	bit_map_to_channel_5G[53] = 185; 
	bit_map_to_channel_5G[54] = 187; 
	bit_map_to_channel_5G[55] = 188; 
	bit_map_to_channel_5G[56] = 189; 
	bit_map_to_channel_5G[57] = 192; 
	bit_map_to_channel_5G[58] = 196; 

	//max 5G channel:196
	if(location >= 64)
	{
		return 0;
	}
	
	return bit_map_to_channel_5G[location];  //return location in bit map

}

unsigned int change_channel_to_bit_map
(
	unsigned char *input_str,
	unsigned int input_str_len,
	unsigned long long *array
)
{
	unsigned int ret = 0;
	int num = 0, index = 0,flag1 = 0, first = 0, number = 0, j = 0;
	int lastindex = -1;

	num = input_str_len;
	
	if(!(input_str[0]>'0' && input_str[0]<='9') || !(input_str[num-1]>='0' && input_str[num-1]<='9'))
	{
	  	goto err;
	}
	
	for(index=0;index<num;index++)
	{
		if(((input_str[index]<'0' && input_str[index]>'9')
		   && input_str[index] != '-'
		   && input_str[index] != ',')
		   ||((input_str[index] == '-' ||input_str[index] == ',') && input_str[index+1] == '0' )
		   )
		{
		  	goto err;
		}
	}

	for(index=0; index<num; index++)
	{
		//handle  '-'
		if ('-' == input_str[index])
		{
			if (1 == flag1)
			{
				goto err;
			}
			flag1 = 1;

			if (-1 == lastindex)
			{
				if (index > 4)
				{
					goto err;
				}
				
				first = strtoint(index, lastindex,input_str);
				if (first > MAX_CHANNEL_2_4G)
				{
					goto err;
				}
			}
			else if ((index - lastindex)<2 ||(index - lastindex)>5)
			{
				goto err;
			}
			else 
			{
				first = strtoint(index, lastindex, input_str);  
				if (first > MAX_CHANNEL_2_4G)
				{
					goto err;
				}
			}
			lastindex = index;
		}
		
		//handle ','
		if (',' == input_str[index])
		{
			
			if ( -1 == lastindex)
			{
				if (index > 4)
				{
					goto err;
				}

				number = strtoint(index, lastindex, input_str);
				if (number > MAX_CHANNEL_2_4G)
				{
					goto err;
				}
				//array[number] = 1;
				(*array) |= BIT(number-1);
			}
			else if ((index - lastindex)<2 ||(index - lastindex)>5)
			{
				goto err;
			}
			else
			{
				number = strtoint(index, lastindex, input_str);
				if (number > MAX_CHANNEL_2_4G)
				{
					goto err;
				}
			}
			
			if (1 == flag1)
			{
				if (number < first)
				{
					goto err;
				}
				for(j=first; j<=number;j++)
				{
					(*array) |= BIT(j-1);
				}
			}
			else
			{
				(*array) |= BIT(number-1);
			}
			flag1 = 0;
			lastindex = index;
		}
		
        	//handle last numer 
		if (index == num-1)
		{
			if (index - lastindex > 4)
			{
				goto err;
			}

			number = strtoint(index+1, lastindex, input_str);
			if (number > MAX_CHANNEL_2_4G ||number < MIN_CHANNEL_2_4G)
			{
				goto err;
			}
			
			if ( '-' == input_str[lastindex])
			{
				if (number < first)
				{
					goto err;
				}
				for(j=first; j<=number; j++)
				{
					(*array) |= BIT(j-1);
				}
			}
			else
			{
				(*array) |= BIT(number-1);
			}
		}
	}
	return ret;
	
	err:
	return 1;
}

unsigned int change_channel_to_bit_map_5_8G
(
	unsigned char *input_str,
	unsigned int input_str_len,
	unsigned long long *array
)
{

	printf("input_str: %s\n",input_str);
	
	unsigned char first = 0;
	unsigned char end = 0;
	unsigned char iterator = 0;
	int num = 0, index = 0;
	int i = 0;

	num = input_str_len;
	
	if(!(input_str[0]>'0' && input_str[0]<='9') || !(input_str[num-1]>='0' && input_str[num-1]<='9'))
	{
		printf("err1\n");
	  	goto err;
	}
	
	for(index=0;index<num;index++)
	{
		if(!((input_str[index]>='0' )&&(input_str[index]<='9'))
		    && ((input_str[index] != ',')
		  	 ||((input_str[index] == ',') && (input_str[index+1] == '0'))
		  	 || ((input_str[index] == ',') && (input_str[index+1] == ','))))
		{
			printf("err2\n");
		  	goto err;
		}
	}

	unsigned int number = 0;
	for(index=0;index<num;index++)
	{
		if(input_str[index]==',')
		{
			for(i=first;i<index;i++)
			{
				number =number *10 + (input_str[i]-'\0'-48);
			}

			if(-1!= check_5G_channel(number))
			{
				(*array) |=((unsigned long long)(1)<<(check_5G_channel(number)-1));
				printf("number : %llu\n",*array);
			}
			
			first = index+1;
			number = 0;
		}

		if((index == num-1)
			&&(input_str[index]!=','))
		{
			for(i=first;i<=index;i++)
			{
				printf("input_str[%d]: %d\n", i, (input_str[i]-'\0'));
				number =number *10 + (input_str[i]-'\0'-48);
			}

			if(-1!= check_5G_channel(number))
			{
				(*array) |= ((unsigned long long)(1)<<(check_5G_channel(number)-1));
			}

		}
	}

	return 0;
	
err:
	return 1;
}

unsigned int wid_add_scanlocate_general_channel
(	
	int localid,
	unsigned int index, 
	unsigned char type,
	unsigned int id,
	unsigned long long channellist,
	unsigned short channel_scan_interval,
	unsigned short channel_scan_dwell,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
)
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	iter;
	DBusError err;
	unsigned int ret = 0;
	
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};
	
	ReInitDbusPath_V2(localid, index, WID_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath_V2(localid, index, WID_DBUS_OBJPATH, OBJPATH);
	ReInitDbusPath_V2(localid, index, WID_DBUS_INTERFACE, INTERFACE);
	query = dbus_message_new_method_call(BUSNAME, OBJPATH,INTERFACE, 
										WID_DBUS_CONF_METHOD_ADD_SCANLOCATE_GENERAL_CHANNEL);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE, &type,		
						DBUS_TYPE_UINT32, &id,
						DBUS_TYPE_UINT16, &channel_scan_interval,
						DBUS_TYPE_UINT16, &channel_scan_dwell,
						DBUS_TYPE_UINT64, &channellist,
						DBUS_TYPE_BYTE, &channel_flag,	
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf( "%s raised: %s", err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	
	dbus_message_unref(reply);	
	return ret;
}

unsigned int wid_add_scanlocate_general_report
(	
	int localid,
	unsigned int index, 
	unsigned char type,
	unsigned int id,
	unsigned char report_pattern,
	unsigned int server_ip,
	unsigned short server_port,
	unsigned short report_interval,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
)
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter iter;
	DBusError err;
	unsigned int ret = 0;
	
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};
	ReInitDbusPath_V2(localid, index, WID_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath_V2(localid, index, WID_DBUS_OBJPATH, OBJPATH);
	ReInitDbusPath_V2(localid, index, WID_DBUS_INTERFACE, INTERFACE);
	query = dbus_message_new_method_call(BUSNAME, OBJPATH,INTERFACE, 
										WID_DBUS_CONF_METHOD_ADD_SCANLOCATE_GENERAL_REPORT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE, &type,		
						DBUS_TYPE_UINT32, &id,
						DBUS_TYPE_BYTE, &report_pattern,
						DBUS_TYPE_UINT32, &server_ip,
						DBUS_TYPE_UINT16, &server_port,
						DBUS_TYPE_UINT16, &report_interval,
						DBUS_TYPE_BYTE, &channel_flag,	
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf( "%s raised: %s", err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	
	dbus_message_unref(reply);	
	return ret;
}

unsigned int wid_add_scanlocate_wifi_special_config
(	
	int localid,
	unsigned int index, 
	unsigned int id,
	unsigned char scan_type,
	unsigned char rssi,
	unsigned char version_num,
	unsigned char result_filter,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
)
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	iter;
	DBusError err;
	unsigned int ret = 0;
	
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};
	ReInitDbusPath_V2(localid, index, WID_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath_V2(localid, index, WID_DBUS_OBJPATH, OBJPATH);
	ReInitDbusPath_V2(localid, index, WID_DBUS_INTERFACE, INTERFACE);
	query = dbus_message_new_method_call(BUSNAME, OBJPATH,INTERFACE, 
					WID_DBUS_CONF_METHOD_ADD_SCANLOCATE_SPECIAL);
	dbus_error_init(&err);

	dbus_message_append_args(query,	
						DBUS_TYPE_UINT32, &id,
						DBUS_TYPE_BYTE, &scan_type,
						DBUS_TYPE_BYTE, &rssi,
						DBUS_TYPE_BYTE, &version_num,
						DBUS_TYPE_BYTE, &result_filter,
						DBUS_TYPE_BYTE, &channel_flag,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf( "%s raised: %s", err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	
	dbus_message_unref(reply);	
	return ret;
}

unsigned int wid_modify_scanlocate_general_config
(	
	int localid,
	unsigned int index, 
	unsigned char type,
	unsigned int id,
	unsigned char modify_type,
	unsigned long long channellist,
	unsigned short channel_scan_dwell,	
	unsigned short channel_scan_interval,
	unsigned char report_pattern,
	unsigned short report_interval,	
	unsigned int server_ip,
	unsigned short server_port,
	unsigned char *wtp_mac,
	unsigned char snmp_flag,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
)
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	iter;
	DBusError err;
	unsigned int ret = 0;
	
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};
	ReInitDbusPath_V2(localid, index, WID_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath_V2(localid, index, WID_DBUS_OBJPATH, OBJPATH);
	ReInitDbusPath_V2(localid, index, WID_DBUS_INTERFACE, INTERFACE);
	query = dbus_message_new_method_call(BUSNAME, OBJPATH,INTERFACE, 
				WID_DBUS_CONF_METHOD_MODIFY_SCANLOCATE_GENERAL);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,	
						DBUS_TYPE_BYTE, &type,	
						DBUS_TYPE_UINT32, &id,
						DBUS_TYPE_BYTE, &modify_type,
						DBUS_TYPE_UINT16, &channel_scan_interval,
						DBUS_TYPE_UINT16, &channel_scan_dwell,
						DBUS_TYPE_UINT64, &channellist,
						DBUS_TYPE_BYTE, &report_pattern,
						DBUS_TYPE_UINT32, &server_ip,
						DBUS_TYPE_UINT16, &server_port,
						DBUS_TYPE_UINT16, &report_interval,
						DBUS_TYPE_BYTE, &wtp_mac[0],
						DBUS_TYPE_BYTE, &wtp_mac[1],
						DBUS_TYPE_BYTE, &wtp_mac[2],
						DBUS_TYPE_BYTE, &wtp_mac[3],
						DBUS_TYPE_BYTE, &wtp_mac[4],
						DBUS_TYPE_BYTE, &wtp_mac[5],
						DBUS_TYPE_BYTE, &snmp_flag,
						DBUS_TYPE_BYTE, &channel_flag,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf( "%s raised: %s", err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	
	dbus_message_unref(reply);	
	return ret;
}

unsigned int wid_modify_scanlocate_wifi_special_config
(	
	int localid,
	unsigned int index, 
	unsigned int id,
	unsigned char modify_type,

	unsigned char scan_type,
	unsigned char rssi,
	unsigned char version_num,
	unsigned char result_filter,
	unsigned char *wtp_mac,
	unsigned char snmp_flag,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
)
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	iter;
	DBusError err;
	unsigned int ret = 0;
	
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};
	ReInitDbusPath_V2(localid, index, WID_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath_V2(localid, index, WID_DBUS_OBJPATH, OBJPATH);
	ReInitDbusPath_V2(localid, index, WID_DBUS_INTERFACE, INTERFACE);
	query = dbus_message_new_method_call(BUSNAME, OBJPATH,INTERFACE, 
										WID_DBUS_CONF_METHOD_MODIFY_SCANLOCATE_WIFI_SPECIAL);
	dbus_error_init(&err);

	dbus_message_append_args(query,	
						DBUS_TYPE_UINT32, &id,
						DBUS_TYPE_BYTE, &modify_type,
						DBUS_TYPE_BYTE, &scan_type,
						DBUS_TYPE_BYTE, &rssi,
						DBUS_TYPE_BYTE, &version_num,
						DBUS_TYPE_BYTE, &result_filter,
						
						DBUS_TYPE_BYTE, &wtp_mac[0],
						DBUS_TYPE_BYTE, &wtp_mac[1],
						DBUS_TYPE_BYTE, &wtp_mac[2],
						DBUS_TYPE_BYTE, &wtp_mac[3],
						DBUS_TYPE_BYTE, &wtp_mac[4],
						DBUS_TYPE_BYTE, &wtp_mac[5],
						DBUS_TYPE_BYTE, &snmp_flag,
						DBUS_TYPE_BYTE, &channel_flag,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf( "%s raised: %s", err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	
	dbus_message_unref(reply);	
	return ret;
}

unsigned int wid_delete_scanlocate_general_channel
(	
	int localid,
	unsigned int index, 
	unsigned char type,
	unsigned int id,
	DBusConnection *dcli_dbus_connection
)
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter iter;
	DBusError err;
	unsigned int ret = 0;
	
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};
	
	ReInitDbusPath_V2(localid, index, WID_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath_V2(localid, index, WID_DBUS_OBJPATH, OBJPATH);
	ReInitDbusPath_V2(localid, index, WID_DBUS_INTERFACE, INTERFACE);
	query = dbus_message_new_method_call(BUSNAME, OBJPATH,INTERFACE, 
					WID_DBUS_CONF_METHOD_DELETE_SCANLOCATE_CONFIG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE, &type,		
						DBUS_TYPE_UINT32, &id,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf( "%s raised: %s", err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	
	dbus_message_unref(reply);	
	return ret;
}

unsigned int wid_set_scanlocate_config_to_radiolist
(
	int localid,
	unsigned int index, 
	unsigned char type,
	unsigned int id,
	update_radio_list *radiolist, 
	unsigned char isadd,
	struct res_head *res_head,
	DBusConnection *dcli_dbus_connection
)
{
  	DBusMessage *query = NULL;
	DBusMessage	*reply = NULL; 
	DBusMessageIter iter;
	DBusError err;

	unsigned int res_num = 0;
	unsigned int ret = 0;
	int i = 0;
	unsigned int radio_count = 0;
	struct tag_radioid *tmp_radiolist = NULL;

	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};
	
	ReInitDbusPath_V2(localid, index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid, index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid, index,WID_DBUS_INTERFACE,INTERFACE); 		
	
	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE,
		WID_DBUS_RADIO_METHOD_SET_SCANLOCATE_CONFIG_TO_RADIOLIST);
																											
	dbus_error_init(&err);	
	
	dbus_message_iter_init_append(query, &iter);
	
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &type);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &id);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &isadd);

	radio_count = (unsigned int)radiolist->count;
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(radio_count));	
	
	tmp_radiolist = radiolist->radioidlist;
	for(i = 0; ((i < radiolist->count) && (tmp_radiolist)); i++)
	{
		dbus_message_iter_append_basic (&iter,
						DBUS_TYPE_UINT32, &(tmp_radiolist->radioid));
		
		tmp_radiolist = tmp_radiolist->next;
	}
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,
												DCLI_WID_DBUS_TIMEOUT,&err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_iter_next(&iter); 
	dbus_message_iter_get_basic(&iter, &res_num);

	res_head->node = (struct res_node *)malloc(sizeof(struct res_node) * res_num);
	if (!res_head->node)
	{
		dbus_message_unref(reply);	
		return WID_DBUS_ERROR;
	}
	memset(res_head->node, 0, sizeof(struct res_node) * res_num);
	res_head->num = res_num;

	for (i = 0; i < res_num; i++)
	{	
		dbus_message_iter_next(&iter); 
		dbus_message_iter_get_basic(&iter, &(res_head->node[i].u.radioid));
		
		dbus_message_iter_next(&iter); 
		dbus_message_iter_get_basic(&iter, &(res_head->node[i].res));
	}

	dbus_message_unref(reply);	
	
	return ret;
}

unsigned int wid_show_wifi_locate_config_group
(
	int localid,
	unsigned int index,
	struct dcli_wifi_locate_public_config* public_config, 
	unsigned char id, 
	DBusConnection *dcli_dbus_connection
)
{    
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 
	DBusMessageIter iter_sub_struct;
	DBusError err;
	int i=0;
	unsigned int ret = 0;	
	unsigned int radio_count = 0;
	
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};

	ReInitDbusPath_V2(localid, index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid, index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid, index,WID_DBUS_INTERFACE,INTERFACE);
	dbus_error_init(&err);

	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, 
			WID_DBUS_WTP_METHOD_SHOW_WIFI_LOCATE_CONFIG_GROUP);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&id,
						DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, 
										DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);

	if(ret == WID_DBUS_SUCCESS)
	{

		dbus_message_iter_get_basic(&iter,&(public_config->groupid));
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter,&(public_config->report_interval));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->scan_type));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->channel));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->channel_scan_interval));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->channel_scan_time));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->rssi));
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter,&(public_config->version_num));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->result_filter));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->server_ip));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->server_port));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->radio_count));
		dbus_message_iter_next(&iter);

		if(public_config->radio_count != 0)
		{

			public_config->radiolist = (dcli_radioList*)malloc((public_config->radio_count)*sizeof(dcli_radioList));
			memset(public_config->radiolist, 0, (public_config->radio_count)*sizeof(dcli_radioList));

			dbus_message_iter_recurse(&iter,&iter_array);  
			
			for(i=0; i<public_config->radio_count; i++)
			{

				dbus_message_iter_recurse(&iter_array,&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&public_config->radiolist[i]);
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_next(&iter_array);
			}
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int wid_show_wifi_locate_config_group_default_5_8G
(
	int localid,
	unsigned int index,
	struct dcli_wifi_locate_public_config* public_config, 
	DBusConnection *dcli_dbus_connection
)
{    
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 
	DBusMessageIter iter_sub_struct;
	DBusError err;
	int i=0;
	unsigned int ret = 0;	
	unsigned int radio_count = 0;
	
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};

	ReInitDbusPath_V2(localid, index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid, index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid, index,WID_DBUS_INTERFACE,INTERFACE);
	dbus_error_init(&err);

	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, 
			WID_DBUS_WTP_METHOD_SHOW_WIFI_LOCATE_CONFIG_GROUP_DEFUALT_5_8G);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, 
										DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);

	if(ret == WID_DBUS_SUCCESS)
	{

		dbus_message_iter_get_basic(&iter,&(public_config->groupid));
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter,&(public_config->report_interval));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->scan_type));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->channel));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->channel_scan_interval));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->channel_scan_time));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->rssi));
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter,&(public_config->version_num));
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter,&(public_config->result_filter));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->server_ip));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->server_port));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->radio_count));
		dbus_message_iter_next(&iter);

		if(public_config->radio_count != 0)
		{

			public_config->radiolist = (dcli_radioList*)malloc((public_config->radio_count)*sizeof(dcli_radioList));
			memset(public_config->radiolist, 0, (public_config->radio_count)*sizeof(dcli_radioList));

			dbus_message_iter_recurse(&iter,&iter_array);  
			
			for(i=0; i<public_config->radio_count; i++)
			{

				dbus_message_iter_recurse(&iter_array,&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&public_config->radiolist[i]);
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_next(&iter_array);
			}
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int wid_show_wifi_locate_config_group_all
(
	int localid,
	unsigned int index,
	struct dcli_wifi_locate_public_config** public_config, 
	unsigned int *group_count,
	DBusConnection *dcli_dbus_connection
)
{    
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 
	DBusMessageIter iter_sub_struct;
	DBusError err;
	int i=0, j=0,k=0;
	unsigned int temp_group_count = 0;
	unsigned int ret = 0;	
	struct dcli_wifi_locate_public_config* wifi_locate_config = NULL;
	unsigned int wtpid = 0;
	
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};

	ReInitDbusPath_V2(localid, index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid, index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid, index,WID_DBUS_INTERFACE,INTERFACE);
	dbus_error_init(&err);

	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, 
					WID_DBUS_WTP_METHOD_SHOW_WIFI_LOCATE_CONFIG_GROUP_ALL);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, 
										DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter,&temp_group_count);
	dbus_message_iter_next(&iter);
	
	if(ret == WID_DBUS_SUCCESS && temp_group_count != 0)
	{

		wifi_locate_config = (struct dcli_wifi_locate_public_config *)malloc(temp_group_count*sizeof(struct dcli_wifi_locate_public_config));
		memset(wifi_locate_config,0,temp_group_count*sizeof(struct dcli_wifi_locate_public_config));

		dbus_message_iter_recurse(&iter,&iter_array);
		
		for(i=0; i<temp_group_count; i++)
		{
			dbus_message_iter_recurse(&iter_array,&iter_struct);	

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].groupid));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].report_interval));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].scan_type));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].channel));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].channel_scan_interval));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].channel_scan_time));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].rssi));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].version_num));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].result_filter));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].server_ip));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].server_port));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].radio_count));
			dbus_message_iter_next(&iter_struct);
			
			if(wifi_locate_config[i].radio_count != 0)
			{

				wifi_locate_config[i].radiolist = (dcli_radioList*)malloc((wifi_locate_config[i].radio_count)*sizeof(dcli_radioList));
				memset(wifi_locate_config[i].radiolist, 0, (wifi_locate_config[i].radio_count)*sizeof(dcli_radioList));
				
				dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
				
				for(j=0; j<wifi_locate_config[i].radio_count ; j++)
				{
					dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);	

					dbus_message_iter_get_basic(&iter_sub_struct,&(wifi_locate_config[i].radiolist[j].radioid));
					dbus_message_iter_next(&iter_sub_struct);

					dbus_message_iter_next(&iter_sub_array);

				}

			}
			
			dbus_message_iter_next(&iter_array);
		}
	}

	*public_config = wifi_locate_config;
	*group_count = temp_group_count;
	
	dbus_message_unref(reply);
	return ret;
}

unsigned int snmp_wifi_locate_change_channellist
(
	int localid,
	unsigned int index, 
	unsigned char* wtp_mac,
	unsigned long long  channellist,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
)
{
	unsigned int ret = 0;
	unsigned char modify_type = 1;
	unsigned char snmp_flag = 1;
	
	unsigned char type = 0;
	unsigned int id= 0;
	unsigned short  channel_scan_interval = 0;	
	unsigned short channel_scan_dwell = 0;	
	unsigned char report_pattern = 0;
	unsigned int server_ip = 0;
	unsigned short server_port = 0;
	unsigned short report_interval = 0;	
	
	ret = wid_modify_scanlocate_general_config(localid, index, type, id, modify_type, channellist,
										channel_scan_dwell, channel_scan_interval,
										report_pattern,  report_interval, server_ip,
										server_port, wtp_mac, snmp_flag,channel_flag,
										dcli_dbus_connection);
	return ret;
}

unsigned int snmp_wifi_locate_change_channel_scan_time
(
	int localid,
	unsigned int index, 
	unsigned char* wtp_mac,
	unsigned short channel_scan_dwell,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
)
{
	unsigned int ret = 0;
	unsigned char modify_type = 2;
	unsigned char snmp_flag = 1;
	
	unsigned char type = 0;
	unsigned int id= 0;
	unsigned long long channellist = 0;
	unsigned short  channel_scan_interval = 0;		
	unsigned char report_pattern = 0;
	unsigned int server_ip = 0;
	unsigned short server_port = 0;
	unsigned short report_interval = 0;	
	
	ret = wid_modify_scanlocate_general_config(localid, index, type, id, modify_type, channellist,
										channel_scan_dwell, channel_scan_interval,
										report_pattern,  report_interval, server_ip,
										server_port, wtp_mac, snmp_flag,channel_flag,
										dcli_dbus_connection);
	return ret;
}

unsigned int snmp_wifi_locate_change_channel_scan_interval
(
	int localid,
	unsigned int index, 
	unsigned char* wtp_mac,
	unsigned short channel_scan_interval,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
	
)
{
	unsigned int ret = 0;
	unsigned char modify_type = 3;
	unsigned char snmp_flag = 1;
	
	unsigned char type = 0;
	unsigned int id= 0;
	unsigned long long channellist = 0;	
	unsigned char report_pattern = 0;
	unsigned short channel_scan_dwell = 0;
	unsigned int server_ip = 0;
	unsigned short server_port = 0;
	unsigned short report_interval = 0;	
	
	ret = wid_modify_scanlocate_general_config(localid, index, type, id, modify_type, channellist,
										channel_scan_dwell, channel_scan_interval,
										report_pattern,  report_interval, server_ip,
										server_port, wtp_mac, snmp_flag,channel_flag,
										dcli_dbus_connection);
	return ret;
}

unsigned int snmp_wifi_locate_change_report_interval
(
	int localid,
	unsigned int index, 
	unsigned char* wtp_mac,
	unsigned short report_interval,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
	
)
{
	unsigned int ret = 0;
	unsigned char modify_type = 5;
	unsigned char snmp_flag = 1;
	
	unsigned char type = 0;
	unsigned int id= 0;
	unsigned long long channellist = 0;	
	unsigned char report_pattern = 0;
	unsigned short channel_scan_dwell = 0;
	unsigned short channel_scan_interval = 0;
	unsigned int server_ip = 0;
	unsigned short server_port = 0;

	
	ret = wid_modify_scanlocate_general_config(localid, index, type, id, modify_type, channellist,
										channel_scan_dwell, channel_scan_interval,
										report_pattern,  report_interval, server_ip,
										server_port, wtp_mac, snmp_flag,channel_flag,
										dcli_dbus_connection);
	return ret;
}

unsigned int snmp_wifi_locate_change_server_ip
(
	int localid,
	unsigned int index, 
	unsigned char* wtp_mac,
	unsigned int server_ip,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
	
)
{
	unsigned int ret = 0;
	unsigned char modify_type = 6;
	unsigned char snmp_flag = 1;
	
	unsigned char type = 0;
	unsigned int id= 0;
	unsigned long long channellist = 0;
	unsigned short  channel_scan_interval = 0;	
	unsigned short channel_scan_dwell = 0;	
	unsigned char report_pattern = 0;
	unsigned short server_port = 0;
	unsigned short report_interval = 0;	

	ret = wid_modify_scanlocate_general_config(localid, index, type, id, modify_type, channellist,
										channel_scan_dwell, channel_scan_interval,
										report_pattern,  report_interval, server_ip,
										server_port, wtp_mac, snmp_flag,channel_flag,
										dcli_dbus_connection);
	return ret;
}

unsigned int snmp_wifi_locate_change_server_port
(
	int localid,
	unsigned int index, 
	unsigned char* wtp_mac,
	unsigned short server_port,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
	
)
{
	unsigned int ret = 0;
	unsigned char modify_type = 7;
	unsigned char snmp_flag = 1;
	
	unsigned char type = 0;
	unsigned int id= 0;
	unsigned long long channellist = 0;	
	unsigned char report_pattern = 0;
	unsigned short channel_scan_dwell = 0;
	unsigned short channel_scan_interval = 0;
	unsigned int server_ip = 0;
	unsigned short report_interval = 0;

	
	ret = wid_modify_scanlocate_general_config(localid, index, type, id, modify_type, channellist,
										channel_scan_dwell, channel_scan_interval,
										report_pattern,  report_interval, server_ip,
										server_port, wtp_mac, snmp_flag,channel_flag,
										dcli_dbus_connection);
	return ret;
}

unsigned int snmp_wifi_locate_change_rssi
(
	int localid,
	unsigned int index, 
	unsigned char* wtp_mac,
	unsigned char rssi,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
	
)
{
	unsigned int ret = 0;
	unsigned char modify_type = 1;
	unsigned char snmp_flag = 1;
	
	unsigned int id= 0;
	unsigned char scan_type = 0;
	unsigned char version_num = 0;
	unsigned char result_filter = 0;
	
	ret = wid_modify_scanlocate_wifi_special_config(localid,index, id, modify_type, scan_type,rssi,  
											version_num, result_filter,
											wtp_mac,snmp_flag, channel_flag,
											dcli_dbus_connection);
	return ret;
}
unsigned int snmp_wifi_locate_change_scan_type
(
	int localid,
	unsigned int index, 
	unsigned char* wtp_mac,
	unsigned char scan_type,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
	
)
{
	unsigned int ret = 0;
	unsigned char modify_type = 2;
	unsigned char snmp_flag = 1;
	
	unsigned int id= 0;
	unsigned char rssi = 0;
	unsigned char version_num = 0;
	unsigned char result_filter = 0;

	ret = wid_modify_scanlocate_wifi_special_config(localid,index, id, modify_type, scan_type, rssi, 
											version_num, result_filter,
											wtp_mac,snmp_flag, channel_flag,
											dcli_dbus_connection);
	return ret;
}

unsigned int snmp_wifi_locate_change_version_num
(
    int localid,
	unsigned int index, 
	unsigned char* wtp_mac,
	unsigned char version_num,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
	
)
{
	unsigned int ret = 0;
	unsigned char modify_type = 3;
	unsigned char snmp_flag = 1;
	
	unsigned int id= 0;
	unsigned char scan_type = 0;
	unsigned char rssi = 0;
	unsigned char result_filter = 0;

	ret = wid_modify_scanlocate_wifi_special_config(localid,index, id, modify_type, rssi, scan_type, 
											version_num, result_filter,
											wtp_mac,snmp_flag, channel_flag,
											dcli_dbus_connection);
	return ret;
}

unsigned int snmp_wifi_locate_change_result_filter
(
    int localid,
	unsigned int index, 
	unsigned char* wtp_mac,
	unsigned char result_filter,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
	
)
{
	unsigned int ret = 0;
	unsigned char modify_type = 4;
	unsigned char snmp_flag = 1;
	
	unsigned int id= 0;
	unsigned char scan_type = 0;
	unsigned char rssi = 0;
	unsigned char version_num = 0;

	ret = wid_modify_scanlocate_wifi_special_config(localid,index, id, modify_type,rssi,  scan_type, 
											version_num, result_filter,
											wtp_mac,snmp_flag, channel_flag,
											dcli_dbus_connection);
	return ret;
}

unsigned int snmp_wifi_locate_change_on_off
(
	int localid,
	unsigned int index, 
	unsigned char* wtp_mac,
	unsigned char isenable,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
	
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned int ret = 0;	
	unsigned char isall = 0;

	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};

	ReInitDbusPath_V2(localid, index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid, index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid, index,WID_DBUS_INTERFACE,INTERFACE);

	dbus_error_init(&err);

	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, WID_DBUS_WTP_METHOD_SET_WIFI_LOCATE_ON_OFF);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&isenable,
						DBUS_TYPE_BYTE,&wtp_mac[0],
						DBUS_TYPE_BYTE,&wtp_mac[1],
						DBUS_TYPE_BYTE,&wtp_mac[2],
						DBUS_TYPE_BYTE,&wtp_mac[3],
						DBUS_TYPE_BYTE,&wtp_mac[4],
						DBUS_TYPE_BYTE,&wtp_mac[5],
						DBUS_TYPE_BYTE,&isall,
						DBUS_TYPE_BYTE,&channel_flag,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_unref(reply);

	
	return ret;
}

unsigned int snmp_wifi_locate_change_on_off_all_wtp
(
	int localid,
	unsigned int index, 
	unsigned char isenable,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
	
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned int ret = 0;	
	unsigned char isall = 1;
	unsigned char wtp_mac[MAC_LEN] = {0};
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};

	ReInitDbusPath_V2(localid, index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid, index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid, index,WID_DBUS_INTERFACE,INTERFACE);

	dbus_error_init(&err);

	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, WID_DBUS_WTP_METHOD_SET_WIFI_LOCATE_ON_OFF);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&isenable,
						DBUS_TYPE_BYTE,&wtp_mac[0],
						DBUS_TYPE_BYTE,&wtp_mac[1],
						DBUS_TYPE_BYTE,&wtp_mac[2],
						DBUS_TYPE_BYTE,&wtp_mac[3],
						DBUS_TYPE_BYTE,&wtp_mac[4],
						DBUS_TYPE_BYTE,&wtp_mac[5],
						DBUS_TYPE_BYTE,&isall,
						DBUS_TYPE_BYTE,&channel_flag,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_unref(reply);

	
	
	return ret;
}

unsigned int snmp_wifi_locate_set_public_config
(
	int localid,
	unsigned int index,
	unsigned char isenable,
	unsigned char *wtp_mac,	
	unsigned short report_interval,
	unsigned char scan_type,
	unsigned long long channel,
	unsigned short channel_scan_interval,
	unsigned short channel_scan_time,
	unsigned char rssi,
	unsigned char version_num,
	unsigned char result_filter,
	unsigned int server_ip,
	unsigned short server_port,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned int ret = 0;	

	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};

	ReInitDbusPath_V2(localid, index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid, index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid, index,WID_DBUS_INTERFACE,INTERFACE);

	dbus_error_init(&err);

	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, WID_DBUS_WTP_METHOD_SET_WIFI_LOCATE_PUBLIC_CONFIG);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&isenable,
						DBUS_TYPE_BYTE,&wtp_mac[0],
						DBUS_TYPE_BYTE,&wtp_mac[1],
						DBUS_TYPE_BYTE,&wtp_mac[2],
						DBUS_TYPE_BYTE,&wtp_mac[3],
						DBUS_TYPE_BYTE,&wtp_mac[4],
						DBUS_TYPE_BYTE,&wtp_mac[5],
						DBUS_TYPE_UINT16,&report_interval,					
						DBUS_TYPE_BYTE,&scan_type,
						DBUS_TYPE_UINT64,&channel,
						DBUS_TYPE_UINT16,&channel_scan_interval,
						DBUS_TYPE_UINT16,&channel_scan_time,
						DBUS_TYPE_BYTE,&rssi,
						DBUS_TYPE_BYTE,&version_num,
						DBUS_TYPE_BYTE,&result_filter,
						DBUS_TYPE_UINT32,&server_ip,
						DBUS_TYPE_UINT16,&server_port,
						DBUS_TYPE_BYTE,&channel_flag,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_unref(reply);

	return ret;
}

unsigned int snmp_wifi_locate_show_public_config_wtp
(
	int localid,
	unsigned int index,
	struct dcli_wifi_locate_public_config* public_config, 
	unsigned char *wtp_mac,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
)
{    
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 
	DBusMessageIter iter_sub_struct;
	DBusError err;
	int i=0, j=0,k=0;
	int wtp_num = 0;
	unsigned int ret = 0;	
	
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};

	ReInitDbusPath_V2(localid, index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid, index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid, index,WID_DBUS_INTERFACE,INTERFACE);
	dbus_error_init(&err);

	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, 
					WID_DBUS_WTP_METHOD_SHOW_WIFI_LOCATE_PUBLIC_CONFIG);
	
	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&wtp_mac[0],
						DBUS_TYPE_BYTE,&wtp_mac[1],
						DBUS_TYPE_BYTE,&wtp_mac[2],
						DBUS_TYPE_BYTE,&wtp_mac[3],
						DBUS_TYPE_BYTE,&wtp_mac[4],
						DBUS_TYPE_BYTE,&wtp_mac[5],
						DBUS_TYPE_BYTE,&channel_flag,
						DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, 
										DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);

	if(ret == WID_DBUS_SUCCESS)
	{
		dbus_message_iter_get_basic(&iter,&(public_config->state));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->report_interval));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->scan_type));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->channel));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->channel_scan_interval));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->channel_scan_time));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->rssi));
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter,&(public_config->version_num));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->result_filter));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->server_ip));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(public_config->server_port));
		dbus_message_iter_next(&iter);
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int snmp_wifi_locate_show_public_config_all_wtp
(
	int localid,
	unsigned int index,
	struct dcli_wifi_locate_public_config** public_config, 
	unsigned int *wtp_num,
	unsigned char channel_flag,
	DBusConnection *dcli_dbus_connection
)
{    
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter  iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 
	DBusMessageIter iter_sub_struct;
	DBusError err;
	int i=0, j=0,k=0;
	unsigned int temp_wtp_num = 0;
	unsigned int ret = 0;	
	struct dcli_wifi_locate_public_config* wifi_locate_config = NULL;
	unsigned int wtpid = 0;
	
	char BUSNAME[PATH_LEN] = {0};
	char OBJPATH[PATH_LEN] = {0};
	char INTERFACE[PATH_LEN] = {0};
printf("#####wangchao enter %s\n",__func__);
	ReInitDbusPath_V2(localid, index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid, index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid, index,WID_DBUS_INTERFACE,INTERFACE);
	dbus_error_init(&err);

	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, 
					WID_DBUS_WTP_METHOD_SHOW_WIFI_LOCATE_PUBLIC_CONFIG_ALL);

	dbus_message_append_args(query,
					DBUS_TYPE_BYTE,&channel_flag,
					DBUS_TYPE_INVALID);

	printf("#############dcli_dbus_connection == %p\n",dcli_dbus_connection);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, 
										DCLI_WID_DBUS_TIMEOUT, &err);
	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return WID_DBUS_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter,&temp_wtp_num);
	dbus_message_iter_next(&iter);
	
	if(ret == WID_DBUS_SUCCESS && temp_wtp_num != 0)
	{

		wifi_locate_config = (struct dcli_wifi_locate_public_config *)malloc(temp_wtp_num*sizeof(struct dcli_wifi_locate_public_config));
	    if (NULL == wifi_locate_config) 
		{
            printf("malloc failed \n");
		}	
		memset(wifi_locate_config,0,temp_wtp_num*sizeof(struct dcli_wifi_locate_public_config));

		dbus_message_iter_recurse(&iter,&iter_array);
		
		for(i=0; i<temp_wtp_num; i++)
		{

			dbus_message_iter_recurse(&iter_array,&iter_struct);	

			wtpid = 0;
			dbus_message_iter_get_basic(&iter_struct,&(wtpid));
			dbus_message_iter_next(&iter_struct);
			
			wifi_locate_config[i].wtpid = wtpid;

			for(j=0;j<MAC_LEN;j++)
			{
				dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].wtp_mac[j]));
				dbus_message_iter_next(&iter_struct);
			}
		
			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].state));
			dbus_message_iter_next(&iter_struct);
			printf("wifi_locate_config[i].state == %d line==%d\n", wifi_locate_config[i].state, __LINE__);
			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].report_interval));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].scan_type));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].channel));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].channel_scan_interval));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].channel_scan_time));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].rssi));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].version_num));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].result_filter));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].server_ip));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wifi_locate_config[i].server_port));
			dbus_message_iter_next(&iter_struct);

			
			
			dbus_message_iter_next(&iter_array);

		}
	}
printf("##########wangchao line == %d, temp_wtp_num == %d\n", __LINE__, temp_wtp_num);	

    if (NULL != wifi_locate_config) {
        wifi_locate_config->wtp_num = temp_wtp_num;
	}
	*public_config = wifi_locate_config;
	
printf("##########wangchao line == %d\n", __LINE__);	

//	(*public_config)->wtp_num = temp_wtp_num; /*wangchao add*/
	
	*wtp_num = temp_wtp_num;
printf("##########wangchao temp_wtp_num == %d line == %d\n",temp_wtp_num, __LINE__);	
	
	dbus_message_unref(reply);
	return ret;
}

#endif
