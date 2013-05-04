

#ifndef _DCLI_FPGA_H
#define _DCLI_FPGA_H

#define FPGA_BM_FILE_PATH	"/dev/bm0"
#define SEM_SLOT_COUNT_PATH         "/dbm/product/slotcount"

#define HASH_ENTRY_NUM (2*1024*1024)
#define CAR_ENTRY_NUM (128*1024)

#define BM_IOC_MAGIC 0xEC 
#define BM_IOC_FPGA_WRITE 				_IOWR(BM_IOC_MAGIC,20,fpga_file) //FPGA online burning
#define BM_IOC_G_  				_IOWR(BM_IOC_MAGIC,1,bm_op_args) // read values
#define BM_IOC_X_ 				_IOWR(BM_IOC_MAGIC,2,bm_op_args) // write and readout wrote value
#define BM_IOC_FPGA_W 				_IOWR(BM_IOC_MAGIC,24,bm_op_args)//write fpga reg no delay


typedef struct
{
    unsigned char       arEther[6];
}ETHERADDR;

typedef struct hash_param_s
{
    unsigned int dip;
	unsigned int sip;
	unsigned short dport;
	unsigned short sport;
	unsigned short protocol;
}hash_param_t;
/*start for read car table counter */
struct car_table_s{

	unsigned long long car_id;
	unsigned long long reload;
	unsigned long long linkup;
	unsigned long long car_valid;
	unsigned long long usr_ip;
    unsigned long long credit;
	unsigned long long byte_drop_count;  
	unsigned long long byte_set_count;
	unsigned long long package_set_count;
	unsigned long long package_drop_count;
};

#define CAR_TABLE_CAR_VALID_REG	(0x800000001d070222ull)
/* end  */
#define FPGA_REG_BASE (0x800000001d070000ull)
#define INDIRECT_ACCESS_CONTROL_REG (0x800000001d070040ull)
#define INDIRECT_ACCESS_ADDRESS_REG_L (0x800000001d070042ull)
#define INDIRECT_ACCESS_ADDRESS_REG_H (0x800000001d070044ull)
#define INDIRECT_ACCESS_STATE_REG (0x800000001d070046ull)
#define Data_Temp_Stor_Base_Addr  (0x800000001d070200ull)

#define READY_REG_VALUE (0x1)

#define HASH_TABLE_1_VALUE (0x10)
#define HASH_TABLE_2_VALUE (0x20)
#define CAR_TABLE_VALUE (0x30)
#define CAM_CORE_VALUE (0x40)
#define CAM_TABLE_VALUE (0x50)

#define	HASH_TABLE_ID_REG_0         (0x800000001d070200ull)
#define	HASH_TABLE_ID_REG_1         (0x800000001d070202ull)
/*start for write car table counter */

/* cam core */
#define	CAM_CORE_REG_0         (0x800000001d070200ull) 	
#define	CAM_CORE_REG_1	       (0x800000001d070202ull)					
#define	CAM_CORE_REG_2         (0x800000001d070204ull) 			
#define	CAM_CORE_REG_3	       (0x800000001d070206ull) 			
#define	CAM_CORE_REG_4         (0x800000001d070208ull)  			
#define	CAM_CORE_REG_5	       (0x800000001d07020aull) 			
#define	CAM_CORE_REG_6	       (0x800000001d07020cull)
#define	CAM_CORE_REG_7         (0x800000001d07020eull)
/**/

/*Cycle_1*/
#define	SOURCE_IP_ADDRESS_REG_H         (0x800000001d070210ull) 	/* 8 */
#define	SOURCE_IP_ADDRESS_REG_L	        (0x800000001d070212ull)	/* 9 */				
#define	DESTINATION_IP_ADDRESS_REG_H    (0x800000001d070214ull) 	/* 10 */		
#define	DESTINATION_IP_ADDRESS_REG_L	(0x800000001d070216ull) 	/* 11 */		
#define	SOURCE_PORT_REG                 (0x800000001d070218ull)  	/* 12 */		
#define	DESTINATION_PORT_REG	        (0x800000001d07021aull) 	/* 13 */		
#define	PROTOCOL_REG	                (0x800000001d07021cull)	/* 14 */			
#define	HASHCTL_REG                     (0x800000001d07021eull)    /* 15 */

