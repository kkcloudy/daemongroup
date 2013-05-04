
#ifndef _D_FPGA_
#define _D_FPGA_

#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <sys/wait.h>
#include <dbus/sem/sem_dbus_def.h>
#include "command.h"
#include "dcli_main.h"

#include "dcli_fpga.h"

/*Indirect access to data temporary storage area*/

long DataTempStorageArea[] = { 				

/*Cycle_1*/
	0x10, 	/* 8 */
	0x12,	/* 9 */				
	0x14, 	/* 10 */		
	0x16, 	/* 11*/		
	0x18,  	/* 12 */		
	0x1a, 	/* 13*/		
	0x1c,	/* 14 */			
	0x1e,   /* 15 */
/*Cycle_2*/
	0x20, 	/* 16 */		
	0x22,	/* 17 */				
	0x24, 	/* 18 */		
	0x26, 	/* 19 */		
	0x28, 	/* 20 */		
	0x2a,   /* 21 */	
	0x2c,	/* 22 */		
	0x2e,   /* 23 */
/*Cycle_3*/
	0x30, 	/* 24 */			
	0x32,	/* 25 */				
	0x34, 	/* 26 */		
	0x36, 	/* 27 */		
	0x38, 	/* 28 */		
	0x3a, 	/* 29 */		
	0x3c,	/* 30 */			
	0x3e,   /* 31 */
/*Cycle_4*/
	0x40, 	/* 32 */		
	0x42, 	/* 33 */		
	0x44, 	/* 34 */		
	0x46,	/* 35 */		
	0x48,	/* 36 */			
	0x4a, 	/* 37 */
	0x4c,	/* 38 */			
	0x4e,	/* 39 */			
/*Cycle_5*/
	0x50, 	/* 40 */			
	0x52,	/* 41 */				
	0x54, 	/* 42 */		
	0x56, 	/* 43 */		
	0x58, 	/* 44 */		
	0x5a,	/* 45 */	
	0x5c,	/* 46 */			
	0x5e,	/* 47 */			
/*Cycle_6*/
	0x60, 	/* 48 */			
	0x62,	/* 49 */				
	0x64, 	/* 50 */		
	0x66, 	/* 51 */		
	0x68, 	/* 52 */		
	0x6a, 	/* 53 */		
	0x6c,	/* 54 */		
	0x6e,	/* 55 */	
/*Cycle_7*/
	0x70, 	/* 56 */			
	0x72,	/* 57 */				
	0x74,	/* 58 */		
	0x76, 	/* 59 */		
	0x78,	/* 60 */		
	0x7a, 	/* 61 */		
	0x7c,	/* 62 */			
	0x7e,   /* 63 */
}; 
/*end*/

/*for counter register*/

long MibXauiRegOffsetAll[] = { 				

    /*xaui112   k=0*/
	0x00,/* rx_good_pkt_num [31:16]*/ 			
	0x02,/* rx_good_pkt_num [15:0]*/ 			
	0x04,/* rx_good_byte_num[39:32]*/ 			
	0x06,/* rx_good_byte_num[31:16] */			
	0x08,/* rx_good_byte_num[15:0]	*/				
	0x0a,/* rx_bad_pkt_num[15:0] */ 	               
	0x0c,/* rx_fcserr_pkt_num[15:0] */				
	0x0e,/* rx_multicast_pkt_num[15:0] */				
	0x10,/* rx_broadcast_pkt_num[15:0] */ 				
	0x12,/* rx_pause_pkt_num[15:0] */
	0x14,/* rx_drop_num[15:0] */
	0x16,/* rx_ttl_drop_num[15:0] */ 			
	0x18,/* rx_car_drop_num[15:0] */ 			
	0x1a,/* rx_hash_col_num[31:16] */ 			
	0x1c,/* rx_hash_col_num[15:0] */ 			
	0x1e,/* rx_to_cpu_num[31:16] */				
	0x20,/* rx_to_cpu_num[15:0] */				
	0x22,/* tx_pkt_num[31:16] */ 				
	0x24,/* tx_pkt_num[15:0] */					
	0x26,/* tx_byte_num[39:32] */ 			
	0x28,/* tx_byte_num[31:16] */ 			
	0x2a,/* tx_byte_num[15:0] */ 			
	0x2c,/* tx_multicast_pkt_num[15:0] */ 			
	0x2e,/* tx_broadcast_pkt_num[15:0] */				
	0x30,/* tx_pause_pkt_num[15:0] */	
	
    /*xaui113   k=25*/
	0x40,/* rx_good_pkt_num [31:16]*/ 			
	0x42,/* rx_good_pkt_num [15:0]*/ 			
	0x44,/* rx_good_byte_num[39:32]*/ 			
	0x46,/* rx_good_byte_num[31:16] */			
	0x48,/* rx_good_byte_num[15:0]	*/				
	0x4a,/* rx_bad_pkt_num[15:0] */ 	
	0x4c,/* rx_fcserr_pkt_num[15:0] */				
	0x4e,/* rx_multicast_pkt_num[15:0] */				
	0x50,/* rx_broadcast_pkt_num[15:0] */ 				
	0x52,/* rx_pause_pkt_num[15:0] */
	0x54,/* rx_drop_num[15:0] */
	0x56,/* rx_ttl_drop_num[15:0] */ 			
	0x58,/* rx_car_drop_num[15:0] */ 			
	0x5a,/* rx_hash_col_num[31:16] */ 			
	0x5c,/* rx_hash_col_num[15:0] */ 			
	0x5e,/* rx_to_cpu_num[31:16] */				
	0x60,/* rx_to_cpu_num[15:0] */				
	0x62,/* tx_pkt_num[31:16] */ 				
	0x64,/* tx_pkt_num[15:0] */					
	0x66,/* tx_byte_num[39:32] */ 			
	0x68,/* tx_byte_num[31:16] */ 			
	0x6a,/* tx_byte_num[15:0] */ 			
	0x6c,/* tx_multicast_pkt_num[15:0] */ 			
	0x6e,/* tx_broadcast_pkt_num[15:0] */				
	0x70,/* tx_pause_pkt_num[15:0] */		
	
/*xaui115   k=50*/
	0x80,/* rx_good_pkt_num [31:16]*/ 			
	0x82,/* rx_good_pkt_num [15:0]*/ 			
	0x84,/* rx_good_byte_num[39:32]*/ 			
	0x86,/* rx_good_byte_num[31:16] */			
	0x88,/* rx_good_byte_num[15:0]	*/				
	0x8a,/* rx_bad_pkt_num[15:0] */ 	
	0x8c,/* rx_fcserr_pkt_num[15:0] */				
	0x8e,/* rx_multicast_pkt_num[15:0] */				
	0x90,/* rx_broadcast_pkt_num[15:0] */ 				
	0x92,/* rx_pause_pkt_num[15:0] */
	0x94,/* rx_drop_num[15:0] */
	0x96,/* rx_ttl_drop_num[15:0] */ 			
	0x98,/* rx_car_drop_num[15:0] */ 			
	0x9a,/* rx_hash_col_num[31:16] */ 			
	0x9c,/* rx_hash_col_num[15:0] */ 			
	0x9e,/* rx_to_cpu_num[31:16] */				
	0xa0,/* rx_to_cpu_num[15:0] */				
	0xa2,/* tx_pkt_num[31:16] */ 				
	0xa4,/* tx_pkt_num[15:0] */					
	0xa6,/* tx_byte_num[39:32] */ 			
	0xa8,/* tx_byte_num[31:16] */ 			
	0xaa,/* tx_byte_num[15:0] */ 			
	0xac,/* tx_multicast_pkt_num[15:0] */ 			
	0xae,/* tx_broadcast_pkt_num[15:0] */				
	0xb0,/* tx_pause_pkt_num[15:0] */			
	
/*xaui116  k=75*/
	0xc0,/* rx_good_pkt_num [31:16]*/ 			
	0xc2,/* rx_good_pkt_num [15:0]*/ 			
	0xc4,/* rx_good_byte_num[39:32]*/ 			
	0xc6,/* rx_good_byte_num[31:16] */			
	0xc8,/* rx_good_byte_num[15:0]	*/				
	0xca,/* rx_bad_pkt_num[15:0] */ 	
	0xcc,/* rx_fcserr_pkt_num[15:0] */				
	0xce,/* rx_multicast_pkt_num[15:0] */				
	0xd0,/* rx_broadcast_pkt_num[15:0] */ 				
	0xd2,/* rx_pause_pkt_num[15:0] */
	0xd4,/* rx_drop_num[15:0] */
	
	0xd6,/* hash_aging_num[31:16] */ 			
	0xd8,/* hash_aging_num[15:0] */ 			
	0xda,/* hash_update_num [31:16] */ 			
	0xdc,/* hash_update_num [15:0] */				
	0xde,/* hash_learn_num [31:16] */				
	0xe0,/* hash_learn_num [15:0] */ 				

	0xe2,/* tx_pkt_num[31:16] */		
	0xe4,/* tx_pkt_num[15:0] */					
	0xe6,/* tx_byte_num[39:32] */ 			
	0xe8,/* tx_byte_num[31:16] */ 			
	0xea,/* tx_byte_num[15:0] */ 			
	0xec,/* tx_multicast_pkt_num[15:0] */ 			
	0xee,/* tx_broadcast_pkt_num[15:0] */				
	0xf0,/* tx_pause_pkt_num[15:0] */				
	
}; /* register address */

unsigned char StorRegCount = sizeof(DataTempStorageArea)/sizeof(long);

unsigned char MibRegCount = sizeof(MibXauiRegOffsetAll)/sizeof(long);

int fpga_parse_int(char* str,unsigned int* shot){
	char *endptr = NULL;

	if (NULL == str) return -1;
	*shot= strtoul(str,&endptr,10);
	return 0;	
}
int fpga_parse_int_H(char* str,unsigned int* shot){
	char *endptr = NULL;

	if (NULL == str) return -1;
	*shot= strtoul(str,&endptr,16);
	return 0;	
}
int fpga_parse_short(char* str,unsigned short* shot){
	char *endptr = NULL;

	if (NULL == str) return -1;
	*shot= strtoul(str,&endptr,10);
	return 0;	
}
int fpga_parse_short_H(char* str,unsigned short* shot){
	char *endptr = NULL;

	if (NULL == str) return -1;
	*shot= strtoul(str,&endptr,16);
	return 0;
}


 int fpga_parse_mac_addr(char* input,ETHERADDR* macAddr) {
 	
	int i = 0;
	char cur = 0,value = 0;
	
	if((NULL == input)||(NULL == macAddr)) {
		return -1;
	}
	if(-1 == mac_format_check(input,strlen(input))) {
		return -1;
	}
	
	for(i = 0; i <6;i++) {
		cur = *(input++);
		if((cur == ':') ||(cur == '-')){
			i--;
			continue;
		}
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		macAddr->arEther[i] = value;
		cur = *(input++);	
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		macAddr->arEther[i] = (macAddr->arEther[i]<< 4)|value;
	}

	/* cheak if mac == 00:00:00:00:00:00, is illegal .zhangdi@autelan.com 2011-07-11 */
    if( (macAddr->arEther[0]==0x00)&&(macAddr->arEther[1]==0x00)&&(macAddr->arEther[2]==0x00)&&   \
		(macAddr->arEther[3]==0x00)&&(macAddr->arEther[4]==0x00)&&(macAddr->arEther[5]==0x00) )
    {
		return -1;		
    }
	
	return 0;
} 
int fpga_is_muti_brc_mac(ETHERADDR *mac)
{
  if(mac->arEther[0] & 0x1)
  	return 1;
  else{ return 0;}
}

unsigned long long str_16_num(char * a)
{  
    unsigned long long sum = 0;
    int i = 0;
    if(a[i]==0)
    {   
        i++;
    }   
    else if(a[i]=='x'||a[i]=='X')
    {   
        i++;
    }   
    //printf("%s\n",a);
    for(;a[i]!='\0';i++)
    {   
        if(a[i]>='0'&&a[i]<='9')
             sum=sum*16+a[i]-'0';
        else if(a[i]>='a'&&a[i]<='f')
             sum=sum*16+a[i]-'a'+10;
        else if(a[i]>='A'&&a[i]<='F')
             sum=sum*16+a[i]-'A'+10;
    }   
    return sum;
}

unsigned short str_16_short_num(char * a)
{  
    unsigned short sum = 0;
    int i = 0;
	int flag =0;
    if(a[i]==0)
    {   
        i++;
    }   
    else if(a[i]=='x'||a[i]=='X')
    {   
        i++;
    }   
    if(strlen(a+i)>4)
    {
        //printf("Bad parameter.0x0~0xffff\n");
		return 0;
	}
    for(;a[i]!='\0';i++)
    {   
        if(a[i]>='0'&&a[i]<='9')
             sum=sum*16+a[i]-'0';
        else if(a[i]>='a'&&a[i]<='f')
             sum=sum*16+a[i]-'a'+10;
        else if(a[i]>='A'&&a[i]<='F')
             sum=sum*16+a[i]-'A'+10;
    }
    return sum;
}

void binary(unsigned int num,char *out)
{
    int a=1<<31;
    char ch;
    int i=0;
    for(;i<32;i++)
    {
        ch=(num&a)?'1':'0';
        out[i]=ch;
        num<<=1;
    }
	//printf("binary ok\n");
}

unsigned long long str_10_num(char * a)
{  
    unsigned long long sum = 0;
    int i = 0;
    if(a[i]==0)
    {   
        i++;
    }   
    else if(a[i]=='x'||a[i]=='X')
    {   
        i++;
    }   
    for(;a[i]!='\0';i++)
    {   
        if(a[i]>='0'&&a[i]<='9')
             sum=sum*10+a[i]-'0';
        else 
            printf("please Input decimal\n");
    }   
    return sum;
}
unsigned short str_10_short_num(char * a)
{  
    unsigned long long sum = 0;
    int i = 0;
    if(a[i]==0)
    {   
        i++;
    }   
    else if(a[i]=='x'||a[i]=='X')
    {   
        i++;
    }
	if(strlen(a+i)>5)
    {
        //printf("Bad parameter.0~65535\n");
		return 0;
	}
    for(;a[i]!='\0';i++)
    {   
        if(a[i] >= '0'&&a[i] <= '9')
        {
             sum=sum*10+a[i]-'0';
        }
        else 
        {
            //printf("please Input decimal\n");
			return 0;
        }
    }
	if(sum >65535)
	{
        //printf("Bad parameter.0~65535\n");
		return 0;
	}
    return sum;
}

int write_fpga_reg(bm_op_args * ops) 
{
	int fd = 0;
	int retval;
	
	fd = open(FPGA_BM_FILE_PATH,0);	
	if (fd == -1)
	{
        printf("Open file:/dev/bm0 error!\n");
		return -1;
	}

	retval = ioctl (fd,BM_IOC_X_,ops);
	if (0 ==retval) 
	{
		printf("R &w at addr [0x%016llx]",ops->op_addr);
		printf("            [0x%04x]\n",(unsigned short)ops->op_value);
	} 
	else 
	{
		printf("Write failed return [%d]\n",retval);
	}

	close(fd);
	return retval;

}

int read_fpga_reg(bm_op_args * ops)
{
	int fd = 0;
	int retval;
	
	fd = open(FPGA_BM_FILE_PATH,0);	
	if (fd == -1)
	{
        printf("Open file:/dev/bm0 error!\n");
		return -1;
	}

	retval = ioctl (fd,BM_IOC_G_,ops);
	if (0 ==retval) 
	{
		printf("Read at addr [0x%016llx]",ops->op_addr);
		printf("            [0x%04x]\n",(unsigned short)ops->op_value);
	} 
	else 
	{
		printf("Read failed return [%d]\n",retval);
	}

	close(fd);
	return retval;

}

/*end for counter register*/	


int burning_fpga_bin(fpga_file *arg) 
{

	FILE * fd = NULL;
	int retval;
        
	fd = open(FPGA_BM_FILE_PATH,0);
	if (fd == NULL)
	{
        printf("Open file:/dev/bm0 error!\n");
		return -1;
	}
    printf("open FPGA_BM_FILE_PATH success!\n");
	
	retval = ioctl (fd,BM_IOC_FPGA_WRITE,arg);
	if (0 == retval)
	{
		if(arg->result == 0){
		    printf("burning_fpga_bin success.return %d\n",arg->result);
			close(fd);
			return 1;
		}else{
            printf("burning_fpga_bin failed.return %d\n",arg->result);
			close(fd);
			return 0;
		}
	} 
	else 
	{
		printf("burning_fpga_bin ioctl failed,return %d!\n",retval);
		close(fd);
		return 0;
	}
}
#define DATA_LEN (128)
unsigned int high_long_bit(hash_param_t *tuple,int num)
{
	unsigned int data;
	if(num >= 104 && num <= 127){
        data = 0;
	}else if(num >= 72 && num <= 103){
        data = (tuple->dip >>(num-72)) & 0x1;
	}else if(num >= 40 && num <= 71){
        data = (tuple->sip >>(num-40)) & 0x1;
	}else if(num >= 24 && num <= 39){
        data = (tuple->dport >>(num-24)) & 0x1;
	}else if(num >= 8 && num <= 23){
        data = (tuple->sport >>(num-8)) & 0x1;
	}else if(num >= 0 && num <= 7){
        data = (tuple->protocol >>(num-0)) & 0x1;
	}else{
        printf("bad parame num=%d(0~127)\n",num);
	}
	return data;
}

unsigned int long_bit(unsigned long long data,int num)
{
    return (unsigned int)((data >> num) & 0x1);
}

unsigned int int_bit(unsigned int data,int num)
{
    return (unsigned int)((data >> num) & 0x1);
}

unsigned int short_bit(unsigned short data,int num)
{
    return (unsigned int)((data >> num) & 0x1);
}

/* get fpga hash id :crc32c_modify algorithm*/
/*
hash_id_crc = nextCRC32_D128({24'b0,usr_dip,usr_sip,usr_dst_port,usr_src_port,usr_ip_protocol},32'b0);
*/
unsigned int crc_get_hash_id_crc32c_modify(hash_param_t *tuple,unsigned int crc)
{
    unsigned int hash_id;
    hash_id = (high_long_bit(tuple,127) ^ high_long_bit(tuple,126) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,54) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,43) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,0) ^ int_bit(crc,4) ^ int_bit(crc,5) ^ int_bit(crc,8) ^ int_bit(crc,10) ^ int_bit(crc,12) ^ int_bit(crc,14) ^ int_bit(crc,17) ^ int_bit(crc,18) ^ int_bit(crc,21) ^ int_bit(crc,22) ^ int_bit(crc,24) ^ int_bit(crc,28) ^ int_bit(crc,29) ^ int_bit(crc,30) ^ int_bit(crc,31))&(1);
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,126) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,55) ^ high_long_bit(tuple,54) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,44) ^ high_long_bit(tuple,43) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,1) ^ int_bit(crc,5) ^ int_bit(crc,6) ^ int_bit(crc,9) ^ int_bit(crc,11) ^ int_bit(crc,13) ^ int_bit(crc,15) ^ int_bit(crc,18) ^ int_bit(crc,19) ^ int_bit(crc,22) ^ int_bit(crc,23) ^ int_bit(crc,25) ^ int_bit(crc,29) ^ int_bit(crc,30) ^ int_bit(crc,31)) & (0x1)) << 1;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,126) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,56) ^ high_long_bit(tuple,55) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,44) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,2) ^ int_bit(crc,6) ^ int_bit(crc,7) ^ int_bit(crc,10) ^ int_bit(crc,12) ^ int_bit(crc,14) ^ int_bit(crc,16) ^ int_bit(crc,19) ^ int_bit(crc,20) ^ int_bit(crc,23) ^ int_bit(crc,24) ^ int_bit(crc,26) ^ int_bit(crc,30) ^ int_bit(crc,31)) & (0x1)) << 2;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,57) ^ high_long_bit(tuple,56) ^ high_long_bit(tuple,54) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,3) ^ int_bit(crc,0) ^ int_bit(crc,7) ^ int_bit(crc,8) ^ int_bit(crc,11) ^ int_bit(crc,13) ^ int_bit(crc,15) ^ int_bit(crc,17) ^ int_bit(crc,20) ^ int_bit(crc,21) ^ int_bit(crc,24) ^ int_bit(crc,25) ^ int_bit(crc,27) ^ int_bit(crc,31)) & (0x1)) << 3;
    hash_id += ((high_long_bit(tuple,124) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,58) ^ high_long_bit(tuple,57) ^ high_long_bit(tuple,55) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,4) ^ int_bit(crc,0) ^ int_bit(crc,1) ^ int_bit(crc,8) ^ int_bit(crc,9) ^ int_bit(crc,12) ^ int_bit(crc,14) ^ int_bit(crc,16) ^ int_bit(crc,18) ^ int_bit(crc,21) ^ int_bit(crc,22) ^ int_bit(crc,25) ^ int_bit(crc,26) ^ int_bit(crc,28)) & (0x1)) << 4;
    hash_id += ((high_long_bit(tuple,125) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,58) ^ high_long_bit(tuple,56) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,5) ^ int_bit(crc,1) ^ int_bit(crc,2) ^ int_bit(crc,9) ^ int_bit(crc,10) ^ int_bit(crc,13) ^ int_bit(crc,15) ^ int_bit(crc,17) ^ int_bit(crc,19) ^ int_bit(crc,22) ^ int_bit(crc,23) ^ int_bit(crc,26) ^ int_bit(crc,27) ^ int_bit(crc,29)) & (0x1)) << 5;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,57) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,0) ^ int_bit(crc,2) ^ int_bit(crc,3) ^ int_bit(crc,4) ^ int_bit(crc,5) ^ int_bit(crc,8) ^ int_bit(crc,11) ^ int_bit(crc,12) ^ int_bit(crc,16) ^ int_bit(crc,17) ^ int_bit(crc,20) ^ int_bit(crc,21) ^ int_bit(crc,22) ^ int_bit(crc,23) ^ int_bit(crc,27) ^ int_bit(crc,29) ^ int_bit(crc,31)) & (0x1)) << 6;
    hash_id += ((high_long_bit(tuple,126) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,58) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,1) ^ int_bit(crc,3) ^ int_bit(crc,4) ^ int_bit(crc,5) ^ int_bit(crc,6) ^ int_bit(crc,9) ^ int_bit(crc,12) ^ int_bit(crc,13) ^ int_bit(crc,17) ^ int_bit(crc,18) ^ int_bit(crc,21) ^ int_bit(crc,22) ^ int_bit(crc,23) ^ int_bit(crc,24) ^ int_bit(crc,28) ^ int_bit(crc,30)) & (0x1)) << 7;
    hash_id += ((high_long_bit(tuple,126) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,0) ^ int_bit(crc,0) ^ int_bit(crc,6) ^ int_bit(crc,7) ^ int_bit(crc,8) ^ int_bit(crc,12) ^ int_bit(crc,13) ^ int_bit(crc,17) ^ int_bit(crc,19) ^ int_bit(crc,21) ^ int_bit(crc,23) ^ int_bit(crc,25) ^ int_bit(crc,28) ^ int_bit(crc,30)) & (0x1)) << 8;
    hash_id += ((high_long_bit(tuple,126) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,1) ^ high_long_bit(tuple,0) ^ int_bit(crc,1) ^ int_bit(crc,4) ^ int_bit(crc,5) ^ int_bit(crc,7) ^ int_bit(crc,9) ^ int_bit(crc,10) ^ int_bit(crc,12) ^ int_bit(crc,13) ^ int_bit(crc,17) ^ int_bit(crc,20) ^ int_bit(crc,21) ^ int_bit(crc,26) ^ int_bit(crc,28) ^ int_bit(crc,30)) & (0x1)) << 9;
    hash_id += ((high_long_bit(tuple,126) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,1) ^ high_long_bit(tuple,0) ^ int_bit(crc,0) ^ int_bit(crc,2) ^ int_bit(crc,4) ^ int_bit(crc,6) ^ int_bit(crc,11) ^ int_bit(crc,12) ^ int_bit(crc,13) ^ int_bit(crc,17) ^ int_bit(crc,24) ^ int_bit(crc,27) ^ int_bit(crc,28) ^ int_bit(crc,30)) & (0x1)) << 10;
    hash_id += ((high_long_bit(tuple,126) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,1) ^ high_long_bit(tuple,0) ^ int_bit(crc,0) ^ int_bit(crc,1) ^ int_bit(crc,3) ^ int_bit(crc,4) ^ int_bit(crc,7) ^ int_bit(crc,8) ^ int_bit(crc,10) ^ int_bit(crc,13) ^ int_bit(crc,17) ^ int_bit(crc,21) ^ int_bit(crc,22) ^ int_bit(crc,24) ^ int_bit(crc,25) ^ int_bit(crc,30)) & (0x1)) << 11;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,43) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,1) ^ int_bit(crc,0) ^ int_bit(crc,1) ^ int_bit(crc,2) ^ int_bit(crc,4) ^ int_bit(crc,5) ^ int_bit(crc,8) ^ int_bit(crc,9) ^ int_bit(crc,11) ^ int_bit(crc,14) ^ int_bit(crc,18) ^ int_bit(crc,22) ^ int_bit(crc,23) ^ int_bit(crc,25) ^ int_bit(crc,26) ^ int_bit(crc,31)) & (0x1)) << 12;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,126) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,54) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,44) ^ high_long_bit(tuple,43) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,0) ^ int_bit(crc,1) ^ int_bit(crc,2) ^ int_bit(crc,3) ^ int_bit(crc,4) ^ int_bit(crc,6) ^ int_bit(crc,8) ^ int_bit(crc,9) ^ int_bit(crc,14) ^ int_bit(crc,15) ^ int_bit(crc,17) ^ int_bit(crc,18) ^ int_bit(crc,19) ^ int_bit(crc,21) ^ int_bit(crc,22) ^ int_bit(crc,23) ^ int_bit(crc,26) ^ int_bit(crc,27) ^ int_bit(crc,28) ^ int_bit(crc,29) ^ int_bit(crc,30) ^ int_bit(crc,31)) & (0x1)) << 13;
    hash_id += ((high_long_bit(tuple,123) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,55) ^ high_long_bit(tuple,54) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,44) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,1) ^ high_long_bit(tuple,0) ^ int_bit(crc,2) ^ int_bit(crc,3) ^ int_bit(crc,7) ^ int_bit(crc,8) ^ int_bit(crc,9) ^ int_bit(crc,12) ^ int_bit(crc,14) ^ int_bit(crc,15) ^ int_bit(crc,16) ^ int_bit(crc,17) ^ int_bit(crc,19) ^ int_bit(crc,20) ^ int_bit(crc,21) ^ int_bit(crc,23) ^ int_bit(crc,27)) & (0x1)) << 14;
    hash_id += ((high_long_bit(tuple,124) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,56) ^ high_long_bit(tuple,55) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,1) ^ int_bit(crc,3) ^ int_bit(crc,4) ^ int_bit(crc,8) ^ int_bit(crc,9) ^ int_bit(crc,10) ^ int_bit(crc,13) ^ int_bit(crc,15) ^ int_bit(crc,16) ^ int_bit(crc,17) ^ int_bit(crc,18) ^ int_bit(crc,20) ^ int_bit(crc,21) ^ int_bit(crc,22) ^ int_bit(crc,24) ^ int_bit(crc,28)) & (0x1)) << 15;
    hash_id += ((high_long_bit(tuple,125) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,57) ^ high_long_bit(tuple,56) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,2) ^ int_bit(crc,0) ^ int_bit(crc,4) ^ int_bit(crc,5) ^ int_bit(crc,9) ^ int_bit(crc,10) ^ int_bit(crc,11) ^ int_bit(crc,14) ^ int_bit(crc,16) ^ int_bit(crc,17) ^ int_bit(crc,18) ^ int_bit(crc,19) ^ int_bit(crc,21) ^ int_bit(crc,22) ^ int_bit(crc,23) ^ int_bit(crc,25) ^ int_bit(crc,29)) & (0x1)) << 16;
    hash_id += ((high_long_bit(tuple,126) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,58) ^ high_long_bit(tuple,57) ^ high_long_bit(tuple,54) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,3) ^ int_bit(crc,0) ^ int_bit(crc,1) ^ int_bit(crc,5) ^ int_bit(crc,6) ^ int_bit(crc,10) ^ int_bit(crc,11) ^ int_bit(crc,12) ^ int_bit(crc,15) ^ int_bit(crc,17) ^ int_bit(crc,18) ^ int_bit(crc,19) ^ int_bit(crc,20) ^ int_bit(crc,22) ^ int_bit(crc,23) ^ int_bit(crc,24) ^ int_bit(crc,26) ^ int_bit(crc,30)) & (0x1)) << 17;
    hash_id += ((high_long_bit(tuple,126) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,58) ^ high_long_bit(tuple,55) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,43) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,0) ^ int_bit(crc,0) ^ int_bit(crc,1) ^ int_bit(crc,2) ^ int_bit(crc,4) ^ int_bit(crc,5) ^ int_bit(crc,6) ^ int_bit(crc,7) ^ int_bit(crc,8) ^ int_bit(crc,10) ^ int_bit(crc,11) ^ int_bit(crc,13) ^ int_bit(crc,14) ^ int_bit(crc,16) ^ int_bit(crc,17) ^ int_bit(crc,19) ^ int_bit(crc,20) ^ int_bit(crc,22) ^ int_bit(crc,23) ^ int_bit(crc,25) ^ int_bit(crc,27) ^ int_bit(crc,28) ^ int_bit(crc,29) ^ int_bit(crc,30)) & (0x1)) << 18;
    hash_id += ((high_long_bit(tuple,122) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,56) ^ high_long_bit(tuple,54) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,44) ^ high_long_bit(tuple,43) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,1) ^ high_long_bit(tuple,0) ^ int_bit(crc,1) ^ int_bit(crc,2) ^ int_bit(crc,3) ^ int_bit(crc,4) ^ int_bit(crc,6) ^ int_bit(crc,7) ^ int_bit(crc,9) ^ int_bit(crc,10) ^ int_bit(crc,11) ^ int_bit(crc,15) ^ int_bit(crc,20) ^ int_bit(crc,22) ^ int_bit(crc,23) ^ int_bit(crc,26)) & (0x1)) << 19;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,126) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,57) ^ high_long_bit(tuple,55) ^ high_long_bit(tuple,54) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,44) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,1) ^ high_long_bit(tuple,0) ^ int_bit(crc,0) ^ int_bit(crc,2) ^ int_bit(crc,3) ^ int_bit(crc,7) ^ int_bit(crc,11) ^ int_bit(crc,14) ^ int_bit(crc,16) ^ int_bit(crc,17) ^ int_bit(crc,18) ^ int_bit(crc,22) ^ int_bit(crc,23) ^ int_bit(crc,27) ^ int_bit(crc,28) ^ int_bit(crc,29) ^ int_bit(crc,30) ^ int_bit(crc,31)) & (0x1)) << 20;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,126) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,58) ^ high_long_bit(tuple,56) ^ high_long_bit(tuple,55) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,1) ^ int_bit(crc,1) ^ int_bit(crc,3) ^ int_bit(crc,4) ^ int_bit(crc,8) ^ int_bit(crc,12) ^ int_bit(crc,15) ^ int_bit(crc,17) ^ int_bit(crc,18) ^ int_bit(crc,19) ^ int_bit(crc,23) ^ int_bit(crc,24) ^ int_bit(crc,28) ^ int_bit(crc,29) ^ int_bit(crc,30) ^ int_bit(crc,31)) & (0x1)) << 21;
    hash_id += ((high_long_bit(tuple,124) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,57) ^ high_long_bit(tuple,56) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,0) ^ int_bit(crc,0) ^ int_bit(crc,2) ^ int_bit(crc,8) ^ int_bit(crc,9) ^ int_bit(crc,10) ^ int_bit(crc,12) ^ int_bit(crc,13) ^ int_bit(crc,14) ^ int_bit(crc,16) ^ int_bit(crc,17) ^ int_bit(crc,19) ^ int_bit(crc,20) ^ int_bit(crc,21) ^ int_bit(crc,22) ^ int_bit(crc,25) ^ int_bit(crc,28)) & (0x1)) << 22;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,126) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,58) ^ high_long_bit(tuple,57) ^ high_long_bit(tuple,54) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,1) ^ high_long_bit(tuple,0) ^ int_bit(crc,1) ^ int_bit(crc,3) ^ int_bit(crc,4) ^ int_bit(crc,5) ^ int_bit(crc,8) ^ int_bit(crc,9) ^ int_bit(crc,11) ^ int_bit(crc,12) ^ int_bit(crc,13) ^ int_bit(crc,15) ^ int_bit(crc,20) ^ int_bit(crc,23) ^ int_bit(crc,24) ^ int_bit(crc,26) ^ int_bit(crc,28) ^ int_bit(crc,30) ^ int_bit(crc,31)) & (0x1)) << 23;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,58) ^ high_long_bit(tuple,55) ^ high_long_bit(tuple,54) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,1) ^ int_bit(crc,2) ^ int_bit(crc,4) ^ int_bit(crc,5) ^ int_bit(crc,6) ^ int_bit(crc,9) ^ int_bit(crc,10) ^ int_bit(crc,12) ^ int_bit(crc,13) ^ int_bit(crc,14) ^ int_bit(crc,16) ^ int_bit(crc,21) ^ int_bit(crc,24) ^ int_bit(crc,25) ^ int_bit(crc,27) ^ int_bit(crc,29) ^ int_bit(crc,31)) & (0x1)) << 24;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,93) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,56) ^ high_long_bit(tuple,55) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,0) ^ int_bit(crc,3) ^ int_bit(crc,4) ^ int_bit(crc,6) ^ int_bit(crc,7) ^ int_bit(crc,8) ^ int_bit(crc,11) ^ int_bit(crc,12) ^ int_bit(crc,13) ^ int_bit(crc,15) ^ int_bit(crc,18) ^ int_bit(crc,21) ^ int_bit(crc,24) ^ int_bit(crc,25) ^ int_bit(crc,26) ^ int_bit(crc,29) ^ int_bit(crc,31)) & (0x1)) << 25;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,94) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,57) ^ high_long_bit(tuple,56) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,43) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,1) ^ high_long_bit(tuple,0) ^ int_bit(crc,7) ^ int_bit(crc,9) ^ int_bit(crc,10) ^ int_bit(crc,13) ^ int_bit(crc,16) ^ int_bit(crc,17) ^ int_bit(crc,18) ^ int_bit(crc,19) ^ int_bit(crc,21) ^ int_bit(crc,24) ^ int_bit(crc,25) ^ int_bit(crc,26) ^ int_bit(crc,27) ^ int_bit(crc,28) ^ int_bit(crc,29) ^ int_bit(crc,31)) & (0x1)) << 26;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,95) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,58) ^ high_long_bit(tuple,57) ^ high_long_bit(tuple,54) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,44) ^ high_long_bit(tuple,43) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,37) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,1) ^ high_long_bit(tuple,0) ^ int_bit(crc,0) ^ int_bit(crc,4) ^ int_bit(crc,5) ^ int_bit(crc,11) ^ int_bit(crc,12) ^ int_bit(crc,19) ^ int_bit(crc,20) ^ int_bit(crc,21) ^ int_bit(crc,24) ^ int_bit(crc,25) ^ int_bit(crc,26) ^ int_bit(crc,27) ^ int_bit(crc,31)) & (0x1)) << 27;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,126) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,120) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,96) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,88) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,71) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,58) ^ high_long_bit(tuple,55) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,44) ^ high_long_bit(tuple,43) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,38) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,31) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,12) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,1) ^ high_long_bit(tuple,0) ^ int_bit(crc,0) ^ int_bit(crc,1) ^ int_bit(crc,4) ^ int_bit(crc,6) ^ int_bit(crc,8) ^ int_bit(crc,10) ^ int_bit(crc,13) ^ int_bit(crc,14) ^ int_bit(crc,17) ^ int_bit(crc,18) ^ int_bit(crc,20) ^ int_bit(crc,24) ^ int_bit(crc,25) ^ int_bit(crc,26) ^ int_bit(crc,27) ^ int_bit(crc,29) ^ int_bit(crc,30) ^ int_bit(crc,31)) & (0x1)) << 28;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,126) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,121) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,114) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,110) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,101) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,97) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,89) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,84) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,75) ^ high_long_bit(tuple,72) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,59) ^ high_long_bit(tuple,56) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,48) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,44) ^ high_long_bit(tuple,43) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,39) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,32) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,18) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,13) ^ high_long_bit(tuple,9) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,2) ^ high_long_bit(tuple,1) ^ int_bit(crc,1) ^ int_bit(crc,2) ^ int_bit(crc,5) ^ int_bit(crc,7) ^ int_bit(crc,9) ^ int_bit(crc,11) ^ int_bit(crc,14) ^ int_bit(crc,15) ^ int_bit(crc,18) ^ int_bit(crc,19) ^ int_bit(crc,21) ^ int_bit(crc,25) ^ int_bit(crc,26) ^ int_bit(crc,27) ^ int_bit(crc,28) ^ int_bit(crc,30) ^ int_bit(crc,31)) & (0x1)) << 29;
    hash_id += ((high_long_bit(tuple,127) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,122) ^ high_long_bit(tuple,118) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,115) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,111) ^ high_long_bit(tuple,108) ^ high_long_bit(tuple,106) ^ high_long_bit(tuple,104) ^ high_long_bit(tuple,102) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,98) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,90) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,85) ^ high_long_bit(tuple,82) ^ high_long_bit(tuple,80) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,76) ^ high_long_bit(tuple,73) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,66) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,62) ^ high_long_bit(tuple,60) ^ high_long_bit(tuple,57) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,51) ^ high_long_bit(tuple,49) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,44) ^ high_long_bit(tuple,43) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,40) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,33) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,28) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,23) ^ high_long_bit(tuple,21) ^ high_long_bit(tuple,19) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,14) ^ high_long_bit(tuple,10) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,3) ^ high_long_bit(tuple,2) ^ int_bit(crc,2) ^ int_bit(crc,3) ^ int_bit(crc,6) ^ int_bit(crc,8) ^ int_bit(crc,10) ^ int_bit(crc,12) ^ int_bit(crc,15) ^ int_bit(crc,16) ^ int_bit(crc,19) ^ int_bit(crc,20) ^ int_bit(crc,22) ^ int_bit(crc,26) ^ int_bit(crc,27) ^ int_bit(crc,28) ^ int_bit(crc,29) ^ int_bit(crc,31)) & (0x1)) << 30;
    hash_id += ((high_long_bit(tuple,126) ^ high_long_bit(tuple,125) ^ high_long_bit(tuple,124) ^ high_long_bit(tuple,123) ^ high_long_bit(tuple,119) ^ high_long_bit(tuple,117) ^ high_long_bit(tuple,116) ^ high_long_bit(tuple,113) ^ high_long_bit(tuple,112) ^ high_long_bit(tuple,109) ^ high_long_bit(tuple,107) ^ high_long_bit(tuple,105) ^ high_long_bit(tuple,103) ^ high_long_bit(tuple,100) ^ high_long_bit(tuple,99) ^ high_long_bit(tuple,92) ^ high_long_bit(tuple,91) ^ high_long_bit(tuple,87) ^ high_long_bit(tuple,86) ^ high_long_bit(tuple,83) ^ high_long_bit(tuple,81) ^ high_long_bit(tuple,79) ^ high_long_bit(tuple,78) ^ high_long_bit(tuple,77) ^ high_long_bit(tuple,74) ^ high_long_bit(tuple,70) ^ high_long_bit(tuple,69) ^ high_long_bit(tuple,68) ^ high_long_bit(tuple,67) ^ high_long_bit(tuple,65) ^ high_long_bit(tuple,64) ^ high_long_bit(tuple,63) ^ high_long_bit(tuple,61) ^ high_long_bit(tuple,58) ^ high_long_bit(tuple,53) ^ high_long_bit(tuple,52) ^ high_long_bit(tuple,50) ^ high_long_bit(tuple,47) ^ high_long_bit(tuple,46) ^ high_long_bit(tuple,45) ^ high_long_bit(tuple,44) ^ high_long_bit(tuple,42) ^ high_long_bit(tuple,41) ^ high_long_bit(tuple,36) ^ high_long_bit(tuple,35) ^ high_long_bit(tuple,34) ^ high_long_bit(tuple,30) ^ high_long_bit(tuple,29) ^ high_long_bit(tuple,27) ^ high_long_bit(tuple,26) ^ high_long_bit(tuple,25) ^ high_long_bit(tuple,24) ^ high_long_bit(tuple,22) ^ high_long_bit(tuple,20) ^ high_long_bit(tuple,17) ^ high_long_bit(tuple,16) ^ high_long_bit(tuple,15) ^ high_long_bit(tuple,11) ^ high_long_bit(tuple,8) ^ high_long_bit(tuple,7) ^ high_long_bit(tuple,6) ^ high_long_bit(tuple,5) ^ high_long_bit(tuple,4) ^ high_long_bit(tuple,3) ^ int_bit(crc,3) ^ int_bit(crc,4) ^ int_bit(crc,7) ^ int_bit(crc,9) ^ int_bit(crc,11) ^ int_bit(crc,13) ^ int_bit(crc,16) ^ int_bit(crc,17) ^ int_bit(crc,20) ^ int_bit(crc,21) ^ int_bit(crc,23) ^ int_bit(crc,27) ^ int_bit(crc,28) ^ int_bit(crc,29) ^ int_bit(crc,30)) & (0x1)) << 31;
    return hash_id;
}
/*get fpga hash id :crc32c_modify end*/