/*Cycle_2*/
#define DESTINATION_MAC_ADDRESS_REG_H	(0x800000001d070220ull) 	/* 16 */		
#define DESTINATION_MAC_ADDRESS_REG_M	(0x800000001d070222ull)	/* 17 */			
#define DESTINATION_MAC_ADDRESS_REG_L	(0x800000001d070224ull) 	/* 18 */		
#define SOURCE_MAC_ADDRESS_REG_H	    (0x800000001d070226ull) 	/* 19 */		
#define SOURCE_MAC_ADDRESS_REG_M	    (0x800000001d070228ull) 	/* 20 */		
#define SOURCE_MAC_ADDRESS_REG_L	    (0x800000001d07022aull)   	/* 21 */
#define VLAN_OUT_REG_H	                (0x800000001d07022cull)	/* 22 */		
#define VLAN_OUT_REG_L      	        (0x800000001d07022eull)    /* 23 */

/*Cycle_3*/
#define VLAN_IN_REG_H	        (0x800000001d070230ull) 	/* 24 */			
#define VLAN_IN_REG_L	        (0x800000001d070232ull)	/* 25 */
#define	ETHERNET_PROTOCOL_REG   (0x800000001d070234ull)    /* 26 */
#define	IP_TOS_REG              (0x800000001d070236ull)    /* 27 */
#define	IP_LEN_REG	            (0x800000001d070238ull) 	/* 28 */		
#define	IP_ID_REG	            (0x800000001d07023aull) 	/* 29 */		
#define	IP_OFFSET_REG	        (0x800000001d07023cull)	/* 30 */	
#define	IP_TTL_REG              (0x800000001d07023eull)    /* 31 */

/*Cycle_4*/
#define	IP_CHECKSUM_REG	                    (0x800000001d070240ull)    /* 32 */ 			
#define	TUNNEL_SOURCE_IP_ADDRESS_REG_H	    (0x800000001d070242ull) 	/* 33 */		
#define	TUNNEL_SOURCE_IP_ADDRESS_REG_L	    (0x800000001d070244ull) 	/* 34 */		
#define	TUNNEL_DESTINATION_IP_ADDRESS_REG_H	(0x800000001d070246ull)	/* 35 */		
#define	TUNNEL_DESTINATION_IP_ADDRESS_REG_L	(0x800000001d070248ull)	/* 36 */			
#define	TUNNEL_SOURCE_PORT_REG	            (0x800000001d07024aull) 	/* 37 */
#define	TUNNEL_DESTINATION_PORT_REG	        (0x800000001d07024cull)	/* 38 */			
#define	UDP_LEN_REG	                        (0x800000001d07024eull)	/* 39 */			

/*Cycle_5*/
#define	UDP_CHECKSUM_REG    	(0x800000001d070250ull) 	/* 40 */			
#define	CAPWAP_HEADER_REG_B0   	(0x800000001d070252ull)	/* 41 */				
#define	CAPWAP_HEADER_REG_B2   	(0x800000001d070254ull) 	/* 42 */		
#define	CAPWAP_HEADER_REG_B4   	(0x800000001d070256ull) 	/* 43 */		
#define	CAPWAP_HEADER_REG_B6   	(0x800000001d070258ull) 	/* 44 */		
#define	CAPWAP_HEADER_REG_B8   	(0x800000001d07025aull)	/* 45 */	
#define	CAPWAP_HEADER_REG_B10  	(0x800000001d07025cull)	/* 46 */			
#define	CAPWAP_HEADER_REG_B12   (0x800000001d07025eull)    /* 47 */

/*Cycle_6*/
#define	CAPWAP_HEADER_REG_B14   	(0x800000001d070260ull) 	/* 48 */			
#define	HEADER_802_11_REG_B0   		(0x800000001d070262ull)	/* 49 */				
#define	HEADER_802_11_REG_B2	    (0x800000001d070264ull) 	/* 50 */		
#define	HEADER_802_11_REG_B4    	(0x800000001d070266ull) 	/* 51 */		
#define	HEADER_802_11_REG_B6    	(0x800000001d070268ull) 	/* 52 */		
#define	HEADER_802_11_REG_B8    	(0x800000001d07026aull) 	/* 53 */		
#define	HEADER_802_11_REG_B10       (0x800000001d07026cull)	/* 54 */		
#define	HEADER_802_11_REG_B12       (0x800000001d07026eull)    /* 55 */