/* get fpga hash id :crc32 algorithm*/
unsigned int crc_get_hash_id(unsigned long long data,unsigned int crc,unsigned char add_data)
{
    unsigned int hash_id;

	//bit 0
    hash_id = (long_bit(data,63)^long_bit(data,61)^long_bit(data,60)^long_bit(data,58)^long_bit(data,55)^long_bit(data,54)^long_bit(data,53)^long_bit(data,50)\
		^long_bit(data,48)^long_bit(data,47)^long_bit(data,45)^long_bit(data,44)^long_bit(data,37)^long_bit(data,34)^long_bit(data,32)^long_bit(data,31)\
		^long_bit(data,30)^long_bit(data,29)^long_bit(data,28)^long_bit(data,26)^long_bit(data,25)^long_bit(data,24)^long_bit(data,16)^long_bit(data,12)\
		^long_bit(data,10)^long_bit(data,9)^long_bit(data,6)^long_bit(data,0)^int_bit(crc,0)^int_bit(crc,2)^int_bit(crc,5)^int_bit(crc,12)\
		^int_bit(crc,13)^int_bit(crc,15)^int_bit(crc,16)^int_bit(crc,18)^int_bit(crc,21)^int_bit(crc,22)^int_bit(crc,23)^int_bit(crc,26)\
		^int_bit(crc,28)^int_bit(crc,29)^int_bit(crc,31)^short_bit(add_data,0))&(1);
	//printf("hash_id 0 = %x\n",hash_id);
	//bit 1
	hash_id += ((long_bit(data,63)^long_bit(data,62)^long_bit(data,60)^long_bit(data,59)^long_bit(data,58)^long_bit(data,56)^long_bit(data,53)^long_bit(data,51)\
		^long_bit(data,50)^long_bit(data,49)^long_bit(data,47)^long_bit(data,46)^long_bit(data,44)^long_bit(data,38)^long_bit(data,37)^long_bit(data,35)\
		^long_bit(data,34)^long_bit(data,33)^long_bit(data,28)^long_bit(data,27)^long_bit(data,24)^long_bit(data,17)^long_bit(data,16)^long_bit(data,13)\
		^long_bit(data,12)^long_bit(data,11)^long_bit(data,9)^long_bit(data,7)^long_bit(data,6)^long_bit(data,1)^long_bit(data,0)^int_bit(crc,1)^int_bit(crc,2)^int_bit(crc,3)^int_bit(crc,5)\
		^int_bit(crc,6)^int_bit(crc,12)^int_bit(crc,14)^int_bit(crc,15)^int_bit(crc,17)^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,21)\
		^int_bit(crc,24)^int_bit(crc,26)^int_bit(crc,27)^int_bit(crc,28)^int_bit(crc,30)^int_bit(crc,31)) &(1))<<1;
	//printf("hash_id 1 = %x\n",hash_id);
	//bit 2
	hash_id += ((long_bit(data,59)^long_bit(data,58)^long_bit(data,57)^long_bit(data,55)^long_bit(data,53)^long_bit(data,52)^long_bit(data,51)^long_bit(data,44)\
		^long_bit(data,39)^long_bit(data,38)^long_bit(data,37)^long_bit(data,36)^long_bit(data,35)^long_bit(data,32)^long_bit(data,31)^long_bit(data,30)\
		^long_bit(data,26)^long_bit(data,24)^long_bit(data,18)^long_bit(data,17)^long_bit(data,16)^long_bit(data,14)^long_bit(data,13)^long_bit(data,9)\
		^long_bit(data,8)^long_bit(data,7)^long_bit(data,6)^long_bit(data,2)^long_bit(data,1)^long_bit(data,0)^int_bit(crc,0)^int_bit(crc,3)^int_bit(crc,4)^int_bit(crc,5)\
		^int_bit(crc,6)^int_bit(crc,7)^int_bit(crc,12)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,21)^int_bit(crc,23)^int_bit(crc,25)^int_bit(crc,26)^int_bit(crc,27)) & (1))<<2;
    //printf("hash_id 2 = %x\n",hash_id);
	//bit 3
	hash_id += ((long_bit(data,60)^long_bit(data,59)^long_bit(data,58)^long_bit(data,56)^long_bit(data,54)^long_bit(data,53)^long_bit(data,52)^long_bit(data,45)\
		^long_bit(data,40)^long_bit(data,39)^long_bit(data,38)^long_bit(data,37)^long_bit(data,36)^long_bit(data,33)^long_bit(data,32)^long_bit(data,31)\
		^long_bit(data,27)^long_bit(data,25)^long_bit(data,19)^long_bit(data,18)^long_bit(data,17)^long_bit(data,15)^long_bit(data,14)^long_bit(data,10)\
		^long_bit(data,9)^long_bit(data,8)^long_bit(data,7)^long_bit(data,3)^long_bit(data,2)^long_bit(data,1)^int_bit(crc,0)^int_bit(crc,1)^int_bit(crc,4)^int_bit(crc,5)\
		^int_bit(crc,6)^int_bit(crc,7)^int_bit(crc,8)^int_bit(crc,13)^int_bit(crc,20)^int_bit(crc,21)^int_bit(crc,22)^int_bit(crc,24)^int_bit(crc,26)^int_bit(crc,27)^int_bit(crc,28)\
		^short_bit(add_data,1))& (1))<<3;	
    //printf("hash_id 3 = %x\n",hash_id);
	//bit 4
	hash_id += ((long_bit(data,63)^long_bit(data,59)^long_bit(data,58)^long_bit(data,57)^long_bit(data,50)^long_bit(data,48)^long_bit(data,47)^long_bit(data,46)\
		^long_bit(data,45)^long_bit(data,44)^long_bit(data,41)^long_bit(data,40)^long_bit(data,39)^long_bit(data,38)^long_bit(data,33)^long_bit(data,31)\
		^long_bit(data,30)^long_bit(data,29)^long_bit(data,25)^long_bit(data,24)^long_bit(data,20)^long_bit(data,19)^long_bit(data,18)^long_bit(data,15)\
		^long_bit(data,12)^long_bit(data,11)^long_bit(data,8)^long_bit(data,6)^long_bit(data,4)^long_bit(data,3)^long_bit(data,2)^long_bit(data,0)^int_bit(crc,1)^int_bit(crc,6)^int_bit(crc,7)^int_bit(crc,8)\
		^int_bit(crc,9)^int_bit(crc,12)^int_bit(crc,13)^int_bit(crc,14)^int_bit(crc,15)^int_bit(crc,16)^int_bit(crc,18)^int_bit(crc,25)^int_bit(crc,26)^int_bit(crc,27)^int_bit(crc,31))& (1))<<4;
    //printf("hash_id 4 = %x\n",hash_id);
	//bit 5
	hash_id += ((long_bit(data,63)^long_bit(data,61)^long_bit(data,59)^long_bit(data,55)^long_bit(data,54)^long_bit(data,53)^long_bit(data,51)^long_bit(data,50)\
		^long_bit(data,49)^long_bit(data,46)^long_bit(data,44)^long_bit(data,42)^long_bit(data,41)^long_bit(data,40)^long_bit(data,39)^long_bit(data,37)\
		^long_bit(data,29)^long_bit(data,28)^long_bit(data,24)^long_bit(data,21)^long_bit(data,20)^long_bit(data,19)^long_bit(data,13)^long_bit(data,10)\
		^long_bit(data,7)^long_bit(data,6)^long_bit(data,5)^long_bit(data,4)^long_bit(data,3)^long_bit(data,1)^long_bit(data,0)^int_bit(crc,5)^int_bit(crc,7)^int_bit(crc,8)^int_bit(crc,9)\
		^int_bit(crc,10)^int_bit(crc,12)^int_bit(crc,14)^int_bit(crc,17)^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,21)^int_bit(crc,22)^int_bit(crc,23)^int_bit(crc,27)^int_bit(crc,29)^int_bit(crc,31))& (1))<<5;
    //printf("hash_id 5 = %x\n",hash_id);
	//bit 6
	hash_id += ((long_bit(data,62)^long_bit(data,60)^long_bit(data,56)^long_bit(data,55)^long_bit(data,54)^long_bit(data,52)^long_bit(data,51)^long_bit(data,50)\
		^long_bit(data,47)^long_bit(data,45)^long_bit(data,43)^long_bit(data,42)^long_bit(data,41)^long_bit(data,40)^long_bit(data,38)^long_bit(data,30)\
		^long_bit(data,29)^long_bit(data,25)^long_bit(data,22)^long_bit(data,21)^long_bit(data,20)^long_bit(data,14)^long_bit(data,11)^long_bit(data,8)\
		^long_bit(data,7)^long_bit(data,6)^long_bit(data,5)^long_bit(data,4)^long_bit(data,2)^long_bit(data,1)^int_bit(crc,6)^int_bit(crc,8)^int_bit(crc,9)^int_bit(crc,10)\
		^int_bit(crc,11)^int_bit(crc,13)^int_bit(crc,15)^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,22)^int_bit(crc,23)^int_bit(crc,24)^int_bit(crc,28)^int_bit(crc,30)^short_bit(add_data,2))& (1))<<6;
    //printf("hash_id 6 = %x\n",hash_id);
	//bit 7
	hash_id += ((long_bit(data,60)^long_bit(data,58)^long_bit(data,57)^long_bit(data,56)^long_bit(data,54)^long_bit(data,52)^long_bit(data,51)^long_bit(data,50)\
		^long_bit(data,47)^long_bit(data,46)^long_bit(data,45)^long_bit(data,43)^long_bit(data,42)^long_bit(data,41)^long_bit(data,39)^long_bit(data,37)\
		^long_bit(data,34)^long_bit(data,32)^long_bit(data,29)^long_bit(data,28)^long_bit(data,25)^long_bit(data,24)^long_bit(data,23)^long_bit(data,22)\
		^long_bit(data,21)^long_bit(data,16)^long_bit(data,15)^long_bit(data,10)^long_bit(data,8)^long_bit(data,7)^long_bit(data,5)^long_bit(data,3)^long_bit(data,2)^long_bit(data,0)^int_bit(crc,0)^int_bit(crc,2)^int_bit(crc,5)^int_bit(crc,7)\
		^int_bit(crc,9)^int_bit(crc,10)^int_bit(crc,11)^int_bit(crc,13)^int_bit(crc,14)^int_bit(crc,15)^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,22)^int_bit(crc,24)^int_bit(crc,25)^int_bit(crc,26)^int_bit(crc,28)) & (1))<<7;
    //printf("hash_id 7 = %x\n",hash_id);
	//bit 8
	hash_id += ((long_bit(data,63)^long_bit(data,60)^long_bit(data,59)^long_bit(data,57)^long_bit(data,54)^long_bit(data,52)^long_bit(data,51)^long_bit(data,50)\
		^long_bit(data,46)^long_bit(data,45)^long_bit(data,43)^long_bit(data,42)^long_bit(data,40)^long_bit(data,38)^long_bit(data,37)^long_bit(data,35)\
		^long_bit(data,34)^long_bit(data,33)^long_bit(data,32)^long_bit(data,31)^long_bit(data,28)^long_bit(data,23)^long_bit(data,22)^long_bit(data,17)\
		^long_bit(data,12)^long_bit(data,11)^long_bit(data,10)^long_bit(data,8)^long_bit(data,4)^long_bit(data,3)^long_bit(data,1)^long_bit(data,0)^int_bit(crc,0)^int_bit(crc,1)^int_bit(crc,2)^int_bit(crc,3)\
		^int_bit(crc,5)^int_bit(crc,6)^int_bit(crc,8)^int_bit(crc,10)^int_bit(crc,11)^int_bit(crc,13)^int_bit(crc,14)^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,22)^int_bit(crc,25)^int_bit(crc,27)^int_bit(crc,28)^int_bit(crc,31)) & (1))<<8;
    //printf("hash_id 8 = %x\n",hash_id);
	//bit 9
	hash_id += ((long_bit(data,61)^long_bit(data,60)^long_bit(data,58)^long_bit(data,55)^long_bit(data,53)^long_bit(data,52)^long_bit(data,51)^long_bit(data,47)\
		^long_bit(data,46)^long_bit(data,44)^long_bit(data,43)^long_bit(data,41)^long_bit(data,39)^long_bit(data,38)^long_bit(data,36)^long_bit(data,35)\
		^long_bit(data,34)^long_bit(data,33)^long_bit(data,32)^long_bit(data,29)^long_bit(data,24)^long_bit(data,23)^long_bit(data,18)^long_bit(data,13)\
		^long_bit(data,12)^long_bit(data,11)^long_bit(data,9)^long_bit(data,5)^long_bit(data,4)^long_bit(data,2)^long_bit(data,1)^int_bit(crc,0)^int_bit(crc,1)^int_bit(crc,2)^int_bit(crc,3)^int_bit(crc,4)\
		^int_bit(crc,6)^int_bit(crc,7)^int_bit(crc,9)^int_bit(crc,11)^int_bit(crc,12)^int_bit(crc,14)^int_bit(crc,15)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,21)^int_bit(crc,23)^int_bit(crc,26)^int_bit(crc,28)^int_bit(crc,29)^short_bit(add_data,3)) & (1))<<9;
    //printf("hash_id 9 = %x\n",hash_id);
	//bit 10
	hash_id += ((long_bit(data,63)^long_bit(data,62)^long_bit(data,60)^long_bit(data,59)^long_bit(data,58)^long_bit(data,56)^long_bit(data,55)^long_bit(data,52)\
		^long_bit(data,50)^long_bit(data,42)^long_bit(data,40)^long_bit(data,39)^long_bit(data,36)^long_bit(data,35)^long_bit(data,33)^long_bit(data,32)\
		^long_bit(data,31)^long_bit(data,29)^long_bit(data,28)^long_bit(data,26)^long_bit(data,19)^long_bit(data,16)^long_bit(data,14)^long_bit(data,13)\
		^long_bit(data,9)^long_bit(data,5)^long_bit(data,3)^long_bit(data,2)^long_bit(data,0)^int_bit(crc,0)^int_bit(crc,1)^int_bit(crc,3)^int_bit(crc,4)\
		^int_bit(crc,7)^int_bit(crc,8)^int_bit(crc,10)^int_bit(crc,18)^int_bit(crc,20)^int_bit(crc,23)^int_bit(crc,24)^int_bit(crc,26)^int_bit(crc,27)^int_bit(crc,28)^int_bit(crc,30)^int_bit(crc,31)) & (1))<<10;
    //printf("hash_id 10 = %x\n",hash_id);
	//bit 11
	hash_id += ((long_bit(data,59)^long_bit(data,58)^long_bit(data,57)^long_bit(data,56)^long_bit(data,55)^long_bit(data,54)^long_bit(data,51)^long_bit(data,50)\
		^long_bit(data,48)^long_bit(data,47)^long_bit(data,45)^long_bit(data,44)^long_bit(data,43)^long_bit(data,41)^long_bit(data,40)^long_bit(data,36)\
		^long_bit(data,33)^long_bit(data,31)^long_bit(data,28)^long_bit(data,27)^long_bit(data,26)^long_bit(data,25)^long_bit(data,24)^long_bit(data,20)\
		^long_bit(data,17)^long_bit(data,16)^long_bit(data,15)^long_bit(data,14)^long_bit(data,12)^long_bit(data,9)^long_bit(data,4)^long_bit(data,3)^long_bit(data,1)^long_bit(data,0)^int_bit(crc,1)^int_bit(crc,4)^int_bit(crc,8)^int_bit(crc,9)\
		^int_bit(crc,11)^int_bit(crc,12)^int_bit(crc,13)^int_bit(crc,15)^int_bit(crc,16)^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,22)^int_bit(crc,23)^int_bit(crc,24)^int_bit(crc,25)^int_bit(crc,26)^int_bit(crc,27)) & (1))<<11;
    //printf("hash_id 11 = %x\n",hash_id);
	//bit 12
	hash_id += ((long_bit(data,63)^long_bit(data,61)^long_bit(data,59)^long_bit(data,57)^long_bit(data,56)^long_bit(data,54)^long_bit(data,53)^long_bit(data,52)\
		^long_bit(data,51)^long_bit(data,50)^long_bit(data,49)^long_bit(data,47)^long_bit(data,46)^long_bit(data,42)^long_bit(data,41)^long_bit(data,31)\
		^long_bit(data,30)^long_bit(data,27)^long_bit(data,24)^long_bit(data,21)^long_bit(data,18)^long_bit(data,17)^long_bit(data,15)^long_bit(data,13)\
		^long_bit(data,12)^long_bit(data,9)^long_bit(data,6)^long_bit(data,5)^long_bit(data,4)^long_bit(data,2)^long_bit(data,1)^long_bit(data,0)^int_bit(crc,9)^int_bit(crc,10)^int_bit(crc,14)^int_bit(crc,15)\
		^int_bit(crc,17)^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,21)^int_bit(crc,22)^int_bit(crc,24)^int_bit(crc,25)^int_bit(crc,27)^int_bit(crc,29)^int_bit(crc,31)^short_bit(add_data,4)) & (1))<<12;
    //printf("hash_id 12 = %x\n",hash_id);
	//bit 13
	hash_id += ((long_bit(data,62)^long_bit(data,60)^long_bit(data,58)^long_bit(data,57)^long_bit(data,55)^long_bit(data,54)^long_bit(data,53)^long_bit(data,52)\
		^long_bit(data,51)^long_bit(data,50)^long_bit(data,48)^long_bit(data,47)^long_bit(data,43)^long_bit(data,42)^long_bit(data,32)^long_bit(data,31)\
		^long_bit(data,28)^long_bit(data,25)^long_bit(data,22)^long_bit(data,19)^long_bit(data,18)^long_bit(data,16)^long_bit(data,14)^long_bit(data,13)\
		^long_bit(data,10)^long_bit(data,7)^long_bit(data,6)^long_bit(data,5)^long_bit(data,3)^long_bit(data,2)^long_bit(data,1)^int_bit(crc,0)^int_bit(crc,10)^int_bit(crc,11)^int_bit(crc,15)\
		^int_bit(crc,16)^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,21)^int_bit(crc,22)^int_bit(crc,23)^int_bit(crc,25)^int_bit(crc,26)^int_bit(crc,28)^int_bit(crc,30)) & (1))<<13;
    //printf("hash_id 13 = %x\n",hash_id);
	//bit 14
	hash_id += ((long_bit(data,63)^long_bit(data,61)^long_bit(data,59)^long_bit(data,58)^long_bit(data,56)^long_bit(data,55)^long_bit(data,54)^long_bit(data,53)\
		^long_bit(data,52)^long_bit(data,51)^long_bit(data,49)^long_bit(data,48)^long_bit(data,44)^long_bit(data,43)^long_bit(data,33)^long_bit(data,32)\
		^long_bit(data,29)^long_bit(data,26)^long_bit(data,23)^long_bit(data,20)^long_bit(data,19)^long_bit(data,17)^long_bit(data,15)^long_bit(data,14)\
		^long_bit(data,11)^long_bit(data,8)^long_bit(data,7)^long_bit(data,6)^long_bit(data,4)^long_bit(data,3)^long_bit(data,2)^int_bit(crc,0)^int_bit(crc,1)^int_bit(crc,11)^int_bit(crc,12)\
		^int_bit(crc,16)^int_bit(crc,17)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,21)^int_bit(crc,22)^int_bit(crc,23)^int_bit(crc,24)^int_bit(crc,26)^int_bit(crc,27)^int_bit(crc,29)^int_bit(crc,31)) & (1))<<14;
    //printf("hash_id 14 = %x\n",hash_id);
	//bit 15
	hash_id += ((long_bit(data,62)^long_bit(data,60)^long_bit(data,59)^long_bit(data,57)^long_bit(data,56)\
	^long_bit(data,55)^long_bit(data,54)^long_bit(data,53)^long_bit(data,52)^long_bit(data,50)\
	^long_bit(data,49)^long_bit(data,45)^long_bit(data,44)^long_bit(data,34)^long_bit(data,33)\
	^long_bit(data,30)^long_bit(data,27)^long_bit(data,24)^long_bit(data,21)^long_bit(data,20)\
	^long_bit(data,18)^long_bit(data,16)^long_bit(data,15)^long_bit(data,12)^long_bit(data,9)\
	^long_bit(data,8)^long_bit(data,7)^long_bit(data,5)^long_bit(data,4)^long_bit(data,3)\
	^int_bit(crc,1)^int_bit(crc,2)^int_bit(crc,12)^int_bit(crc,13)^int_bit(crc,17)^int_bit(crc,18)\
	^int_bit(crc,20)^int_bit(crc,21)^int_bit(crc,22)^int_bit(crc,23)^int_bit(crc,24)^int_bit(crc,25)\
	^int_bit(crc,27)^int_bit(crc,28)^int_bit(crc,30)^short_bit(add_data,5)) & (1))<<15;
    //printf("hash_id 15 = %x\n",hash_id);
	//bit 16
	hash_id += ((long_bit(data,57)^long_bit(data,56)^long_bit(data,51)^long_bit(data,48)^long_bit(data,47)\
	^long_bit(data,46)^long_bit(data,44)^long_bit(data,37)^long_bit(data,35)^long_bit(data,32)\
	^long_bit(data,30)^long_bit(data,29)^long_bit(data,26)^long_bit(data,24)^long_bit(data,22)\
	^long_bit(data,21)^long_bit(data,19)^long_bit(data,17)^long_bit(data,13)^long_bit(data,12)\
	^long_bit(data,8)^long_bit(data,5)^long_bit(data,4)^long_bit(data,0)\
	^int_bit(crc,0)^int_bit(crc,3)^int_bit(crc,5)^int_bit(crc,12)^int_bit(crc,14)^int_bit(crc,15)\
	^int_bit(crc,16)^int_bit(crc,19)^int_bit(crc,24)^int_bit(crc,25)) & (1))<<16;
    //printf("hash_id 16 = %x\n",hash_id);
	//bit 17
	hash_id += ((long_bit(data,58)^long_bit(data,57)^long_bit(data,52)^long_bit(data,49)^long_bit(data,48)\
	^long_bit(data,47)^long_bit(data,45)^long_bit(data,38)^long_bit(data,36)^long_bit(data,33)\
	^long_bit(data,31)^long_bit(data,30)^long_bit(data,27)^long_bit(data,25)^long_bit(data,23)\
	^long_bit(data,22)^long_bit(data,20)^long_bit(data,18)^long_bit(data,14)^long_bit(data,13)\
	^long_bit(data,9)^long_bit(data,6)^long_bit(data,5)^long_bit(data,1)\
	^int_bit(crc,1)^int_bit(crc,4)^int_bit(crc,6)^int_bit(crc,13)^int_bit(crc,15)^int_bit(crc,16)\
	^int_bit(crc,17)^int_bit(crc,20)^int_bit(crc,25)^int_bit(crc,26)) & (1))<<17;
    //printf("hash_id 17 = %x\n",hash_id);
	//bit 18
	hash_id += ((long_bit(data,59)^long_bit(data,58)^long_bit(data,53)^long_bit(data,50)^long_bit(data,49)\
	^long_bit(data,48)^long_bit(data,46)^long_bit(data,39)^long_bit(data,37)^long_bit(data,34)\
	^long_bit(data,32)^long_bit(data,31)^long_bit(data,28)^long_bit(data,26)^long_bit(data,24)\
	^long_bit(data,23)^long_bit(data,21)^long_bit(data,19)^long_bit(data,15)^long_bit(data,14)\
	^long_bit(data,10)^long_bit(data,7)^long_bit(data,6)^long_bit(data,2)\
	^int_bit(crc,0)^int_bit(crc,2)^int_bit(crc,5)^int_bit(crc,7)^int_bit(crc,14)^int_bit(crc,16)\
	^int_bit(crc,17)^int_bit(crc,18)^int_bit(crc,21)^int_bit(crc,26)^int_bit(crc,27)^short_bit(add_data,6)) & (1))<<18;
    //printf("hash_id 18 = %x\n",hash_id);
	//bit 19
	hash_id += ((long_bit(data,60)^long_bit(data,59)^long_bit(data,54)^long_bit(data,51)^long_bit(data,50)\
	^long_bit(data,49)^long_bit(data,47)^long_bit(data,40)^long_bit(data,38)^long_bit(data,35)\
	^long_bit(data,33)^long_bit(data,32)^long_bit(data,29)^long_bit(data,27)^long_bit(data,25)\
	^long_bit(data,24)^long_bit(data,22)^long_bit(data,20)^long_bit(data,16)^long_bit(data,15)\
	^long_bit(data,11)^long_bit(data,8)^long_bit(data,7)^long_bit(data,3)\
	^int_bit(crc,0)^int_bit(crc,1)^int_bit(crc,3)^int_bit(crc,6)^int_bit(crc,8)^int_bit(crc,15)\
	^int_bit(crc,17)^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,22)^int_bit(crc,27)^int_bit(crc,28)) & (1))<<19;
    //printf("hash_id 19 = %x\n",hash_id);
	//bit 20
	hash_id += ((long_bit(data,61)^long_bit(data,60)^long_bit(data,55)^long_bit(data,52)^long_bit(data,51)\
	^long_bit(data,50)^long_bit(data,48)^long_bit(data,41)^long_bit(data,39)^long_bit(data,36)\
	^long_bit(data,34)^long_bit(data,33)^long_bit(data,30)^long_bit(data,28)^long_bit(data,26)\
	^long_bit(data,25)^long_bit(data,23)^long_bit(data,21)^long_bit(data,17)^long_bit(data,16)\
	^long_bit(data,12)^long_bit(data,9)^long_bit(data,8)^long_bit(data,4)\
	^int_bit(crc,1)^int_bit(crc,2)^int_bit(crc,4)^int_bit(crc,7)^int_bit(crc,9)^int_bit(crc,16)\
	^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,23)^int_bit(crc,28)^int_bit(crc,29)) & (1))<<20;
    //printf("hash_id 20 = %x\n",hash_id);
	//bit 21
	hash_id += ((long_bit(data,62)^long_bit(data,61)^long_bit(data,56)^long_bit(data,53)^long_bit(data,52)\
	^long_bit(data,51)^long_bit(data,49)^long_bit(data,42)^long_bit(data,40)^long_bit(data,37)\
	^long_bit(data,35)^long_bit(data,34)^long_bit(data,31)^long_bit(data,29)^long_bit(data,27)\
	^long_bit(data,26)^long_bit(data,24)^long_bit(data,22)^long_bit(data,18)^long_bit(data,17)\
	^long_bit(data,13)^long_bit(data,10)^long_bit(data,9)^long_bit(data,5)\
	^int_bit(crc,2)^int_bit(crc,3)^int_bit(crc,5)^int_bit(crc,8)^int_bit(crc,10)^int_bit(crc,17)\
	^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,21)^int_bit(crc,24)^int_bit(crc,29)^int_bit(crc,30)^short_bit(add_data,7)) & (1))<<21;
    //printf("hash_id 21 = %x\n",hash_id);
	//bit 22
	hash_id += ((long_bit(data,62)^long_bit(data,61)^long_bit(data,60)^long_bit(data,58)^long_bit(data,57)\
	^long_bit(data,55)^long_bit(data,52)^long_bit(data,48)^long_bit(data,47)^long_bit(data,45)\
	^long_bit(data,44)^long_bit(data,43)^long_bit(data,41)^long_bit(data,38)^long_bit(data,37)\
	^long_bit(data,36)^long_bit(data,35)^long_bit(data,34)^long_bit(data,31)^long_bit(data,29)\
	^long_bit(data,27)^long_bit(data,26)^long_bit(data,24)^long_bit(data,23)^long_bit(data,19)\
	^long_bit(data,18)^long_bit(data,16)^long_bit(data,14)^long_bit(data,12)^long_bit(data,11)^long_bit(data,9)^long_bit(data,0)\
	^int_bit(crc,2)^int_bit(crc,3)^int_bit(crc,4)^int_bit(crc,5)^int_bit(crc,6)^int_bit(crc,9)\
	^int_bit(crc,11)^int_bit(crc,12)^int_bit(crc,13)^int_bit(crc,15)^int_bit(crc,16)^int_bit(crc,20)\
	^int_bit(crc,23)^int_bit(crc,25)^int_bit(crc,26)^int_bit(crc,28)^int_bit(crc,29)^int_bit(crc,30)) & (1))<<22;
    //printf("hash_id 22 = %x\n",hash_id);
	//bit 23
	hash_id += ((long_bit(data,62)^long_bit(data,60)^long_bit(data,59)^long_bit(data,56)^long_bit(data,55)\
	^long_bit(data,54)^long_bit(data,50)^long_bit(data,49)^long_bit(data,47)^long_bit(data,46)\
	^long_bit(data,42)^long_bit(data,39)^long_bit(data,38)^long_bit(data,36)^long_bit(data,35)\
	^long_bit(data,34)^long_bit(data,31)^long_bit(data,29)^long_bit(data,27)^long_bit(data,26)\
	^long_bit(data,20)^long_bit(data,19)^long_bit(data,17)^long_bit(data,16)^long_bit(data,15)\
	^long_bit(data,13)^long_bit(data,9)^long_bit(data,6)^long_bit(data,1)^long_bit(data,0)\
	^int_bit(crc,2)^int_bit(crc,3)^int_bit(crc,4)^int_bit(crc,6)^int_bit(crc,7)^int_bit(crc,10)\
	^int_bit(crc,14)^int_bit(crc,15)^int_bit(crc,17)^int_bit(crc,18)^int_bit(crc,22)^int_bit(crc,23)\
	^int_bit(crc,24)^int_bit(crc,27)^int_bit(crc,28)^int_bit(crc,30)) & (1))<<23;
    //printf("hash_id 23 = %x\n",hash_id);
	//bit 24
	hash_id += ((long_bit(data,63)^long_bit(data,61)^long_bit(data,60)^long_bit(data,57)^long_bit(data,56)\
	^long_bit(data,55)^long_bit(data,51)^long_bit(data,50)^long_bit(data,48)^long_bit(data,47)\
	^long_bit(data,43)^long_bit(data,40)^long_bit(data,39)^long_bit(data,37)^long_bit(data,36)\
	^long_bit(data,35)^long_bit(data,32)^long_bit(data,30)^long_bit(data,28)^long_bit(data,27)\
	^long_bit(data,21)^long_bit(data,20)^long_bit(data,18)^long_bit(data,17)^long_bit(data,16)\
	^long_bit(data,14)^long_bit(data,10)^long_bit(data,7)^long_bit(data,2)^long_bit(data,1)\
	^int_bit(crc,0)^int_bit(crc,3)^int_bit(crc,4)^int_bit(crc,5)^int_bit(crc,7)^int_bit(crc,8)\
	^int_bit(crc,11)^int_bit(crc,15)^int_bit(crc,16)^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,23)\
	^int_bit(crc,24)^int_bit(crc,25)^int_bit(crc,28)^int_bit(crc,29)^int_bit(crc,31)) & (1))<<24;
    //printf("hash_id 24 = %x\n",hash_id);
	//bit 25
	hash_id += ((long_bit(data,62)^long_bit(data,61)^long_bit(data,58)^long_bit(data,57)^long_bit(data,56)\
	^long_bit(data,52)^long_bit(data,51)^long_bit(data,49)^long_bit(data,48)^long_bit(data,44)\
	^long_bit(data,41)^long_bit(data,40)^long_bit(data,38)^long_bit(data,37)^long_bit(data,36)\
	^long_bit(data,33)^long_bit(data,31)^long_bit(data,29)^long_bit(data,28)^long_bit(data,22)\
	^long_bit(data,21)^long_bit(data,19)^long_bit(data,18)^long_bit(data,17)^long_bit(data,15)\
	^long_bit(data,11)^long_bit(data,8)^long_bit(data,3)^long_bit(data,2)\
	^int_bit(crc,1)^int_bit(crc,4)^int_bit(crc,5)^int_bit(crc,6)^int_bit(crc,8)^int_bit(crc,9)\
	^int_bit(crc,12)^int_bit(crc,16)^int_bit(crc,17)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,24)\
	^int_bit(crc,25)^int_bit(crc,26)^int_bit(crc,29)^int_bit(crc,30)) & (1))<<25;
    //printf("hash_id 25 = %x\n",hash_id);
	//bit 26
	hash_id += ((long_bit(data,62)^long_bit(data,61)^long_bit(data,60)^long_bit(data,59)^long_bit(data,57)\
	^long_bit(data,55)^long_bit(data,54)^long_bit(data,52)^long_bit(data,49)^long_bit(data,48)\
	^long_bit(data,47)^long_bit(data,44)^long_bit(data,42)^long_bit(data,41)^long_bit(data,39)\
	^long_bit(data,38)^long_bit(data,31)^long_bit(data,28)^long_bit(data,26)^long_bit(data,25)\
	^long_bit(data,24)^long_bit(data,23)^long_bit(data,22)^long_bit(data,20)^long_bit(data,19)\
	^long_bit(data,18)^long_bit(data,10)^long_bit(data,6)^long_bit(data,4)^long_bit(data,3)^long_bit(data,0)\
	^int_bit(crc,6)^int_bit(crc,7)^int_bit(crc,9)^int_bit(crc,10)^int_bit(crc,12)^int_bit(crc,15)\
	^int_bit(crc,16)^int_bit(crc,17)^int_bit(crc,20)^int_bit(crc,22)^int_bit(crc,23)^int_bit(crc,25)\
	^int_bit(crc,27)^int_bit(crc,28)^int_bit(crc,29)^int_bit(crc,30)) & (1))<<26;
    //printf("hash_id 26 = %x\n",hash_id);
	//bit 27
	hash_id += ((long_bit(data,63)^long_bit(data,62)^long_bit(data,61)^long_bit(data,60)^long_bit(data,58)\
	^long_bit(data,56)^long_bit(data,55)^long_bit(data,53)^long_bit(data,50)^long_bit(data,49)\
	^long_bit(data,48)^long_bit(data,45)^long_bit(data,43)^long_bit(data,42)^long_bit(data,40)\
	^long_bit(data,39)^long_bit(data,32)^long_bit(data,29)^long_bit(data,27)^long_bit(data,26)\
	^long_bit(data,25)^long_bit(data,24)^long_bit(data,23)^long_bit(data,21)^long_bit(data,20)\
	^long_bit(data,19)^long_bit(data,11)^long_bit(data,7)^long_bit(data,5)^long_bit(data,4)^long_bit(data,1)\
	^int_bit(crc,0)^int_bit(crc,7)^int_bit(crc,8)^int_bit(crc,10)^int_bit(crc,11)^int_bit(crc,13)\
	^int_bit(crc,16)^int_bit(crc,17)^int_bit(crc,18)^int_bit(crc,21)^int_bit(crc,23)^int_bit(crc,24)\
	^int_bit(crc,26)^int_bit(crc,28)^int_bit(crc,29)^int_bit(crc,30)^int_bit(crc,31)) & (1))<<27;
    //printf("hash_id 27 = %x\n",hash_id);
	//bit 28
	hash_id += ((long_bit(data,63)^long_bit(data,62)^long_bit(data,61)^long_bit(data,59)^long_bit(data,57)\
	^long_bit(data,56)^long_bit(data,54)^long_bit(data,51)^long_bit(data,50)^long_bit(data,49)\
	^long_bit(data,46)^long_bit(data,44)^long_bit(data,43)^long_bit(data,41)^long_bit(data,40)\
	^long_bit(data,33)^long_bit(data,30)^long_bit(data,28)^long_bit(data,27)^long_bit(data,26)\
	^long_bit(data,25)^long_bit(data,24)^long_bit(data,22)^long_bit(data,21)^long_bit(data,20)\
	^long_bit(data,12)^long_bit(data,8)^long_bit(data,6)^long_bit(data,5)^long_bit(data,2)\
	^int_bit(crc,1)^int_bit(crc,8)^int_bit(crc,9)^int_bit(crc,11)^int_bit(crc,12)^int_bit(crc,14)\
	^int_bit(crc,17)^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,22)^int_bit(crc,24)^int_bit(crc,25)\
	^int_bit(crc,27)^int_bit(crc,29)^int_bit(crc,30)^int_bit(crc,31)) & (1))<<28;
    //printf("hash_id 28 = %x\n",hash_id);
	//bit 29
	hash_id += ((long_bit(data,63)^long_bit(data,62)^long_bit(data,60)^long_bit(data,58)^long_bit(data,57)\
	^long_bit(data,55)^long_bit(data,52)^long_bit(data,51)^long_bit(data,50)^long_bit(data,47)\
	^long_bit(data,45)^long_bit(data,44)^long_bit(data,42)^long_bit(data,41)^long_bit(data,34)\
	^long_bit(data,31)^long_bit(data,29)^long_bit(data,28)^long_bit(data,27)^long_bit(data,26)\
	^long_bit(data,25)^long_bit(data,23)^long_bit(data,22)^long_bit(data,21)^long_bit(data,13)\
	^long_bit(data,9)^long_bit(data,7)^long_bit(data,6)^long_bit(data,3)\
	^int_bit(crc,2)^int_bit(crc,9)^int_bit(crc,10)^int_bit(crc,12)^int_bit(crc,13)^int_bit(crc,15)\
	^int_bit(crc,18)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,23)^int_bit(crc,25)^int_bit(crc,26)\
	^int_bit(crc,28)^int_bit(crc,30)^int_bit(crc,31)) & (1))<<29;
    //printf("hash_id 29 = %x\n",hash_id);
	//bit 30
	hash_id += ((long_bit(data,63)^long_bit(data,61)^long_bit(data,59)^long_bit(data,58)^long_bit(data,56)\
	^long_bit(data,53)^long_bit(data,52)^long_bit(data,51)^long_bit(data,48)^long_bit(data,46)\
	^long_bit(data,45)^long_bit(data,43)^long_bit(data,42)^long_bit(data,35)^long_bit(data,32)\
	^long_bit(data,30)^long_bit(data,29)^long_bit(data,28)^long_bit(data,27)^long_bit(data,26)\
	^long_bit(data,24)^long_bit(data,23)^long_bit(data,22)^long_bit(data,14)^long_bit(data,10)\
	^long_bit(data,8)^long_bit(data,7)^long_bit(data,4)\
	^int_bit(crc,0)^int_bit(crc,3)^int_bit(crc,10)^int_bit(crc,11)^int_bit(crc,13)^int_bit(crc,14)\
	^int_bit(crc,16)^int_bit(crc,19)^int_bit(crc,20)^int_bit(crc,21)^int_bit(crc,24)^int_bit(crc,26)\
	^int_bit(crc,27)^int_bit(crc,29)^int_bit(crc,31)) & (1))<<30;
    //printf("hash_id 30 = %x\n",hash_id);
	//bit 31
	hash_id += ((long_bit(data,62)^long_bit(data,60)^long_bit(data,59)^long_bit(data,57)^long_bit(data,54)\
	^long_bit(data,53)^long_bit(data,52)^long_bit(data,49)^long_bit(data,47)^long_bit(data,46)\
	^long_bit(data,44)^long_bit(data,43)^long_bit(data,36)^long_bit(data,33)^long_bit(data,31)\
	^long_bit(data,30)^long_bit(data,29)^long_bit(data,28)^long_bit(data,27)^long_bit(data,25)\
	^long_bit(data,24)^long_bit(data,23)^long_bit(data,15)^long_bit(data,11)^long_bit(data,9)\
	^long_bit(data,8)^long_bit(data,5)\
	^int_bit(crc,1)^int_bit(crc,4)^int_bit(crc,11)^int_bit(crc,12)^int_bit(crc,14)^int_bit(crc,15)\
	^int_bit(crc,17)^int_bit(crc,20)^int_bit(crc,21)^int_bit(crc,22)^int_bit(crc,25)^int_bit(crc,27)\
	^int_bit(crc,28)^int_bit(crc,30)) & (1))<<31;
    //printf("##################hash_id = %x\n",hash_id);
	return hash_id;
}
/* get fpga hash id end*/