/*Cycle_7*/
#define	HEADER_802_11_REG_B14   	(0x800000001d070270ull) 	/* 56 */			
#define	HEADER_802_11_REG_B16       (0x800000001d070272ull)	/* 57 */				
#define	HEADER_802_11_REG_B18	    (0x800000001d070274ull)	/* 58 */		
#define	HEADER_802_11_REG_B20	    (0x800000001d070276ull) 	/* 59 */		
#define	HEADER_802_11_REG_B22	    (0x800000001d070278ull)	/* 60 */		
#define	HEADER_802_11_REG_B24	    (0x800000001d07027aull) 	/* 61 */		
#define	HEADER_802_11_REG_B26	    (0x800000001d07027cull)	/* 62 */			
#define	CAPWAP_HEADER_PROTOCOL_REG	(0x800000001d07027eull)    /* 63 */
/* end */

/*start for read hash table counter */
struct hash_cam_table_s 
{
	struct src_s 
	{
		unsigned long long sip;
		unsigned long long sport;
		unsigned long long smac;
		unsigned long long tunnel_sip;
		unsigned long long tunnel_sport;

	} src;
	struct dst_s 
	{
		unsigned long long dip;
		unsigned long long dport;
		unsigned long long dmac;
		unsigned long long tunnel_dip;
		unsigned long long tunnel_dport;

	} dst;
    unsigned long long protocol;
	
	unsigned long long setup_time;  /* table creation time */
	unsigned long long hash_ctl;
	unsigned long long vlan_out;
	unsigned long long vlan_in;
	
	unsigned long long ethernet_protocol;
	unsigned long long ip_tos;
	unsigned long long ip_len;
	unsigned long long ip_identification;
	unsigned long long ip_offset;
	unsigned long long ip_ttl;
	unsigned long long ip_protocol;
    unsigned long long ip_checksum;
	
	unsigned long long udp_len;
    unsigned long long udp_checksum;	
    unsigned long long capwap_header_b0;
	unsigned long long capwap_header_b2;
	unsigned long long capwap_header_b4;
	unsigned long long capwap_header_b6;
	unsigned long long capwap_header_b8;
	unsigned long long capwap_header_b10;
	unsigned long long capwap_header_b12;
	unsigned long long capwap_header_b14;
	
    unsigned long long header_802_11_b0;
	unsigned long long header_802_11_b2;
	unsigned long long header_802_11_b4;
	unsigned long long header_802_11_b6;
	unsigned long long header_802_11_b8;
	unsigned long long header_802_11_b10;
	unsigned long long header_802_11_b12;
	unsigned long long header_802_11_b14;
	unsigned long long header_802_11_b16;
	unsigned long long header_802_11_b18;
	unsigned long long header_802_11_b20;
	unsigned long long header_802_11_b22;
	unsigned long long header_802_11_b24;
	unsigned long long header_802_11_b26;
	unsigned long long capwap_protocol;
};


/*end*/

/*for show port counter */
#define FPGA_LOCK_CLEAR_REG  (0x800000001d070016)
#define Mib_Xaui_Reg_Base_Addr  (0x800000001d070100)

struct xaui_port_counter_s {
	struct tx_s {
		unsigned long long package_num;
		unsigned long long bytenum;
		unsigned long long multicast_package_num;
		unsigned long long broadcast_package_num;
		unsigned long long pause_package_num;
	} tx;
	struct rx_s {
		unsigned long long good_package_num;
		unsigned long long good_byte_num;
		unsigned long long bad_package_num;
		unsigned long long fcserr_package_num;
		unsigned long long multicast_package_num;
		unsigned long long broadcast_package_num;
		unsigned long long pause_package_num;
		unsigned long long drop_num;
		unsigned long long ttl_drop_num;
		unsigned long long car_drop_num;
		unsigned long long hash_col_num;
		unsigned long long to_cpu_num;
	} rx;
    //unsigned long long hash_aging_num;
    //unsigned long long hash_update_num;
    //unsigned long long hash_learn_num;

};

typedef struct bm_op_args_s {
	unsigned long long op_addr;	/*FPGA register address*/
	unsigned long long op_value;  // ignore on read in arg, fill read value on write return value
	unsigned short op_byteorder; // destination byte order. default is bigendiana.
	unsigned short op_len;
	unsigned short op_ret; // 0 for success, other value indicate different failure.
} bm_op_args;

/*end*/
typedef struct fpga_file
{
	char fpga_bin_name[256];		//FPGA burning filename
	int result;
}fpga_file;

void dcli_fpga_init(void);

#endif