unsigned long long crc32_algorithm(unsigned long long data)
{
    unsigned long long  POLY = 0x104C11DB7;
    int crcbitnumber=32;
    int  databitnumber=32;
    unsigned long long  regi = 0x0;
    int cur_bit = databitnumber+crcbitnumber-1;
    data <<= crcbitnumber; 
     
    for ( ;cur_bit >= 0;--cur_bit )
    { 
        if ( ( ( regi >> crcbitnumber ) & 0x0001 ) == 0x1 )
    	regi = regi ^ POLY;
     
        regi <<= 1;
        unsigned short tmp = ( data >> cur_bit ) & 0x0001;
        regi |= tmp;
    } 
    if ( ( ( regi >> crcbitnumber ) & 0x0001 ) == 0x1 )
       regi = regi ^ POLY;
    return regi;
}

unsigned long long crc32c_algorithm(unsigned long long data)
{
    unsigned long long  POLY = 0x11EDC6F41;
    int crcbitnumber=32;
    int  databitnumber=32;
    unsigned long long  regi = 0x0;
    int cur_bit = databitnumber+crcbitnumber-1;
    data <<= crcbitnumber; 
     
    for ( ;cur_bit >= 0;--cur_bit )
    { 
        if ( ( ( regi >> crcbitnumber ) & 0x0001 ) == 0x1 )
    	regi = regi ^ POLY;
     
        regi <<= 1;
        unsigned short tmp = ( data >> cur_bit ) & 0x0001;
        regi |= tmp;
    } 
    if ( ( ( regi >> crcbitnumber ) & 0x0001 ) == 0x1 )
       regi = regi ^ POLY;
    return regi;
}

/*
*
*return:1 for fpga board;0 for not
*/
int check_board_type(struct vty* vty,unsigned short slot_id)
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret;

    query = dbus_message_new_method_call(
        			SEM_DBUS_BUSNAME,
        			SEM_DBUS_OBJPATH,
        			SEM_DBUS_INTERFACE,
        			SEM_DBUS_CHECK_BOARD_TYPE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT16, &slot_id,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,60000, &err);
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return 0;
		}
		
        if(dbus_message_get_args ( reply, &err,
    		                     DBUS_TYPE_INT32,&ret,
    		                     DBUS_TYPE_INVALID))
        {
             if(ret == 1)
             {
                //vty_out(vty,"the board is ax81_ac_1x12g12s\n");
                dbus_message_unref(reply);
				return 1;
			 }
			 else
			 {
			 	dbus_message_unref(reply);
                return 0;
			 }
		}
		else
		{
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
			}
			dbus_message_unref(reply);
            return 0;
		}
	}
	else
	{
		vty_out(vty,"no connection to slot %d\n", slot_id);
		return 0;
	}

}
/*
*
*return:1 for fpga board;0 for not
*/
int local_check_board_type(struct vty* vty)
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret;
	printf("\nfunction:local_check_board_type\n");
    query = dbus_message_new_method_call(
        			SEM_DBUS_BUSNAME,
        			SEM_DBUS_OBJPATH,
        			SEM_DBUS_INTERFACE,
        			SEM_DBUS_LOCAL_CHECK_BOARD_TYPE);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply)
	{
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 0;
	}
	
    if(dbus_message_get_args ( reply, &err,
		                     DBUS_TYPE_INT32,&ret,
		                     DBUS_TYPE_INVALID))
    {
         if(ret == 1)
         {
            vty_out(vty,"the board is ax81_ac_1x12g12s\n");
            dbus_message_unref(reply);
			return 1;
		 }
		 else
		 {
		 	dbus_message_unref(reply);
            return 0;
		 }
	}
	else
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
        return 0;
	}
}


DEFUN(ax86_fpga_online_burning,
	ax86_fpga_online_burning_cmd,
	"burning fpgabin (default |FILENAME)",
	"burning\n"
	"burning FPGA BIN \n"
	"default FPGA bin file ,file in CF card,name is erd_full_version.bin\n"
	"FPGA bin file name\n")
{
	int ret;
	fpga_file opt;

	memset(&opt,0,sizeof(fpga_file));
	if( strncmp(argv[0],"default",sizeof("default"))== 0 )
	{
		strncpy(opt.fpga_bin_name,"/blk/erd_full_version.bin",sizeof("/blk/erd_full_version.bin"));
	}
	else
	{
		sprintf(opt.fpga_bin_name,"%s",argv[0]);
	}
    vty_out(vty,"burning file name:%s!\n",opt.fpga_bin_name);
	opt.result = 7;//can't be zero
	
	ret =burning_fpga_bin(&opt);
	if(ret){
		vty_out(vty,"burning fpgabin %s success.\n",opt.fpga_bin_name);
	}else{
		vty_out(vty,"burning fpgabin %s failure.\n",opt.fpga_bin_name);
	}
	return CMD_SUCCESS;
}

DEFUN(show_hash_statistics,
	show_hash_statistics_cmd,
	"show fpga hash-statistics SLOT_ID [-c]",
	"Show system information\n"
	"Show fpga information\n"
	"Show fpga hash statistics information\n"
	"the slot id num\n"
	"Clear all the statistics value\n")
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret;
	int slot_id;
    unsigned long long hash_aging_num;
    unsigned long long hash_update_num;
    unsigned long long hash_learn_num;
	unsigned short flag_c =0;
	char *clear = "-c";
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	
    if(argc ==2)
    {
        if(strcmp(clear,argv[1]) == 0)
        {
            flag_c = 1;
        }
		else
		{
            vty_out(vty,"bad parameter!if you want to clear.parameter [-c].\n");
			return CMD_WARNING;
		}
    }
	
    vty_out(vty,"begin to show hash statistics,wait.");
	
    query = dbus_message_new_method_call(
        			SEM_DBUS_BUSNAME,
        			SEM_DBUS_OBJPATH,
        			SEM_DBUS_INTERFACE,
        			SEM_DBUS_SHOW_HASH_STATISTICS);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT16, &flag_c,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
		vty_out(vty,".");

        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,60000, &err);
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		vty_out(vty,".");
        if(0 == flag_c)
        {
			vty_out(vty,".\n");
        	if (dbus_message_get_args ( reply, &err,
        		                     DBUS_TYPE_INT32,&ret,
        							 DBUS_TYPE_UINT64,&hash_aging_num,
        							 DBUS_TYPE_UINT64,&hash_update_num,
            						 DBUS_TYPE_UINT64,&hash_learn_num,
        							 DBUS_TYPE_INVALID))
                 {
                     if(ret == 1)
                     {
                    	vty_out(vty,"\nDetail Information of hash\n");
                     	vty_out(vty,"======================================================\n");
                        vty_out(vty,"\nHASH");
                        vty_out(vty,"\thash_aging_num:             %lld\n",	hash_aging_num);		
                        vty_out(vty,"\thash_update_num:            %lld\n",	hash_update_num);		
                        vty_out(vty,"\thash_learn_num:             %lld\n", hash_learn_num);
                        vty_out(vty,"======================================================\n");
						dbus_message_unref(reply);
    					return CMD_SUCCESS;
                     }else if(ret == 0)
                     {
                        vty_out(vty, "show_port_counter command fail\n");
						dbus_message_unref(reply);
            			return CMD_WARNING;
            		 }
        		//return CMD_SUCCESS;
        	    }
    			else 
        	    {
            		vty_out(vty,"Failed get args.\n");
            		if (dbus_error_is_set(&err))
            		{
            				printf("%s raised: %s",err.name,err.message);
            				dbus_error_free_for_dcli(&err);
            		}
					dbus_message_unref(reply);
            		return CMD_WARNING;
            	}
	    }
		else if(1 == flag_c)
        {
			vty_out(vty,".    ");
            if(dbus_message_get_args ( reply, &err,
        		                     DBUS_TYPE_INT32,&ret,
        		                     DBUS_TYPE_INVALID))
            {
                 if(ret == 1)
                 {
                    vty_out(vty,"Clear success!\n");
					dbus_message_unref(reply);
					return CMD_SUCCESS;
				 }			
			}
		    else 
    	    {
        		vty_out(vty,"Failed get args.\n");
        		if (dbus_error_is_set(&err))
        		{
        				printf("%s raised: %s",err.name,err.message);
        				dbus_error_free_for_dcli(&err);
        		}
				dbus_message_unref(reply);
        		return CMD_WARNING;
        	}

	    }
	}
	else
	{
		vty_out(vty,".    \n");
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
}

DEFUN(show_port_counter,
	show_port_counter_cmd,
	"show fpga port_counter SLOT_ID (112|113|115|116) [-c]",
	"Show system information\n"
	"Show fpga information\n"
	"Show fpga port counter information\n"
	"the slot id num\n"
	"Port number 112\n"
	"Port number 113\n"
	"Port number 115\n"
	"Port number 116\n"
	"Clear all the statistics value\n")
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret;
	int slot_id;
	char *endptr = NULL;
    unsigned short port_number;
	unsigned short flag_c =0;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	unsigned long long package_num;
	unsigned long long bytenum;
	unsigned long long tx_multicast_package_num;
	unsigned long long tx_broadcast_package_num;
	unsigned long long tx_pause_package_num;
	unsigned long long good_package_num;
	unsigned long long good_byte_num;
	unsigned long long bad_package_num;
	unsigned long long fcserr_package_num;
	unsigned long long rx_multicast_package_num;
	unsigned long long rx_broadcast_package_num;
	unsigned long long rx_pause_package_num;
	unsigned long long drop_num;
	unsigned long long ttl_drop_num;
	unsigned long long car_drop_num;
	unsigned long long hash_col_num;
	unsigned long long to_cpu_num;
	char *clear = "-c";
    if(argc ==3)
    {
        if(strcmp(clear,argv[2]) == 0)
        {
            flag_c = 1;
        }
		else
		{
            vty_out(vty,"bad parameter!if you want to clear.parameter [-c].\n");
			return CMD_WARNING;
		}
    }
	
	port_number = strtol(argv[1],NULL,10);
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
    vty_out(vty,"begin to show port counter,wait.");
    query = dbus_message_new_method_call(
        			SEM_DBUS_BUSNAME,
        			SEM_DBUS_OBJPATH,
        			SEM_DBUS_INTERFACE,
        			SEM_DBUS_SHOW_PORT_COUNTER);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT16, &port_number,
                        DBUS_TYPE_UINT16, &flag_c,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
		vty_out(vty,".");

        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,60000, &err);
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		vty_out(vty,".");
        if(0 == flag_c)
        {
			vty_out(vty,".");
        	if(port_number == 112 || port_number == 113 || port_number == 115)
            {
				vty_out(vty,".\n");
            	if (dbus_message_get_args ( reply, &err,
            		                     DBUS_TYPE_UINT32,&ret,
            		                     DBUS_TYPE_UINT64,&package_num,
            		                     DBUS_TYPE_UINT64,&bytenum,
            							 DBUS_TYPE_UINT64,&tx_multicast_package_num,
            							 DBUS_TYPE_UINT64,&tx_broadcast_package_num,
            							 DBUS_TYPE_UINT64,&tx_pause_package_num,
            							 DBUS_TYPE_UINT64,&good_package_num,
            							 DBUS_TYPE_UINT64,&good_byte_num,
            							 DBUS_TYPE_UINT64,&bad_package_num,
            							 DBUS_TYPE_UINT64,&fcserr_package_num,
            							 DBUS_TYPE_UINT64,&rx_multicast_package_num,
            							 DBUS_TYPE_UINT64,&rx_broadcast_package_num,
            							 DBUS_TYPE_UINT64,&rx_pause_package_num,
                						 DBUS_TYPE_UINT64,&drop_num,
            							 DBUS_TYPE_UINT64,&ttl_drop_num,
            							 DBUS_TYPE_UINT64,&car_drop_num,
                						 DBUS_TYPE_UINT64,&hash_col_num,
            							 DBUS_TYPE_UINT64,&to_cpu_num,
            		                     DBUS_TYPE_INVALID))
                 {
                     if(ret == 1)
                     {
                    	vty_out(vty,"\nDetail Information of XAUI Port %d\n",port_number);
                     	vty_out(vty,"======================================================\n");
                        vty_out(vty,"\nRX");

                    	vty_out(vty,"\tgood_package_num:           %lld\n",	good_package_num);		
                        vty_out(vty,"\tgood_byte_num:              %lld\n",	good_byte_num);		
                    	vty_out(vty,"\tbad_package_num:            %lld\n", bad_package_num);
                    	vty_out(vty,"\tfcserr_package_num:         %lld\n", fcserr_package_num);
                    	vty_out(vty,"\tmulticast_package_num:      %lld\n", rx_multicast_package_num);
                        vty_out(vty,"\tbroadcast_package_num:      %lld\n", rx_broadcast_package_num);
                        vty_out(vty,"\tpause_package_num:          %lld\n", rx_pause_package_num);
                    	vty_out(vty,"\tdrop_num:                   %lld\n", drop_num);
    					
                    	vty_out(vty,"\nTX");
                    	vty_out(vty,"\tpackage_num:                %lld\n", package_num);
                        vty_out(vty,"\tbytenum:                    %lld\n", bytenum);
                        vty_out(vty,"\tmulticast_package_num:      %lld\n", tx_multicast_package_num);
                        vty_out(vty,"\tbroadcast_package_num:      %lld\n", tx_broadcast_package_num);
                    	vty_out(vty,"\tpause_package_num:          %lld\n", tx_pause_package_num);
                        vty_out(vty,"\n");
                        vty_out(vty,"======================================================\n");
						dbus_message_unref(reply);
    					return CMD_SUCCESS;
    				}else if(ret == 0)
                    {
                        vty_out(vty, "show_port_counter command fail\n");
						dbus_message_unref(reply);
            			return CMD_WARNING;
            		}
        		//return CMD_SUCCESS;
        	    }
				else 
        	    {
            		vty_out(vty,"Failed get args.\n");
            		if (dbus_error_is_set(&err))
            		{
            				printf("%s raised: %s",err.name,err.message);
            				dbus_error_free_for_dcli(&err);
            		}
					dbus_message_unref(reply);
            		return CMD_WARNING;
            	}
        	}
        	else if(port_number == 116)
        	{
				vty_out(vty,".\n");
            	if (dbus_message_get_args ( reply, &err,
            		                     DBUS_TYPE_UINT32,&ret,
            		                     DBUS_TYPE_UINT64,&package_num,
            		                     DBUS_TYPE_UINT64,&bytenum,
            							 DBUS_TYPE_UINT64,&tx_multicast_package_num,
            							 DBUS_TYPE_UINT64,&tx_broadcast_package_num,
            							 DBUS_TYPE_UINT64,&tx_pause_package_num,
            							 DBUS_TYPE_UINT64,&good_package_num,
            							 DBUS_TYPE_UINT64,&good_byte_num,
            							 DBUS_TYPE_UINT64,&bad_package_num,
            							 DBUS_TYPE_UINT64,&fcserr_package_num,
            							 DBUS_TYPE_UINT64,&rx_multicast_package_num,
            							 DBUS_TYPE_UINT64,&rx_broadcast_package_num,
            							 DBUS_TYPE_UINT64,&rx_pause_package_num,
                						 DBUS_TYPE_UINT64,&drop_num,
            							 DBUS_TYPE_UINT64,&ttl_drop_num,
            							 DBUS_TYPE_UINT64,&car_drop_num,
                						 DBUS_TYPE_UINT64,&hash_col_num,
            							 DBUS_TYPE_UINT64,&to_cpu_num,
            							 //DBUS_TYPE_UINT64,&hash_aging_num,
            							 //DBUS_TYPE_UINT64,&hash_update_num,
                						 //DBUS_TYPE_UINT64,&hash_learn_num,
            							 DBUS_TYPE_INVALID))
                 {
                     if(ret == 1)
                     {
                    	vty_out(vty,"\nDetail Information of XAUI Port %d\n",port_number);
                     	vty_out(vty,"======================================================\n");
    					
                        vty_out(vty,"\nRX");
                    	vty_out(vty,"\tgood_package_num:           %lld\n",	good_package_num);		
                        vty_out(vty,"\tgood_byte_num:              %lld\n",	good_byte_num);		
                    	vty_out(vty,"\tbad_package_num:            %lld\n", bad_package_num);
                    	vty_out(vty,"\tfcserr_package_num:         %lld\n", fcserr_package_num);
                    	vty_out(vty,"\tmulticast_package_num:      %lld\n", rx_multicast_package_num);
                        vty_out(vty,"\tbroadcast_package_num:      %lld\n", rx_broadcast_package_num);
                        vty_out(vty,"\tpause_package_num:          %lld\n", rx_pause_package_num);
                    	vty_out(vty,"\tdrop_num:                   %lld\n", drop_num);
                    	vty_out(vty,"\nTX");
                    	vty_out(vty,"\tpackage_num:                %lld\n", package_num);
                        vty_out(vty,"\tbytenum:                    %lld\n", bytenum);
                        vty_out(vty,"\tmulticast_package_num:      %lld\n", tx_multicast_package_num);
                        vty_out(vty,"\tbroadcast_package_num:      %lld\n", tx_broadcast_package_num);
                    	vty_out(vty,"\tpause_package_num:          %lld\n", tx_pause_package_num);
                        vty_out(vty,"\n");
                        vty_out(vty,"======================================================\n");
						dbus_message_unref(reply);
    					return CMD_SUCCESS;
                     }else if(ret == 0)
                     {
                        vty_out(vty, "show_port_counter command fail\n");
						dbus_message_unref(reply);
            			return CMD_WARNING;
            		 }
        		//return CMD_SUCCESS;
        	    }
				else 
        	    {
            		vty_out(vty,"Failed get args.\n");
            		if (dbus_error_is_set(&err))
            		{
            				printf("%s raised: %s",err.name,err.message);
            				dbus_error_free_for_dcli(&err);
            		}
					dbus_message_unref(reply);
            		return CMD_WARNING;
            	}
        	}
        }
		else if(1 == flag_c)
        {
			vty_out(vty,".    ");
            if(dbus_message_get_args ( reply, &err,
        		                     DBUS_TYPE_UINT32,&ret,
        		                     DBUS_TYPE_INVALID))
            {
                 if(ret == 1)
                 {
                    vty_out(vty,"Clear success!\n");
					dbus_message_unref(reply);
					return CMD_SUCCESS;
				 }
				 else
				 {
                    vty_out(vty,"Clear fail!\n");
					dbus_message_unref(reply);
					return CMD_SUCCESS;
				 }
			}
			else 
    	    {
        		vty_out(vty,"Failed get args.\n");
        		if (dbus_error_is_set(&err))
        		{
        				printf("%s raised: %s",err.name,err.message);
        				dbus_error_free_for_dcli(&err);
        		}
				dbus_message_unref(reply);
        		return CMD_WARNING;
        	}
	    }
    }
	else
	{
		vty_out(vty,".    \n");
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
}

DEFUN(car_table_read,
	car_table_read_cmd,
	"show fpga car-table slot SLOT_ID (uplink|downlink) ip IP",
	"Show system information\n"
	"Show fpga information\n"
	"read car table \n"
	"the slot id \n"
	"the slot id num \n"
	"uplink car \n"
	"downlink car\n"
	"user IP address\n"
	"IP address\n")
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int flag;
	int slot_id;
	char ip_str[16];

	unsigned short Indirect_add_reg_H = 0;
	unsigned short Indirect_add_reg_L = 0;
    unsigned short LINKUP_STATE =0;
	unsigned short property =0;
    unsigned long long car_id;
	unsigned long long reload;
	unsigned long long linkup;
	unsigned long long car_valid;
	unsigned int usr_ip;
    unsigned long long credit;
	unsigned long long byte_drop_count;  
	unsigned long long byte_set_count;
	unsigned long long package_set_count;
	unsigned long long package_drop_count;
	char *endptr = NULL;
	char *property_s =NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    vty_out(vty,"begin to read car table,wait.");
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
    ret = inet_aton(argv[2], &userip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //vty_out(vty,"**the userip = %x\n",userip_val.s_addr);
    }else
    {   
        vty_out(vty,"**ip conversion error,invalid parameter!\n");
		return CMD_WARNING;
    }
	
	if(0 == strncmp("uplink",argv[1],strlen((char*)argv[1])))   /*"uplink" bit 0 for 1*/
	{
		vty_out(vty,". ");
		LINKUP_STATE = 1;
	}
	else if(0 == strncmp("downlink",argv[1],strlen((char*)argv[1]))) /*"downlink" bit 0 for 0*/
	{
		vty_out(vty,". ");
		LINKUP_STATE = 0;
	}
	else 
	{
    	vty_out(vty,"%% Unknow input param\n");
		return CMD_WARNING;
	}
	
    Indirect_add_reg_H = (userip_val.s_addr &0xffff0000) >> 16;
	Indirect_add_reg_L = userip_val.s_addr &0xffff;	

/*
	vty_out(vty,"******slot_id = 0x%x******\n",slot_id);
	vty_out(vty,"******Indirect_add_reg_H = 0x%x******\n",Indirect_add_reg_H);
	vty_out(vty,"******Indirect_add_reg_L = 0x%x******\n",Indirect_add_reg_L);
    vty_out(vty,"******LINKUP_STATE = 0x%x******\n",LINKUP_STATE);
*/
    query = dbus_message_new_method_call(
        			SEM_DBUS_BUSNAME,
        			SEM_DBUS_OBJPATH,
        			SEM_DBUS_INTERFACE,
        			SEM_DBUS_CAR_READ);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT16, &Indirect_add_reg_H,
	                    DBUS_TYPE_UINT16, &Indirect_add_reg_L,
						DBUS_TYPE_UINT16, &LINKUP_STATE,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,60000, &err);
		dbus_message_unref(query);
        vty_out(vty,". ");
		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

    	if (dbus_message_get_args ( reply, &err,
			                     DBUS_TYPE_INT32,&flag,
    		                     DBUS_TYPE_INT32,&ret,
    		                     DBUS_TYPE_UINT64,&reload,
    		                     DBUS_TYPE_UINT64,&linkup,
    							 DBUS_TYPE_UINT64,&car_valid,
    							 DBUS_TYPE_UINT32,&usr_ip,
    							 DBUS_TYPE_UINT64,&credit,
    							 DBUS_TYPE_UINT64,&byte_drop_count,
    							 DBUS_TYPE_UINT64,&byte_set_count,
    							 DBUS_TYPE_UINT64,&package_set_count,
    							 DBUS_TYPE_UINT64,&package_drop_count,
    							 DBUS_TYPE_UINT16,&property,
    		                     DBUS_TYPE_INVALID))
         {
			 usr_ip = htonl(usr_ip);
		 	 if(inet_ntop(AF_INET,&usr_ip,ip_str,16) == NULL)
		 	 {
                vty_out(vty,"ip conversion error \n");
			 }
			 
			 if(property ==0)
			 	property_s = "static";
			 else if(property ==1)
			 	property_s = "dynamic";
			 else 
			 	vty_out(vty,"get bad property parameter!\n");
			 
			 if(flag == 1)
			 {
                vty_out(vty,"%s %s is not exit in the list!\n",argv[1],argv[2]);
				return CMD_WARNING;
			 }
			 else
			 {
                 if(ret == 0)
                 {
                    vty_out(vty,"   OK!\n");
                	
                	vty_out(vty,"Detail Information of CAR Table %s\n",ip_str);
                 	vty_out(vty,"=====================================================================\n");
                    vty_out(vty,"\n");
                	vty_out(vty,"\tproperty:                  %s\n",property_s);
                	vty_out(vty,"\tlinkstate:                 %llx\n",linkup);
                    vty_out(vty,"\tspeed limit:               %lld\n",reload);		
                	vty_out(vty,"\tcar valid:                 %llx\n",car_valid);
                	vty_out(vty,"\tusr ip:                    %s\n",ip_str);
                	vty_out(vty,"\tcredit:                    %llx\n",credit);
                	vty_out(vty,"\tbyte sent count:           %llx\n",byte_set_count);
                    vty_out(vty,"\tpackage sent count:        %llx\n",package_set_count);
					vty_out(vty,"\tbyte drop count:           %llx\n",byte_drop_count);
                    vty_out(vty,"\tpackage drop count:        %llx\n",package_drop_count);

                    vty_out(vty,"\n");
                    vty_out(vty,"=====================================================================\n");	
                 }
        		 else if(ret == -1)
                 {
                    vty_out(vty, "read car fail\n");
        			return CMD_WARNING;
        		 }
			 }
		//return CMD_SUCCESS;
	    }
		else 
	    {
    		vty_out(vty,"Failed get args.\n");
    		if (dbus_error_is_set(&err))
    		{
    				printf("%s raised: %s",err.name,err.message);
    				dbus_error_free_for_dcli(&err);
    		}
    		return CMD_WARNING;
    	}
		dbus_message_unref(reply);
	} 
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(car_subnet_write,
	car_subnet_write_cmd,
	"set fpga car-subnet slot SLOT_ID (add|delete|modify) ip USRIP (uplink|downlink) speed_limit <0-1024> (static|dynamic) netmask SUBNET_MASK",
	"set system parameters\n"
	"set fpga parameters\n"
	"set fpga car table \n"
	"the slot id \n"
	"the slot id num \n"
	"delete for invalid car entry\n"
	"add for valid car entry\n"
	"modify for modify car entry\n"
	"usr ip \n"
	"ip address \n"
	"uplink\n"
	"downlink\n"
	"speed_limit value\n"
	"speed_limit range:0k~1024k\n"
	"static for car entry property\n"
	"dynamic for valid car property\n"
	"the subnet mask\n"
	"the subnet mask.eg:255.255.255.0\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	struct in_addr maskip_val;
	int ret;
	int flag;
	int slot_id;
	unsigned short Indirect_add_reg_H = 0;
	unsigned short Indirect_add_reg_L = 0;
	unsigned short subnet_mask_H = 0;
	unsigned short subnet_mask_L = 0;
	unsigned int RELOAD = 0;
    unsigned short RELOAD_H = 0;
	unsigned short RELOAD_L = 0;
    unsigned short LINKUP = 0;
    unsigned short CAR_VALID =0;
	unsigned short CAR_PROPERTY =0;
	unsigned int num = 0;
	unsigned int i=0;
	unsigned int user_ip;
	char *endptr = NULL;
	char ip_str[16];
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

	vty_out(vty,"begin to write car subnet,wait..");

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}

	ret = inet_aton(argv[2], &userip_val);
    if(ret)
    {   
		vty_out(vty,".");
        //vty_out(vty,"**the userip = %x\n",userip_val.s_addr);
    }else
    {   
        vty_out(vty,"**ip conversion error,invalid parameter!\n");
		return CMD_WARNING;
    }

	ret = inet_aton(argv[6], &maskip_val);
    if(ret)
    {   
		vty_out(vty,".");
        //vty_out(vty,"**the subnet mask = %x\n",maskip_val.s_addr);
    }else
    {   
        vty_out(vty,"**ip mask conversion error,invalid parameter!\n");
		return CMD_WARNING;
    }

	if(0 == strncmp("static",argv[5],strlen((char*)argv[5])))
	{
		vty_out(vty,".");
		CAR_PROPERTY = 0;
	}
	else if(0 == strncmp("dynamic",argv[5],strlen((char*)argv[5])))
	{
		vty_out(vty,".");
		CAR_PROPERTY = 1;
	}
	else
	{
    	vty_out(vty,"%% Unknow input param 5\n");
		return CMD_WARNING;
	}

	if(0 == strncmp("uplink",argv[3],strlen((char*)argv[3])))   /*"uplink" bit 0 for 1*/
	{
		vty_out(vty,".");
		LINKUP = 1;
	}
	else if(0 == strncmp("downlink",argv[3],strlen((char*)argv[3]))) /*"downlink" bit 0 for 0*/
	{
		vty_out(vty,".");
		LINKUP = 0;
	}
	else 
	{
    	vty_out(vty,"%% Unknow input param\n");
		return CMD_WARNING;
	}
	
	Indirect_add_reg_H = (userip_val.s_addr &0xffff0000) >> 16;
	Indirect_add_reg_L = userip_val.s_addr &0xffff;
    subnet_mask_H = (maskip_val.s_addr &0xffff0000) >> 16;
	subnet_mask_L = maskip_val.s_addr &0xffff;
    Indirect_add_reg_H &= subnet_mask_H;
    Indirect_add_reg_L &= subnet_mask_L;
	Indirect_add_reg_L++;

	ret = fpga_parse_int((char *)argv[4],&RELOAD);
	if(0 != ret){
		vty_out(vty,"parse param error\n");
		return CMD_WARNING;
	}
	if((RELOAD < 0)||(RELOAD > 16777215)) {
		vty_out(vty,"%% bad RELOAD value input!\n");
		return CMD_WARNING;
	}
	RELOAD_H = (RELOAD & 0xff0000) >> 16;
    RELOAD_L =RELOAD & 0xffff;
	
	if(0 == strncmp("add",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		CAR_VALID = 1;
	}
	else if(0 == strncmp("delete",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		CAR_VALID = 0;
	}
	else if(0 == strncmp("modify",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		CAR_VALID = 2;
	}
	else
	{
    	vty_out(vty,"%% Unknow input param 2\n");
		return CMD_WARNING;
	}
	
	if(subnet_mask_H == 0xffff)
	{
        num = 0xffff - subnet_mask_L;
	}
	else
	{
        num = (0xffff - subnet_mask_H)<<16+0xffff;
	}

	if(num >= CAR_ENTRY_NUM)
	{
    	vty_out(vty,"the max num of car entry is 128K!\n");
		return CMD_WARNING;
	}
/*
	vty_out(vty,"******slot_id = 0x%x******\n",slot_id);
	vty_out(vty,"******Indirect_add_reg_H = 0x%x******\n",Indirect_add_reg_H);
	vty_out(vty,"******Indirect_add_reg_L = 0x%x******\n",Indirect_add_reg_L);
	vty_out(vty,"******RELOAD_H = 0x%x******\n",RELOAD_H);
	vty_out(vty,"******RELOAD_L = 0x%x******\n",RELOAD_L);
	vty_out(vty,"******LINKUP = 0x%x******\n",LINKUP);
	vty_out(vty,"******CAR_VALID = 0x%x******\n",CAR_VALID);
	vty_out(vty,"******subnet_mask_H = 0x%x******\n",subnet_mask_H);
	vty_out(vty,"******subnet_mask_L = 0x%x******\n",subnet_mask_L);
	vty_out(vty,"******num = %d******\n",num);
*/
    for(i=0;i < num-1;i++)
    {
	    user_ip = Indirect_add_reg_H ;
		user_ip = (user_ip <<16)+Indirect_add_reg_L;
		user_ip = htonl(user_ip);
     	if(inet_ntop(AF_INET,&user_ip,ip_str,16) == NULL)
     	{
           vty_out(vty,"ip conversion error \n");
    	}
		
    	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
    												 SEM_DBUS_INTERFACE, SEM_DBUS_CAR_SUBNET_WRITE);
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
                            DBUS_TYPE_UINT16, &Indirect_add_reg_H,
    	                    DBUS_TYPE_UINT16, &Indirect_add_reg_L,
    						DBUS_TYPE_UINT16, &RELOAD_H,
    						DBUS_TYPE_UINT16, &RELOAD_L,
    						DBUS_TYPE_UINT16, &LINKUP,
    						DBUS_TYPE_UINT16, &CAR_VALID,
    						DBUS_TYPE_UINT16, &CAR_PROPERTY,
    						DBUS_TYPE_INVALID);
    	
    	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
    			query, 60000, &err);
    		dbus_message_unref(query);

    		if (NULL == reply){
    			vty_out(vty,"<error> failed get reply.\n");
    			if (dbus_error_is_set(&err)) {
    				vty_out(vty,"%s raised: %s",err.name,err.message);
    				dbus_error_free_for_dcli(&err);
    			}
    			continue;
    		}
/*  0 for insert fail;1 for user already exit;3 for user exit but reload is different, modify reload value;
    2 for user is static;4 for chang user property to static;5 for chang user property to static and modify reload val
    6 for user is in the white list
*/
            if(CAR_VALID == 1)//add
            {
        		if (dbus_message_get_args (reply, &err,
        						DBUS_TYPE_UINT32,&flag,
        						DBUS_TYPE_INVALID)) {
        			if(flag == 0){
        				vty_out(vty,"add %s  %s fail!\n",argv[3],ip_str);
        			}
					else if(flag == 1){
						vty_out(vty,"%s  %s is already exit!\n",argv[3],ip_str);
					}
					else if(flag == 3){
						vty_out(vty,"%s  %s is exit but speed limit is modifyed %u!\n",argv[3],ip_str,RELOAD);
					}
					else if(flag == 2){
						vty_out(vty,"%s  %s is static.if change to dynamic ,fist delete it!\n",argv[3],ip_str);
					}
					else if(flag == 5){
						vty_out(vty,"%s  %s has already changed to static and modify speed limit to %u!\n",argv[3],ip_str,RELOAD);
					}
					else if(flag == 6){
						vty_out(vty,"%s  %s is in white car list!\n",argv[3],ip_str);
					}
					else if(flag == 4){
						vty_out(vty,"%s  %s has already changed to static!\n",argv[3],ip_str);
					}
					else if(flag == 8){
						vty_out(vty,"insert %s  %s  in car list fail!\n",argv[3],ip_str);
					}
					else if(flag == 7){
        				//do nothing
        			}
					else{
						vty_out(vty,"%s  %s get bad flag result!\n",argv[3],ip_str);
				    }
        		}
            }
/*
return 0 for this node don't exit;1 for delete success;4 for delete fail;
       2,3 for parameter is wrong.
*/
			else if(CAR_VALID == 0)//delete
			{
        		if (dbus_message_get_args (reply, &err,
        						DBUS_TYPE_UINT32,&flag,
        						DBUS_TYPE_INVALID)) {
        			if(flag == 0){
        				vty_out(vty,"%s  %s is not exit!\n",argv[3],ip_str);
        			}
					else if(flag == 4){
                        vty_out(vty,"delete %s  %s fail!\n",argv[3],ip_str);
					}
        			else if(flag == 2 || flag == 3){
        				vty_out(vty,"when delete %s  %s ,parameter static/dynamic is not match!\n",argv[3],ip_str);
        			}
					else if(flag == 1){
        				//do nothing
        			}
					else{
                        vty_out(vty,"%s  %s get bad flag result!\n",argv[3],ip_str);
					}
        		}
			}
//0 for this node don't exit;-1 for parameter wrong;1 for write car table wrong			
			else if(CAR_VALID == 2)//modify
			{
        		if (dbus_message_get_args (reply, &err,
        						DBUS_TYPE_UINT32,&flag,
        						DBUS_TYPE_INVALID)) {
        			if(flag == 0){
        				vty_out(vty,"%s  %s is not exit!\n",argv[3],ip_str);
        			}
        			else if(flag == -1){
        				vty_out(vty,"when modify %s  %s ,parameter static/dynamic is not match!\n",argv[3],ip_str);
        			}
        			else if(flag == 1){
        				vty_out(vty,"write %s  %s fail!\n",argv[3],ip_str);
        			}
        			else if(flag == 2){
        				//do nothing
        			}
        			else{
        				vty_out(vty,"%s  %s get bad flag result!\n",argv[3],ip_str);
        			}
        		}
			}
    		
    		dbus_message_unref(reply);
			
    	    Indirect_add_reg_L +=1;
    		if(Indirect_add_reg_L == 0xffff)
    		{
                Indirect_add_reg_H += 1;
    			Indirect_add_reg_L = 0;
    		}
			
    	} else {
        	vty_out(vty, "no connection to slot %d\n", slot_id);
        	return CMD_WARNING;
    	}
    }
	vty_out(vty, "write car end!\n");
	return CMD_SUCCESS;
}

DEFUN(car_table_write,
	car_table_write_cmd,
	"set fpga car-table slot SLOT_ID (add|delete|modify) ip USRIP (uplink|downlink) speed_limit <0-1024> (static|dynamic) num <1-8192>",
	"set system parameters\n"
	"set fpga parameters\n"
	"set fpga car table \n"
	"the slot id \n"
	"the slot id num \n"
	"delete for invalid car entry\n"
	"add for valid car entry\n"
	"modify for modify car entry\n"
	"usr ip \n"
	"ip address \n"
	"uplink\n"
	"downlink\n"
	"speed_limit value\n"
	"speed_limit range:0k~1024k\n"
	"static for car entry property\n"
	"dynamic for valid car property\n"
	"the number of Batch Process\n"
	"the number range:1-1024\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int flag;
	int i =0;
	int slot_id;
	unsigned short Indirect_add_reg_H = 0;
	unsigned short Indirect_add_reg_L = 0;
	unsigned int RELOAD = 0;
	unsigned int user_ip = 0;
    unsigned short RELOAD_H =0;
	unsigned short RELOAD_L =0;
    unsigned short LINKUP =0;
    unsigned short CAR_VALID =0;
	unsigned short CAR_PROPERTY =0;
	unsigned short num = 0;
	char *endptr = NULL;
	char ip_str[16];
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
	vty_out(vty,"begin to write car table,wait...\n");
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	
    ret = inet_aton(argv[2], &userip_val);
    if(ret)
    {   
        //vty_out(vty,"**the userip = %x\n",userip_val.s_addr);
    }else
    {   
        printf("**ip conversion error,invalid parameter!\n");
		return CMD_WARNING;
    }
    Indirect_add_reg_H = (userip_val.s_addr &0xffff0000) >> 16;
	Indirect_add_reg_L = userip_val.s_addr &0xffff;

	if((Indirect_add_reg_L & 0x00ff) == 0xff)
	{
        vty_out(vty,"%% the ip is Broadcast packets!\n");
		return CMD_WARNING;
	}
	if((Indirect_add_reg_L & 0x00ff) == 0x0)
	{
        vty_out(vty,"%% the ip is invalid!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp("uplink",argv[3],strlen((char*)argv[3])))   /*"uplink" bit 0 for 1*/
	{
		LINKUP = 1;
	}
	else if(0 == strncmp("downlink",argv[3],strlen((char*)argv[3]))) /*"downlink" bit 0 for 0*/
	{
		LINKUP = 0;
	}
	else 
	{
    	vty_out(vty,"%% Unknow input param 3\n");
		return CMD_WARNING;
	}

	ret = fpga_parse_int((char *)argv[4],&RELOAD);
	if(0 != ret){
		vty_out(vty,"parse param error\n");
		return CMD_WARNING;
	}

	if((RELOAD < 0)||(RELOAD > 0xffffff)) {
		vty_out(vty,"%% bad value RELOAD input!\n");
		return CMD_WARNING;
	}
	RELOAD_H = (RELOAD & 0xff0000) >> 16;
    RELOAD_L =RELOAD & 0xffff;

	if(0 == strncmp("add",argv[1],strlen((char*)argv[1])))
	{
		CAR_VALID = 1;
	}
	else if(0 == strncmp("delete",argv[1],strlen((char*)argv[1])))
	{
		CAR_VALID = 0;
	}
	else if(0 == strncmp("modify",argv[1],strlen((char*)argv[1])))
	{
		CAR_VALID = 2;
	}
	else
	{
    	vty_out(vty,"%% Unknow input param 2\n");
		return CMD_WARNING;
	}

	if(0 == strncmp("static",argv[5],strlen((char*)argv[5])))
	{
		CAR_PROPERTY = 0;
	}
	else if(0 == strncmp("dynamic",argv[5],strlen((char*)argv[5])))
	{
		CAR_PROPERTY = 1;
	}
	else
	{
    	vty_out(vty,"%% Unknow input param 5\n");
		return CMD_WARNING;
	}

	num = strtol(argv[6],NULL,10);
	if(num > 8192||num < 0)
	{
        vty_out(vty,"Invalid num parameter,0~131072!\n");
		return CMD_WARNING;	
	}

/*	
	vty_out(vty,"******slot_id = 0x%x******\n",slot_id);
	vty_out(vty,"******Indirect_add_reg_H = 0x%x******\n",Indirect_add_reg_H);
	vty_out(vty,"******Indirect_add_reg_L = 0x%x******\n",Indirect_add_reg_L);
	vty_out(vty,"******RELOAD_H = 0x%x******\n",RELOAD_H);
	vty_out(vty,"******RELOAD_L = 0x%x******\n",RELOAD_L);
	vty_out(vty,"******LINKUP = 0x%x******\n",LINKUP);
	vty_out(vty,"******CAR_VALID = 0x%x******\n",CAR_VALID);
    vty_out(vty,"******num = %d******\n",num);
*/
    for(i=0;i <= num-1;i++)
    {
		if((Indirect_add_reg_L & 0x00ff)==0)
		{
			Indirect_add_reg_L++;
			i--;
            continue;
		}
		if((Indirect_add_reg_L & 0x00ff)==0xff)
		{
			Indirect_add_reg_L++;
			i--;
            continue;
		}
	    user_ip = Indirect_add_reg_H ;
		user_ip = (user_ip <<16)+Indirect_add_reg_L;
		user_ip = htonl(user_ip);
     	if(inet_ntop(AF_INET,&user_ip,ip_str,16) == NULL)
     	{
           vty_out(vty,"ip conversion error \n");
    	}
		
    	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
    												 SEM_DBUS_INTERFACE, SEM_DBUS_CAR_WRITE);
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
                            DBUS_TYPE_UINT16, &Indirect_add_reg_H,
    	                    DBUS_TYPE_UINT16, &Indirect_add_reg_L,
    						DBUS_TYPE_UINT16, &RELOAD_H,
    						DBUS_TYPE_UINT16, &RELOAD_L,
    						DBUS_TYPE_UINT16, &LINKUP,
    						DBUS_TYPE_UINT16, &CAR_VALID,
    						DBUS_TYPE_UINT16, &CAR_PROPERTY,
    						DBUS_TYPE_INVALID);
    	
    	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
    			query, 60000, &err);
    		dbus_message_unref(query);

    		if (NULL == reply){
    			vty_out(vty,"<error> failed get reply.\n");
    			if (dbus_error_is_set(&err)) {
    				vty_out(vty,"%s raised: %s",err.name,err.message);
    				dbus_error_free_for_dcli(&err);
    			}
    			continue;
    		}
/*
*return :
0 for write car table fail;
1 for user already exit;
2 for user is static;
3 for user exit but reload is different, modify reload value;
4 for chang user property to static;
5 for chang user property to static and modify reload val
6 for user is in the white list;
7 for user is not exit,write success,insert success!;
8 for user is not exit,insert fail.
*/
            if(CAR_VALID == 1)//add
            {
        		if (dbus_message_get_args (reply, &err,
        						DBUS_TYPE_UINT32,&flag,
        						DBUS_TYPE_INVALID)) {
        			if(flag == 0){
        				vty_out(vty,"write %s  %s table fail!\n",argv[3],ip_str);
        			}
					else if(flag == 1){
						vty_out(vty,"%s  %s is already exit!\n",argv[3],ip_str);
					}
					else if(flag == 3){
						vty_out(vty,"%s  %s is exit but speed limit is modifyed %u!\n",argv[3],ip_str,RELOAD);
					}
					else if(flag == 2){
						vty_out(vty,"%s  %s is static.if change to dynamic ,fist delete it!\n",argv[3],ip_str);
					}
					else if(flag == 5){
						vty_out(vty,"%s  %s has already changed to static and modify speed limit to %u!\n",argv[3],ip_str,RELOAD);
					}
					else if(flag == 4){
						vty_out(vty,"%s  %s has already changed to static!\n",argv[3],ip_str);
					}
					else if(flag == 6){
						vty_out(vty,"%s  %s is in white car list!\n",argv[3],ip_str);
					}
					else if(flag == 8){
						vty_out(vty,"insert %s  %s  in car list fail!\n",argv[3],ip_str);
					}
					else if(flag == 7){
        				//do nothing
        			}
					else{
						vty_out(vty,"%s  %s get bad flag result!\n",argv[3],ip_str);
				    }
        		}
            }
/*
return :
0 for this node don't exit;
1 for delete success;
4 for delete fail;
2,3 for parameter property is not match.
*/
			else if(CAR_VALID == 0)//delete
			{
        		if (dbus_message_get_args (reply, &err,
        						DBUS_TYPE_UINT32,&flag,
        						DBUS_TYPE_INVALID)) {
        			if(flag == 0){
        				vty_out(vty,"%s  %s is not exit!\n",argv[3],ip_str);
        			}
					else if(flag == 4){
                        vty_out(vty,"delete %s  %s fail!\n",argv[3],ip_str);
					}
        			else if(flag == 2 || flag == 3){
        				vty_out(vty,"when delete %s  %s ,parameter static/dynamic is not match!\n",argv[3],ip_str);
        			}
					else if(flag == 1){
        				//do nothing
        			}
					else{
                        vty_out(vty,"%s  %s get bad flag result!\n",argv[3],ip_str);
					}
        		}
			}
/*
*return :
0 for this node don't exit;
-1 for parameter wrong;
1 for write car table wrong;
2 for success
*/
			else if(CAR_VALID == 2)//modify
			{
        		if (dbus_message_get_args (reply, &err,
        						DBUS_TYPE_UINT32,&flag,
        						DBUS_TYPE_INVALID)) {
        			if(flag == 0){
        				vty_out(vty,"%s  %s is not exit!\n",argv[3],ip_str);
        			}
        			else if(flag == -1){
        				vty_out(vty,"when modify %s  %s ,parameter static/dynamic is not match!\n",argv[3],ip_str);
        			}
        			else if(flag == 1){
        				vty_out(vty,"write %s  %s fail!\n",argv[3],ip_str);
        			}
        			else if(flag == 2){
        				//do nothing
        			}
        			else{
        				vty_out(vty,"%s  %s get bad flag result!\n",argv[3],ip_str);
        			}
        		}
			}
    		
    		dbus_message_unref(reply);
			
    	    Indirect_add_reg_L +=1;
    		if(Indirect_add_reg_L == 0xffff)
    		{
                Indirect_add_reg_H += 1;
    			Indirect_add_reg_L = 0;
    		}

    		} else {
    		vty_out(vty, "no connection to slot %d\n", slot_id);
    		return CMD_WARNING;
    		}
    }
	vty_out(vty, "write car end!\n");
	return CMD_SUCCESS;
}

DEFUN(car_white_list_write,
	car_white_list_write_cmd,
	"set fpga car-white-list slot SLOT_ID (add|delete) ip USRIP (uplink|downlink) num <1-1024>",
	"set system parameters\n"
	"set fpga parameters\n"
	"set fpga car white list \n"
	"the slot id \n"
	"the slot id num \n"
	"delete for invalid car entry\n"
	"add for valid car entry\n"
	"usr ip \n"
	"ip address \n"
	"uplink\n"
	"downlink\n"
	"the number of Batch Process\n"
	"the number range:1-1024\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int i =0;
	int slot_id;
	unsigned short Indirect_add_reg_H = 0;
	unsigned short Indirect_add_reg_L = 0;
    unsigned short LINKUP =0;
    unsigned short CAR_WHITE_FLAG =0;
	unsigned short num = 0;
	unsigned int car_node_ip;
	char *endptr = NULL;
	char ip_str[16];
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
	vty_out(vty,"begin to write car table,wait .");
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	
    ret = inet_aton(argv[2], &userip_val);
    if(ret)
    {   
		vty_out(vty,".");
        //vty_out(vty,"**the userip = %x\n",userip_val.s_addr);
    }else
    {   
        printf("**ip conversion error,invalid parameter!\n");
		return CMD_WARNING;
    }
    Indirect_add_reg_H = (userip_val.s_addr &0xffff0000) >> 16;
	Indirect_add_reg_L = userip_val.s_addr &0xffff;
	if((Indirect_add_reg_L & 0x00ff) == 0xff)
	{
        vty_out(vty,"%% the ip is Broadcast packets!\n");
		return CMD_WARNING;
	}
	if((Indirect_add_reg_L & 0x00ff) == 0x0)
	{
        vty_out(vty,"%% the ip is invalid!\n");
		return CMD_WARNING;
	}

	
	if(0 == strncmp("uplink",argv[3],strlen((char*)argv[3])))   /*"uplink" bit 0 for 1*/
	{
		vty_out(vty,".");
		LINKUP = 1;
	}
	else if(0 == strncmp("downlink",argv[3],strlen((char*)argv[3]))) /*"downlink" bit 0 for 0*/
	{
		vty_out(vty,".");
		LINKUP = 0;
	}
	else 
	{
    	vty_out(vty,"%% Unknow input param 3\n");
		return CMD_WARNING;
	}

	if(0 == strncmp("add",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		CAR_WHITE_FLAG = 1;
	}
	else if(0 == strncmp("delete",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		CAR_WHITE_FLAG = 0;
	}
	else
	{
    	vty_out(vty,"%% Unknow input param 2\n");
		return CMD_WARNING;
	}

	num = strtol(argv[4],NULL,10);
	if(num > 2048||num < 0)
	{
        vty_out(vty,"Invalid num parameter,0~256!\n");
		return CMD_WARNING;	
	}
/*
	vty_out(vty,"******slot_id = 0x%x******\n",slot_id);
	vty_out(vty,"******Indirect_add_reg_H = 0x%x******\n",Indirect_add_reg_H);
	vty_out(vty,"******Indirect_add_reg_L = 0x%x******\n",Indirect_add_reg_L);
	vty_out(vty,"******LINKUP = 0x%x******\n",LINKUP);
	vty_out(vty,"******CAR_VALID = 0x%x******\n",CAR_WHITE_FLAG);
    vty_out(vty,"******num = %d******\n",num);
*/
    for(i=0;i <= num-1;i++)
    {
		if((Indirect_add_reg_L & 0x00ff)==0)
		{
			Indirect_add_reg_L++;
			i--;
            continue;
		}
		if((Indirect_add_reg_L & 0x00ff)==0xff)
		{
			Indirect_add_reg_L++;
			i--;
            continue;
		}
		
		car_node_ip = Indirect_add_reg_H;
	    car_node_ip = car_node_ip << 16;
	    car_node_ip = car_node_ip + Indirect_add_reg_L;
        car_node_ip = htonl(car_node_ip);
        if(inet_ntop(AF_INET,&car_node_ip,ip_str,16) == NULL)
        {
            vty_out(vty,"ip conversion error \n");
        }
		
        query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
        											 SEM_DBUS_INTERFACE, SEM_DBUS_CAR_WHITE_WRITE);
        dbus_error_init(&err);

        dbus_message_append_args(query,
        	                //DBUS_TYPE_UINT16, &num,
                            DBUS_TYPE_UINT16, &Indirect_add_reg_H,
                            DBUS_TYPE_UINT16, &Indirect_add_reg_L,
        					DBUS_TYPE_UINT16, &LINKUP,
        					DBUS_TYPE_UINT16, &CAR_WHITE_FLAG,
        					DBUS_TYPE_INVALID);

        if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
        	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
        		query, 60000, &err);
        	dbus_message_unref(query);

        	if (NULL == reply){
        		vty_out(vty,"<error> failed get reply.\n");
        		if (dbus_error_is_set(&err)) {
        			vty_out(vty,"%s raised: %s",err.name,err.message);
        			dbus_error_free_for_dcli(&err);
        		}
        		continue;
        	}

        	if (dbus_message_get_args (reply, &err,
        					DBUS_TYPE_INT32,&ret,
        					DBUS_TYPE_INVALID)) {
				if(CAR_WHITE_FLAG == 1)
				{
            		if(ret == 0)
					{
            			vty_out(vty, "linkstate %s ip %s insert fail\n",argv[3],ip_str);
            		}
            		else if(ret == 1)
					{
            			//vty_out(vty, "linkstate %s ip %s insert success\n",argv[3],argv[2] );
            			
            		}
					else if(ret == 2)
					{
            			vty_out(vty, "linkstate %s ip %s already exit\n",argv[3],ip_str);
            			
            		}
        		}
				else if(CAR_WHITE_FLAG == 0)
				{
                    if(ret == 0)
					{
            			vty_out(vty, "linkstate %s ip %s no exit\n",argv[3],ip_str);
            		}
            		else if(ret == 1)
					{
            			//vty_out(vty, "linkstate %s ip %s delete success\n",argv[3],ip_str);
            		}
				}
				else
				{
                    vty_out(vty, "linkstate %s ip %s bad dbus return!\n",argv[3],ip_str);
				}
        	}
        	
        	dbus_message_unref(reply);
			
			Indirect_add_reg_L +=1;
    		if(Indirect_add_reg_L == 0xffff)
    		{
                Indirect_add_reg_H += 1;
    			Indirect_add_reg_L = 0;
    		}
			
        	} else {
        	vty_out(vty, "no connection to slot %d\n", slot_id);
        	return CMD_WARNING;
        	}
    }
	vty_out(vty, "\nwrite car white list node success!\n");
	return CMD_SUCCESS;	
	
}


DEFUN(car_list_show,
	car_list_read_cmd,
	"show fpga car-list slot SLOT_ID",
	"Show system information\n"
	"Show fpga information\n"
	"show car list \n"
	"the slot id \n"
	"the slot id num \n")
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int flag =0;
	int slot_id;
	char ip_str[16];
	unsigned long long car_count;
	unsigned long long i;
	unsigned int car_node_ip;
	unsigned int car_node_reload;
	unsigned short car_node_link;
	unsigned short car_node_property;
	unsigned int usr_ip;
	char *endptr = NULL;
	char *linkstate = NULL;
	char *property = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
	vty_out(vty,"begin to show car list,wait...\n");
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	/*get the car list count*/

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_GET_CAR_LIST_COUNT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &car_node_ip,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	{
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT64,&car_count,
						DBUS_TYPE_INVALID))
		{		
				vty_out(vty,"the total number of the list node is %lld\n",car_count);
		}
	}
    else
	{
		vty_out(vty, "when get the the total number of the list node,no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
    /*end*/
	#if 1
	vty_out(vty, "ip                speed limit     link      property\n");
	for (i = 1; i <=car_count; i++)
	{   
	    query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_CAR_LIST);
	    dbus_error_init(&err);

	    dbus_message_append_args(query,
						DBUS_TYPE_UINT64, &i,
						DBUS_TYPE_INVALID);
	
	    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	    {
		    reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		    dbus_message_unref(query);

		    if (NULL == reply)
			{
        		vty_out(vty,"<error> failed get reply.\n");
        		if (dbus_error_is_set(&err)) 
				{
        			vty_out(vty,"%s raised: %s",err.name,err.message);
        			dbus_error_free_for_dcli(&err);
			    }
			    continue;
		    } 
			
			if (dbus_message_get_args (reply, &err,
				        DBUS_TYPE_INT32,&flag,
    					DBUS_TYPE_UINT32,&car_node_ip,
                        DBUS_TYPE_UINT32,&car_node_reload,
			            DBUS_TYPE_UINT16,&car_node_link,
			            DBUS_TYPE_UINT16,&car_node_property,
    					DBUS_TYPE_INVALID))
    		{
        		 car_node_ip = htonl(car_node_ip);
        	 	 if(inet_ntop(AF_INET,&car_node_ip,ip_str,16) == NULL)
        	 	 {
                    vty_out(vty,"ip conversion error \n");
        		 }
				 
				 if(car_node_link == 1)
				 {
                    linkstate = "uplink";
				 }
				 else if(car_node_link == 0)
				 {
                    linkstate = "downlink";
				 }
				 else
				 {
                    vty_out(vty,"bad car_node_link parameter.\n");
				 }

				 if(car_node_property == 1)
				 {
                    property = "dynamic";
				 }
				 else if(car_node_property == 0)
				 {
                    property = "static";
				 }
				 else
				 {
                    vty_out(vty,"bad car_node_property parameter.\n");
				 }
				 
				 if(flag == 0)
				 {
            	    vty_out(vty,"%-22s%-10u%-12s%s\n",ip_str,car_node_reload,linkstate,property);
				 }
				 else if(flag == 1)
				 {
                    vty_out(vty,"read %ull node fail\n",i);
				 }
				 else
				 {
				 	vty_out(vty,"when read %ull node ,rev flag is wrong!\n",i);
				 }
    	    }
    		dbus_message_unref(reply);
    	} 
    	else 
    	{
    		vty_out(vty, "when get the car list node,no connection to slot %d\n", slot_id);
    		return CMD_WARNING;
    	}
	}
	return CMD_SUCCESS;
	#endif
}
DEFUN(car_white_list_show,
	car_white_list_read_cmd,
	"show fpga car-white-list slot SLOT_ID",
	"Show system information\n"
	"Show fpga information\n"
	"show car white list \n"
	"the slot id \n"
	"the slot id num \n")
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int flag=0;
	int slot_id;
	char ip_str[16];
	unsigned long long car_count;
	unsigned long long i;
	unsigned int car_node_ip;
	unsigned int car_node_reload;
	unsigned short car_node_link;
	unsigned short car_node_property;
	unsigned int usr_ip;
	char *endptr = NULL;
	char *linkstate = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
	vty_out(vty,"begin to show car list,wait...\n");
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	/*get the car list count*/

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_GET_CAR_WHITE_LIST_COUNT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT16, &car_node_ip,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	{
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT64,&car_count,
						DBUS_TYPE_INVALID))
		{		
				vty_out(vty,"the total number of the list node is %lld\n",car_count);
		}
	}
    else
	{
		vty_out(vty, "when get the the total number of the list node,no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
    /*end*/

	vty_out(vty, "ip             linkstate\n");
	for (i = 1; i <=car_count; i++)
	{   
	    query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_CAR_WHITE_LIST);
	    dbus_error_init(&err);

	    dbus_message_append_args(query,
						DBUS_TYPE_UINT64, &i,
						DBUS_TYPE_INVALID);
	
	    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	    {
		    reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		    dbus_message_unref(query);

		    if (NULL == reply)
			{
        		vty_out(vty,"<error> failed get reply.\n");
        		if (dbus_error_is_set(&err)) 
				{
        			vty_out(vty,"%s raised: %s",err.name,err.message);
        			dbus_error_free_for_dcli(&err);
			    }
			    continue;
		    } 
			
			if (dbus_message_get_args (reply, &err,
				        DBUS_TYPE_INT32,&flag,
    					DBUS_TYPE_UINT32,&car_node_ip,
			            DBUS_TYPE_UINT16,&car_node_link,
    					DBUS_TYPE_INVALID))
    		{
        		 car_node_ip = htonl(car_node_ip);
        	 	 if(inet_ntop(AF_INET,&car_node_ip,ip_str,16) == NULL)
        	 	 {
                    vty_out(vty,"ip conversion error \n");
        		 }
				 if(car_node_link == 1)
				 {
                    linkstate = "uplink";
				 }
				 else if(car_node_link == 0)
				 {
                    linkstate = "downlink";
				 }
				 else
				 {
                    vty_out(vty,"bad car_node_link parameter.\n");
				 }
				 
                 if(flag == 0)
                 {
            	    vty_out(vty,"%-16s%s\n",ip_str,linkstate);
				 }
				 else if(flag == 1)
				 {
                    vty_out(vty,"read %ull node fail,the car linklist is empty!\n",i);
				 }
				 else
				 {
				 	vty_out(vty,"when read %ull node ,rev flag is wrong!\n",i);
				 }
    	    }
    		dbus_message_unref(reply);
    	} 
    	else 
    	{
    		vty_out(vty, "when get the car list node,no connection to slot %d\n", slot_id);
    		return CMD_WARNING;
    	}
	}
	return CMD_SUCCESS;

}

DEFUN(reset_car_list,
	reset_car_list_cmd,
	"set fpga car-restore slot SLOT_ID",
	"Show system information\n"
	"Show fpga information\n"
	"set car table restore\n"
	"the slot id \n"
	"the slot id num \n")
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret;
	int slot_id;
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	int result;
	
	vty_out(vty,"begin to restore car table,wait...\n");
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_GET_CAR_TABLE_RESTORE);
	dbus_error_init(&err);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	{
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_INT32,&result,
						DBUS_TYPE_INVALID))
		{
			vty_out(vty,"ret = %d\n",ret);
		    if(result == 0){
			    vty_out(vty,"restore the car table success\n");
	        }else if(result == 1){
                vty_out(vty,"when restore the car table,malloc failed\n");
			}else if(result == 2){
                vty_out(vty,"when restore the car table,read linklist failed\n");
			}else if(result == 3){
                vty_out(vty,"when restore the car table,write car table failed\n");
			}else{
                vty_out(vty,"bad return value!\n");
			}
		}
	}
    else
	{
		vty_out(vty, "when restore the car table,no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}

DEFUN(empty_car_list,
	empty_car_list_cmd,
	"set fpga car-empty slot SLOT_ID",
	"Show system information\n"
	"Show fpga information\n"
	"set car table empty\n"
	"the slot id \n"
	"the slot id num \n")
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret;
	int slot_id;
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	int result;
	
	vty_out(vty,"begin to empty car table,wait...\n");
	vty_out(vty,"The time required is about 4 minutes!\n");
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_GET_CAR_TABLE_EMPTY);
	dbus_error_init(&err);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	{
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_INT32,&result,
						DBUS_TYPE_INVALID))
		{
		    if(result == 0){
			    vty_out(vty,"Empty the car table success\n");
	        }else if(result == 1){
                vty_out(vty,"when Empty the car table,something wrong!\n");
			}else{
                vty_out(vty,"bad return value!\n");
			}
		}
	}
    else
	{
		vty_out(vty, "when restore the car table,no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}


/*test the car table hash collision rate*/
DEFUN(show_fpga_car_valid,
	show_fpga_car_valid_cmd,
	"show fpga car-valid SLOT_ID",
	"show system parameters\n"
	"show fpga parameters\n"
	"show fpga car table valid\n"
	"the slot id num \n")
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	int slot_id;
	unsigned int car_id=0;
	unsigned int count_valid=0;
	unsigned int i=0;
	int flag;
	unsigned int car_id_print=0;

	struct timeval start, end;
    int interval;
	
	vty_out(vty,"begin to show fpga car table valid,wait .\n");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}

    for(car_id =0;car_id<10;car_id++)
    {
		flag = 1;
		dbus_error_init(&err);
	    query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_FPGA_CAR_VALID);
	
	    dbus_message_append_args(query,
                        DBUS_TYPE_UINT32, &car_id,
                        DBUS_TYPE_INT32, &flag,
                        DBUS_TYPE_UINT32, &count_valid,
						DBUS_TYPE_INVALID);

	    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&count_valid,
						DBUS_TYPE_INVALID)) {
			car_id_print = car_id * 10000;
            vty_out(vty, "car_id %d~%d,car valid count = %d\n",car_id_print,car_id_print+9999,count_valid);
		}
		dbus_message_unref(reply);
		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
    }

	flag = 0;
	car_id = 10;
	dbus_error_init(&err);
    query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
											 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_FPGA_CAR_VALID);

    dbus_message_append_args(query,
                    DBUS_TYPE_UINT32, &car_id,
                    DBUS_TYPE_INT32, &flag,
                    DBUS_TYPE_UINT32, &count_valid,
					DBUS_TYPE_INVALID);

    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
		query, 60000, &err);
	dbus_message_unref(query);

	if (NULL == reply){
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args (reply, &err,
					DBUS_TYPE_UINT32,&count_valid,
					DBUS_TYPE_INVALID)) {
		car_id = car_id * 10000;
        vty_out(vty, "car_id %d~%d,car valid count = %d\n",car_id,car_id+7151,count_valid);
	}
	dbus_message_unref(reply);
	} else {
	vty_out(vty, "no connection to slot %d\n", slot_id);
	return CMD_WARNING;
	}

    vty_out(vty, "***************car table valid count = %d*************\n",count_valid);
return CMD_WARNING;
}


#if 0
DEFUN(car_list_show_debug,
	car_list_read_debug_cmd,
	"show car-list-debug SLOT_ID",
	"Show system information\n"
	"show car list \n"
	"the slot id num \n")
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	unsigned long long car_count;
	
    vty_out(vty,"begin to show car list,wait...");

	slot_id = str_16_num(argv[0]);
	vty_out(vty,"******slot_id = 0x%x******\n",slot_id);

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_CAR_LIST_DEBUG);
	dbus_error_init(&err);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	{
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		if (dbus_message_get_args (reply, &err,
			DBUS_TYPE_INT32,&ret,
			DBUS_TYPE_INVALID))
		{
            if(ret == 1)
            {
                vty_out(vty," show car list,success");
			}
			vty_out(vty," show car list,fail");
	    }
		dbus_message_unref(reply);

	}
    else
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}

	return CMD_SUCCESS;

}
#endif

DEFUN(cam_table_read,
	cam_table_read_cmd,
	"show fpga cam-table SLOTID CAMID",
	"Show system information\n"
	"Show fpga information\n"
	"show fpga cam table \n"
	"the slot id num \n"
	"the cam id (0~63) \n")
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short ret;
	int slot_id;
	unsigned int cam_id;
	unsigned short Indirect_add_reg_H = 0;
	unsigned short Indirect_add_reg_L = 0;
	unsigned short cam_table_id = 0;
	unsigned short sip_H = 0;
	unsigned short sip_L = 0;
	unsigned short sport = 0;
	unsigned short smac_H = 0;
    unsigned short smac_M = 0;
	unsigned short smac_L = 0;
	unsigned short tunnel_sip_H = 0;
	unsigned short tunnel_sip_L = 0;
	unsigned short tunnel_sport = 0;
	unsigned short dip_H = 0;
	unsigned short dip_L = 0;
	unsigned short dport = 0;
	unsigned short dmac_H = 0;
    unsigned short dmac_M = 0;
	unsigned short dmac_L = 0;
	unsigned short tunnel_dip_H = 0;
	unsigned short tunnel_dip_L = 0;
	unsigned short tunnel_dport = 0;
    unsigned short protocol = 0;
	unsigned short setup_time = 0;
	unsigned short hash_ctl = 0;
	unsigned short vlan_out_H = 0;
	unsigned short vlan_out_L = 0;
	unsigned short vlan_in_H = 0;
	unsigned short vlan_in_L = 0;
	unsigned short ethernet_protocol = 0;
	unsigned short ip_tos = 0;
	unsigned short ip_len = 0;
	unsigned short ip_offset = 0;
	unsigned short ip_ttl = 0;
	unsigned short ip_protocol = 0;
    unsigned short ip_checksum = 0;
	unsigned short udp_len = 0;
    unsigned short udp_checksum = 0;	
    unsigned short capwap_header_b0 = 0;
	unsigned short capwap_header_b2 = 0;
	unsigned short capwap_header_b4 = 0;
	unsigned short capwap_header_b6 = 0;
	unsigned short capwap_header_b8 = 0;
	unsigned short capwap_header_b10 = 0;
	unsigned short capwap_header_b12 = 0;
	unsigned short capwap_header_b14 = 0;
    unsigned short header_802_11_b0 = 0;
	unsigned short header_802_11_b2 = 0;
	unsigned short header_802_11_b4 = 0;
	unsigned short header_802_11_b6 = 0;
	unsigned short header_802_11_b8 = 0;
	unsigned short header_802_11_b10 = 0;
	unsigned short header_802_11_b12 = 0;
	unsigned short header_802_11_b14 = 0;
	unsigned short header_802_11_b16 = 0;
	unsigned short header_802_11_b18 = 0;
	unsigned short header_802_11_b20 = 0;
	unsigned short header_802_11_b22 = 0;
	unsigned short header_802_11_b24 = 0;
	unsigned short header_802_11_b26 = 0;
	unsigned short capwap_protocol = 0;	

	char sip_str[16];
	char dip_str[16];
	char tunnel_sip_str[16];
	char tunnel_dip_str[16];
	unsigned int usr_sip;
	unsigned int usr_dip;
	unsigned int usr_tunnel_sip;
	unsigned int usr_tunnel_dip;

	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	ret = fpga_parse_int((char *)argv[1],&cam_id);
	if(0 != ret){
		vty_out(vty,"parse param error\n");
		return CMD_WARNING;
	}
	if(cam_id > 63 || cam_id < 0)
	{
        vty_out(vty,"parse cam_id error!0~63\n");
		return CMD_WARNING;
	}


    Indirect_add_reg_H = cam_id >> 16;
	Indirect_add_reg_L = cam_id & 0x0000ffff;
/*	
    //test whether the convert is right
	vty_out(vty,"******slot_id = 0x%x******\n",slot_id);
	vty_out(vty,"******Indirect_add_reg_H = 0x%x******\n",Indirect_add_reg_H);
	vty_out(vty,"******Indirect_add_reg_L = 0x%x******\n",Indirect_add_reg_L);
*/	
    vty_out(vty,"begin to read cam table,wait...\n");
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CAM_READ);
	dbus_error_init(&err);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT16, &Indirect_add_reg_H,
	                    DBUS_TYPE_UINT16, &Indirect_add_reg_L,
						DBUS_TYPE_INVALID);

	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		   
		//vty_out(vty,"******dbus_message_get_args reply******\n");
		if (dbus_message_get_args (reply, &err,
    					 DBUS_TYPE_UINT16,&ret,
    					 DBUS_TYPE_UINT16,&cam_table_id,
                         DBUS_TYPE_UINT16,&sip_H,
                         DBUS_TYPE_UINT16,&sip_L,
    					 DBUS_TYPE_UINT16,&sport,
    					 DBUS_TYPE_UINT16,&smac_H,
    					 DBUS_TYPE_UINT16,&smac_M,
    					 DBUS_TYPE_UINT16,&smac_L,
    					 DBUS_TYPE_UINT16,&tunnel_sip_H,
    					 DBUS_TYPE_UINT16,&tunnel_sip_L,
    					 DBUS_TYPE_UINT16,&tunnel_sport,
    					 DBUS_TYPE_UINT16,&dip_H,
                         DBUS_TYPE_UINT16,&dip_L,
                         DBUS_TYPE_UINT16,&dport,
    					 DBUS_TYPE_UINT16,&dmac_H,
    					 DBUS_TYPE_UINT16,&dmac_M,
    					 DBUS_TYPE_UINT16,&dmac_L,
    					 DBUS_TYPE_UINT16,&tunnel_dip_H,
    					 DBUS_TYPE_UINT16,&tunnel_dip_L,
    					 DBUS_TYPE_UINT16,&tunnel_dport,
    					 DBUS_TYPE_UINT16,&protocol,
    					 DBUS_TYPE_UINT16,&setup_time,
                         DBUS_TYPE_UINT16,&hash_ctl,
                         DBUS_TYPE_UINT16,&vlan_out_H,
    					 DBUS_TYPE_UINT16,&vlan_out_L,
    					 DBUS_TYPE_UINT16,&vlan_in_H,
    					 DBUS_TYPE_UINT16,&vlan_in_L,
    					 DBUS_TYPE_UINT16,&ethernet_protocol,
    					 DBUS_TYPE_UINT16,&ip_tos,
    					 DBUS_TYPE_UINT16,&ip_len,
    					 DBUS_TYPE_UINT16,&ip_offset,
    					 DBUS_TYPE_UINT16,&ip_ttl,
                         DBUS_TYPE_UINT16,&ip_protocol,
                         DBUS_TYPE_UINT16,&ip_checksum,
    					 DBUS_TYPE_UINT16,&udp_len,
    					 DBUS_TYPE_UINT16,&udp_checksum,
    					 DBUS_TYPE_UINT16,&capwap_header_b0,
    					 DBUS_TYPE_UINT16,&capwap_header_b2,
    					 DBUS_TYPE_UINT16,&capwap_header_b4,
    					 DBUS_TYPE_UINT16,&capwap_header_b6,
    					 DBUS_TYPE_UINT16,&capwap_header_b8,
    					 DBUS_TYPE_UINT16,&capwap_header_b10,
                         DBUS_TYPE_UINT16,&capwap_header_b12,
                         DBUS_TYPE_UINT16,&capwap_header_b14,
    					 DBUS_TYPE_UINT16,&header_802_11_b0,
    					 DBUS_TYPE_UINT16,&header_802_11_b2,
    					 DBUS_TYPE_UINT16,&header_802_11_b4,
    					 DBUS_TYPE_UINT16,&header_802_11_b6,
    					 DBUS_TYPE_UINT16,&header_802_11_b8,
    					 DBUS_TYPE_UINT16,&header_802_11_b10,
    					 DBUS_TYPE_UINT16,&header_802_11_b12,
    					 DBUS_TYPE_UINT16,&header_802_11_b14,
                         DBUS_TYPE_UINT16,&header_802_11_b16,
                         DBUS_TYPE_UINT16,&header_802_11_b18,
    					 DBUS_TYPE_UINT16,&header_802_11_b20,
    					 DBUS_TYPE_UINT16,&header_802_11_b22,
    					 DBUS_TYPE_UINT16,&header_802_11_b24,
    					 DBUS_TYPE_UINT16,&header_802_11_b26,
    					 DBUS_TYPE_UINT16,&capwap_protocol,
    					 DBUS_TYPE_INVALID))
    	{
                usr_sip = sip_H ;
            	usr_sip = (usr_sip <<16)+sip_L;
            	usr_sip = htonl(usr_sip);
             	if(inet_ntop(AF_INET,&usr_sip,sip_str,16) == NULL)
             	{
                   vty_out(vty,"sip conversion error \n");
            	}
                usr_dip = dip_H ;
            	usr_dip = (usr_dip <<16)+dip_L;
            	usr_dip = htonl(usr_dip);
             	if(inet_ntop(AF_INET,&usr_dip,dip_str,16) == NULL)
             	{
                   vty_out(vty,"dip conversion error \n");
            	}
                usr_tunnel_sip = tunnel_sip_H ;
            	usr_tunnel_sip = (usr_tunnel_sip <<16)+tunnel_sip_L;
            	usr_tunnel_sip = htonl(usr_tunnel_sip);
             	if(inet_ntop(AF_INET,&usr_tunnel_sip,tunnel_sip_str,16) == NULL)
             	{
                   vty_out(vty,"tunnel_sip conversion error \n");
            	}
				
                usr_tunnel_dip = tunnel_dip_H;
            	usr_tunnel_dip = (usr_tunnel_dip <<16)+tunnel_dip_L;
            	usr_tunnel_dip = htonl(usr_tunnel_dip);
             	if(inet_ntop(AF_INET,&usr_tunnel_dip,tunnel_dip_str,16) == NULL)
             	{
                   vty_out(vty,"tunnel_dip conversion error \n");
            	}
				
			    if(ret == 0)
				{
                	vty_out(vty,"\nDetail Information of CAM Table %x\n",cam_table_id);
                 	vty_out(vty,"=====================================================================\n");
					
                    vty_out(vty,"\nHash Label\n");
                	vty_out(vty,"\thash table creation time:           0x%x\n",setup_time);
                	vty_out(vty,"\thash ctl:                           0x%x\n",hash_ctl);

                    vty_out(vty,"\nFive Tuple\n");

                	vty_out(vty,"\tsource ip address:                %s\n",sip_str);
                    vty_out(vty,"\tdestination ip address:           %s\n",dip_str);
                	vty_out(vty,"\tsource port:                      %d\n",sport);
                	vty_out(vty,"\tdestination port:                 %d\n",dport);
                	vty_out(vty,"\tprotocol:                         %d\n",protocol);

                    vty_out(vty,"\n MAC header -> IP header -> UDP header\n");
                    vty_out(vty,"\tdestination mac H:                0x%x\n",dmac_H);
                	vty_out(vty,"\tdestination mac M:                0x%x\n",dmac_M);
                	vty_out(vty,"\tdestination mac L:                0x%x\n",dmac_L);
                    vty_out(vty,"\tsource mac H:                     0x%x\n",smac_H);
                	vty_out(vty,"\tsource mac M:                     0x%x\n",smac_M);
                	vty_out(vty,"\tsource mac L:                     0x%x\n",smac_L);
                	vty_out(vty,"\tethernet protocol:                0x%x\n",ethernet_protocol);
                	vty_out(vty,"\tvlan out H:                       0x%x\n",vlan_out_H);
                	vty_out(vty,"\tvlan out L:                       0x%x\n",vlan_out_L);
                    vty_out(vty,"\tvlan in H:                        0x%x\n",vlan_in_H);
                	vty_out(vty,"\tvlan in L:                        0x%x\n",vlan_in_L);
                    vty_out(vty,"\tip tos:                           0x%x\n",ip_tos);
                	vty_out(vty,"\tip length:                        0x%x\n",ip_len);
                	vty_out(vty,"\tip offset:                        0x%x\n",ip_offset);
                    vty_out(vty,"\tip ttl:                           %d\n",ip_ttl);
                	vty_out(vty,"\tip protocol:                      0x%x\n",ip_protocol);
					vty_out(vty,"\tip checksum:                      0x%x\n",ip_checksum);
                    vty_out(vty,"\ttunnel source ip:                 %s\n",tunnel_sip_str);
                    vty_out(vty,"\ttunnel destination ip:            %s\n",tunnel_dip_str);
                    vty_out(vty,"\ttunnel source port:               %d\n",tunnel_sport);
                    vty_out(vty,"\ttunnel destination port:          %d\n",tunnel_dport);
                	vty_out(vty,"\tudp length:                       0x%x\n",udp_len);
                	vty_out(vty,"\tudp checksum:                     0x%x\n",udp_checksum);

                    vty_out(vty,"\n capwap header -> 802.11 LLC header\n");
                	
                	vty_out(vty,"\tcapwap header:                    0x%x %x %x %x %x %x %x %x\n",capwap_header_b0,capwap_header_b2,capwap_header_b4,capwap_header_b6,capwap_header_b8,capwap_header_b10,capwap_header_b12,capwap_header_b14);
                	vty_out(vty,"\t802.11 header:                    0x%x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",header_802_11_b0,header_802_11_b2,header_802_11_b4,header_802_11_b6,header_802_11_b8,header_802_11_b10,header_802_11_b12,header_802_11_b14,header_802_11_b16,header_802_11_b18,header_802_11_b20,header_802_11_b22,header_802_11_b24,header_802_11_b26);

                    vty_out(vty,"\n");
                    vty_out(vty,"=====================================================================\n");

			}
			else
			{
				vty_out(vty,"read cam table dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		dbus_message_unref(reply);
		vty_out(vty, "read cam table command success\n");
		return CMD_SUCCESS;

	} 
	else
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
}


DEFUN(cam_table_write,
	cam_table_write_cmd,
	"set fpga cam-table SLOTID_CAMID_NUM SIP DIP SPORT DPORT PROTOCOL HASH_CTL DMAC SMAC VLAN_OUT VLAN_IN \
	IP_TOS IP_TTL_PROTOCOL TUNNEL_SIP TUNNEL_DIP TUNNEL_SPORT TUNNEL_DPORT CAPWAP_HEADER_H8 CAPWAP_HEADER_L8 \
	HEADER_1 HEADER_2 HEADER_3 HEADER_4",
	"set system parameters\n"
	"set fpga parameters\n"
	"set fpga cam table \n"
	"the slot id and cam id and num of process,eg 1:1:32\n"
	"source ip address.eg:192.168.1.1 \n"
	"destination ip address.eg:192.168.1.1 \n"
	"source port(D).eg:2000 \n"
	"destination port(D);3000 \n"
	"protocol(D) eg:17\n"
	"hash ctl(H) [7]:valid.[6]:1->to_ap 0->to_internet.[5]:1->to_wan 0->to_trunk.eg:0081\n"
	"destination mac.eg:001f64862010 \n"
	"source mac.eg:001f64861030 \n"
	"vlan out(H) eg:81000064\n"
	"vlan in(H) eg:810000c8\n"
	"ip tos(D) \n"
	"ip ttl and protocol(H) eg:4006 \n"
	"tunnel source ip.eg:192.168.1.1 \n"
	"tunnel destination ip.eg:192.168.1.1 \n"
	"tunnel source port(D).eg:2000 \n"
	"tunnel destination port(D).eg:3000 \n"
	"capwap header B0~B7(H)(0x0~0xffff ffff ffff ffff) \n"
	"capwap header B8~B15(H)(0x0~0xffff ffff ffff ffff) \n"
	"802.11 header B0~B7 byte(H)(0x0~0xffff ffff ffff ffff)\n"
	"802.11 header B8~B15 byte(H)(0x0~0xffff ffff ffff ffff)\n"
	"802.11 header B16~B23 byte(H)(0x0~0xffff ffff ffff ffff)\n"
	"802.11 header B24~B27 byte(H)(0x0~0xffff ffff)\n"
	)
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr sip_val;
	struct in_addr dip_val;
	struct in_addr tunnel_sip_val;
	struct in_addr tunnel_dip_val;
	int ret;
	int slot_id;
	unsigned short Indirect_add_reg_H = 0;
	unsigned short Indirect_add_reg_L = 0;
	unsigned short SIP_H=0;
	unsigned short SIP_L=0;
    unsigned short DIP_H=0;
	unsigned short DIP_L=0;
	unsigned short SPORT=0;
	unsigned short DPORT=0;
	unsigned short PROTOCOL=0;
	unsigned short HASH_CTL=0;
	unsigned short DMAC_H=0;
	unsigned short DMAC_M=0;
	unsigned short DMAC_L=0;
	unsigned short SMAC_H=0;
	unsigned short SMAC_M=0;
	unsigned short SMAC_L=0;
	unsigned int VLAN_OUT=0;
	unsigned short VLAN_OUT_H=0;
	unsigned short VLAN_OUT_L=0;
	unsigned int VLAN_IN=0;
	unsigned short VLAN_IN_H=0;
	unsigned short VLAN_IN_L=0;
	unsigned short IPTOS=0;
	unsigned short IPTOS_temp=0;
	unsigned short IP_TTL_IP_PROTOCOL=0;
	unsigned short IP_CHECKSUM=0;
	unsigned int IP_CHECKSUM_temp=0;
	unsigned short TUNNEL_SIP_H=0;
	unsigned short TUNNEL_SIP_L=0;
	unsigned short TUNNEL_DIP_H=0;
	unsigned short TUNNEL_DIP_L=0;
	unsigned short TUNNEL_SRC_PORT=0;
	unsigned short TUNNEL_DST_PORT=0;
	unsigned short CAPWAP_B0B1=0;
	unsigned short CAPWAP_B2B3=0;
	unsigned short CAPWAP_B4B5=0;
	unsigned short CAPWAP_B6B7=0;
	unsigned short CAPWAP_B8B9=0;
	unsigned short CAPWAP_B10B11=0;
	unsigned short CAPWAP_B12B13=0;
	unsigned short CAPWAP_B14B15=0;
	unsigned short H_802_11_B0B1=0;
	unsigned short H_802_11_B2B3=0;
	unsigned short H_802_11_B4B5=0;
	unsigned short H_802_11_B6B7=0;
	unsigned short H_802_11_B8B9=0;
	unsigned short H_802_11_B10B11=0;
	unsigned short H_802_11_B12B13=0;
	unsigned short H_802_11_B14B15=0;
	unsigned short H_802_11_B16B17=0;
	unsigned short H_802_11_B18B19=0;
	unsigned short H_802_11_B20B21=0;
	unsigned short H_802_11_B22B23=0;
	unsigned short H_802_11_B24B25=0;
	unsigned short H_802_11_B26B27=0;
	unsigned short NUM=0;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int i = 0;
	int val[3];

	vty_out(vty,"begin to write cam table,wait .");
	memset(val,0,sizeof(val));
    sscanf(argv[0], "%d:%d:%d", &val[0], &val[1], &val[2]);
    slot_id = val[0];
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	Indirect_add_reg_L = val[1];
    NUM = val[2];
	vty_out(vty,". ");

	ret = inet_aton(argv[1], &sip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //vty_out("**the sip = %x\n",sip_val.s_addr);
    }
	else
    {   
        printf("**sip conversion error\n");
    }
    ret = inet_aton(argv[2], &dip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //vty_out("**the dip = %x\n",dip_val.s_addr);
    }
	else
    {   
        vty_out(vty,"**dip conversion error\n");
    }  

    ret = inet_aton(argv[13], &tunnel_sip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //vty_out("**the tunnel_sip = %x\n",tunnel_sip_val.s_addr);
    }
	else
    {   
		vty_out(vty,". ");
        printf("**tunnel_sip conversion error\n");
    }
    ret = inet_aton(argv[14], &tunnel_dip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //printf("**the tunnel_dip = %x\n",tunnel_dip_val.s_addr);
    }
	else
    {   
        printf("**tunnel_dip conversion error\n");
    }

    SIP_H = sip_val.s_addr >> 16;
	SIP_L = sip_val.s_addr & 0x0000ffff;
    DIP_H = dip_val.s_addr >> 16;
    DIP_L = dip_val.s_addr & 0x0000ffff;

	ret = fpga_parse_short((char *)argv[3],&SPORT);
	if(0 != ret){
		vty_out(vty,"parse SPORT param error\n");
		return CMD_WARNING;
	}
    if(SPORT<0||SPORT>65535)
	{
		vty_out(vty," Bad Parameters,SPORT outrange!\n");
		return CMD_WARNING;
	}
	
	ret = fpga_parse_short((char *)argv[4],&DPORT);
	if(0 != ret){
		vty_out(vty,"parse DPORT param error\n");
		return CMD_WARNING;
	}
    if(DPORT<0||DPORT>65535)
	{
		vty_out(vty," Bad Parameters,DPORT outrange!\n");
		return CMD_WARNING;
	}
	
	ret = fpga_parse_short((char *)argv[5],&PROTOCOL);
	if(0 != ret){
		vty_out(vty,"parse PROTOCOL param error\n");
		return CMD_WARNING;
	}
	PROTOCOL = PROTOCOL << 8;

    ret = fpga_parse_short_H((char *)argv[6],&HASH_CTL);
	if(0 != ret){
		vty_out(vty,"parse HASH_CTL param error\n");
		return CMD_WARNING;
	}
	if(HASH_CTL<0||HASH_CTL>0xffff)
	{
		vty_out(vty," Bad Parameters,PROTOCOL outrange!\n");
		return CMD_WARNING;
	}
	HASH_CTL = HASH_CTL & 0x00ff;
	
	DMAC_H = (str_16_num(argv[7]) & 0xffff00000000) >> 32;
	DMAC_M = (str_16_num(argv[7]) & 0x0000ffff0000) >> 16;
	DMAC_L = (str_16_num(argv[7]) & 0x00000000ffff);
	SMAC_H = (str_16_num(argv[8]) & 0xffff00000000) >> 32;
	SMAC_M = (str_16_num(argv[8]) & 0x0000ffff0000) >> 16;
	SMAC_L = (str_16_num(argv[8]) & 0x00000000ffff);

    ret = fpga_parse_int_H((char *)argv[9],&VLAN_OUT);
	if(0 != ret){
		vty_out(vty,"parse VLAN_OUT param error\n");
		return CMD_WARNING;
	}	
	VLAN_OUT_H=VLAN_OUT >> 16;
	VLAN_OUT_L=VLAN_OUT & 0x0000ffff;
    ret = fpga_parse_int_H((char *)argv[10],&VLAN_IN);
	if(0 != ret){
		vty_out(vty,"parse VLAN_IN param error\n");
		return CMD_WARNING;
	}
	VLAN_IN_H = VLAN_IN >> 16;
	VLAN_IN_L = VLAN_IN & 0x0000ffff;
	
    ret = fpga_parse_short((char *)argv[11],&IPTOS);
	if(0 != ret){
		vty_out(vty,"parse IPTOS param error\n");
		return CMD_WARNING;
	}
	IPTOS_temp = IPTOS;
	IPTOS=(IPTOS & 0x00ff) +(0x45 << 8);
	
    ret = fpga_parse_short_H((char *)argv[12],&IP_TTL_IP_PROTOCOL);
	if(0 != ret){
		vty_out(vty,"parse IPTOS param error\n");
		return CMD_WARNING;
	}
	//IP_TTL_IP_PROTOCOL = IP_TTL_IP_PROTOCOL<<8;
		
	TUNNEL_SIP_H=tunnel_sip_val.s_addr >> 16;
	TUNNEL_SIP_L=tunnel_sip_val.s_addr & 0x0000ffff;
	TUNNEL_DIP_H=tunnel_dip_val.s_addr >> 16;
	TUNNEL_DIP_L=tunnel_dip_val.s_addr & 0x0000ffff;

	IP_CHECKSUM_temp = ((0x45<<8)+IPTOS_temp)+(IP_TTL_IP_PROTOCOL) +(TUNNEL_SIP_L) +(TUNNEL_SIP_H)+(TUNNEL_DIP_L)+(TUNNEL_DIP_H);
	if(IP_CHECKSUM_temp > 0xffff)
	{
        IP_CHECKSUM = (IP_CHECKSUM_temp & 0xffff) + IP_CHECKSUM_temp>>16;
	}
	else
	{
        IP_CHECKSUM = IP_CHECKSUM_temp;
	}
	
    ret = fpga_parse_short((char *)argv[15],&TUNNEL_SRC_PORT);
	if(0 != ret){
		vty_out(vty,"parse TUNNEL_SRC_PORT param error\n");
		return CMD_WARNING;
	}
	if(TUNNEL_SRC_PORT<0||TUNNEL_SRC_PORT>65535)
	{
		vty_out(vty," Bad Parameters,TUNNEL_SRC_PORT outrange!\n");
		return CMD_WARNING;
	}
	
	ret = fpga_parse_short((char *)argv[16],&TUNNEL_DST_PORT);
	if(0 != ret){
		vty_out(vty,"parse TUNNEL_DST_PORT param error\n");
		return CMD_WARNING;
	}
	if(TUNNEL_DST_PORT<0||TUNNEL_DST_PORT>65535)
	{
		vty_out(vty," Bad Parameters,TUNNEL_DST_PORT outrange!\n");
		return CMD_WARNING;
	}
	
	CAPWAP_B6B7=str_16_num(argv[17]) & 0xffff;
	CAPWAP_B4B5=(str_16_num(argv[17]) & 0xffff0000)>>16;
	CAPWAP_B2B3=(str_16_num(argv[17]) & 0xffff00000000) >> 32;
	CAPWAP_B0B1=(str_16_num(argv[17]) >> 48);
	
	CAPWAP_B14B15=str_16_num(argv[18]) & 0xffff;
	CAPWAP_B12B13=(str_16_num(argv[18]) & 0xffff0000)>>16;
	CAPWAP_B10B11=(str_16_num(argv[18]) & 0xffff00000000) >> 32;
	CAPWAP_B8B9=(str_16_num(argv[18]) >> 48);
	
	H_802_11_B6B7=str_16_num(argv[19]) & 0xffff;
	H_802_11_B4B5=(str_16_num(argv[19]) & 0xffff0000)>>16;
	H_802_11_B2B3=(str_16_num(argv[19]) & 0xffff00000000) >> 32;
	H_802_11_B0B1=(str_16_num(argv[19]) >> 48);
	
	H_802_11_B14B15=str_16_num(argv[20]) & 0xffff;
	H_802_11_B12B13=(str_16_num(argv[20]) & 0xffff0000)>>16;
	H_802_11_B10B11=(str_16_num(argv[20]) & 0xffff00000000) >> 32;
	H_802_11_B8B9=(str_16_num(argv[20]) >> 48);
	
	H_802_11_B22B23=str_16_num(argv[21]) & 0xffff;
	H_802_11_B20B21=(str_16_num(argv[21]) & 0xffff0000)>>16;
	H_802_11_B18B19=(str_16_num(argv[21]) & 0xffff00000000) >> 32;
	H_802_11_B16B17=(str_16_num(argv[21]) >> 48);
	
	H_802_11_B26B27=(str_16_num(argv[22]) & 0xffff);
	H_802_11_B24B25=(str_16_num(argv[22]) & 0xffff0000)>>16;
/*	
    //test whether the convert is right	
	vty_out(vty,"******slot_id = %d******\n",slot_id);
	vty_out(vty,"******Indirect_add_reg_H = 0x%x******\n",Indirect_add_reg_H);
	vty_out(vty,"******Indirect_add_reg_L = 0x%x******\n",Indirect_add_reg_L);
	vty_out(vty,"******SIP_H = 0x%x******\n",SIP_H);
	vty_out(vty,"******SIP_L = 0x%x******\n",SIP_L);
	vty_out(vty,"******DIP_H = 0x%x******\n",DIP_H);
	vty_out(vty,"******DIP_L = 0x%x******\n",DIP_L);
	vty_out(vty,"******SPORT = 0x%x******\n",SPORT);
    vty_out(vty,"******DPORT = 0x%x******\n",DPORT);
    vty_out(vty,"******PROTOCOL = 0x%x******\n",PROTOCOL);
    vty_out(vty,"******HASH_CTL = 0x%x******\n",HASH_CTL);
	vty_out(vty,"******DMAC_H = 0x%x******\n",DMAC_H);
	vty_out(vty,"******DMAC_M = 0x%x******\n",DMAC_M);
	vty_out(vty,"******DMAC_L = 0x%x******\n",DMAC_L);
	vty_out(vty,"******SMAC_H = 0x%x******\n",SMAC_H);
	vty_out(vty,"******SMAC_M = 0x%x******\n",SMAC_M);
	vty_out(vty,"******SMAC_L = 0x%x******\n",SMAC_L);
	vty_out(vty,"******VLAN_OUT_H = 0x%x******\n",VLAN_OUT_H);
	vty_out(vty,"******VLAN_OUT_L = 0x%x******\n",VLAN_OUT_L);
	vty_out(vty,"******VLAN_IN_H = 0x%x******\n",VLAN_IN_H);
	vty_out(vty,"******VLAN_IN_L = 0x%x******\n",VLAN_IN_L);
	vty_out(vty,"******IP_TOS = 0x%x******\n",IPTOS);
	vty_out(vty,"******IP_TTL_IP_PROTOCOL = 0x%x******\n",IP_TTL_IP_PROTOCOL);
	vty_out(vty,"******TUNNEL_SIP_H = 0x%x******\n",TUNNEL_SIP_H);
	vty_out(vty,"******TUNNEL_SIP_L = 0x%x******\n",TUNNEL_SIP_L);
	vty_out(vty,"******TUNNEL_DIP_H = 0x%x******\n",TUNNEL_DIP_H);
	vty_out(vty,"******TUNNEL_DIP_L = 0x%x******\n",TUNNEL_DIP_L);
	vty_out(vty,"******TUNNEL_SRC_PORT = 0x%x******\n",TUNNEL_SRC_PORT);
	vty_out(vty,"******TUNNEL_DST_PORT = 0x%x******\n",TUNNEL_DST_PORT);
	vty_out(vty,"******CAPWAP_B0B1 = 0x%x******\n",CAPWAP_B0B1);
	vty_out(vty,"******CAPWAP_B2B3 = 0x%x******\n",CAPWAP_B2B3);
	vty_out(vty,"******CAPWAP_B4B5 = 0x%x******\n",CAPWAP_B4B5);
	vty_out(vty,"******CAPWAP_B6B7 = 0x%x******\n",CAPWAP_B6B7);
	vty_out(vty,"******CAPWAP_B8B9 = 0x%x******\n",CAPWAP_B8B9);
	vty_out(vty,"******CAPWAP_B10B11 = 0x%x******\n",CAPWAP_B10B11);
	vty_out(vty,"******CAPWAP_B12B13 = 0x%x******\n",CAPWAP_B12B13);
	vty_out(vty,"******CAPWAP_B14B15 = 0x%x******\n",CAPWAP_B14B15);
	vty_out(vty,"******H_802_11_B0B1 = 0x%x******\n",H_802_11_B0B1);
	vty_out(vty,"******H_802_11_B2B3 = 0x%x******\n",H_802_11_B2B3);
	vty_out(vty,"******H_802_11_B4B5 = 0x%x******\n",H_802_11_B4B5);
	vty_out(vty,"******H_802_11_B6B7 = 0x%x******\n",H_802_11_B6B7);
	vty_out(vty,"******H_802_11_B8B9 = 0x%x******\n",H_802_11_B8B9);
	vty_out(vty,"******H_802_11_B10B11 = 0x%x******\n",H_802_11_B10B11);
	vty_out(vty,"******H_802_11_B12B13 = 0x%x******\n",H_802_11_B12B13);
	vty_out(vty,"******H_802_11_B14B15 = 0x%x******\n",H_802_11_B14B15);
	vty_out(vty,"******H_802_11_B16B17 = 0x%x******\n",H_802_11_B16B17);
	vty_out(vty,"******H_802_11_B18B19 = 0x%x******\n",H_802_11_B18B19);
	vty_out(vty,"******H_802_11_B20B21 = 0x%x******\n",H_802_11_B20B21);
	vty_out(vty,"******H_802_11_B22B23 = 0x%x******\n",H_802_11_B22B23);
	vty_out(vty,"******H_802_11_B24B25 = 0x%x******\n",H_802_11_B24B25);
	vty_out(vty,"******H_802_11_B26B27 = 0x%x******\n",H_802_11_B26B27);
	vty_out(vty,"******NUM = %d******\n",NUM);
*/
    vty_out(vty,". ");
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CAM_WRITE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT16, &Indirect_add_reg_H,
	                    DBUS_TYPE_UINT16, &Indirect_add_reg_L,
						DBUS_TYPE_UINT16, &SIP_H,
						DBUS_TYPE_UINT16, &SIP_L,
						DBUS_TYPE_UINT16, &DIP_H,
						DBUS_TYPE_UINT16, &DIP_L,
						DBUS_TYPE_UINT16, &SPORT,
						DBUS_TYPE_UINT16, &DPORT,
						DBUS_TYPE_UINT16, &PROTOCOL,
						DBUS_TYPE_UINT16, &HASH_CTL,
						DBUS_TYPE_UINT16, &DMAC_H,
						DBUS_TYPE_UINT16, &DMAC_M,
						DBUS_TYPE_UINT16, &DMAC_L,
						DBUS_TYPE_UINT16, &SMAC_H,
						DBUS_TYPE_UINT16, &SMAC_M,
						DBUS_TYPE_UINT16, &SMAC_L,
						DBUS_TYPE_UINT16, &VLAN_OUT_H,
						DBUS_TYPE_UINT16, &VLAN_OUT_L,
						DBUS_TYPE_UINT16, &VLAN_IN_H,
						DBUS_TYPE_UINT16, &VLAN_IN_L,
						DBUS_TYPE_UINT16, &IPTOS,
						DBUS_TYPE_UINT16, &IP_TTL_IP_PROTOCOL,
						DBUS_TYPE_UINT16, &IP_CHECKSUM,
						DBUS_TYPE_UINT16, &TUNNEL_SIP_H,
						DBUS_TYPE_UINT16, &TUNNEL_SIP_L,
						DBUS_TYPE_UINT16, &TUNNEL_DIP_H,
						DBUS_TYPE_UINT16, &TUNNEL_DIP_L,
						DBUS_TYPE_UINT16, &TUNNEL_SRC_PORT,
						DBUS_TYPE_UINT16, &TUNNEL_DST_PORT,
						DBUS_TYPE_UINT16, &CAPWAP_B0B1,
						DBUS_TYPE_UINT16, &CAPWAP_B2B3,
						DBUS_TYPE_UINT16, &CAPWAP_B4B5,
						DBUS_TYPE_UINT16, &CAPWAP_B6B7,
						DBUS_TYPE_UINT16, &CAPWAP_B8B9,
						DBUS_TYPE_UINT16, &CAPWAP_B10B11,
						DBUS_TYPE_UINT16, &CAPWAP_B12B13,
						DBUS_TYPE_UINT16, &CAPWAP_B14B15,
						DBUS_TYPE_UINT16, &H_802_11_B0B1,
						DBUS_TYPE_UINT16, &H_802_11_B2B3,
						DBUS_TYPE_UINT16, &H_802_11_B4B5,
						DBUS_TYPE_UINT16, &H_802_11_B6B7,
						DBUS_TYPE_UINT16, &H_802_11_B8B9,
						DBUS_TYPE_UINT16, &H_802_11_B10B11,
						DBUS_TYPE_UINT16, &H_802_11_B12B13,
						DBUS_TYPE_UINT16, &H_802_11_B14B15,
						DBUS_TYPE_UINT16, &H_802_11_B16B17,
						DBUS_TYPE_UINT16, &H_802_11_B18B19,
						DBUS_TYPE_UINT16, &H_802_11_B20B21,
						DBUS_TYPE_UINT16, &H_802_11_B22B23,
						DBUS_TYPE_UINT16, &H_802_11_B24B25,
						DBUS_TYPE_UINT16, &H_802_11_B26B27,
						DBUS_TYPE_UINT16, &NUM,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty,". ");
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"write cam table dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		vty_out(vty, "  OK!\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;

		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}    
}

DEFUN(hash_table_read,
	hash_table_read_cmd,
	"show fpga hash-table slot SLOT_ID sip USER_SIP dip USER_DIP sport USER_SPORT dport USER_DPORT protocol USER_PROTOCOL",
	"Show system information\n"
	"Show fpga information\n"
	"read hash table \n"
	"the slot id \n"
	"the slot id num \n"
	"the packet flow source ip\n"
	"user source ip"
	"the packet flow destinations ip\n"
	"user destinations ip"
	"the packet flow source port\n"
	"user source port"
	"the packet flow destinations port\n"
	"user destinations port"
	"the packet flow protocol\n"
	"user protocol")
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	int k;
	int start = 0;	
	int slot_id;
	unsigned long ready = 0;
	unsigned int hash_id = 0;
	
	struct in_addr usersip_val;
	struct in_addr userdip_val;
	
	char sip_str[16];
	char dip_str[16];
	char tunnel_sip_str[16];
	char tunnel_dip_str[16];
	unsigned int usr_sip;
	unsigned int usr_dip;
	unsigned int usr_tunnel_sip;
	unsigned int usr_tunnel_dip;

	
	unsigned long long data;
	unsigned int crc;
	unsigned add_data;
	unsigned short sip_H = 0;
	unsigned short sip_L = 0;
	unsigned short sport = 0;
	unsigned short smac_H = 0;
    unsigned short smac_M = 0;
	unsigned short smac_L = 0;
	unsigned short tunnel_sip_H = 0;
	unsigned short tunnel_sip_L = 0;
	unsigned short tunnel_sport = 0;
	unsigned short dip_H = 0;
	unsigned short dip_L = 0;
	unsigned short dport = 0;
	unsigned short dmac_H = 0;
    unsigned short dmac_M = 0;
	unsigned short dmac_L = 0;
	unsigned short tunnel_dip_H = 0;
	unsigned short tunnel_dip_L = 0;
	unsigned short tunnel_dport = 0;
    unsigned short protocol = 0;
	unsigned short setup_time = 0;
	unsigned short hash_ctl = 0;
	unsigned short vlan_out_H = 0;
	unsigned short vlan_out_L = 0;
	unsigned short vlan_in_H = 0;
	unsigned short vlan_in_L = 0;
	unsigned short ethernet_protocol = 0;
	unsigned short ip_tos = 0;
	unsigned short ip_len = 0;
	unsigned short ip_offset = 0;
	unsigned short ip_ttl = 0;
	unsigned short ip_protocol = 0;
    unsigned short ip_checksum = 0;
	unsigned short udp_len = 0;
    unsigned short udp_checksum = 0;	
    unsigned short capwap_header_b0 = 0;
	unsigned short capwap_header_b2 = 0;
	unsigned short capwap_header_b4 = 0;
	unsigned short capwap_header_b6 = 0;
	unsigned short capwap_header_b8 = 0;
	unsigned short capwap_header_b10 = 0;
	unsigned short capwap_header_b12 = 0;
	unsigned short capwap_header_b14 = 0;
    unsigned short header_802_11_b0 = 0;
	unsigned short header_802_11_b2 = 0;
	unsigned short header_802_11_b4 = 0;
	unsigned short header_802_11_b6 = 0;
	unsigned short header_802_11_b8 = 0;
	unsigned short header_802_11_b10 = 0;
	unsigned short header_802_11_b12 = 0;
	unsigned short header_802_11_b14 = 0;
	unsigned short header_802_11_b16 = 0;
	unsigned short header_802_11_b18 = 0;
	unsigned short header_802_11_b20 = 0;
	unsigned short header_802_11_b22 = 0;
	unsigned short header_802_11_b24 = 0;
	unsigned short header_802_11_b26 = 0;
	unsigned short capwap_protocol = 0;	
	unsigned short flag = 0;
    unsigned short temp_short = 0;
	unsigned int temp_int = 0;
	
    struct hash_cam_table_s stat;
	unsigned int temp_value1 = 0;
	unsigned int temp_value2 = 0;
	unsigned int value[sizeof(DataTempStorageArea)/sizeof(long)]; /* register value */
	char *endptr = NULL;
	hash_param_t *tuple = NULL;
	
	tuple = (hash_param_t *)malloc(sizeof(hash_param_t));
	if(!tuple)
	{
		vty_out(vty,"malloc struct hash_param_s failed!\n");
        return CMD_WARNING;
	}
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}

    ret = inet_aton(argv[1], &usersip_val);
    if(ret)
    {   
		vty_out(vty,". ");
    }else
    {   
        vty_out(vty,"**ip conversion error,invalid parameter!\n");
		return CMD_WARNING;
    }
	usersip_val.s_addr = (usersip_val.s_addr << 24)+((usersip_val.s_addr << 8) & 0x00ff0000)+((usersip_val.s_addr >> 8) & 0x0000ff00)+(usersip_val.s_addr >> 24);
	//vty_out(vty,"**after the sip = %x\n",usersip_val.s_addr);
	
    ret = inet_aton(argv[2], &userdip_val);
    if(ret)
    {   
		vty_out(vty,". ");
    }else
    {   
        vty_out(vty,"**ip conversion error,invalid parameter!\n");
		return CMD_WARNING;
    }
	userdip_val.s_addr = (userdip_val.s_addr << 24)+((userdip_val.s_addr << 8) & 0x00ff0000)+((userdip_val.s_addr >> 8) & 0x0000ff00)+(userdip_val.s_addr >> 24);
	//vty_out(vty,"**after the dip = %x\n",userdip_val.s_addr);
	
	dport = str_10_short_num((char *)argv[4]);
	if(dport == 0)
	{
		vty_out(vty,"Bad param DPORT.range 1~65535!\n");
	    return CMD_WARNING;
	}
	temp_short = dport >> 8;
	dport = temp_short + (dport << 8);
	//vty_out(vty,"** after the dport = %x\n",dport);
	
	sport = str_10_short_num((char *)argv[3]);
	if(sport == 0)
	{
		vty_out(vty,"Bad param sport.range 1~65535!\n");
	    return CMD_WARNING;
	}
	temp_short = sport >> 8;
	sport = temp_short + (sport << 8);
	//vty_out(vty,"**after the sport = %x\n",sport);
	
	protocol = str_10_short_num((char *)argv[5]);
	if(protocol == 0)
	{
		vty_out(vty,"Bad param protocol.range 1~256!\n");
	    return CMD_WARNING;
	}
	//vty_out(vty,"**the protocol = %d\n",protocol);
	
	data = userdip_val.s_addr ;
	data = data << 32;
    data = data + usersip_val.s_addr;

	crc = dport;
	crc = crc << 16;
	crc = crc + sport;
	add_data = protocol;
	/*for the algorithm of crc32*/
	//vty_out(vty,"data:                %llx\n",data);
	//vty_out(vty,"crc:                %u\n",crc);
	//vty_out(vty,"add_data:                %u\n",add_data);
    //hash_id = crc_get_hash_id(data,crc,add_data);
    //vty_out(vty,"hash_id:                %x\n",hash_id);
    /*end*/

	/*for the algorithm of crc32c modify*/
	tuple->dip = userdip_val.s_addr;
	tuple->sip = usersip_val.s_addr;
	tuple->dport = dport;
	tuple->sport = sport;
	tuple->protocol = protocol;
	/*end*/
	
    hash_id = crc_get_hash_id_crc32c_modify(tuple,0);

    vty_out(vty,"begin to read hash table,wait...\n");
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_HASH_TABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT32, &hash_id,
						DBUS_TYPE_INVALID);

	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		   
		if (dbus_message_get_args (reply, &err,
    					 DBUS_TYPE_UINT16,&flag,
                         DBUS_TYPE_UINT16,&sip_H,
                         DBUS_TYPE_UINT16,&sip_L,
    					 DBUS_TYPE_UINT16,&sport,
    					 DBUS_TYPE_UINT16,&smac_H,
    					 DBUS_TYPE_UINT16,&smac_M,
    					 DBUS_TYPE_UINT16,&smac_L,
    					 DBUS_TYPE_UINT16,&tunnel_sip_H,
    					 DBUS_TYPE_UINT16,&tunnel_sip_L,
    					 DBUS_TYPE_UINT16,&tunnel_sport,
    					 DBUS_TYPE_UINT16,&dip_H,
                         DBUS_TYPE_UINT16,&dip_L,
                         DBUS_TYPE_UINT16,&dport,
    					 DBUS_TYPE_UINT16,&dmac_H,
    					 DBUS_TYPE_UINT16,&dmac_M,
    					 DBUS_TYPE_UINT16,&dmac_L,
    					 DBUS_TYPE_UINT16,&tunnel_dip_H,
    					 DBUS_TYPE_UINT16,&tunnel_dip_L,
    					 DBUS_TYPE_UINT16,&tunnel_dport,
    					 DBUS_TYPE_UINT16,&protocol,
    					 DBUS_TYPE_UINT16,&setup_time,
                         DBUS_TYPE_UINT16,&hash_ctl,
                         DBUS_TYPE_UINT16,&vlan_out_H,
    					 DBUS_TYPE_UINT16,&vlan_out_L,
    					 DBUS_TYPE_UINT16,&vlan_in_H,
    					 DBUS_TYPE_UINT16,&vlan_in_L,
    					 DBUS_TYPE_UINT16,&ethernet_protocol,
    					 DBUS_TYPE_UINT16,&ip_tos,
    					 DBUS_TYPE_UINT16,&ip_len,
    					 DBUS_TYPE_UINT16,&ip_offset,
    					 DBUS_TYPE_UINT16,&ip_ttl,
                         DBUS_TYPE_UINT16,&ip_protocol,
                         DBUS_TYPE_UINT16,&ip_checksum,
    					 DBUS_TYPE_UINT16,&udp_len,
    					 DBUS_TYPE_UINT16,&udp_checksum,
    					 DBUS_TYPE_UINT16,&capwap_header_b0,
    					 DBUS_TYPE_UINT16,&capwap_header_b2,
    					 DBUS_TYPE_UINT16,&capwap_header_b4,
    					 DBUS_TYPE_UINT16,&capwap_header_b6,
    					 DBUS_TYPE_UINT16,&capwap_header_b8,
    					 DBUS_TYPE_UINT16,&capwap_header_b10,
                         DBUS_TYPE_UINT16,&capwap_header_b12,
                         DBUS_TYPE_UINT16,&capwap_header_b14,
    					 DBUS_TYPE_UINT16,&header_802_11_b0,
    					 DBUS_TYPE_UINT16,&header_802_11_b2,
    					 DBUS_TYPE_UINT16,&header_802_11_b4,
    					 DBUS_TYPE_UINT16,&header_802_11_b6,
    					 DBUS_TYPE_UINT16,&header_802_11_b8,
    					 DBUS_TYPE_UINT16,&header_802_11_b10,
    					 DBUS_TYPE_UINT16,&header_802_11_b12,
    					 DBUS_TYPE_UINT16,&header_802_11_b14,
                         DBUS_TYPE_UINT16,&header_802_11_b16,
                         DBUS_TYPE_UINT16,&header_802_11_b18,
    					 DBUS_TYPE_UINT16,&header_802_11_b20,
    					 DBUS_TYPE_UINT16,&header_802_11_b22,
    					 DBUS_TYPE_UINT16,&header_802_11_b24,
    					 DBUS_TYPE_UINT16,&header_802_11_b26,
    					 DBUS_TYPE_UINT16,&capwap_protocol,
    					 DBUS_TYPE_INVALID))
    	{
                usr_sip = sip_H ;
            	usr_sip = (usr_sip <<16)+sip_L;
            	usr_sip = htonl(usr_sip);
             	if(inet_ntop(AF_INET,&usr_sip,sip_str,16) == NULL)
             	{
                   vty_out(vty,"sip conversion error \n");
            	}
                usr_dip = dip_H ;
            	usr_dip = (usr_dip <<16)+dip_L;
            	usr_dip = htonl(usr_dip);
             	if(inet_ntop(AF_INET,&usr_dip,dip_str,16) == NULL)
             	{
                   vty_out(vty,"dip conversion error \n");
            	}
                usr_tunnel_sip = tunnel_sip_H ;
            	usr_tunnel_sip = (usr_tunnel_sip <<16)+tunnel_sip_L;
            	usr_tunnel_sip = htonl(usr_tunnel_sip);
             	if(inet_ntop(AF_INET,&usr_tunnel_sip,tunnel_sip_str,16) == NULL)
             	{
                   vty_out(vty,"tunnel_sip conversion error \n");
            	}
                usr_tunnel_dip = tunnel_dip_H ;
            	usr_tunnel_dip = (usr_tunnel_dip <<16)+tunnel_dip_L;
            	usr_tunnel_dip = htonl(usr_tunnel_dip);
             	if(inet_ntop(AF_INET,&usr_tunnel_dip,tunnel_dip_str,16) == NULL)
             	{
                   vty_out(vty,"tunnel_dip conversion error \n");
            	}	
				
			    if(flag == 0)
				{
                	vty_out(vty,"\nDetail Information of HASH Table %x\n",hash_id);
                 	vty_out(vty,"=====================================================================\n");
					
                    vty_out(vty,"\nHash Label\n");
                	vty_out(vty,"\thash table creation time:            0x%x\n",setup_time);
                	vty_out(vty,"\thash ctl:                            0x%x\n",hash_ctl);

                    vty_out(vty,"\nFive Tuple\n");

                	vty_out(vty,"\tsource ip address:                 %s\n",sip_str);
                    vty_out(vty,"\tdestination ip address:            %s\n",dip_str);
                	vty_out(vty,"\tsource port:                       %d\n",sport);
                	vty_out(vty,"\tdestination port:                  %d\n",dport);
                	vty_out(vty,"\tprotocol:                          %d\n",protocol);

                    vty_out(vty,"\n MAC header -> IP header -> UDP header\n");
                    vty_out(vty,"\tdestination mac H:                 0x%x\n",dmac_H);
                	vty_out(vty,"\tdestination mac M:                 0x%x\n",dmac_M);
                	vty_out(vty,"\tdestination mac L:                 0x%x\n",dmac_L);
                    vty_out(vty,"\tsource mac H:                      0x%x\n",smac_H);
                	vty_out(vty,"\tsource mac M:                      0x%x\n",smac_M);
                	vty_out(vty,"\tsource mac L:                      0x%x\n",smac_L);
                	vty_out(vty,"\tethernet protocol:                 0x%x\n",ethernet_protocol);
                	vty_out(vty,"\tvlan out H:                        0x%x\n",vlan_out_H);
                	vty_out(vty,"\tvlan out L:                        0x%x\n",vlan_out_L);
                    vty_out(vty,"\tvlan in H:                         0x%x\n",vlan_in_H);
                	vty_out(vty,"\tvlan in L:                         0x%x\n",vlan_in_L);
                    vty_out(vty,"\tip tos:                            0x%x\n",ip_tos);
                	vty_out(vty,"\tip length:                         0x%x\n",ip_len);
                	vty_out(vty,"\tip offset:                         0x%x\n",ip_offset);
                    vty_out(vty,"\tip ttl:                            %d\n",ip_ttl);
                	vty_out(vty,"\tip protocol:                       0x%x\n",ip_protocol);
					vty_out(vty,"\tip checksum:                       0x%x\n",ip_checksum);
                    vty_out(vty,"\ttunnel source ip:                  %s\n",tunnel_sip_str);
                    vty_out(vty,"\ttunnel destination ip:             %s\n",tunnel_dip_str);
                    vty_out(vty,"\ttunnel source port:                %d\n",tunnel_sport);
                    vty_out(vty,"\ttunnel destination port:           %d\n",tunnel_dport);
                	vty_out(vty,"\tudp length:                        0x%x\n",udp_len);
                	vty_out(vty,"\tudp checksum:                      0x%x\n",udp_checksum);

                    vty_out(vty,"\n capwap header -> 802.11 LLC header\n");
                	
                	vty_out(vty,"\tcapwap header:                     0x%x %x %x %x %x %x %x %x\n",capwap_header_b0,capwap_header_b2,capwap_header_b4,capwap_header_b6,capwap_header_b8,capwap_header_b10,capwap_header_b12,capwap_header_b14);
                	vty_out(vty,"\t802.11 header:                     0x%x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",header_802_11_b0,header_802_11_b2,header_802_11_b4,header_802_11_b6,header_802_11_b8,header_802_11_b10,header_802_11_b12,header_802_11_b14,header_802_11_b16,header_802_11_b18,header_802_11_b20,header_802_11_b22,header_802_11_b24,header_802_11_b26);

                    vty_out(vty,"\n");
                    vty_out(vty,"=====================================================================\n");
			}
			else
			{
				vty_out(vty,"read hash table dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		dbus_message_unref(reply);
		//vty_out(vty, "read hash table command success\n");
		return CMD_SUCCESS;

	} 
	else
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

#if 0
DEFUN(hash_table_write,
	hash_table_write_cmd,
	"set fpga hash-table SLOTID SIP DIP SPORT DPORT PROTOCOL HASH_CTL DMAC SMAC VLAN_OUT VLAN_IN \
	IP_TOS IP_TTL TUNNEL_SIP TUNNEL_DIP TUNNEL_SPORT TUNNEL_DPORT CAPWAP_HEADER_H8 CAPWAP_HEADER_L8 \
	HEADER_1 HEADER_2 HEADER_3 HEADER_4",
	"set system parameters\n"
	"set fpga parameters\n"
	"set fpga hash table \n"
	"the slot id.eg:1\n"
	"source ip address.eg:192.168.1.1 \n"
	"destination ip address.eg:192.168.1.1 \n"
	"source port(D).eg:2000 \n"
	"destination port(D);3000 \n"
	"protocol(H) \n"
	"hash ctl(H) \n"
	"destination mac.eg:001f64862010 \n"
	"source mac.eg:001f64861030 \n"
	"vlan out(H) \n"
	"vlan in(H) \n"
	"ip tos(D) \n"
	"ip ttl(D) \n"
	"tunnel source ip.eg:192.168.1.1 \n"
	"tunnel destination ip.eg:192.168.1.1 \n"
	"tunnel source port(D).eg:2000 \n"
	"tunnel destination port(D).eg:3000 \n"
	"capwap header B0~B7(H)(0x0~0xffff ffff ffff ffff) \n"
	"capwap header B8~B15(H)(0x0~0xffff ffff ffff ffff) \n"
	"802.11 header B0~B7 byte(H)(0x0~0xffff ffff ffff ffff)\n"
	"802.11 header B8~B15 byte(H)(0x0~0xffff ffff ffff ffff)\n"
	"802.11 header B16~B23 byte(H)(0x0~0xffff ffff ffff ffff)\n"
	"802.11 header B24~B27 byte(H)(0x0~0xffff ffff)\n"
	)
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr sip_val;
	struct in_addr dip_val;
	struct in_addr tunnel_sip_val;
	struct in_addr tunnel_dip_val;
	int ret;
	int slot_id;
	unsigned short Indirect_add_reg_H = 0;
	unsigned short Indirect_add_reg_L = 0;
	unsigned short SIP_H=0;
	unsigned short SIP_L=0;
    unsigned short DIP_H=0;
	unsigned short DIP_L=0;
	unsigned short SPORT=0;
	unsigned short DPORT=0;
	unsigned short PROTOCOL=0;
	unsigned short HASH_CTL=0;
	unsigned short DMAC_H=0;
	unsigned short DMAC_M=0;
	unsigned short DMAC_L=0;
	unsigned short SMAC_H=0;
	unsigned short SMAC_M=0;
	unsigned short SMAC_L=0;
	unsigned int VLAN_OUT=0;
	unsigned short VLAN_OUT_H=0;
	unsigned short VLAN_OUT_L=0;
	unsigned int VLAN_IN=0;
	unsigned short VLAN_IN_H=0;
	unsigned short VLAN_IN_L=0;
	unsigned short IPTOS=0;
	unsigned short IP_TTL_IP_PROTOCOL=0;
	unsigned short IP_CHECKSUM=0;
	unsigned short TUNNEL_SIP_H=0;
	unsigned short TUNNEL_SIP_L=0;
	unsigned short TUNNEL_DIP_H=0;
	unsigned short TUNNEL_DIP_L=0;
	unsigned short TUNNEL_SRC_PORT=0;
	unsigned short TUNNEL_DST_PORT=0;
	unsigned short CAPWAP_B0B1=0;
	unsigned short CAPWAP_B2B3=0;
	unsigned short CAPWAP_B4B5=0;
	unsigned short CAPWAP_B6B7=0;
	unsigned short CAPWAP_B8B9=0;
	unsigned short CAPWAP_B10B11=0;
	unsigned short CAPWAP_B12B13=0;
	unsigned short CAPWAP_B14B15=0;
	unsigned short H_802_11_B0B1=0;
	unsigned short H_802_11_B2B3=0;
	unsigned short H_802_11_B4B5=0;
	unsigned short H_802_11_B6B7=0;
	unsigned short H_802_11_B8B9=0;
	unsigned short H_802_11_B10B11=0;
	unsigned short H_802_11_B12B13=0;
	unsigned short H_802_11_B14B15=0;
	unsigned short H_802_11_B16B17=0;
	unsigned short H_802_11_B18B19=0;
	unsigned short H_802_11_B20B21=0;
	unsigned short H_802_11_B22B23=0;
	unsigned short H_802_11_B24B25=0;
	unsigned short H_802_11_B26B27=0;
	unsigned short NUM=0;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int i = 0;
	int val[3];

	vty_out(vty,"begin to write cam table,wait .");
	memset(val,0,sizeof(val));
    sscanf(argv[0], "%d", &val[0]);
    slot_id = val[0];
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	Indirect_add_reg_L = val[1];
    NUM = val[2];
	vty_out(vty,". ");

	ret = inet_aton(argv[1], &sip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //vty_out("**the sip = %x\n",sip_val.s_addr);
    }
	else
    {   
        printf("**sip conversion error\n");
    }
    ret = inet_aton(argv[2], &dip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //vty_out("**the dip = %x\n",dip_val.s_addr);
    }
	else
    {   
        vty_out(vty,"**dip conversion error\n");
    }  

    ret = inet_aton(argv[13], &tunnel_sip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //vty_out("**the tunnel_sip = %x\n",tunnel_sip_val.s_addr);
    }
	else
    {   
		vty_out(vty,". ");
        printf("**tunnel_sip conversion error\n");
    }
    ret = inet_aton(argv[14], &tunnel_dip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //printf("**the tunnel_dip = %x\n",tunnel_dip_val.s_addr);
    }
	else
    {   
        printf("**tunnel_dip conversion error\n");
    }

    SIP_H = sip_val.s_addr >> 16;
	SIP_L = sip_val.s_addr & 0x0000ffff;
    DIP_H = dip_val.s_addr >> 16;
    DIP_L = dip_val.s_addr & 0x0000ffff;

	ret = fpga_parse_short((char *)argv[3],&SPORT);
	if(0 != ret){
		vty_out(vty,"parse SPORT param error\n");
		return CMD_WARNING;
	}
	
	ret = fpga_parse_short((char *)argv[4],&DPORT);
	if(0 != ret){
		vty_out(vty,"parse DPORT param error\n");
		return CMD_WARNING;
	}
	
    ret = fpga_parse_short_H((char *)argv[5],&PROTOCOL);
	if(0 != ret){
		vty_out(vty,"parse PROTOCOL param error\n");
		return CMD_WARNING;
	}

    ret = fpga_parse_short_H((char *)argv[6],&HASH_CTL);
	if(0 != ret){
		vty_out(vty,"parse HASH_CTL param error\n");
		return CMD_WARNING;
	}
	HASH_CTL = HASH_CTL & 0x00ff;
	
	DMAC_H = (str_16_num(argv[7]) & 0xffff00000000) >> 32;
	DMAC_M = (str_16_num(argv[7]) & 0x0000ffff0000) >> 16;
	DMAC_L = (str_16_num(argv[7]) & 0x00000000ffff);
	SMAC_H = (str_16_num(argv[8]) & 0xffff00000000) >> 32;
	SMAC_M = (str_16_num(argv[8]) & 0x0000ffff0000) >> 16;
	SMAC_L = (str_16_num(argv[8]) & 0x00000000ffff);

    ret = fpga_parse_int_H((char *)argv[9],&VLAN_OUT);
	if(0 != ret){
		vty_out(vty,"parse VLAN_OUT param error\n");
		return CMD_WARNING;
	}	
	VLAN_OUT_H=VLAN_OUT >> 16;
	VLAN_OUT_L=VLAN_OUT & 0x0000ffff;
    ret = fpga_parse_int_H((char *)argv[10],&VLAN_IN);
	if(0 != ret){
		vty_out(vty,"parse VLAN_IN param error\n");
		return CMD_WARNING;
	}
	VLAN_IN_H = VLAN_IN >> 16;
	VLAN_IN_L = VLAN_IN & 0x0000ffff;
	
    ret = fpga_parse_short((char *)argv[11],&IPTOS);
	if(0 != ret){
		vty_out(vty,"parse IPTOS param error\n");
		return CMD_WARNING;
	}
	IPTOS=(IPTOS & 0x00ff) +(0x45 << 8);
	
    ret = fpga_parse_short((char *)argv[12],&IPTOS);
	if(0 != ret){
		vty_out(vty,"parse IPTOS param error\n");
		return CMD_WARNING;
	}
	IP_TTL_IP_PROTOCOL = IPTOS<<8;
	
	IP_CHECKSUM=0;
	TUNNEL_SIP_H=tunnel_sip_val.s_addr >> 16;
	TUNNEL_SIP_L=tunnel_sip_val.s_addr & 0x0000ffff;
	TUNNEL_DIP_H=tunnel_sip_val.s_addr >> 16;
	TUNNEL_DIP_L=tunnel_sip_val.s_addr & 0x0000ffff;
	
    ret = fpga_parse_short((char *)argv[15],&TUNNEL_SRC_PORT);
	if(0 != ret){
		vty_out(vty,"parse TUNNEL_SRC_PORT param error\n");
		return CMD_WARNING;
	}
	if(TUNNEL_SRC_PORT<0||TUNNEL_SRC_PORT>65535)
	{
		vty_out(vty," Bad Parameters,TUNNEL_SRC_PORT outrange!\n");
		return CMD_WARNING;
	}
	
	ret = fpga_parse_short((char *)argv[16],&TUNNEL_DST_PORT);
	if(0 != ret){
		vty_out(vty,"parse TUNNEL_DST_PORT param error\n");
		return CMD_WARNING;
	}
	if(TUNNEL_DST_PORT<0||TUNNEL_DST_PORT>65535)
	{
		vty_out(vty," Bad Parameters,TUNNEL_DST_PORT outrange!\n");
		return CMD_WARNING;
	}
	
	CAPWAP_B0B1=str_16_num(argv[17]) & 0xffff;
	CAPWAP_B2B3=(str_16_num(argv[17]) & 0xffff0000)>>16;
	CAPWAP_B4B5=(str_16_num(argv[17]) & 0xffff00000000) >> 32;
	CAPWAP_B6B7=(str_16_num(argv[17]) & 0xffff000000000000 >> 48);
	CAPWAP_B8B9=str_16_num(argv[18]) & 0xffff;
	CAPWAP_B10B11=(str_16_num(argv[18]) & 0xffff0000)>>16;
	CAPWAP_B12B13=(str_16_num(argv[18]) & 0xffff00000000) >> 32;
	CAPWAP_B14B15=(str_16_num(argv[18]) & 0xffff000000000000 >> 48);
	H_802_11_B0B1=str_16_num(argv[19]) & 0xffff;
	H_802_11_B2B3=(str_16_num(argv[19]) & 0xffff0000)>>16;
	H_802_11_B4B5=(str_16_num(argv[19]) & 0xffff00000000) >> 32;
	H_802_11_B6B7=(str_16_num(argv[19]) & 0xffff000000000000 >> 48);
	H_802_11_B8B9=str_16_num(argv[20]) & 0xffff;
	H_802_11_B10B11=(str_16_num(argv[20]) & 0xffff0000)>>16;
	H_802_11_B12B13=(str_16_num(argv[20]) & 0xffff00000000) >> 32;
	H_802_11_B14B15=(str_16_num(argv[20]) & 0xffff000000000000 >> 48);
	H_802_11_B16B17=str_16_num(argv[21]) & 0xffff;
	H_802_11_B18B19=(str_16_num(argv[21]) & 0xffff0000)>>16;
	H_802_11_B20B21=(str_16_num(argv[21]) & 0xffff00000000) >> 32;
	H_802_11_B22B23=(str_16_num(argv[21]) & 0xffff000000000000 >> 48);
	H_802_11_B24B25=(str_16_num(argv[22]) & 0xffff);
	H_802_11_B26B27=(str_16_num(argv[22]) & 0xffff0000)>>16;
	
    //test whether the convert is right	
	vty_out(vty,"******slot_id = %d******\n",slot_id);
	vty_out(vty,"******Indirect_add_reg_H = 0x%x******\n",Indirect_add_reg_H);
	vty_out(vty,"******Indirect_add_reg_L = 0x%x******\n",Indirect_add_reg_L);
	vty_out(vty,"******SIP_H = 0x%x******\n",SIP_H);
	vty_out(vty,"******SIP_L = 0x%x******\n",SIP_L);
	vty_out(vty,"******DIP_H = 0x%x******\n",DIP_H);
	vty_out(vty,"******DIP_L = 0x%x******\n",DIP_L);
	vty_out(vty,"******SPORT = 0x%x******\n",SPORT);
    vty_out(vty,"******DPORT = 0x%x******\n",DPORT);
    vty_out(vty,"******PROTOCOL = 0x%x******\n",PROTOCOL);
    vty_out(vty,"******HASH_CTL = 0x%x******\n",HASH_CTL);
	vty_out(vty,"******DMAC_H = 0x%x******\n",DMAC_H);
	vty_out(vty,"******DMAC_M = 0x%x******\n",DMAC_M);
	vty_out(vty,"******DMAC_L = 0x%x******\n",DMAC_L);
	vty_out(vty,"******SMAC_H = 0x%x******\n",SMAC_H);
	vty_out(vty,"******SMAC_M = 0x%x******\n",SMAC_M);
	vty_out(vty,"******SMAC_L = 0x%x******\n",SMAC_L);
	vty_out(vty,"******VLAN_OUT_H = 0x%x******\n",VLAN_OUT_H);
	vty_out(vty,"******VLAN_OUT_L = 0x%x******\n",VLAN_OUT_L);
	vty_out(vty,"******VLAN_IN_H = 0x%x******\n",VLAN_IN_H);
	vty_out(vty,"******VLAN_IN_L = 0x%x******\n",VLAN_IN_L);
	vty_out(vty,"******IP_TOS = 0x%x******\n",IPTOS);
	vty_out(vty,"******IP_TTL_IP_PROTOCOL = 0x%x******\n",IP_TTL_IP_PROTOCOL);
	vty_out(vty,"******TUNNEL_SIP_H = 0x%x******\n",TUNNEL_SIP_H);
	vty_out(vty,"******TUNNEL_SIP_L = 0x%x******\n",TUNNEL_SIP_L);
	vty_out(vty,"******TUNNEL_DIP_H = 0x%x******\n",TUNNEL_DIP_H);
	vty_out(vty,"******TUNNEL_DIP_L = 0x%x******\n",TUNNEL_DIP_L);
	vty_out(vty,"******TUNNEL_SRC_PORT = 0x%x******\n",TUNNEL_SRC_PORT);
	vty_out(vty,"******TUNNEL_DST_PORT = 0x%x******\n",TUNNEL_DST_PORT);
	vty_out(vty,"******CAPWAP_B0B1 = 0x%x******\n",CAPWAP_B0B1);
	vty_out(vty,"******CAPWAP_B2B3 = 0x%x******\n",CAPWAP_B2B3);
	vty_out(vty,"******CAPWAP_B4B5 = 0x%x******\n",CAPWAP_B4B5);
	vty_out(vty,"******CAPWAP_B6B7 = 0x%x******\n",CAPWAP_B6B7);
	vty_out(vty,"******CAPWAP_B8B9 = 0x%x******\n",CAPWAP_B8B9);
	vty_out(vty,"******CAPWAP_B10B11 = 0x%x******\n",CAPWAP_B10B11);
	vty_out(vty,"******CAPWAP_B12B13 = 0x%x******\n",CAPWAP_B12B13);
	vty_out(vty,"******CAPWAP_B14B15 = 0x%x******\n",CAPWAP_B14B15);
	vty_out(vty,"******H_802_11_B0B1 = 0x%x******\n",H_802_11_B0B1);
	vty_out(vty,"******H_802_11_B2B3 = 0x%x******\n",H_802_11_B2B3);
	vty_out(vty,"******H_802_11_B4B5 = 0x%x******\n",H_802_11_B4B5);
	vty_out(vty,"******H_802_11_B6B7 = 0x%x******\n",H_802_11_B6B7);
	vty_out(vty,"******H_802_11_B8B9 = 0x%x******\n",H_802_11_B8B9);
	vty_out(vty,"******H_802_11_B10B11 = 0x%x******\n",H_802_11_B10B11);
	vty_out(vty,"******H_802_11_B12B13 = 0x%x******\n",H_802_11_B12B13);
	vty_out(vty,"******H_802_11_B14B15 = 0x%x******\n",H_802_11_B14B15);
	vty_out(vty,"******H_802_11_B16B17 = 0x%x******\n",H_802_11_B16B17);
	vty_out(vty,"******H_802_11_B18B19 = 0x%x******\n",H_802_11_B18B19);
	vty_out(vty,"******H_802_11_B20B21 = 0x%x******\n",H_802_11_B20B21);
	vty_out(vty,"******H_802_11_B22B23 = 0x%x******\n",H_802_11_B22B23);
	vty_out(vty,"******H_802_11_B24B25 = 0x%x******\n",H_802_11_B24B25);
	vty_out(vty,"******H_802_11_B26B27 = 0x%x******\n",H_802_11_B26B27);
	vty_out(vty,"******NUM = %d******\n",NUM);

    vty_out(vty,". ");
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CAM_WRITE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT16, &Indirect_add_reg_H,
	                    DBUS_TYPE_UINT16, &Indirect_add_reg_L,
						DBUS_TYPE_UINT16, &SIP_H,
						DBUS_TYPE_UINT16, &SIP_L,
						DBUS_TYPE_UINT16, &DIP_H,
						DBUS_TYPE_UINT16, &DIP_L,
						DBUS_TYPE_UINT16, &SPORT,
						DBUS_TYPE_UINT16, &DPORT,
						DBUS_TYPE_UINT16, &PROTOCOL,
						DBUS_TYPE_UINT16, &HASH_CTL,
						DBUS_TYPE_UINT16, &DMAC_H,
						DBUS_TYPE_UINT16, &DMAC_M,
						DBUS_TYPE_UINT16, &DMAC_L,
						DBUS_TYPE_UINT16, &SMAC_H,
						DBUS_TYPE_UINT16, &SMAC_M,
						DBUS_TYPE_UINT16, &SMAC_L,
						DBUS_TYPE_UINT16, &VLAN_OUT_H,
						DBUS_TYPE_UINT16, &VLAN_OUT_L,
						DBUS_TYPE_UINT16, &VLAN_IN_H,
						DBUS_TYPE_UINT16, &VLAN_IN_L,
						DBUS_TYPE_UINT16, &IPTOS,
						DBUS_TYPE_UINT16, &IP_TTL_IP_PROTOCOL,
						DBUS_TYPE_UINT16, &TUNNEL_SIP_H,
						DBUS_TYPE_UINT16, &TUNNEL_SIP_L,
						DBUS_TYPE_UINT16, &TUNNEL_DIP_H,
						DBUS_TYPE_UINT16, &TUNNEL_DIP_L,
						DBUS_TYPE_UINT16, &TUNNEL_SRC_PORT,
						DBUS_TYPE_UINT16, &TUNNEL_DST_PORT,
						DBUS_TYPE_UINT16, &CAPWAP_B0B1,
						DBUS_TYPE_UINT16, &CAPWAP_B2B3,
						DBUS_TYPE_UINT16, &CAPWAP_B4B5,
						DBUS_TYPE_UINT16, &CAPWAP_B6B7,
						DBUS_TYPE_UINT16, &CAPWAP_B8B9,
						DBUS_TYPE_UINT16, &CAPWAP_B10B11,
						DBUS_TYPE_UINT16, &CAPWAP_B12B13,
						DBUS_TYPE_UINT16, &CAPWAP_B14B15,
						DBUS_TYPE_UINT16, &H_802_11_B0B1,
						DBUS_TYPE_UINT16, &H_802_11_B2B3,
						DBUS_TYPE_UINT16, &H_802_11_B4B5,
						DBUS_TYPE_UINT16, &H_802_11_B6B7,
						DBUS_TYPE_UINT16, &H_802_11_B8B9,
						DBUS_TYPE_UINT16, &H_802_11_B10B11,
						DBUS_TYPE_UINT16, &H_802_11_B12B13,
						DBUS_TYPE_UINT16, &H_802_11_B14B15,
						DBUS_TYPE_UINT16, &H_802_11_B16B17,
						DBUS_TYPE_UINT16, &H_802_11_B18B19,
						DBUS_TYPE_UINT16, &H_802_11_B20B21,
						DBUS_TYPE_UINT16, &H_802_11_B22B23,
						DBUS_TYPE_UINT16, &H_802_11_B24B25,
						DBUS_TYPE_UINT16, &H_802_11_B26B27,
						DBUS_TYPE_UINT16, &NUM,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty,". ");
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"write cam table dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		vty_out(vty, "  OK!\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;

		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}    
}
#endif
DEFUN(cam_core_read,
	cam_core_read_cmd,
	"show fpga cam-core SLOT_ID SIP DIP SPORT DPORT PROTOCOL",
	"Show system information\n"
	"Show fpga information\n"
	"show fpga cam core \n"
	"the slot id num \n"
	"source ip address.eg:192.168.1.1 \n"
	"destination ip address.eg:192.168.1.1 \n"
	"source port(D).eg:2000 \n"
	"destination port(D).eg:3000 \n"
	"protocol(D).eg:17 \n")

{
	DBusMessage *query, *reply;
	DBusError err;
	struct in_addr sip_val;
	struct in_addr dip_val;
	int ret;
	int slot_id;
	unsigned short SIP_H=0;
	unsigned short SIP_L=0;
    unsigned short DIP_H=0;
	unsigned short DIP_L=0;
	unsigned short SPORT=0;
	unsigned short DPORT=0;
	unsigned short PROTOCOL=0;
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

	vty_out(vty,"begin to read cam core table,wait .");
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
    ret = inet_aton(argv[1], &sip_val);
    if(ret)
    {
		vty_out(vty,". ");
        //vty_out(vty,"**the sip = %x\n",sip_val.s_addr);
    }else
    {   
        vty_out(vty,"**ip conversion error\n");
		return CMD_WARNING;
    }
    ret = inet_aton(argv[2], &dip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //vty_out(vty,"**the dip = %x\n",dip_val.s_addr);
    }else
    {   
        vty_out(vty,"**ip conversion error\n");
		return CMD_WARNING;
    }  
	
	SIP_H = sip_val.s_addr >> 16;
	SIP_L = sip_val.s_addr & 0x0000ffff;
    DIP_H = dip_val.s_addr >> 16;
    DIP_L = dip_val.s_addr & 0x0000ffff;

	DPORT = str_10_short_num((char *)argv[4]);
	if(DPORT == 0)
	{
		vty_out(vty,"Bad param DPORT.range 0~65535!\n");
	    return CMD_WARNING;
	}
	SPORT = str_10_short_num((char *)argv[3]);
	if(SPORT == 0)
	{
		vty_out(vty,"Bad param SPORT.range 0~65535!\n");
	    return CMD_WARNING;
	}
	ret = fpga_parse_short((char *)argv[5],&PROTOCOL);
	if(0 != ret){
		vty_out(vty,"parse PROTOCOL param error\n");
		return CMD_WARNING;
	}
	PROTOCOL = PROTOCOL << 8;

/*	
	vty_out(vty,"******slot_id = 0x%x******\n",slot_id);
	vty_out(vty,"******SIP_H = 0x%x******\n",SIP_H);
	vty_out(vty,"******SIP_L = 0x%x******\n",SIP_L);
	vty_out(vty,"******DIP_H = 0x%x******\n",DIP_H);
	vty_out(vty,"******DIP_L = 0x%x******\n",DIP_L);
	vty_out(vty,"******SPORT = 0x%x******\n",SPORT);
    vty_out(vty,"******DPORT = 0x%x******\n",DPORT);
    vty_out(vty,"******PROTOCOL = 0x%x******\n",PROTOCOL);
*/	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CAM_CORE_READ);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT16, &SIP_H,
						DBUS_TYPE_UINT16, &SIP_L,
						DBUS_TYPE_UINT16, &DIP_H,
						DBUS_TYPE_UINT16, &DIP_L,
						DBUS_TYPE_UINT16, &SPORT,
						DBUS_TYPE_UINT16, &DPORT,
						DBUS_TYPE_UINT16, &PROTOCOL,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty,". ");
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {		
			if(ret == -1){
				vty_out(vty,"there is not this entry.\n");
			}
			else{
				vty_out(vty,".    ");
				vty_out(vty,"the CAM ID = %d\n",ret);
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(cam_core_write,
	cam_core_write_cmd,
	"set fpga cam-core SLOT_ID CAM_ID SIP DIP SPORT DPORT PROTOCOL NUM",
	"set system parameters\n"
	"set fpga parameters\n"
	"set fpga cam core \n"
	"the slot id num \n"
	"the cam id num.(0~63) \n"
	"source ip address.eg:192.168.1.10 \n"
	"destination ip address.eg:192.168.1.10 \n"
	"source port(D).eg:100 \n"
	"destination port(D).eg:100 \n"
	"protocol(D).eg:17 \n"
	"batch process\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr sip_val;
	struct in_addr dip_val;
	int ret;
	int slot_id;
	unsigned int cam_id=0;
	unsigned short Indirect_add_reg_H = 0;
	unsigned short Indirect_add_reg_L = 0;
	unsigned short SIP_H=0;
	unsigned short SIP_L=0;
    unsigned short DIP_H=0;
	unsigned short DIP_L=0;
	unsigned short SPORT=0;
	unsigned short DPORT=0;
	unsigned short PROTOCOL=0;
	unsigned short num=0;
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    vty_out(vty,"begin to write cam core table,wait . ");
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	
    ret = inet_aton(argv[2], &sip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //vty_out(vty,"**the ip = %x\n",sip_val.s_addr);
    }else
    {   
        printf("**ip conversion error\n");
		return CMD_WARNING;
    }
    ret = inet_aton(argv[3], &dip_val);
    if(ret)
    {   
		vty_out(vty,". ");
        //vty_out(vty,"**the ip = %x\n",dip_val.s_addr);
    }else
    {   
        printf("**ip conversion error\n");
		return CMD_WARNING;
    }  
	ret = fpga_parse_int((char *)argv[1],&cam_id);
	if(0 != ret){
		vty_out(vty,"parse cam_id param error\n");
		return CMD_WARNING;
	}
	if(cam_id>63 ||cam_id<0)
	{
       	vty_out(vty,"Param cam_id error,range 0~63!\n");
		return CMD_WARNING; 
	}
    Indirect_add_reg_H = cam_id >> 16;
	Indirect_add_reg_L = cam_id & 0x0000ffff;
	
    SIP_H = sip_val.s_addr >> 16;
	SIP_L = sip_val.s_addr & 0x0000ffff;
    DIP_H = dip_val.s_addr >> 16;
    DIP_L = dip_val.s_addr & 0x0000ffff;

	DPORT = str_10_short_num((char *)argv[5]);
	if(DPORT == 0)
	{
		vty_out(vty,"Bad param DPORT.range 0~65535!\n");
	    return CMD_WARNING;
	}
	SPORT = str_10_short_num((char *)argv[4]);
	if(SPORT == 0)
	{
		vty_out(vty,"Bad param SPORT.range 0~65535!\n");
	    return CMD_WARNING;
	}
	ret = fpga_parse_short((char *)argv[6],&PROTOCOL);
	if(0 != ret){
		vty_out(vty,"parse PROTOCOL param error\n");
		return CMD_WARNING;
	}
	PROTOCOL = PROTOCOL << 8;

	
	ret = fpga_parse_short((char *)argv[7],&num);
	if(0 != ret){
		vty_out(vty,"parse num param error\n");
		return CMD_WARNING;
	}
	if(num > 64 || num < 0)
	{
        vty_out(vty,"Invalid num parameter!range 0~63.\n");
		return CMD_WARNING;
	}
/*	
	vty_out(vty,"******slot_id = 0x%x******\n",slot_id);
	vty_out(vty,"******Indirect_add_reg_H = 0x%x******\n",Indirect_add_reg_H);
	vty_out(vty,"******Indirect_add_reg_L = 0x%x******\n",Indirect_add_reg_L);
	vty_out(vty,"******SIP_H = 0x%x******\n",SIP_H);
	vty_out(vty,"******SIP_L = 0x%x******\n",SIP_L);
	vty_out(vty,"******DIP_H = 0x%x******\n",DIP_H);
	vty_out(vty,"******DIP_L = 0x%x******\n",DIP_L);
	vty_out(vty,"******SPORT = 0x%x******\n",SPORT);
    vty_out(vty,"******DPORT = 0x%x******\n",DPORT);
    vty_out(vty,"******PROTOCOL = 0x%x******\n",PROTOCOL);
    vty_out(vty,"******num = 0x%x******\n",num);
*/
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CAM_CORE_WRITE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT16, &Indirect_add_reg_H,
	                    DBUS_TYPE_UINT16, &Indirect_add_reg_L,
						DBUS_TYPE_UINT16, &SIP_H,
						DBUS_TYPE_UINT16, &SIP_L,
						DBUS_TYPE_UINT16, &DIP_H,
						DBUS_TYPE_UINT16, &DIP_L,
						DBUS_TYPE_UINT16, &SPORT,
						DBUS_TYPE_UINT16, &DPORT,
						DBUS_TYPE_UINT16, &PROTOCOL,
						DBUS_TYPE_UINT16, &num,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty,". ");
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID))
		{
			if(ret)
			{
				vty_out(vty,".    ");
			}
			else
			{
				vty_out(vty,"write cam coret dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		vty_out(vty, "  OK!\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;
		} 
	    else 
		{
    		vty_out(vty, "no connection to slot %d\n", slot_id);
    		return CMD_WARNING;
		}
}

DEFUN(hash_aging_time_set,
	set_hash_aging_time_cmd,
	"config fpga hash-aging-time slot SLOT_ID time <0-65535>",
	"config fpga the hash aging time \n"
	"config fpga parameters \n"
	"config fpga hash aging time \n"
	"the slot number \n"
	"slot number \n"
	"the hash aging time. \n"
	"Aging time range:1s-65535s\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	int slot_id;
	unsigned short time = 0;
    vty_out(vty,"begin to set hash aging time,wait. ");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	
	ret = fpga_parse_short((char*)argv[1],&time);
	if(0 != ret){
		vty_out(vty,"parse time param error\n");
		return CMD_WARNING;
	}
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SET_HASH_AGING_TIME);
	dbus_error_init(&err);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT16, &time,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty,". ");
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 0){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"write hash time dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		vty_out(vty, "  OK!\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;

		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}
DEFUN(hash_update_time_set,
	set_hash_update_time_cmd,
	"config fpga hash-update-time slot SLOT_ID time <0-65535>",
	"config fpga the hash update time \n"
	"config fpga parameters \n"
	"config fpga hash update time \n"
	"the slot number \n"
	"slot number \n"
	"the hash update time. \n"
	"Update time range:1s-65535s\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	int slot_id;
	unsigned short time = 0;
    vty_out(vty,"begin to set hash aging time,wait. ");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	ret = fpga_parse_short((char*)argv[1],&time);
	if(0 != ret){
		vty_out(vty,"parse time param error\n");
		return CMD_WARNING;
	}
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SET_HASH_UPDATE_TIME);
	dbus_error_init(&err);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT16, &time,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty,". ");
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 0){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"write hash update time dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		vty_out(vty, "  OK!\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;

		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(show_hash_capacity,
	show_hash_capacity_cmd,
	"show fpga hash capacity SLOT_ID",
	"Show system information\n"
	"Show fpga information\n"
	"show the fpga hash capacity factor \n"
	"show the hash capacity factor \n"
	"the slot id num \n")
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned long long num = 0 ;
	int slot_id;
	unsigned short time = 0;
    vty_out(vty,"Is retrieving utilization value,wait.");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_HASH_CAPACITY);
	dbus_error_init(&err);

    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	{
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 65530, &err);
		dbus_message_unref(query);
		
		vty_out(vty,".");

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT64,&num,
						DBUS_TYPE_INVALID))
		{
			vty_out(vty,".    OK!\n");
		    vty_out(vty, "the num of having used entries is %lld\n",num);
			vty_out(vty, "the capacity of hash table is %f%%\n",(double)(num)/HASH_ENTRY_NUM);
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		vty_out(vty, "get args from replay fail\n");
		dbus_message_unref(reply);
    	return CMD_WARNING;
	}
	else 
	{
    	vty_out(vty, "no connection to slot %d\n", slot_id);
    	return CMD_WARNING;
	}
}

DEFUN(show_fpga_systerm_register,
	show_fpga_systerm_register_cmd,
	"show fpga system SLOT_ID",
	"Show system information\n"
	"Show fpga information\n"
	"show the important state of the FPGA \n"
	"the slot id num \n")
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	int slot_id;
	
    unsigned int device_id = 0;
    unsigned int sys_version = 0;
    unsigned int sys_ctl_1 = 0;
	unsigned int sys_ctl_2 = 0;
    unsigned int sys_state = 0;
    unsigned int hash_age = 0;
    unsigned int hash_updata = 0;
	char *work=NULL;
    char *QoS=NULL;
	char *hash=NULL;
	char *car=NULL;
	char *DSA=NULL;
	char *wan=NULL;
	char *linkdown=NULL;
	char *linkup=NULL;
	
	vty_out(vty,"Is retrieving systerm register value,wait.");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_FPGA_REG);
	dbus_error_init(&err);

    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	{
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 65530, &err);
		dbus_message_unref(query);
		
		vty_out(vty,".");

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
    	if (dbus_message_get_args ( reply, &err,
    		                     DBUS_TYPE_UINT32,&device_id,
    		                     DBUS_TYPE_UINT32,&sys_version,
    		                     DBUS_TYPE_UINT32,&sys_ctl_1,
    		                     DBUS_TYPE_UINT32,&sys_ctl_2,
    							 DBUS_TYPE_UINT32,&sys_state,
    							 DBUS_TYPE_UINT32,&hash_age,
    							 DBUS_TYPE_UINT32,&hash_updata,
    		                     DBUS_TYPE_INVALID))
         {
		 	if(sys_state == 0xffff)
		 	{
				if((sys_ctl_1 & 0x8000)>>15 ==1)
				    work = "work                enable";
				else
				    work = "work                disable";
				
				if((sys_ctl_1 & 0x4000)>>14 ==1)
					QoS = "QoS                 enable";
				else
					QoS = "QoS                 disable";
				
				if((sys_ctl_1 & 0x2000)>>13 ==1)
					hash = "hash                enable";
				else
					hash = "cam                enable";
				
				if((sys_ctl_1 & 0xc0)>>6 == 0)
					car = "car                 1s";
				else if((sys_ctl_1 & 0xc0)>>6 == 1)
					car = "car                 0.5s";
				else if((sys_ctl_1 & 0xc0)>>6 == 3)
					car = "car                 0.25s";

				if((sys_ctl_1 & 0x30)>>4 == 3)
					DSA = "DSA                 enable";
				else
					DSA = "DSA                 disable";

				if((sys_ctl_1 & 0xc)>>2 == 2)
					wan = "wan                 double vlan";
				else if((sys_ctl_1 & 0xc)>>2 == 1)
					wan = "wan                 vlan";
				else
					wan = "wan                 no tag";

				if((sys_ctl_1 & 0x2)>>1 == 1)
					linkdown = "linkdown            enable";
				else
					linkdown = "linkdown            disable";

				if((sys_ctl_1 & 0x1) == 1)
					linkup = "linkup              enable";
				else
					linkup = "linkup              disable";
					
                vty_out(vty,".    OK!\n");
                vty_out(vty, "Show the state of FPGA\n");
                vty_out(vty,"===========================================\n");
                vty_out(vty,"FPGA version        V%d\n",sys_version);
                //vty_out(vty,"system control--0X%x\n",sys_ctl_1);
                vty_out(vty,"hash age time       %dS\n",hash_age);
                vty_out(vty,"hash updata time    %dS\n",hash_updata);
				vty_out(vty,"%s\n",work);
				vty_out(vty,"%s\n",QoS);
				vty_out(vty,"%s\n",hash);
				vty_out(vty,"%s\n",car);
				vty_out(vty,"%s\n",DSA);
				vty_out(vty,"%s\n",wan);
				vty_out(vty,"%s\n",linkdown);
				vty_out(vty,"%s\n",linkup);
                vty_out(vty,"===========================================\n");
		 	}
			else
			{
                vty_out(vty,"\nFPGA state work not normal!!!system state register = 0X%x\n",sys_state);
			}
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		vty_out(vty, "get args from replay fail\n");
		dbus_message_unref(reply);
    	return CMD_WARNING;

	}
	else 
	{
    	vty_out(vty, "no connection to slot %d\n", slot_id);
    	return CMD_WARNING;
	}
	
}

DEFUN(config_system_control_reg_working,
	config_system_control_reg_working_cmd,
	"config fpga work SLOT_ID (enable|disable)",
	"config system parameters\n"
	"config fpga parameters\n"
	"config fpga system control register working\n"
	"the slot id num \n"
	"enable fpga working \n"
	"disable fpga working \n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	unsigned short work_flag = 1;
	
	vty_out(vty,"begin to config fpga sysreg work,wait .");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	if(0 == strncmp("enable",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		work_flag = 1;
	}
	else if(0 == strncmp("disable",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		work_flag = 0;
	}
	else 
	{
    	vty_out(vty,"%% Unknow input param\n");
		return CMD_WARNING;
	}

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_FPGA_SYSREG_WORKING);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_UINT16, &work_flag,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 1){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"config fpga system control register working dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		if(work_flag)
		{
		    vty_out(vty, "\nEnable fpga working success!\n");
		}
		else
		{
            vty_out(vty, "\nDisable fpga working success!\n");
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(config_system_control_reg_QoS,
	config_system_control_reg_QoS_cmd,
	"config fpga qos SLOT_ID (enable|disable)",
	"config system parameters\n"
	"config fpga parameters\n"
	"config fpga system control register QoS\n"
	"the slot id num \n"
	"enable fpga QoS \n"
	"disable fpga QoS \n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	unsigned short QoS_flag = 1;
	
	vty_out(vty,"begin to config fpga sysreg QoS,wait .");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	if(0 == strncmp("enable",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		QoS_flag = 1;
	}
	else if(0 == strncmp("disable",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		QoS_flag = 0;
	}
	else 
	{
    	vty_out(vty,"%% Unknow input param\n");
		return CMD_WARNING;
	}
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_FPGA_SYSREG_QOS);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_UINT16, &QoS_flag,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 1){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"config fpga system control register QoS dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		if(QoS_flag)
		{
		    vty_out(vty, "\nEnable fpga QoS success!\n");
		}
		else
		{
            vty_out(vty, "\nDisable fpga QoS success!\n");
		}
		
		dbus_message_unref(reply);
		return CMD_SUCCESS;
		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(config_system_control_reg_mode,
	config_system_control_reg_mode_cmd,
	"config fpga mode SLOT_ID (hash|cam)",
	"config system parameters\n"
	"config fpga parameters\n"
	"config fpga system control register mode\n"
	"the slot id num \n"
	"fpga hash mode \n"
	"fpga cam mode \n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	unsigned short mode_flag = 1;
	
	vty_out(vty,"begin to config fpga sysreg work mode,wait .");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	if(0 == strncmp("hash",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		mode_flag = 1;
	}
	else if(0 == strncmp("cam",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		mode_flag = 0;
	}
	else 
	{
    	vty_out(vty,"%% Unknow input param\n");
		return CMD_WARNING;
	}
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_FPGA_SYSREG_MODE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_UINT16, &mode_flag,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 1){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"config fpga system control register mode dbus failed.\n");
				return CMD_WARNING;
			}
		}
		vty_out(vty, "\nconfig fpga %s mode success!\n",argv[1]);
		dbus_message_unref(reply);
		return CMD_SUCCESS;
		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(config_system_control_reg_car_update_cfg,
	config_system_control_reg_car_update_cfg_cmd,
	"config fpga car_update_cfg SLOT_ID (0.25s|0.5s|1s)",
	"config system parameters\n"
	"config fpga parameters\n"
	"config fpga system control register car_update_cfg\n"
	"the slot id num \n"
	"0.25S update \n"
	"0.5S update \n"
	"1S update \n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	unsigned short car_update_cfg_flag = 0;
	
	vty_out(vty,"begin to config fpga sysreg car_update_cfg,wait .");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	if(0 == strncmp("0.25s",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		car_update_cfg_flag = 2;
	}
	else if(0 == strncmp("0.5s",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		car_update_cfg_flag = 1;
	}
	else if(0 == strncmp("1s",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		car_update_cfg_flag = 0;
	}
	else
	{
    	vty_out(vty,"%% Unknow input param\n");
		return CMD_WARNING;
	}

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_FPGA_SYSREG_CAR_UPDATE_CFG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_UINT16, &car_update_cfg_flag,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 1){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"config fpga system control register car_update_cfg dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		vty_out(vty, "\nconfig fpga car update cfg %s success\n",argv[1]);
		dbus_message_unref(reply);
		return CMD_SUCCESS;

		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(config_system_control_reg_trunk_tag_type,
	config_system_control_reg_trunk_tag_type_cmd,
	"config fpga dsa SLOT_ID (enable|disable)",
	"config system parameters\n"
	"config fpga parameters\n"
	"config fpga system control register reg_trunk_tag_type\n"
	"the slot id num \n"
	"receive and transmit DSA tag\n"
	"receive and transmit VLAN tag \n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	unsigned short vlan_flag = 0;
	
	vty_out(vty,"begin to config fpga sysreg trunk_tag_type,wait .");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	if(0 == strncmp("enable",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		vlan_flag = 0;
	}
	else if(0 == strncmp("disable",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		vlan_flag = 1;
	}
	else
	{
    	vty_out(vty,"%% Unknow input param\n");
		return CMD_WARNING;
	}

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_FPGA_SYSREG_TRUNK_TAG_TYPE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_UINT16, &vlan_flag,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 1){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"config fpga system control register trunk tag type dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		if(vlan_flag)
		{
		    vty_out(vty, "\nconfig fpga trunk tag type is not DSA success!\n");
		}
		else
		{
            vty_out(vty, "\nconfig fpga trunk tag type DSA success!\n");
		}
		
		dbus_message_unref(reply);
		return CMD_SUCCESS;

		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(config_system_control_reg_wan_tag_type,
	config_system_control_reg_wan_tag_type_cmd,
	"config fpga wan SLOT_ID (double|one|untag)",
	"config system parameters\n"
	"config fpga parameters\n"
	"config fpga system control register wan_tag_type\n"
	"the slot id num \n"
	"transmit DOUBLE-VLAN-TAG\n"
	"transmit VLAN-TAG \n"
	"transmit UNTAG \n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	unsigned short wan_vlan_flag = 0;
	
	vty_out(vty,"begin to config fpga sysreg work,wait .");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	if(0 == strncmp("double",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		wan_vlan_flag = 2;
	}
	else if(0 == strncmp("one",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		wan_vlan_flag = 1;
	}
	else if(0 == strncmp("untag",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		wan_vlan_flag = 0;
	}
	else
	{
    	vty_out(vty,"%% Unknow input param\n");
		return CMD_WARNING;
	}

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_FPGA_SYSREG_WAN_TAG_TYPE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_UINT16, &wan_vlan_flag,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 1){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"config fpga system control register wan tag type dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		if(wan_vlan_flag == 2)
		{
		    vty_out(vty, "\nconfig fpga wan tag type double vlan tag success!\n");
		}else if(wan_vlan_flag == 1)
		{
            vty_out(vty, "\nconfig fpga wan tag type vlan tag success!\n");
		}else
		{
            vty_out(vty, "\nconfig fpga wan tag type untag success!\n");
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;

		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(config_system_control_reg_car_linkup,
	config_system_control_reg_car_linkup_cmd,
	"config fpga linkup SLOT_ID (enable|disable)",
	"config system parameters\n"
	"config fpga parameters\n"
	"config fpga system control register car_linkup\n"
	"the slot id num \n"
	"enable linkup car\n"
	"disable linkup car\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	unsigned short car_linkup_flag = 0;
	
	vty_out(vty,"begin to config fpga sysreg car_linkup,wait .");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	if(0 == strncmp("enable",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		car_linkup_flag = 1;
	}
	else if(0 == strncmp("disable",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		car_linkup_flag = 0;
	}
	else
	{
    	vty_out(vty,"%% Unknow input param\n");
		return CMD_WARNING;
	}

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_FPGA_SYSREG_CAR_LINKUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_UINT16, &car_linkup_flag,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 1){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"config fpga system control register car_linkup dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		if(car_linkup_flag)
		{
		    vty_out(vty, "\nEnable fpga car linkup success!\n");
		}
		else
		{
            vty_out(vty, "\nDisable fpga car linkup success!\n");
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;

		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(config_system_control_reg_car_linkdown,
	config_system_control_reg_car_linkdown_cmd,
	"config fpga linkdown SLOT_ID (enable|disable)",
	"config system parameters\n"
	"config fpga parameters\n"
	"config fpga system control register car_linkdown\n"
	"the slot id num \n"
	"enable linkdown car\n"
	"disable linkdown car\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	unsigned short car_linkdown_flag = 0;
	
	vty_out(vty,"begin to config fpga sysreg car_linkdown,wait .");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	if(0 == strncmp("enable",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		car_linkdown_flag = 1;
	}
	else if(0 == strncmp("disable",argv[1],strlen((char*)argv[1])))
	{
		vty_out(vty,".");
		car_linkdown_flag = 0;
	}
	else
	{
    	vty_out(vty,"%% Unknow input param\n");
		return CMD_WARNING;
	}

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_FPGA_SYSREG_CAR_LINKDOWN);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_UINT16, &car_linkdown_flag,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 1){
				vty_out(vty,".    ");
			}
			else{
				vty_out(vty,"config fpga system control register car_linkdown dbus failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		if(car_linkdown_flag)
		{
		    vty_out(vty, "\nEnable fpga car linkdown success!\n");
		}else{
            vty_out(vty, "\nDisable fpga car linkdown success!\n");
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;

		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(set_cpld_reg,
	set_cpld_reg_cmd,
	"set cpld slot SLOT_ID reg ADDRESS val VALUE",
	"set cpld reg\n"
	"set cpld reg\n"
	"slot id\n"
	"slot id\n"
	"the address of cpld\n"
	"the address of cpld\n"
	"the val of reg\n"
	"the val of reg\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	int reg_add;
	int reg_val;
	
	vty_out(vty,"begin to write cpld reg");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	reg_add = strtol(argv[1],NULL,16);
	reg_val = strtol(argv[2],NULL,16);
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_WRITE_CPLD);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_INT32, &reg_add,
		                DBUS_TYPE_INT32, &reg_val,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, -1, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
        vty_out(vty,"...    \n");
		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_INT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 0){
				vty_out(vty,"write cpld reg success.\n");
			}
			else{
				vty_out(vty,"write cpld reg failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(set_fpga_reset,
	set_fpga_reset_cmd,
	"set fpga reset slot SLOT_ID",
	"set cpld reg\n"
	"set cpld reg\n"
	"slot id\n"
	"slot id\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	
	vty_out(vty,"begin to reset FPGA");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SET_FPGA_RESET);
	dbus_error_init(&err);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, -1, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
        vty_out(vty,"...    \n");
		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_INT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret){
				vty_out(vty,"reset fpga success.\n");
			}
			else{
				vty_out(vty,"reset fpga failed.return %d\n",ret);
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(read_cpld_reg,
	read_cpld_reg_cmd,
	"show cpld slot SLOT_ID reg ADDRESS",
	"read cpld reg\n"
	"read cpld reg\n"
	"slot id\n"
	"slot id\n"
	"the address of cpld\n"
	"the address of cpld\n")
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	int reg_add;
	int reg_val;
	
	vty_out(vty,"begin to read cpld reg");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	reg_add = strtol(argv[1],NULL,16);
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_READ_CPLD);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_INT32, &reg_add,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, -1, &err);
		dbus_message_unref(query);
        
		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
        vty_out(vty,"...\n");
		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_INT32,&ret,
						DBUS_TYPE_INT32,&reg_val,
						DBUS_TYPE_INVALID)) {
			if(ret == 0){
				vty_out(vty,"read cpld reg success.val = 0x%x\n",reg_val);
			}
			else{
				vty_out(vty,"read cpld reg failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}


DEFUN(show_fpga_hash_valid,
	show_fpga_hash_valid_cmd,
	"show fpga hash-valid SLOT_ID",
	"show system parameters\n"
	"show fpga parameters\n"
	"show fpga hash value valid\n"
	"the slot id num \n")
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	int slot_id;
	unsigned int hash_id=0;
	unsigned int count_valid=0;
	unsigned short car_linkdown_flag = 0;
	unsigned int i=0;
	int flag;
	unsigned int hash_id_print=0;

	struct timeval start, end;
    int interval;
	
	vty_out(vty,"begin to show fpga hash value valid,wait .");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"SLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}

    for(hash_id =0;hash_id<209;hash_id++)
    {
		flag = 1;
		dbus_error_init(&err);
	    query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_FPGA_HASH_VALID);
	
	    dbus_message_append_args(query,
                        DBUS_TYPE_UINT32, &hash_id,
                        DBUS_TYPE_INT32, &flag,
                        DBUS_TYPE_UINT32, &count_valid,
						DBUS_TYPE_INVALID);

	    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&count_valid,
						DBUS_TYPE_INVALID)) {
			hash_id_print = hash_id * 10000;
            vty_out(vty, "hash_id %d~%d,hash valid count = %d\n",hash_id_print,hash_id_print+9999,count_valid);
		}
		dbus_message_unref(reply);
		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
    }

	flag = 0;
	hash_id = 209;
	dbus_error_init(&err);
    query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
											 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_FPGA_HASH_VALID);

    dbus_message_append_args(query,
                    DBUS_TYPE_UINT32, &hash_id,
                    DBUS_TYPE_INT32, &flag,
                    DBUS_TYPE_UINT32, &count_valid,
					DBUS_TYPE_INVALID);

    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
		query, 60000, &err);
	dbus_message_unref(query);

	if (NULL == reply){
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args (reply, &err,
					DBUS_TYPE_UINT32,&count_valid,
					DBUS_TYPE_INVALID)) {
		hash_id = hash_id * 10000;
        vty_out(vty, "hash_id %d~%d,hash valid count = %d\n",hash_id,hash_id+7151,count_valid);
	}
	dbus_message_unref(reply);
	} else {
	vty_out(vty, "no connection to slot %d\n", slot_id);
	return CMD_WARNING;
	}

    vty_out(vty, "***************hash table valid count = %d*************\n",count_valid);
return CMD_WARNING;
}


DEFUN(show_fpga_ddr_qdr_test,
	show_fpga_ddr_qdr_test_cmd,
	"show fpga ram-detection SLOT_ID",
	"Show system information\n"
	"Show fpga information\n"
	"fpga DDR and QDR detection\n"
	"the slot id num\n")
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret;
	int i;
	int slot_id;
	char *endptr = NULL;
	unsigned int num;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    unsigned int hash_table_id = 0;
	unsigned int flag_hash_data_1 = 0;
	unsigned int flag_hash_data_2 = 0;
	unsigned int flag_hash_addr_low_1 = 0;
	unsigned int flag_hash_addr_1 = 0;
	unsigned int flag_hash_addr_low_2 = 0;
	unsigned int flag_hash_addr_2 = 0;
	
    unsigned int flag_car_data = 0;
	unsigned int flag_car_addr_low = 0;
	unsigned int flag_car_addr = 0;
	
	unsigned int flag_car = 0;
    unsigned int flag_hash = 0;
	
	char hash_flag_binary[32];
	char car_flag_binary[32];
	
	vty_out(vty,"begin to detection fpga DDR and QDR...");
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"\nSLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	//hash_table_id = strtol(argv[1],NULL,16);
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_FPGA_DDR_QDR);
	dbus_error_init(&err);

    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	{
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, -1, &err);
		dbus_message_unref(query);
		
		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT32,&flag_hash_data_1,
						DBUS_TYPE_UINT32,&flag_hash_data_2,
						DBUS_TYPE_UINT32,&flag_hash_addr_low_1,
						DBUS_TYPE_UINT32,&flag_hash_addr_1,
						DBUS_TYPE_UINT32,&flag_hash_addr_low_2,
						DBUS_TYPE_UINT32,&flag_hash_addr_2,
						DBUS_TYPE_UINT32,&flag_car_data,
						DBUS_TYPE_UINT32,&flag_car_addr_low,
						DBUS_TYPE_UINT32,&flag_car_addr,
						DBUS_TYPE_INVALID))
		{			
			vty_out(vty,"   OK!The result is:\n");

            if(flag_hash_data_1 == 0)
            {
                vty_out(vty,"Detection DDR1 Data lines OK!\n");
			}
			else
			{
                vty_out(vty,"Detection DDR1 Data lines flag_hash_data_1 = %x\n",flag_hash_data_1);
			}
            if(flag_hash_data_2 == 0)
            {
                vty_out(vty,"Detection DDR2 Data lines OK!\n");
			}
			else
			{
                vty_out(vty,"Detection DDR2 Data lines flag_hash_data_2 = %x\n",flag_hash_data_2);
			}
            if(flag_hash_addr_low_1 == 0)
            {
                vty_out(vty,"Detection DDR 1 address low lines OK!\n");
			}
			else
			{
                vty_out(vty,"Detection DDR 1 address low lines flag_hash_addr_low = %x\n",flag_hash_addr_low_1);
			}
            if(flag_hash_addr_1 == 0)
            {
                vty_out(vty,"Detection DDR 1 address lines OK!\n");
			}
			else
			{
                vty_out(vty,"Detection DDR 1 address lines flag_hash_addr_1 = %x\n",flag_hash_addr_1);
			}
            if(flag_hash_addr_low_2 == 0)
            {
                vty_out(vty,"Detection DDR 2 address low lines OK!\n");
			}
			else
			{
                vty_out(vty,"Detection DDR 2 address low lines flag_hash_addr_low = %x\n",flag_hash_addr_low_2);
			}
            if(flag_hash_addr_2 == 0)
            {
                vty_out(vty,"Detection DDR 2 address lines OK!\n");
			}
			else
			{
                vty_out(vty,"Detection DDR 2 address lines flag_hash_addr_2 = %x\n",flag_hash_addr_2);
			}

            if(flag_car_data == 0)
            {
                vty_out(vty,"Detection QDR Data lines ...OK!\n");
			}
			else
			{
                vty_out(vty,"Detection QDR Data lines...error!flag_car_data = %x\n",flag_car_data);
			}
            if(flag_car_addr_low == 0)
            {
                vty_out(vty,"Detection QDR low address lines ...OK!\n");
			}
			else
			{
				//binary(flag_car,car_flag_binary);
                vty_out(vty,"Detection QDR low address ...error!flag_car_addr_low = %x\n",flag_car_addr_low);
				//vty_out(vty,"********************************************\n");
				//for(i=0;i<32;i++)
				//{
                //   vty_out(vty,"address line %d --------%c\n",i,car_flag_binary[i]);
				//}
				//vty_out(vty,"********************************************\n");
			}
            if(flag_car_addr == 0)
            {
                vty_out(vty,"Detection QDR address lines ...OK!\n");
			}
			else
			{
                vty_out(vty,"Detection QDR address ...error!flag_car_addr = %x\n",flag_car_addr);
			}
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		vty_out(vty, "get args from replay fail\n");
		dbus_message_unref(reply);
    	return CMD_WARNING;
	}
	else 
	{
    	vty_out(vty, "no connection to slot %d\n", slot_id);
    	return CMD_WARNING;
	}
}

DEFUN(show_fpga_reg,
	show_fpga_reg_cmd,
	"show fpga reg slot SLOT_ID start_address ADDRESS num NUM",
	"Show system information\n"
	"Show fpga information\n"
	"fpga register\n"
	"the slot id num\n"
    "the slot id num\n"
    "the start address\n"
    "the start address,eg:0x0002\n"
    "the num of reg \n"
)
{
    DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusError err;
	int ret;
	int i;
	int slot_id;
	unsigned short address =0;
	unsigned short val =0;
	char *endptr = NULL;
	unsigned int num;
	unsigned long long reg_base=0x800000001d070000ull;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
	vty_out(vty,"begin to read fpga reg...\n");
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	ret = check_board_type(vty,slot_id);
	if(ret != 1)
	{
		vty_out(vty,"\nSLOT %d is not AX81_1X12G12S\n", slot_id);
        return CMD_WARNING;
	}
	
    ret = fpga_parse_short_H((char *)argv[1],&address);
	if(0 != ret){
		vty_out(vty,"parse ADDRESS param error\n");
		return CMD_WARNING;
	}
	if(address<0||address>0xffff)
	{
		vty_out(vty," Bad Parameters,ADDRESS outrange!\n");
		return CMD_WARNING;
	}

    num = strtoul(argv[2], &endptr, 10);
	if(!(num > 0 && num < 31))
	{
		vty_out(vty," Bad Parameters,num outrange!\n");
		return CMD_WARNING;
	}
	
    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) 
	{
		for(i=0;i<num;i++)
    	{
			query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_FPGA_REG_ARR);
        	dbus_error_init(&err);
        	
        	dbus_message_append_args(query,
        		                DBUS_TYPE_UINT16, &address,
        						DBUS_TYPE_INVALID);

    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
    			query, -1, &err);
    		dbus_message_unref(query);

    		if (NULL == reply)
    		{
    			vty_out(vty,"<error> failed get reply.\n");
    			if (dbus_error_is_set(&err))
    			{
    				vty_out(vty,"%s raised: %s",err.name,err.message);
    				dbus_error_free_for_dcli(&err);
    			}
    			continue;
    		}
            
    		if (dbus_message_get_args (reply, &err,
    						DBUS_TYPE_UINT16,&val,
    						DBUS_TYPE_INVALID))
    		{			
				vty_out(vty,"reg:%llx   value:0x%x\n",reg_base+address,val);
    			dbus_message_unref(reply);
    		}
			else
			{
        		vty_out(vty, "get args from replay fail\n");
        		dbus_message_unref(reply);
			}
			address = address + 2;
        }
	}
	else 
    {
        vty_out(vty, "no connection to slot %d\n", slot_id);
        return CMD_WARNING;
    }
	return CMD_SUCCESS;
}

DEFUN(write_fpga_reg_single,
	write_fpga_reg_single_cmd,
	"set fpga reg slot SLOT_ID address ADDRESS val VAL",
	"set system information\n"
	"set fpga reg\n"
	"fpga register\n"
	"the slot id num\n"
    "the slot id num\n"
    "the start address\n"
    "the start address,eg:0x0002\n"
    "the val of reg (0x0~0xffff)\n"
    "the val of reg (0x0~0xffff)\n"
)
{
    DBusMessage *query, *reply;
	DBusError err;
	struct in_addr userip_val;
	int ret;
	int slot_id;
	int reg_add;
	int reg_val;
	
	vty_out(vty,"begin to write fpga reg");
	char *endptr = NULL;
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	reg_add = strtol(argv[1],NULL,16);
	reg_val = strtol(argv[2],NULL,16);
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_CONFIG_WRITE_FPGA);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                DBUS_TYPE_INT32, &reg_add,
		                DBUS_TYPE_INT32, &reg_val,
						DBUS_TYPE_INVALID);
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,\
			query, -1, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
        vty_out(vty,"...    \n");
		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_INT32,&ret,
						DBUS_TYPE_INVALID)) {
			if(ret == 0){
				vty_out(vty,"write fpga reg success.\n");
			}
			else{
				vty_out(vty,"write fpga reg failed.\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
		} else {
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
		}
}

DEFUN(show_wan_if,
	show_wan_if_cmd,
	"show fpga wan-if",
	"Show system information\n"
	"Show fpga information\n"
	"show fpga wan out interface\n"
	)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	int slot_id;
    int FLAG = 0;
	unsigned long long wan_if_num=0;
	unsigned long long i=0;
	char *if_name = NULL;	
	vty_out(vty,"begin to show fpga wan interface...");
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_GET_FPGA_WAN_IF_NUM);
	dbus_error_init(&err);
/*
	dbus_message_append_args(query,
		                DBUS_TYPE_INT32, &reg_add,
		                DBUS_TYPE_INT32, &reg_val,
						DBUS_TYPE_INVALID);
*/
	if (dbus_connection_dcli[active_master_slot]->dcli_dbus_connection) {
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[active_master_slot]->dcli_dbus_connection,\
			query, -1, &err);
		dbus_message_unref(query);

		if (NULL == reply){
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
        vty_out(vty,"...    \n");
		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_UINT64,&wan_if_num,
						DBUS_TYPE_INVALID)) {
				vty_out(vty,"the total interface of fpga wan port is %lld.\n",wan_if_num);
		}
		} else {
		vty_out(vty, "no connection to slot %d\n",active_master_slot);
		}

		for(i=1;i<=wan_if_num;i++)
		{
        	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
        												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_FPGA_WAN_IF);
        	dbus_error_init(&err);
        
        	dbus_message_append_args(query,
        		                DBUS_TYPE_UINT64, &i,
        						DBUS_TYPE_INVALID);
        
        	if (dbus_connection_dcli[active_master_slot]->dcli_dbus_connection) {
        		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[active_master_slot]->dcli_dbus_connection,\
        			query, 60000, &err);
        		dbus_message_unref(query);

        		if (NULL == reply){
        			vty_out(vty,"<error> failed get reply.\n");
        			if (dbus_error_is_set(&err)) {
        				vty_out(vty,"%s raised: %s",err.name,err.message);
        				dbus_error_free_for_dcli(&err);
        			}
        			return CMD_WARNING;
        		}

        		if (dbus_message_get_args (reply, &err,
        						DBUS_TYPE_INT32,&FLAG,
        						DBUS_TYPE_STRING,&if_name,
        						DBUS_TYPE_INVALID)) {
    				if(FLAG){
                        vty_out(vty,"Get the interface failed!\n");
					}else{
                        vty_out(vty,"Interface %lld:%s\n",i,if_name);
					}
        		}
        		} else {
        		vty_out(vty, "no connection to slot %d\n",active_master_slot);
        		}
		}
		dbus_message_unref(reply);
}


int dcli_fpga_show_running_config(struct vty* vty) 
{	
	char *showStr = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	//int *slot_id;
	int ret;
	
	ret = local_check_board_type(vty);
	if(ret != 1)
	{
		vty_out(vty,"SLOT is not AX81_1X12G12S\n");
        return -1;
	}
	vty_out(vty,"slot is AX81_1X12G12S\n");
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_FPGA_SHOW_RUNNING_CONFIG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("show fpga running config failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"FPGA");
		if(showStr == NULL)
		{
            return -1;
		}
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return 0;
}


void dcli_fpga_init(void) 
{
	//install_node (&fpga_node, dcli_fpga_show_running_config, "FPGA_NODE");
	
	install_element(ENABLE_NODE, &ax86_fpga_online_burning_cmd);
	
	install_element(CONFIG_NODE, &show_port_counter_cmd);

	install_element(CONFIG_NODE, &show_hash_statistics_cmd);
	install_element(CONFIG_NODE, &hash_table_read_cmd);

	install_element(CONFIG_NODE, &cam_table_read_cmd);
	install_element(CONFIG_NODE, &cam_table_write_cmd);
	
	install_element(CONFIG_NODE, &cam_core_read_cmd);
	install_element(CONFIG_NODE, &cam_core_write_cmd);

	install_element(CONFIG_NODE, &car_table_read_cmd);
	install_element(CONFIG_NODE, &car_table_write_cmd);
	install_element(CONFIG_NODE, &car_list_read_cmd);
	install_element(CONFIG_NODE, &show_fpga_car_valid_cmd);
	install_element(CONFIG_NODE, &car_subnet_write_cmd);
	install_element(CONFIG_NODE, &car_white_list_write_cmd);
	install_element(CONFIG_NODE, &car_white_list_read_cmd);
	install_element(CONFIG_NODE, &reset_car_list_cmd);
    install_element(CONFIG_NODE, &empty_car_list_cmd);
	
	install_element(CONFIG_NODE, &set_hash_aging_time_cmd);		
    install_element(CONFIG_NODE, &set_hash_update_time_cmd);	
	
	install_element(CONFIG_NODE, &show_hash_capacity_cmd);

	install_element(CONFIG_NODE, &show_fpga_systerm_register_cmd);
    install_element(CONFIG_NODE, &show_fpga_reg_cmd);
	install_element(CONFIG_NODE, &config_system_control_reg_working_cmd);
    install_element(CONFIG_NODE, &config_system_control_reg_QoS_cmd);
	install_element(CONFIG_NODE, &config_system_control_reg_mode_cmd);
	install_element(CONFIG_NODE, &config_system_control_reg_wan_tag_type_cmd);
	install_element(CONFIG_NODE, &config_system_control_reg_car_update_cfg_cmd);
	install_element(CONFIG_NODE, &config_system_control_reg_trunk_tag_type_cmd);
	install_element(CONFIG_NODE, &config_system_control_reg_car_linkup_cmd);
	install_element(CONFIG_NODE, &config_system_control_reg_car_linkdown_cmd);
	install_element(CONFIG_NODE, &show_fpga_ddr_qdr_test_cmd);
	install_element(CONFIG_NODE, &show_fpga_hash_valid_cmd);
	install_element(HIDDENDEBUG_NODE, &set_cpld_reg_cmd);
	install_element(HIDDENDEBUG_NODE, &read_cpld_reg_cmd);
	install_element(CONFIG_NODE, &set_fpga_reset_cmd);
	install_element(CONFIG_NODE, &write_fpga_reg_single_cmd);
	install_element(CONFIG_NODE, &show_wan_if_cmd);
}

#endif


