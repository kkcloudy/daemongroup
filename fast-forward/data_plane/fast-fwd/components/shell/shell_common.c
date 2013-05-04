#include "cvmx-config.h"
#include "executive-config.h"

#include "cvmx.h"
#include "cvmx-pip.h"
#include "cvmx-wqe.h"
#include "cvmx-fau.h"
#include "cvmx-fpa.h"
#include "cvmx-pow.h"
#include "cvmx-malloc.h"
#include "cvmx-atomic.h"

#include <stdio.h>
#include <string.h>
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "uart.h"
#include "shell.h"
#ifdef SDK_VERSION_2_2
#include "fastfwd-common-defs.h"
#include "fastfwd-common-fpa.h"
#else
#include "cvm-common-defs.h"
#include "cvm-common-fpa.h"
#endif
#include "autelan_product_info.h"
#include "acl.h"
#include "cvmx-csr-db.h"
#include "car.h"

extern CVMX_SHARED uint8_t *base_mac_table;
extern CVMX_SHARED uint32_t base_mac_size;



CVMX_SHARED five_tuple_t gl_five_tuple; /* for cmd: acl insert any any any any any action_type */

extern CVMX_SHARED product_info_t product_info;
extern CVMX_SHARED port_info_t pip_port_info[CVMX_PIP_NUM_INPUT_PORTS]; /*IPD/PKO port to index*/
extern int uart_index;
extern struct cmd 	cmd_list[MAX_CMDS];
extern CVMX_SHARED rule_cnt_info_t rule_cnt_info;   // add by zhaohan
extern CVMX_SHARED	int flow_sequence_enable;
extern CVMX_SHARED uint32_t acl_aging_enable;
extern CVMX_SHARED int cvm_ip_icmp_enable;
extern CVMX_SHARED int cvm_ip_only_enable; /*Only fot the evaluation test*/
extern CVMX_SHARED int cvm_car_enable;
extern CVMX_SHARED int cvm_passthough_enable;
extern CVMX_SHARED uint16_t passthough_vlan_id;
extern CVMX_SHARED short passthough_port;


extern uint32_t set_tag_flag;	/*Prevent deadlocks*/
extern void cvm_common_dump_pko(void);  /*add by chengjing*/
extern void cvm_common_read_csr(uint64_t addr);  /*add by chengjing*/
extern void cvm_common_write_csr(uint64_t addr, uint64_t val); /*add by chengjing*/


typedef struct global_various_s{
	uint64_t car_rate_scaled;
	uint64_t car_rate_pps_scaled;	
	uint64_t cvm_rate_limit_drop_counter;
	uint64_t cvm_rate_limit_pass_counter;
	int		 cvm_rate_limit_enabled;
	uint32_t last_valid_index;
	uint64_t unused[11];
} global_various_t;


int cvm_set_debug_coremask(char *mask)
{
	uint64_t ilevel;
	ilevel = strtol(mask, NULL ,16);
	core_mask = ilevel;
	return 0;
}

void show_port_statistics(uint16_t port_num, uint16_t flag)
{
	cvmx_pip_port_status_t status;
	cvmx_pip_get_port_status(port_num, flag, &status);

	printf("Port PIP %d statistics:  \n", port_num);

	printf("Inbound octets marked to be dropped by the IPD QOS widget per port =%d\r\n", status.dropped_octets);
	printf("Inbound packets marked to be dropped by the IPD QOS widget per port  =%d\r\n", status.dropped_packets);
	printf("RAWFULL + RAWSCH Packets without an L1/L2 error received by PIP per port =%d\r\n", status.pci_raw_packets);
	printf("Number of octets processed by PIP = %d\r\n", status.octets);
	printf("Number of packets processed by PIP  =%d\r\n", status.packets);
	printf("Number of identified L2 multicast packets =%d\r\n", status.multicast_packets);
	printf("Number of identified L2 broadcast packets =%d\r\n", status.broadcast_packets);
	printf("Number of 64B packets = %d\r\n", status.len_64_packets);
	printf("Number of 65-127B packets = %d\r\n", status.len_65_127_packets);
	printf("Number of 128-255B packets = %d\r\n", status.len_128_255_packets);
	printf("Number of 256-511B packets = %d\r\n", status.len_256_511_packets);
	printf("Number of 512-1023B packets = %d\r\n", status.len_512_1023_packets);
	printf("Number of 1024-1518B packets = %d\r\n", status.len_1024_1518_packets);
	printf("Number of 1519-max packets = %d\r\n", status.len_1519_max_packets);
	printf("Number of packets with FCS or Align opcode errors = %d\r\n", status.fcs_align_err_packets);
	printf("Number of packets with length < min  =%d\r\n", status.runt_packets);
	printf("Number of packets with length < min and FCS error =%d\r\n", status.runt_crc_packets);
	printf("Number of packets with length > max  =%d\r\n", status.oversize_packets);
	printf("Number of packets with length > max and FCS error  =%d\r\n ", status.oversize_crc_packets);
	printf("Number of packets without GMX/SPX/PCI errors received by PIP =%d\r\n", status.inb_packets);
	printf("Total number of octets from all packets received by PIP =%lu\r\n", status.inb_octets);
	printf("Number of packets with GMX/SPX/PCI errors received by PIP = %d\r\n", status.inb_errors);
}

void clear_pip_statistics(uint16_t index)
{	
	cvmx_write_csr(CVMX_PIP_STAT0_PRTX(index), 0);
	cvmx_write_csr(CVMX_PIP_STAT1_PRTX(index), 0);
	cvmx_write_csr(CVMX_PIP_STAT2_PRTX(index), 0);
	cvmx_write_csr(CVMX_PIP_STAT3_PRTX(index), 0);
	cvmx_write_csr(CVMX_PIP_STAT4_PRTX(index), 0);
	cvmx_write_csr(CVMX_PIP_STAT5_PRTX(index), 0);
	cvmx_write_csr(CVMX_PIP_STAT6_PRTX(index), 0);
	cvmx_write_csr(CVMX_PIP_STAT7_PRTX(index), 0);
	cvmx_write_csr(CVMX_PIP_STAT8_PRTX(index), 0);
	cvmx_write_csr(CVMX_PIP_STAT9_PRTX(index), 0);

	return ;
}

void clear_pko_port_statistics(uint16_t index)
{        
	int  localPort, pko_interface;

	pko_interface= cvmx_helper_get_interface_num (index);
	localPort = cvmx_helper_get_interface_index_num (index);

	cvmx_write_csr(CVMX_GMXX_TXX_STAT0(localPort,pko_interface), 0);
	cvmx_write_csr(CVMX_GMXX_TXX_STAT1(localPort,pko_interface), 0);
	cvmx_write_csr(CVMX_GMXX_TXX_STAT2(localPort,pko_interface), 0);
	cvmx_write_csr(CVMX_GMXX_TXX_STAT3(localPort,pko_interface), 0);
	cvmx_write_csr(CVMX_GMXX_TXX_STAT4(localPort,pko_interface), 0);
	cvmx_write_csr(CVMX_GMXX_TXX_STAT5(localPort,pko_interface), 0);
	cvmx_write_csr(CVMX_GMXX_TXX_STAT6(localPort,pko_interface), 0);
	cvmx_write_csr(CVMX_GMXX_TXX_STAT7(localPort,pko_interface), 0);
	cvmx_write_csr(CVMX_GMXX_TXX_STAT8(localPort,pko_interface), 0);
	cvmx_write_csr(CVMX_GMXX_TXX_STAT9(localPort,pko_interface), 0);

	return ;
}

void clear_fau_64()
{
	uint64_t fau_addr = 0;
	int32_t i = (CVM_FAU_PKO_ERRORS - CVMX_FAU_REG_64_START) >> 3;

	for(;fau_addr < CVMX_FAU_REG_64_END; i++)
	{
		fau_addr = CVMX_FAU_REG_64_ADDR(i);
		cvmx_fau_atomic_write64(fau_addr, 0);
	}
}

int32_t cmd_reboot(int argc, char *argv[])
{
	char ch = 0;
	uart_write_string(uart_index,"Are you sure reboot the system? [y|n]");

	ch = uart_read_byte_wait(uart_index);
	if (ch=='y' || ch=='Y')
		cvmx_reset_octeon();

	return CMD_EXEC_SUCCESS; 
}


static int32_t cmd_clear(int argc, char *argv[])
{
	uint16_t pip_port = 0;

	if (argc < 3 )
	{	
		printf("clear pip counter {$port} \n");
		printf("clear pko port {$port} \n");
		printf("clear fau 64 \n");
		return CMD_ERR_TOO_FEW_ARGC;
	}

	if (argv[1] ==NULL)
		return CMD_ERR_NOT_MATCH;

	if (strcmp(argv[1] , "pip") == 0)
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;

		if (strcmp(argv[2] , "counter") == 0)
		{			
			if (argv[3] ==NULL)
				return CMD_ERR_NOT_MATCH;

			pip_port = atoi(argv[3]);
			if(pip_port < CVMX_PIP_NUM_INPUT_PORTS)
				clear_pip_statistics(pip_port);		
			else
				return CMD_ERR_NOT_MATCH;	
		}
	}
	else if (0 == strcmp(argv[1], "pko"))
	{
		if(0 == strcmp(argv[2], "port"))
		{
			if(NULL != argv[3])
			{
				clear_pko_port_statistics(atoi(argv[3]));
			}
		}
	}
	else if (0 == strcmp(argv[1], "fau"))
	{
		if(0 == strcmp(argv[2], "64"))
		{
			clear_fau_64();
			cvmx_atomic_set64(&rule_cnt_info.fwd_eth_cnt, 0);
			cvmx_atomic_set64(&rule_cnt_info.fwd_cw_cnt, 0);
			cvmx_atomic_set64(&rule_cnt_info.fwd_cw_802_3_cnt, 0);
		}
	}
	else 
		return CMD_ERR_NOT_MATCH;			

	return CMD_EXEC_SUCCESS;  
}


void set_tag_type(cvmx_pow_tag_type_t tag_type)                          /*add by chengjing*/
{
	int port;
	int num_ports = 32;

	if(tag_type == CVMX_POW_TAG_TYPE_NULL)
	{
		set_tag_flag = 1;
	}
	else
	{
		set_tag_flag = 0;
	}
	for (port = 0; port < num_ports; port++)
	{
		cvmx_pip_port_tag_cfg_t tag_config;
		tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port));
		tag_config.s.tcp6_tag_type = tag_type; 
		tag_config.s.tcp4_tag_type = tag_type;
		tag_config.s.ip6_tag_type = tag_type;
		tag_config.s.ip4_tag_type = tag_type;
		tag_config.s.non_tag_type = tag_type;		
		cvmx_write_csr(CVMX_PIP_PRT_TAGX(port), tag_config.u64);
	}
}
void get_tag_type(void)
{
	int port = 1;
	cvmx_pip_port_tag_cfg_t tag_config;
	cvmx_pow_tag_type_t tag_type;
	tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port));
	tag_type = tag_config.s.ip4_tag_type;
	if(CVMX_POW_TAG_TYPE_ORDERED == tag_type)
	{
		printf("tag type : ORDERED \n");
	}
	else if(CVMX_POW_TAG_TYPE_ATOMIC == tag_type)
	{
		printf("tag type : ATOMIC \n");
	}
	else if(CVMX_POW_TAG_TYPE_NULL == tag_type)
	{
		printf("tag type : NULL \n");
	}

}

extern CVMX_SHARED int32_t fwd_debug_log_enable;

#ifdef OCTEON_DEBUG_LEVEL
static int32_t cmd_set(int argc, char *argv[])
{
	int64_t level = 0;

	if(argc < 3)
	{		
	    printf("set debug {enable | disable} \n");
		printf("set debug $level \n");
		printf("set coremask $hex \n");
		printf("set module [shell|acl|main] [enable|disable] \r\n");
		printf("set aging time $sec \n");
		printf("set tag type [ordered|atomic|null] \n");
		printf("set seq [enable | disable] \n");
		printf("set aging [enable | disable] \n");
		printf("set frame gap [short | normal] \n");
		printf("set icmp [enable | disable]\n");
		printf("set one-tuple [enable | disable]\n");
		printf("set car [enable | disable]\n");
		printf("set car speed {$car-index (0~63)} {$meter-template (0~7)}\n");
		printf("set meter $meter-index(0~7) $cir(kbps) $cbs(bytes)\n");
		printf("set passthough [enable | disable | vlan | port]\n");
		return CMD_ERR_TOO_FEW_ARGC;
	}

	if(strcmp(argv[1] , "debug") == 0)
	{
	    if(strcmp(argv[2] , "enable") == 0)
	    {
            fwd_debug_log_enable = FUNC_ENABLE;
	    }
	    else if(strcmp(argv[2] , "disable") == 0)
	    {
            fwd_debug_log_enable = FUNC_DISABLE;
	    }
	    else
	    {
    		level = atoi(argv[2]);
    		cvm_common_set_debug_level(level);		
		}
		return CMD_EXEC_SUCCESS;
	}

	if(strcmp(argv[1] , "coremask") == 0)
	{
		level = strtol(argv[2], NULL ,16);
		printf("\nDebug Core mask changed from 0x%lx to 0x%lx\n", core_mask, level);		
		core_mask = level;	
		return CMD_EXEC_SUCCESS;
	}

	if(strcmp(argv[1] , "module") == 0)
	{
		if(strcmp(argv[2] , "main") == 0)
		{
			if(strcmp(argv[3] , "enable") == 0)
			{
				module_print[FASTFWD_COMMON_MOUDLE_MAIN]=1;
				return CMD_EXEC_SUCCESS;
			}

			if(strcmp(argv[3] , "disable") == 0)
			{
				module_print[FASTFWD_COMMON_MOUDLE_MAIN]=0;
				return CMD_EXEC_SUCCESS;
			}
		}

		if(strcmp(argv[2] , "shell") == 0)
		{
			if(strcmp(argv[3] , "enable") == 0)
			{
				module_print[FASTFWD_COMMON_MOUDLE_SHELL]=1;
				return CMD_EXEC_SUCCESS;
			}

			if(strcmp(argv[3] , "disable") == 0)
			{
				module_print[FASTFWD_COMMON_MOUDLE_SHELL]=0;
				return CMD_EXEC_SUCCESS;
			}
		}	

		if(strcmp(argv[2] , "acl") == 0)
		{
			if(strcmp(argv[3] , "enable") == 0)
			{
				module_print[FASTFWD_COMMON_MOUDLE_FLOWTABLE]=1;
				return CMD_EXEC_SUCCESS;
			}

			if(strcmp(argv[3] , "disable") == 0)
			{
				module_print[FASTFWD_COMMON_MOUDLE_FLOWTABLE]=0;
				return CMD_EXEC_SUCCESS;
			}
		}
	}

	if(strcmp(argv[1] , "aging") == 0)          /*add by chengjing*/
	{
		if(strcmp(argv[2] , "time") == 0)
		{
			if(argc == 4)
			{
				uint32_t aging_timer = atoi(argv[3]);
				if(RETURN_ERROR == acl_set_aging_timer(aging_timer))
				{
					printf("aging time beyond the scope[1-0xffffffff], please reset value.\n");
					return RETURN_ERROR;
				}
				printf("set aging time is %d \n", acl_get_aging_timer());
				return CMD_EXEC_SUCCESS;
			}
		}   
	}
	if(strcmp(argv[1] , "tag") == 0)          /*add by chengjing*/
	{
		if(strcmp(argv[2] , "type") == 0)
		{
			if(argc != 4)
			{
				printf("set tag type [ordered|atomic|null] \n");            
				return CMD_ERR_TOO_FEW_ARGC;
			}
			if(strcmp(argv[3], "ordered") == 0)
			{
				set_tag_type(CVMX_POW_TAG_TYPE_ORDERED);
				return CMD_EXEC_SUCCESS;
			}
			else if(strcmp(argv[3] , "atomic") == 0)
			{
				set_tag_type(CVMX_POW_TAG_TYPE_ATOMIC);
				return CMD_EXEC_SUCCESS;
			}

			else if(strcmp(argv[3], "null") == 0)
			{
				set_tag_type(CVMX_POW_TAG_TYPE_NULL);
				return CMD_EXEC_SUCCESS;
			}
		}   
	}
	if(strcmp(argv[1] , "seq") == 0)
	{
		if(strcmp(argv[2] , "enable") == 0)
		{
			flow_sequence_enable = FUNC_ENABLE;
			return CMD_EXEC_SUCCESS;
		}
		if(strcmp(argv[2] , "disable") == 0)
		{
			flow_sequence_enable = FUNC_DISABLE;
			return CMD_EXEC_SUCCESS;
		}
	}
	if(strcmp(argv[1] , "aging") == 0)
	{
		if(strcmp(argv[2] , "enable") == 0)
		{
			acl_aging_enable = FUNC_ENABLE;
			return CMD_EXEC_SUCCESS;
		}
		if(strcmp(argv[2] , "disable") == 0)
		{
			acl_aging_enable = FUNC_DISABLE;
			return CMD_EXEC_SUCCESS;
		}
	}
	if(strcmp(argv[1] , "frame") == 0)
	{
		if(strcmp(argv[2] , "gap") == 0)
		{
			if(strcmp(argv[3] , "short") == 0)
			{
				cvm_common_write_csr(CVMX_GMXX_TX_IFG(0), 0x44);
				cvm_common_write_csr(CVMX_GMXX_TX_IFG(1), 0x44);
				return CMD_EXEC_SUCCESS;
			}
			if(strcmp(argv[3] , "normal") == 0)
			{
				cvm_common_write_csr(CVMX_GMXX_TX_IFG(0), 0x48);
				cvm_common_write_csr(CVMX_GMXX_TX_IFG(1), 0x48);
				return CMD_EXEC_SUCCESS;
			}
		}
	}
	if(strcmp(argv[1] , "icmp") == 0)
	{
		if(strcmp(argv[2] , "enable") == 0)
		{
			cvm_ip_icmp_enable = FUNC_ENABLE;
			return CMD_EXEC_SUCCESS;
		}
		else if(strcmp(argv[2] , "disable") == 0)
		{
			cvm_ip_icmp_enable = FUNC_DISABLE;
			return CMD_EXEC_SUCCESS;
		}
	}
	if(strcmp(argv[1] , "one-tuple") == 0)
	{
		if(strcmp(argv[2] , "enable") == 0)
		{
			if(cvm_ip_only_enable == FUNC_DISABLE)
			{
				acl_clear_rule();   /* clear all rules */
				cvm_ip_only_enable = FUNC_ENABLE;
			}
			return CMD_EXEC_SUCCESS;
		}
		else if(strcmp(argv[2] , "disable") == 0)
		{
			if(cvm_ip_only_enable == FUNC_ENABLE)
			{
				acl_clear_rule();   /* clear all rules */
				cvm_ip_only_enable = FUNC_DISABLE;
			}
			return CMD_EXEC_SUCCESS;
		}
	}
	if(strcmp(argv[1] , "car") == 0)
	{
		if(strcmp(argv[2] , "enable") == 0)
		{
			cvm_car_enable = FUNC_ENABLE;
			return CMD_EXEC_SUCCESS;
		}
		else if(strcmp(argv[2] , "disable") == 0)
		{
			cvm_car_enable = FUNC_DISABLE;
			return CMD_EXEC_SUCCESS;
		}
		else if(strcmp(argv[2] , "speed") == 0)
		{
			if(argv[3] == NULL)
			{
				printf("set car speed {$car-index (0~63)} {$meter-template (0~7)}\n");
				return CMD_EXEC_SUCCESS;
			}
		}
	}
	if(strcmp(argv[1] , "meter") == 0)
	{
		if((argv[2] == NULL) || (argv[3] == NULL) || (argv[4] == NULL))
		{
			printf("set meter $meter-index(0~7) $cir(kbps) $cbs(bytes)\n");
			return CMD_EXEC_SUCCESS;
		}
		cvm_car_set_template(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
		return CMD_EXEC_SUCCESS;
	}
	if(strcmp(argv[1] , "passthough") == 0)
	{
		if(argv[2] == NULL)
		{
			printf("set passthough [enable | disable | vlan]\n");
			return CMD_EXEC_ERROR;
		}
		if(strcmp(argv[2] , "enable") == 0)
		{
			cvm_passthough_enable = FUNC_ENABLE;
			return CMD_EXEC_SUCCESS;
		}
		else if(strcmp(argv[2] , "disable") == 0)
		{
			cvm_passthough_enable = FUNC_DISABLE;
			return CMD_EXEC_SUCCESS;
		}
		else if(strcmp(argv[2] , "vlan") == 0)
		{
			uint16_t vlan_id = 0;
			if(argv[3] == NULL)
			{
				printf("please input vlan id 0~4095\n");
				return CMD_EXEC_ERROR;
			}
			vlan_id = atoi(argv[3]);
			if(vlan_id > 4095)
			{
				printf("please input vlan id 0~4095\n");
				return CMD_EXEC_ERROR;
			}
			passthough_vlan_id = vlan_id;
			return CMD_EXEC_SUCCESS;
		}
		else if(strcmp(argv[2] , "port") == 0)
		{
			short port = 0;
			if(argv[3] == NULL)
			{
				printf("please input fwd port\n");
				return CMD_EXEC_ERROR;
			}
			port = atoi(argv[3]);
			if(port < 0)
			{
				printf("please input fwd port\n");
				return CMD_EXEC_ERROR;
			}
			passthough_port = port;
			return CMD_EXEC_SUCCESS;
		}

	}



	printf("set debug $level \n");
	printf("set coremask $hex \n");
	printf("set module [shell|acl|main] [enable|disable] \r\n");
	printf("set aging time $sec \n");
	printf("set tag type [ordered|atomic|null] \n");
	printf("set seq [enable | disable] \n");
	printf("set aging [enable | disable] \n");
	printf("set frame gap [short | normal] \n");
	printf("set icmp [enable | disable]\n");
	printf("set one-tuple [enable | disable]\n");
	printf("set car [enable | disable]\n");
	printf("set car speed {$car-index (0~63)} {$meter-template (0~7)}\n");
	printf("set meter $meter-index(0~7) $cir(kbps) $cbs(bytes)\n");
	printf("set passthough [enable | disable | vlan | port]\n");

	return CMD_ERR_NOT_MATCH;
}
#endif

/*help handler*/
static int32_t cmd_help(int argc, char *argv[])
{
	int i;

	if(argc == 1)
	{

		for(i = 0; i < MAX_CMDS; i++)
		{
			if (cmd_list[i].name[0])		
			{			
				uart_write_string(uart_index, cmd_list[i].name);
				uart_write_string(uart_index, "\t");
				uart_write_string(uart_index, cmd_list[i].help);
				uart_write_string(uart_index, "\r\n");
			}		
		}		
	}
	else
	{
		for(i = 0; (i < MAX_CMDS) && strcmp(cmd_list[i].name, argv[1]); i++)
			;

		if(i == MAX_CMDS)
		{
			uart_write_string(uart_index, "Invalid command!\r\n");
			return CMD_EXEC_ERROR;
		}
		else	
		{	
			uart_write_string(uart_index, cmd_list[i].usage);
			uart_write_string(uart_index, "\r\n");
			uart_write_string(uart_index, "\t-");
			uart_write_string(uart_index, cmd_list[i].help);
			uart_write_string(uart_index, "\r\n");
		}
	}		

	return CMD_EXEC_SUCCESS;
}


static void show_fau_dump_64()
{

	//  printf("FAU registers for the position in PKO command buffers = %lu\r\n", cvmx_fau_fetch_and_add64(CVMX_FAU_REG_OQ_ADDR_INDEX, 0));

	printf("---------------------------------------------------\n");
	printf("CVM_FAU_SE_COEXIST_FLAG = 0x%08lx\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_SE_COEXIST_FLAG, 0));
	printf("CVM_FAU_INIT_STATE_FLAG = 0x%08lx\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_INIT_STATE_FLAG, 0));

	printf("---------------------------------------------------\n");
	printf("64-bit counter used for total received works counter (packets and fccp) = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_RECV_TOTAL_WORKS, 0));
	printf("64-bit counter used for total received fccp packets counter = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_RECV_FCCP_PACKETS, 0));
	printf("64-bit counter used for total ethernet input bytes = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_INPUT_BYTES, 0));
	printf("64-bit counter used for total ethernet input packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_INPUT_PACKETS, 0));

	printf("---------------------------------------------------\n");
	printf("64-bit counter used for total ethernet input noneip packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_NONIP_PACKETS, 0));
	printf("64-bit counter used for total ethernet input eth pppoe noneip packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_ETH_PPPOE_NONIP_PACKETS, 0));
	printf("64-bit counter used for total ethernet input capwap pppoe noneip packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_CAPWAP_PPPOE_NONIP_PACKETS, 0));
	printf("64-bit counter used for total ethernet input error packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_ERROR_PACKETS, 0));
	printf("64-bit counter used for total mcast packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_MCAST_PACKETS, 0));
	printf("64-bit counter used for total rpa packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_RPA_PACKETS, 0));
	printf("64-bit counter used for total rpa to linux packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_RPA_TOLINUX_PACKETS, 0));
	printf("64-bit counter used for total tipc packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_TIPC_PACKETS, 0));
	printf("64-bit counter used for total large eth->capwap packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_LARGE_ETH2CW_PACKET, 0));
	printf("64-bit counter used for total large xxx->cw_rpa packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_LARGE_CW_RPA_FWD_PACKET, 0));
    printf("64-bit counter used for total large xxx->cw8023_rpa packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_LARGE_CW8023_RPA_FWD_PACKET, 0));

	printf("64-bit counter used for total ethernet input frag packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_FRAGIP_PACKETS, 0));
	printf("64-bit counter used for short IP packets rcvd = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_IP_SHORT_PACKETS, 0));
	printf("64-bit counter used for IP packets with bad hdr len = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_IP_BAD_HDR_LEN, 0));
	printf("64-bit counter used for IP packets with bad len = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_IP_BAD_LEN, 0));
	printf("64-bit counter used for IP packets with bad version = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_IP_BAD_VERSION, 0));
	printf("64-bit counter used for IP packets with SKIP addr = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_IP_SKIP_ADDR, 0));
	printf("64-bit counter used for ip packets with proto error = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_IP_PROTO_ERROR, 0));
    printf("64-bit counter used for special tcp hearder = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_SPE_TCP_HDR, 0));
    printf("64-bit counter used for special tcp hearder over capwap = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_CW_SPE_TCP_HDR, 0));
	printf("64-bit counter used for udp dport=0 packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_UDP_BAD_DPORT, 0));
	printf("64-bit counter used for udp packets with len error = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_UDP_BAD_LEN, 0));
	printf("64-bit counter used for udp packets that trap to Linux = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_UDP_TO_LINUX, 0));
	printf("64-bit counter used for total flow lookup failed counter = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_FLOW_LOOKUP_ERROR, 0));
	printf("64-bit counter used for ICMP packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_IP_ICMP, 0));
	printf("64-bit counter used for Capwap ICMP packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_CAPWAP_ICMP, 0));
	printf("64-bit counter used for total ethernet output 802.1qpackets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS_8021Q, 0));
	printf("64-bit counter used for total ethernet output qinq packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS_QINQ, 0));
	printf("64-bit counter used for total ethernet output eth pppoe packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS_ETH_PPPOE, 0));
	printf("64-bit counter used for total ethernet output capwap pppoe packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS_CAPWAP_PPPOE, 0));
	printf("64-bit counter used for total capwap 802.11 decap error = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_CW802_11_DECAP_ERROR, 0));
	printf("64-bit counter used for total capwap 802.3 decap error = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_CW802_3_DECAP_ERROR, 0));
	printf("---------------------------------------------------\n");
	printf("64-bit counter used for total acl lookup counter = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_TOTAL_ACL_LOOKUP, 0));
	printf("64-bit counter used for ACL HIT packets number = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_FLOWTABLE_HIT_PACKETS, 0));
	printf("64-bit counter used for total acl setup and regist packets counter = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ACL_REG, 0));     
	printf("---------------------------------------------------\n");    
	printf("64-bit counter used for total ethernet drop packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_DROP_PACKETS, 0));
	printf("64-bit counter used for total ethernet to linux bytes = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_TO_LINUX_BYTES, 0));
	printf("64-bit counter used for total ethernet to linux packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_TO_LINUX_PACKETS, 0));
	printf("64-bit counter used for total ethernet output bytes = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_BYTES, 0));
	printf("64-bit counter used for total ethernet output packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS, 0));
	printf("64-bit counter used for total ethernet output error packets = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_PKO_ERRORS, 0));
	printf("---------------------------------------------------\n");
	printf("64-bit counter used for total alloc rule fail counter = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_ALLOC_RULE_FAIL, 0));
	printf("64-bit counter used for reach max rule entries = %lu\r\n", cvmx_fau_fetch_and_add64(CVM_FAU_MAX_RULE_ENTRIES, 0));
	printf("---------------------------------------------------\n");
}

static void show_fau_dump_32()
{
	printf("FPA out of buffer counters = %d\r\n", cvmx_fau_fetch_and_add32(CVM_FAU_REG_FPA_OOB_COUNT, 0));
	printf("pool 0 use count = %d\r\n", cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_0_USE_COUNT, 0));
	printf("pool 1 use count = %d\r\n", cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_1_USE_COUNT, 0));
	printf("pool 2 use count = %d\r\n", cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_2_USE_COUNT, 0));
	printf("pool 3 use count = %d\r\n", cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_3_USE_COUNT, 0));
	printf("pool 4 use count = %d\r\n", cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_4_USE_COUNT, 0));
	printf("pool 5 use count = %d\r\n", cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_5_USE_COUNT, 0));
	printf("pool 6 use count = %d\r\n", cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_6_USE_COUNT, 0));
	printf("pool 7 use count = %d\r\n", cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_7_USE_COUNT, 0));
}

void show_pko_port_statistics(uint16_t index)
{
	cvmx_gmxx_txx_stat0_t pkoStat0;
	cvmx_gmxx_txx_stat1_t pkoStat1;
	cvmx_gmxx_txx_stat2_t pkoStat2;
	cvmx_gmxx_txx_stat3_t pkoStat3;
	cvmx_gmxx_txx_stat4_t pkoStat4;
	cvmx_gmxx_txx_stat5_t pkoStat5;
	cvmx_gmxx_txx_stat6_t pkoStat6;
	cvmx_gmxx_txx_stat7_t pkoStat7;
	cvmx_gmxx_txx_stat8_t pkoStat8;
	cvmx_gmxx_txx_stat9_t pkoStat9;
	int  localPort, pko_interface;

	pko_interface= cvmx_helper_get_interface_num (index);
	localPort = cvmx_helper_get_interface_index_num (index);

	pkoStat0.u64= cvmx_read_csr(CVMX_GMXX_TXX_STAT0(localPort,pko_interface));
	pkoStat1.u64= cvmx_read_csr(CVMX_GMXX_TXX_STAT1(localPort,pko_interface));
	pkoStat2.u64= cvmx_read_csr(CVMX_GMXX_TXX_STAT2(localPort,pko_interface));
	pkoStat3.u64= cvmx_read_csr(CVMX_GMXX_TXX_STAT3(localPort,pko_interface));
	pkoStat4.u64= cvmx_read_csr(CVMX_GMXX_TXX_STAT4(localPort,pko_interface));
	pkoStat5.u64= cvmx_read_csr(CVMX_GMXX_TXX_STAT5(localPort,pko_interface));
	pkoStat6.u64= cvmx_read_csr(CVMX_GMXX_TXX_STAT6(localPort,pko_interface));
	pkoStat7.u64= cvmx_read_csr(CVMX_GMXX_TXX_STAT7(localPort,pko_interface));
	pkoStat8.u64= cvmx_read_csr(CVMX_GMXX_TXX_STAT8(localPort,pko_interface));
	pkoStat9.u64= cvmx_read_csr(CVMX_GMXX_TXX_STAT9(localPort,pko_interface));

	printf("OCTEON PKO Port %d statistics:\r\n", index);
	printf("Number of packets dropped (never successfully sent) due to excessive deferal = %ld\r\n",(uint64_t)pkoStat0.s.xsdef);
	printf("Number of packets dropped (never successfully sent) due to excessive collision. Defined by GMX_TX_COL_ATTEMPT[LIMIT]= %ld\r\n",(uint64_t)pkoStat0.s.xscol);
	printf("Number of packets sent with a single collision=%ld \r\n",(uint64_t)pkoStat1.s.scol);
	printf("Number of packets sent with multiple collisions but <GMX_TX_COL_ATTEMPT[LIMIT]=%ld \r\n",(uint64_t)pkoStat1.s.mcol);
	printf("Number of total octets sent on the interface.Does not count octets from frames that were truncated due to collisions in halfdup mode=%ld \r\n",(uint64_t)pkoStat2.s.octs);
	printf("Number of total frames sent on the interface.Does not count frames that were truncated due to collisions in halfdup mode=%ld \r\n",(uint64_t)pkoStat3.s.pkts);
	printf("Number of packets sent with an octet count of 64=%ld \r\n",(uint64_t)pkoStat4.s.hist1);
	printf("Number of packets sent with an octet count of < 64=%ld \r\n",(uint64_t)pkoStat4.s.hist0);
	printf("Number of packets sent with an octet count of 128 - 255=%ld \r\n",(uint64_t)pkoStat5.s.hist3);
	printf("Number of packets sent with an octet count of 65 - 127=%ld \r\n",(uint64_t)pkoStat5.s.hist2);
	printf("Number of packets sent with an octet count of 512 - 1023=%ld \r\n",(uint64_t)pkoStat6.s.hist5);
	printf("Number of packets sent with an octet count of 256 - 511=%ld \r\n",(uint64_t)pkoStat6.s.hist4);
	printf("Number of packets sent with an octet countof > 1518=%ld \r\n",(uint64_t)pkoStat7.s.hist7);
	printf("Number of packets sent with an octet count of 1024 - 1518=%ld \r\n",(uint64_t)pkoStat7.s.hist6);
	printf("Number of packets sent to multicast DMAC. Does not include BCST packets=%ld\r\n",(uint64_t)pkoStat8.s.mcst);
	printf("Number of packets sent to broadcast DMAC.Does not include MCST packets=%ld\r\n",(uint64_t)pkoStat8.s.bcst);
	printf("Number of underflow packets=%ld \r\n",(uint64_t)pkoStat9.s.undflw);
	printf("Number of Control packets (PAUSE flow control)generated by GMX.  It does not include controlpackets forwarded =%ld \r\n",(uint64_t)pkoStat9.s.ctl);
	printf("----------------------------------------------------------------\r\t");
}


void show_xaui_statistics()
{
	cvmx_agl_gmx_rxx_stats_pkts_t rx_stat0;
	cvmx_agl_gmx_rxx_stats_octs_t rx_stat1;
	cvmx_agl_gmx_rxx_stats_pkts_ctl_t rx_stat2;
	cvmx_agl_gmx_rxx_stats_octs_ctl_t rx_stat3;
	cvmx_agl_gmx_rxx_stats_pkts_dmac_t rx_stat4;
	cvmx_agl_gmx_rxx_stats_octs_dmac_t rx_stat5;
	cvmx_agl_gmx_rxx_stats_pkts_drp_t rx_stat6;
	cvmx_agl_gmx_rxx_stats_octs_drp_t rx_stat7;
	cvmx_agl_gmx_rxx_stats_pkts_bad_t rx_stat8;
	cvmx_agl_gmx_txx_stat2_t tx_stat2;
	cvmx_agl_gmx_txx_stat3_t tx_stat3;
	cvmx_agl_gmx_txx_stat4_t tx_stat4;
	cvmx_agl_gmx_txx_stat5_t tx_stat5;
	cvmx_agl_gmx_txx_stat6_t tx_stat6;
	cvmx_agl_gmx_txx_stat7_t tx_stat7;
	cvmx_agl_gmx_txx_stat8_t tx_stat8;
	cvmx_agl_gmx_txx_stat9_t tx_stat9;

	rx_stat0.u64 = cvmx_read_csr(CVMX_AGL_GMX_RXX_STATS_PKTS(CVMX_CSR_DB_TYPE_RSL));
	rx_stat1.u64 = cvmx_read_csr(CVMX_AGL_GMX_RXX_STATS_OCTS(CVMX_CSR_DB_TYPE_RSL));
	rx_stat2.u64 = cvmx_read_csr(CVMX_AGL_GMX_RXX_STATS_PKTS_CTL(CVMX_CSR_DB_TYPE_RSL));
	rx_stat3.u64 = cvmx_read_csr(CVMX_AGL_GMX_RXX_STATS_OCTS_CTL(CVMX_CSR_DB_TYPE_RSL));
	rx_stat4.u64 = cvmx_read_csr(CVMX_AGL_GMX_RXX_STATS_PKTS_DMAC(CVMX_CSR_DB_TYPE_RSL));
	rx_stat5.u64 = cvmx_read_csr(CVMX_AGL_GMX_RXX_STATS_OCTS_DMAC(CVMX_CSR_DB_TYPE_RSL));
	rx_stat6.u64 = cvmx_read_csr(CVMX_AGL_GMX_RXX_STATS_PKTS_DRP(CVMX_CSR_DB_TYPE_RSL));
	rx_stat7.u64 = cvmx_read_csr(CVMX_AGL_GMX_RXX_STATS_OCTS_DRP(CVMX_CSR_DB_TYPE_RSL));
	rx_stat8.u64 = cvmx_read_csr(CVMX_AGL_GMX_RXX_STATS_PKTS_BAD(CVMX_CSR_DB_TYPE_RSL));

	tx_stat2.u64 = cvmx_read_csr(CVMX_AGL_GMX_TXX_STAT2(CVMX_CSR_DB_TYPE_RSL));
	tx_stat3.u64 = cvmx_read_csr(CVMX_AGL_GMX_TXX_STAT3(CVMX_CSR_DB_TYPE_RSL));
	tx_stat4.u64 = cvmx_read_csr(CVMX_AGL_GMX_TXX_STAT4(CVMX_CSR_DB_TYPE_RSL));
	tx_stat5.u64 = cvmx_read_csr(CVMX_AGL_GMX_TXX_STAT5(CVMX_CSR_DB_TYPE_RSL));
	tx_stat6.u64 = cvmx_read_csr(CVMX_AGL_GMX_TXX_STAT6(CVMX_CSR_DB_TYPE_RSL));
	tx_stat7.u64 = cvmx_read_csr(CVMX_AGL_GMX_TXX_STAT7(CVMX_CSR_DB_TYPE_RSL));
	tx_stat8.u64 = cvmx_read_csr(CVMX_AGL_GMX_TXX_STAT8(CVMX_CSR_DB_TYPE_RSL));
	tx_stat9.u64 = cvmx_read_csr(CVMX_AGL_GMX_TXX_STAT9(CVMX_CSR_DB_TYPE_RSL));

	printf("OCTEON GMX_RX0_STATS_PKTS(count of all good packets): %u \r\n", rx_stat0.s.cnt);
	printf("OCTEON GMX_RX0_STATS_OCTS(count of all bytes from good packets): %lu \r\n", (uint64_t)rx_stat1.s.cnt);
	printf("OCTEON GMX_RX0_STATS_PKTS_CTL(count of all control/pause packets): %u \r\n", rx_stat2.s.cnt);
	printf("OCTEON GMX_RX0_STATS_OCTS_CTL(count of all bytes from control/pause packets): %lu \r\n", (uint64_t)rx_stat3.s.cnt);
	printf("OCTEON GMX_RX0_STATS_PKTS_DMAC(count of all DMAC filtered packets): %u \r\n", rx_stat4.s.cnt);
	printf("OCTEON GMX_RX0_STATS_OCTS_DMAC(count of all bytes from DMAC filtered packets): %lu \r\n", (uint64_t)rx_stat5.s.cnt);
	printf("OCTEON GMX_RX0_STATS_PKTS_DRP(count of all packets dropped due to RX FIFO full): %u \r\n", rx_stat6.s.cnt);         
	printf("OCTEON GMX_RX0_STATS_OCTS_DRP(count of all bytes from packets dropped due to RX FIFO full): %lu \r\n", (uint64_t)rx_stat7.s.cnt);
	printf("OCTEON GMX_RX0_STATS_PKTS_BAD(count of all bad packets): %u \r\n", rx_stat8.s.cnt);
	printf("OCTEON GMX_TX0_STAT2[OCTS](count of all bytes sent): %lu \r\n", (uint64_t)tx_stat2.s.octs);
	printf("OCTEON GMX_TX0_STAT3[PKTS](count of all packets sent): %u \r\n", tx_stat3.s.pkts);
	printf("OCTEON GMX_TX0_STAT4[HIST0](count of packets sent with octet count < 64): %u \r\n", tx_stat4.s.hist0);
	printf("OCTEON GMX_TX0_STAT4[HIST1](count of packets sent with octet count == 64): %u \r\n", tx_stat4.s.hist1);
	printf("OCTEON GMX_TX0_STAT5[HIST2](count of packets sent with octet count of 65-127): %u \r\n", tx_stat5.s.hist2);
	printf("OCTEON GMX_TX0_STAT5[HIST3](count of packets sent with octet count of 128-255): %u \r\n", tx_stat5.s.hist3);
	printf("OCTEON GMX_TX0_STAT6[HIST4](count of packets sent with octet count of 256-511): %u \r\n", tx_stat6.s.hist4);
	printf("OCTEON GMX_TX0_STAT6[HIST5](count of packets sent with octet count of 512-1023): %u \r\n", tx_stat6.s.hist5);
	printf("OCTEON GMX_TX0_STAT7[HIST6](count of packets sent with octet count of 1024-1518): %u \r\n", tx_stat7.s.hist6);
	printf("OCTEON GMX_TX0_STAT7[HIST7](count of packets sent with octet count of > 1518): %u \r\n", tx_stat7.s.hist7);
	printf("OCTEON GMX_TX0_STAT8[BCST](count of packets sent to a multicast DMAC): %u \r\n", tx_stat8.s.bcst);
	printf("OCTEON GMX_TX0_STAT8[MCST](count of packets sent to the broadcast DMAC): %u \r\n", tx_stat8.s.mcst);
	printf("OCTEON GMX_TX0_STAT9[CTL](count of control/pause packets sent): %u \r\n", tx_stat9.s.ctl);
	printf("OCTEON GMX_TX0_STAT9[UNDFLW](count of packets sent that expericed a transmit underflow and were truncated): %u \r\n", tx_stat9.s.undflw);
}

void show_pip_registers(uint16_t port_num)
{
	printf("PIP_BIST_STATUS : 0x%lx \n", cvmx_read_csr(CVMX_PIP_BIST_STATUS));
	printf("PIP_INT_REG : 0x%lx \n", cvmx_read_csr(CVMX_PIP_INT_REG));
	printf("PIP_INT_EN : 0x%lx \n", cvmx_read_csr(CVMX_PIP_INT_EN));
	printf("PIP_STAT_CTL : 0x%lx \n", cvmx_read_csr(CVMX_PIP_STAT_CTL));
	printf("PIP_GBL_CTL : 0x%lx \n", cvmx_read_csr(CVMX_PIP_GBL_CTL));
	printf("PIP_GBL_CFG : 0x%lx \n", cvmx_read_csr(CVMX_PIP_GBL_CFG));
	printf("PIP_SFT_RST : 0x%lx \n", cvmx_read_csr(CVMX_PIP_SFT_RST));
	printf("PIP_BCK_PRS : 0x%lx \n", cvmx_read_csr(CVMX_PIP_BCK_PRS));
	printf("PIP_CRC_CTL0 : 0x%lx \n", cvmx_read_csr(CVMX_PIP_CRC_CTLX(0)));
	printf("PIP_CRC_CTL1 : 0x%lx \n", cvmx_read_csr(CVMX_PIP_CRC_CTLX(1)));
	printf("PIP_CRC_IV0 : 0x%lx \n", cvmx_read_csr(CVMX_PIP_CRC_IVX(0)));
	printf("PIP_CRC_IV1 : 0x%lx \n", cvmx_read_csr(CVMX_PIP_CRC_IVX(1)));
	printf("PIP_IP_OFFSET : 0x%lx \n", cvmx_read_csr(CVMX_PIP_IP_OFFSET));
	printf("PIP_TAG_SECRET : 0x%lx \n", cvmx_read_csr(CVMX_PIP_TAG_SECRET));
	printf("PIP_TAG_MASK : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_MASK));
	printf("PIP_TODO_ENTRY : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TODO_ENTRY));
	printf("PIP_DEC_IPSEC0 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_DEC_IPSECX(0)));
	printf("PIP_DEC_IPSEC1 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_DEC_IPSECX(1)));
	printf("PIP_DEC_IPSEC2 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_DEC_IPSECX(2)));
	printf("PIP_DEC_IPSEC3 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_DEC_IPSECX(3)));
	printf("PIP_RAW_WORD : 0x%lx \n",cvmx_read_csr(CVMX_PIP_RAW_WORD));
	printf("PIP_QOS_VLAN0 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_VLANX(0)));
	printf("PIP_QOS_VLAN1 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_VLANX(1)));
	printf("PIP_QOS_VLAN2 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_VLANX(2)));
	printf("PIP_QOS_VLAN3 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_VLANX(3)));
	printf("PIP_QOS_VLAN4 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_VLANX(4)));
	printf("PIP_QOS_VLAN5 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_VLANX(5)));
	printf("PIP_QOS_VLAN6 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_VLANX(6)));
	printf("PIP_QOS_VLAN7 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_VLANX(7)));
	printf("PIP_QOS_WATCH0 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_WATCHX(0)));
	printf("PIP_QOS_WATCH1 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_WATCHX(1)));
	printf("PIP_QOS_WATCH2 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_WATCHX(2)));
	printf("PIP_QOS_WATCH3 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_WATCHX(3)));
	printf("PIP_PRT_CFG%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_PRT_CFGX(port_num)));
	printf("PIP_PRT_TAG%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_PRT_TAGX(port_num)));
	printf("PIP_QOS_DIFF0: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(0)));	
	printf("PIP_QOS_DIFF1: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(1)));	
	printf("PIP_QOS_DIFF2: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(2)));	
	printf("PIP_QOS_DIFF3: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(3)));	
	printf("PIP_QOS_DIFF4: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(4)));	
	printf("PIP_QOS_DIFF5: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(5)));	
	printf("PIP_QOS_DIFF6: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(6)));	
	printf("PIP_QOS_DIFF7: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(7)));	
	printf("PIP_QOS_DIFF8: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(8)));	
	printf("PIP_QOS_DIFF9: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(9)));	
	printf("PIP_QOS_DIFF10: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(10)));	
	printf("PIP_QOS_DIFF11: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(11)));	
	printf("PIP_QOS_DIFF12: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(12)));	
	printf("PIP_QOS_DIFF13: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(13)));	
	printf("PIP_QOS_DIFF14: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(14)));	
	printf("PIP_QOS_DIFF15: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(15)));	
	printf("PIP_QOS_DIFF16: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(16)));	
	printf("PIP_QOS_DIFF17: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(17)));	
	printf("PIP_QOS_DIFF18: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(18)));	
	printf("PIP_QOS_DIFF19: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(19)));
	printf("PIP_QOS_DIFF20: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(20)));	
	printf("PIP_QOS_DIFF21: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(21)));	
	printf("PIP_QOS_DIFF22: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(22)));	
	printf("PIP_QOS_DIFF23: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(23)));	
	printf("PIP_QOS_DIFF24: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(24)));	
	printf("PIP_QOS_DIFF25: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(25)));	
	printf("PIP_QOS_DIFF26: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(26)));	
	printf("PIP_QOS_DIFF27: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(27)));	
	printf("PIP_QOS_DIFF28: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(28)));	
	printf("PIP_QOS_DIFF29: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(29)));
	printf("PIP_QOS_DIFF30: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(30)));	
	printf("PIP_QOS_DIFF31: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(31)));	
	printf("PIP_QOS_DIFF32: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(32)));	
	printf("PIP_QOS_DIFF33: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(33)));	
	printf("PIP_QOS_DIFF34: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(34)));	
	printf("PIP_QOS_DIFF35: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(35)));	
	printf("PIP_QOS_DIFF36: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(36)));	
	printf("PIP_QOS_DIFF37: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(37)));	
	printf("PIP_QOS_DIFF38: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(38)));	
	printf("PIP_QOS_DIFF39: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(39)));
	printf("PIP_QOS_DIFF40: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(40)));	
	printf("PIP_QOS_DIFF41: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(41)));	
	printf("PIP_QOS_DIFF42: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(42)));	
	printf("PIP_QOS_DIFF43: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(43)));	
	printf("PIP_QOS_DIFF44: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(44)));	
	printf("PIP_QOS_DIFF45: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(45)));	
	printf("PIP_QOS_DIFF46: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(46)));	
	printf("PIP_QOS_DIFF47: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(47)));	
	printf("PIP_QOS_DIFF48: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(48)));	
	printf("PIP_QOS_DIFF49: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(49)));
	printf("PIP_QOS_DIFF50: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(50)));	
	printf("PIP_QOS_DIFF51: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(51)));	
	printf("PIP_QOS_DIFF52: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(52)));	
	printf("PIP_QOS_DIFF53: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(53)));	
	printf("PIP_QOS_DIFF54: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(54)));	
	printf("PIP_QOS_DIFF55: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(55)));	
	printf("PIP_QOS_DIFF56: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(56)));	
	printf("PIP_QOS_DIFF57: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(57)));	
	printf("PIP_QOS_DIFF58: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(58)));	
	printf("PIP_QOS_DIFF59: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(59)));
	printf("PIP_QOS_DIFF60: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(60)));	
	printf("PIP_QOS_DIFF61: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(61)));	
	printf("PIP_QOS_DIFF62: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(62)));	
	printf("PIP_QOS_DIFF63: 0x%lx \n",cvmx_read_csr(CVMX_PIP_QOS_DIFFX(63)));	
	printf("PIP_STAT0_PRT%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT0_PRTX(port_num)));
	printf("PIP_STAT1_PRT%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT1_PRTX(port_num)));
	printf("PIP_STAT2_PRT%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT2_PRTX(port_num)));
	printf("PIP_STAT3_PRT%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT3_PRTX(port_num)));
	printf("PIP_STAT4_PRT%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT4_PRTX(port_num)));
	printf("PIP_STAT5_PRT%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT5_PRTX(port_num)));
	printf("PIP_STAT6_PRT%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT6_PRTX(port_num)));
	printf("PIP_STAT7_PRT%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT7_PRTX(port_num)));
	printf("PIP_STAT8_PRT%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT8_PRTX(port_num)));
	printf("PIP_STAT9_PRT%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT9_PRTX(port_num)));
	printf("PIP_TAG_INC0 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(0)));
	printf("PIP_TAG_INC1 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(1)));
	printf("PIP_TAG_INC2 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(2)));
	printf("PIP_TAG_INC3 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(3)));
	printf("PIP_TAG_INC4 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(4)));
	printf("PIP_TAG_INC5 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(5)));
	printf("PIP_TAG_INC6 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(6)));
	printf("PIP_TAG_INC7 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(7)));
	printf("PIP_TAG_INC8 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(8)));
	printf("PIP_TAG_INC9 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(9)));
	printf("PIP_TAG_INC10 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(10)));
	printf("PIP_TAG_INC11 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(11)));
	printf("PIP_TAG_INC12 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(12)));
	printf("PIP_TAG_INC13 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(13)));
	printf("PIP_TAG_INC14 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(14)));
	printf("PIP_TAG_INC15 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(15)));
	printf("PIP_TAG_INC16 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(16)));
	printf("PIP_TAG_INC17 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(17)));
	printf("PIP_TAG_INC18 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(18)));
	printf("PIP_TAG_INC19 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(19)));
	printf("PIP_TAG_INC20 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(20)));
	printf("PIP_TAG_INC21 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(21)));
	printf("PIP_TAG_INC22 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(22)));
	printf("PIP_TAG_INC23 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(23)));
	printf("PIP_TAG_INC24 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(24)));
	printf("PIP_TAG_INC25 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(25)));
	printf("PIP_TAG_INC26 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(26)));
	printf("PIP_TAG_INC27 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(27)));
	printf("PIP_TAG_INC28 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(28)));
	printf("PIP_TAG_INC29 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(29)));
	printf("PIP_TAG_INC30 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(30)));
	printf("PIP_TAG_INC31 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(31)));
	printf("PIP_TAG_INC32 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(32)));
	printf("PIP_TAG_INC33 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(33)));
	printf("PIP_TAG_INC34 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(34)));
	printf("PIP_TAG_INC35 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(35)));
	printf("PIP_TAG_INC36 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(36)));
	printf("PIP_TAG_INC37 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(37)));
	printf("PIP_TAG_INC38 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(38)));
	printf("PIP_TAG_INC39 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(39)));
	printf("PIP_TAG_INC40 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(40)));
	printf("PIP_TAG_INC41 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(41)));
	printf("PIP_TAG_INC42 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(42)));
	printf("PIP_TAG_INC43 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(43)));
	printf("PIP_TAG_INC44 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(44)));
	printf("PIP_TAG_INC45 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(45)));
	printf("PIP_TAG_INC46 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(46)));
	printf("PIP_TAG_INC47 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(47)));
	printf("PIP_TAG_INC48 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(48)));
	printf("PIP_TAG_INC49 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(49)));
	printf("PIP_TAG_INC50 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(50)));
	printf("PIP_TAG_INC51 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(51)));
	printf("PIP_TAG_INC52 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(52)));
	printf("PIP_TAG_INC53 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(53)));
	printf("PIP_TAG_INC54 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(54)));
	printf("PIP_TAG_INC55 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(55)));
	printf("PIP_TAG_INC56 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(56)));
	printf("PIP_TAG_INC57 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(57)));
	printf("PIP_TAG_INC58 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(58)));
	printf("PIP_TAG_INC59 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(59)));
	printf("PIP_TAG_INC60 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(60)));
	printf("PIP_TAG_INC61 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(61)));
	printf("PIP_TAG_INC62 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(62)));
	printf("PIP_TAG_INC63 : 0x%lx \n",cvmx_read_csr(CVMX_PIP_TAG_INCX(63)));	
	printf("PIP_STAT_INB_PKTS%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT_INB_PKTSX(port_num)));
	printf("PIP_STAT_INB_OCTS%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT_INB_OCTSX(port_num)));
	printf("PIP_STAT_INB_ERRS%d : 0x%lx \n",port_num, cvmx_read_csr(CVMX_PIP_STAT_INB_ERRSX(port_num)));
}
void show_ipd_registers(uint16_t port_num)
{
	printf("IPD_1ST_MBUFF_SKIP : 0x%lx \n", cvmx_read_csr(CVMX_IPD_1ST_MBUFF_SKIP));
	printf("IPD_NOT_1ST_MBUFF_SKIP : 0x%lx \n", cvmx_read_csr(CVMX_IPD_NOT_1ST_MBUFF_SKIP));
	printf("IPD_PACKET_MBUFF_SIZE : 0x%lx \n", cvmx_read_csr(CVMX_IPD_PACKET_MBUFF_SIZE));
	printf("IPD_CTL_STATUS : 0x%lx \n", cvmx_read_csr(CVMX_IPD_CTL_STATUS));
	printf("IPD_WQE_FPA_QUEUE : 0x%lx \n", cvmx_read_csr(CVMX_IPD_WQE_FPA_QUEUE));
	printf("IPD_PORT%d_BP_PAGE_CNT : 0x%lx \n", port_num, cvmx_read_csr(CVMX_IPD_PORTX_BP_PAGE_CNT(port_num)));
	printf("IPD_SUB_PORT_BP_PAGE_CNT : 0x%lx \n", cvmx_read_csr(CVMX_IPD_SUB_PORT_BP_PAGE_CNT));
	printf("IPD_1st_NEXT_PTR_BACK : 0x%lx \n", cvmx_read_csr(CVMX_IPD_1st_NEXT_PTR_BACK));
	printf("IPD_2nd_NEXT_PTR_BACK : 0x%lx \n", cvmx_read_csr(CVMX_IPD_2nd_NEXT_PTR_BACK));
	printf("IPD_INT_ENB : 0x%lx \n", cvmx_read_csr(CVMX_IPD_INT_ENB));
	printf("IPD_INT_SUM : 0x%lx \n", cvmx_read_csr(CVMX_IPD_INT_SUM));
	printf("IPD_SUB_PORT_FCS : 0x%lx \n", cvmx_read_csr(CVMX_IPD_SUB_PORT_FCS));
	printf("IPD_QOS0_RED_MARKS : 0x%lx \n", cvmx_read_csr(CVMX_IPD_QOSX_RED_MARKS(0)));
	printf("IPD_QOS1_RED_MARKS : 0x%lx \n", cvmx_read_csr(CVMX_IPD_QOSX_RED_MARKS(1)));
	printf("IPD_QOS2_RED_MARKS : 0x%lx \n", cvmx_read_csr(CVMX_IPD_QOSX_RED_MARKS(2)));
	printf("IPD_QOS3_RED_MARKS : 0x%lx \n", cvmx_read_csr(CVMX_IPD_QOSX_RED_MARKS(3)));
	printf("IPD_QOS4_RED_MARKS : 0x%lx \n", cvmx_read_csr(CVMX_IPD_QOSX_RED_MARKS(4)));
	printf("IPD_QOS5_RED_MARKS : 0x%lx \n", cvmx_read_csr(CVMX_IPD_QOSX_RED_MARKS(5)));
	printf("IPD_QOS6_RED_MARKS : 0x%lx \n", cvmx_read_csr(CVMX_IPD_QOSX_RED_MARKS(6)));
	printf("IPD_QOS7_RED_MARKS : 0x%lx \n", cvmx_read_csr(CVMX_IPD_QOSX_RED_MARKS(7)));
	printf("IPD_PORT_BP_COUNTERS_PAIR%d : 0x%lx \n", port_num, cvmx_read_csr(CVMX_IPD_PORT_BP_COUNTERS_PAIRX(port_num)));
	printf("IPD_RED_PORT_ENABLE : 0x%lx \n", cvmx_read_csr(CVMX_IPD_RED_PORT_ENABLE));
	printf("IPD_RED_QUE0_PARAM : 0x%lx \n", cvmx_read_csr(CVMX_IPD_RED_QUE0_PARAM));
	printf("IPD_RED_QUE1_PARAM : 0x%lx \n", cvmx_read_csr(CVMX_IPD_RED_QUE1_PARAM));
	printf("IPD_RED_QUE2_PARAM : 0x%lx \n", cvmx_read_csr(CVMX_IPD_RED_QUE2_PARAM));
	printf("IPD_RED_QUE3_PARAM : 0x%lx \n", cvmx_read_csr(CVMX_IPD_RED_QUE3_PARAM));
	printf("IPD_RED_QUE4_PARAM : 0x%lx \n", cvmx_read_csr(CVMX_IPD_RED_QUE4_PARAM));
	printf("IPD_RED_QUE5_PARAM : 0x%lx \n", cvmx_read_csr(CVMX_IPD_RED_QUE5_PARAM));
	printf("IPD_RED_QUE6_PARAM : 0x%lx \n", cvmx_read_csr(CVMX_IPD_RED_QUE6_PARAM));
	printf("IPD_RED_QUE7_PARAM : 0x%lx \n", cvmx_read_csr(CVMX_IPD_RED_QUE7_PARAM));
	printf("IPD_PTR_COUNT : 0x%lx \n", cvmx_read_csr(CVMX_IPD_PTR_COUNT));
	printf("IPD_BP_PRT_RED_END : 0x%lx \n", cvmx_read_csr(CVMX_IPD_BP_PRT_RED_END));
	printf("IPD_QUE0_FREE_PAGE_CNT : 0x%lx \n", cvmx_read_csr(CVMX_IPD_QUE0_FREE_PAGE_CNT));
	printf("IPD_CLK_COUNT : 0x%lx \n", cvmx_read_csr(CVMX_IPD_CLK_COUNT));
	printf("IPD_PWP_PTR_FIFO_CTL : 0x%lx \n", cvmx_read_csr(CVMX_IPD_PWP_PTR_FIFO_CTL));
	printf("IPD_PRC_HOLD_PTR_FIFO_CTL : 0x%lx \n", cvmx_read_csr(CVMX_IPD_PRC_HOLD_PTR_FIFO_CTL));
	printf("IPD_PRC_PORT_PTR_FIFO_CTL : 0x%lx \n", cvmx_read_csr(CVMX_IPD_PRC_PORT_PTR_FIFO_CTL));
	printf("IPD_PKT_PTR_VALID : 0x%lx \n", cvmx_read_csr(CVMX_IPD_PKT_PTR_VALID));
	printf("IPD_WQE_PTR_VALID : 0x%lx \n", cvmx_read_csr(CVMX_IPD_WQE_PTR_VALID));
	printf("IPD_BIST_STATUS : 0x%lx \n", cvmx_read_csr(CVMX_IPD_BIST_STATUS));
}
void show_gmx_registers(uint16_t block_id, uint16_t offset)
{
	printf("GMX%d_RX%d_INT_REG : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_INT_REG(offset, block_id)));
	printf("GMX%d_RX%d_INT_EN : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_INT_REG(offset, block_id)));
	printf("GMX%d_PRT%d_CFG : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_PRTX_CFG(offset, block_id)));
	printf("GMX%d_RX%d_FRM_CTL : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_FRM_CTL(offset, block_id)));
	printf("GMX%d_RX%d_FRM_CHK : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_FRM_CHK(offset, block_id)));
	printf("GMX%d_RX%d_FRM_MIN : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_FRM_MIN(offset, block_id)));
	printf("GMX%d_RX%d_FRM_MAX : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_FRM_MAX(offset, block_id)));
	printf("GMX%d_RX%d_JABBER : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_JABBER(offset, block_id)));
	printf("GMX%d_RX%d_DECISION : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_DECISION(offset, block_id)));
	printf("GMX%d_RX%d_UDD_SKP : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_UDD_SKP(offset, block_id)));
	printf("GMX%d_RX%d_STATS_CTL : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_STATS_CTL(offset, block_id)));
	printf("GMX%d_RX%d_IFG : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_IFG(offset, block_id)));
	printf("GMX%d_RX%d_RX_INBND : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_RX_INBND(offset, block_id)));
	printf("GMX%d_RX%d_PAUSE_DROP_TIME : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_PAUSE_DROP_TIME(offset, block_id)));
	printf("GMX%d_RX%d_STATS_PKTS : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS(offset, block_id)));
	printf("GMX%d_RX%d_STATS_OCTS : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_STATS_OCTS(offset, block_id)));
	printf("GMX%d_RX%d_STATS_PKTS_CTL : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS_CTL(offset, block_id)));
	printf("GMX%d_RX%d_STATS_OCTS_CTL : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_STATS_OCTS_CTL(offset, block_id)));
	printf("GMX%d_RX%d_STATS_PKTS_DMAC : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS_DMAC(offset, block_id)));
	printf("GMX%d_RX%d_STATS_OCTS_DMAC : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_STATS_OCTS_DMAC(offset, block_id)));
	printf("GMX%d_RX%d_STATS_PKTS_DRP : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS_DRP(offset, block_id)));
	printf("GMX%d_RX%d_STATS_OCTS_DRP : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_STATS_OCTS_DRP(offset, block_id)));
	printf("GMX%d_RX%d_STATS_PKTS_BAD : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS_BAD(offset, block_id)));
	printf("GMX%d_RX%d_ADR_CTL : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_ADR_CTL(offset, block_id)));
	printf("GMX%d_RX%d_ADR_CAM_EN : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_ADR_CAM_EN(offset, block_id)));
	printf("GMX%d_RX%d_ADR_CAM0 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_ADR_CAM0(offset, block_id)));
	printf("GMX%d_RX%d_ADR_CAM1 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_ADR_CAM1(offset, block_id)));
	printf("GMX%d_RX%d_ADR_CAM2 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_ADR_CAM2(offset, block_id)));
	printf("GMX%d_RX%d_ADR_CAM3 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_ADR_CAM3(offset, block_id)));
	printf("GMX%d_RX%d_ADR_CAM4 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_ADR_CAM4(offset, block_id)));
	printf("GMX%d_RX%d_ADR_CAM5 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_RXX_ADR_CAM5(offset, block_id)));
	printf("GMX%d_TX%d_CLK : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_CLK(offset, block_id)));
	printf("GMX%d_TX%d_THRESH : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_THRESH(offset, block_id)));
	printf("GMX%d_TX%d_APPEND : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_APPEND(offset, block_id)));
	printf("GMX%d_TX%d_SLOT : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_SLOT(offset, block_id)));
	printf("GMX%d_TX%d_BURST : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_BURST(offset, block_id)));
	printf("GMX%d_SMAC%d : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_SMACX(offset, block_id)));
	printf("GMX%d_TX%d_PAUSE_PKT_TIME : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_PAUSE_PKT_TIME(offset, block_id)));
	printf("GMX%d_TX%d_MIN_PKT : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_MIN_PKT(offset, block_id)));
	printf("GMX%d_TX%d_PAUSE_PKT_INTERVAL : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_PAUSE_PKT_INTERVAL(offset, block_id)));
	printf("GMX%d_TX%d_SOFT_PAUSE : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_SOFT_PAUSE(offset, block_id)));
	printf("GMX%d_TX%d_PAUSE_TOGO : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_PAUSE_TOGO(offset, block_id)));
	printf("GMX%d_TX%d_PAUSE_ZERO : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_PAUSE_ZERO(offset, block_id)));
	printf("GMX%d_TX%d_STATS_CTL : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_STATS_CTL(offset, block_id)));
	printf("GMX%d_TX%d_CTL : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_CTL(offset, block_id)));
	printf("GMX%d_TX%d_STAT0 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_STAT0(offset, block_id)));
	printf("GMX%d_TX%d_STAT1 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_STAT1(offset, block_id)));
	printf("GMX%d_TX%d_STAT2 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_STAT2(offset, block_id)));
	printf("GMX%d_TX%d_STAT3 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_STAT3(offset, block_id)));
	printf("GMX%d_TX%d_STAT4 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_STAT4(offset, block_id)));
	printf("GMX%d_TX%d_STAT5 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_STAT5(offset, block_id)));
	printf("GMX%d_TX%d_STAT6 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_STAT6(offset, block_id)));
	printf("GMX%d_TX%d_STAT7 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_STAT7(offset, block_id)));
	printf("GMX%d_TX%d_STAT8 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_STAT8(offset, block_id)));
	printf("GMX%d_TX%d_STAT9 : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_GMXX_TXX_STAT9(offset, block_id)));
	printf("GMX%d_BIST : 0x%lx \n",block_id, cvmx_read_csr(CVMX_GMXX_BIST(block_id)));
	printf("GMX%d_RX_PRTS : 0x%lx \n",block_id, cvmx_read_csr(CVMX_GMXX_RX_PRTS(block_id)));
	printf("GMX%d_RX_BP_DROP0 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_DROPX(0, block_id)));
	printf("GMX%d_RX_BP_DROP1 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_DROPX(1, block_id)));
	printf("GMX%d_RX_BP_DROP2 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_DROPX(2, block_id)));
	printf("GMX%d_RX_BP_DROP3 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_DROPX(3, block_id)));
	printf("GMX%d_RX_BP_ON0 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_ONX(0, block_id)));
	printf("GMX%d_RX_BP_ON1 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_ONX(1, block_id)));
	printf("GMX%d_RX_BP_ON2 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_ONX(2, block_id)));
	printf("GMX%d_RX_BP_ON3 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_ONX(3, block_id)));
	printf("GMX%d_RX_BP_OFF0 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_OFFX(0, block_id)));
	printf("GMX%d_RX_BP_OFF1 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_OFFX(1, block_id)));
	printf("GMX%d_RX_BP_OFF2 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_OFFX(2, block_id)));
	printf("GMX%d_RX_BP_OFF3 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_BP_OFFX(3, block_id)));
	printf("GMX%d_TX_PRTS : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_PRTS(block_id)));
	printf("GMX%d_TX_IFG : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_IFG(block_id)));
	printf("GMX%d_TX_JAM : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_JAM(block_id)));
	printf("GMX%d_TX_COL_ATTEMPT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_COL_ATTEMPT(block_id)));
	printf("GMX%d_TX_PAUSE_PKT_DMAC : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_PAUSE_PKT_DMAC(block_id)));
	printf("GMX%d_TX_PAUSE_PKT_TYPE : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_PAUSE_PKT_TYPE(block_id)));
	printf("GMX%d_TX_SPI_MAX : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_MAX(block_id)));
	printf("GMX%d_TX_SPI_THRESH : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_THRESH(block_id)));
	printf("GMX%d_TX_SPI_CTL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_CTL(block_id)));
	printf("GMX%d_TX_OVR_BP : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_OVR_BP(block_id)));
	printf("GMX%d_TX_BP : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_BP(block_id)));
	printf("GMX%d_TX_CORRUPT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_CORRUPT(block_id)));
	printf("GMX%d_TX_SPI_DRAIN : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_DRAIN(block_id)));
	printf("GMX%d_RX_PRT_INFO : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PRT_INFO(block_id)));
	printf("GMX%d_TX_LFSR : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_LFSR(block_id)));
	printf("GMX%d_TX_INT_REG : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_INT_REG(block_id)));
	printf("GMX%d_TX_INT_EN : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_INT_EN(block_id)));
	printf("GMX%d_NXA_ADR : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_NXA_ADR(block_id)));
	printf("GMX%d_BAD_REG : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_BAD_REG(block_id)));
	printf("GMX%d_STAT_BP : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_STAT_BP(block_id)));
	printf("GMX%d_RX_PASS_EN : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_EN(block_id)));
	printf("GMX%d_RX_PASS_MAP0 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(0, block_id)));
	printf("GMX%d_RX_PASS_MAP1 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(1, block_id)));
	printf("GMX%d_RX_PASS_MAP2 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(2, block_id)));
	printf("GMX%d_RX_PASS_MAP3 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(3, block_id)));
	printf("GMX%d_RX_PASS_MAP4 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(4, block_id)));
	printf("GMX%d_RX_PASS_MAP5 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(5, block_id)));
	printf("GMX%d_RX_PASS_MAP6 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(6, block_id)));
	printf("GMX%d_RX_PASS_MAP7 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(7, block_id)));
	printf("GMX%d_RX_PASS_MAP8 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(8, block_id)));
	printf("GMX%d_RX_PASS_MAP9 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(9, block_id)));
	printf("GMX%d_RX_PASS_MAP10 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(10, block_id)));
	printf("GMX%d_RX_PASS_MAP11 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(11, block_id)));
	printf("GMX%d_RX_PASS_MAP12 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(12, block_id)));
	printf("GMX%d_RX_PASS_MAP13 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(13, block_id)));
	printf("GMX%d_RX_PASS_MAP14 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(14, block_id)));
	printf("GMX%d_RX_PASS_MAP15 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_RX_PASS_MAPX(15, block_id)));
	printf("GMX%d_TX_SPI_ROUND0 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(0, block_id)));
	printf("GMX%d_TX_SPI_ROUND1 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(1, block_id)));
	printf("GMX%d_TX_SPI_ROUND2 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(2, block_id)));
	printf("GMX%d_TX_SPI_ROUND3 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(3, block_id)));
	printf("GMX%d_TX_SPI_ROUND4 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(4, block_id)));
	printf("GMX%d_TX_SPI_ROUND5 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(5, block_id)));
	printf("GMX%d_TX_SPI_ROUND6 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(6, block_id)));
	printf("GMX%d_TX_SPI_ROUND7 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(7, block_id)));
	printf("GMX%d_TX_SPI_ROUND8 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(8, block_id)));
	printf("GMX%d_TX_SPI_ROUND9 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(9, block_id)));
	printf("GMX%d_TX_SPI_ROUND10 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(10, block_id)));
	printf("GMX%d_TX_SPI_ROUND11 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(11, block_id)));
	printf("GMX%d_TX_SPI_ROUND12 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(12, block_id)));
	printf("GMX%d_TX_SPI_ROUND13 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(13, block_id)));
	printf("GMX%d_TX_SPI_ROUND14 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(14, block_id)));
	printf("GMX%d_TX_SPI_ROUND15 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(15, block_id)));
	printf("GMX%d_TX_SPI_ROUND16 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(16, block_id)));
	printf("GMX%d_TX_SPI_ROUND17 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(17, block_id)));
	printf("GMX%d_TX_SPI_ROUND18 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(18, block_id)));
	printf("GMX%d_TX_SPI_ROUND19 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(19, block_id)));
	printf("GMX%d_TX_SPI_ROUND20 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(20, block_id)));
	printf("GMX%d_TX_SPI_ROUND21 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(21, block_id)));
	printf("GMX%d_TX_SPI_ROUND22 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(22, block_id)));
	printf("GMX%d_TX_SPI_ROUND23 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(23, block_id)));
	printf("GMX%d_TX_SPI_ROUND24 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(24, block_id)));
	printf("GMX%d_TX_SPI_ROUND25 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(25, block_id)));
	printf("GMX%d_TX_SPI_ROUND26 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(26, block_id)));
	printf("GMX%d_TX_SPI_ROUND27 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(27, block_id)));
	printf("GMX%d_TX_SPI_ROUND28 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(28, block_id)));
	printf("GMX%d_TX_SPI_ROUND29 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(29, block_id)));
	printf("GMX%d_TX_SPI_ROUND30 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(30, block_id)));
	printf("GMX%d_TX_SPI_ROUND31 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_TX_SPI_ROUNDX(31, block_id)));
	printf("GMX%d_INF_MODE : 0x%lx \n", block_id, cvmx_read_csr(CVMX_GMXX_INF_MODE(block_id)));
}
void show_asx_registers(uint16_t block_id)
{
	printf("ASX%d_RX_PRT_EN : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RX_PRT_EN(block_id)));
	printf("ASX%d_TX_PRT_EN : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_TX_PRT_EN(block_id)));
	printf("ASX%d_INT_REG : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_INT_REG(block_id)));
	printf("ASX%d_INT_EN : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_INT_EN(block_id)));
	printf("ASX%d_RX_CLK_SET0 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RX_CLK_SETX(0, block_id)));
	printf("ASX%d_RX_CLK_SET1 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RX_CLK_SETX(1, block_id)));
	printf("ASX%d_RX_CLK_SET2 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RX_CLK_SETX(2, block_id)));
	printf("ASX%d_RX_CLK_SET3 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RX_CLK_SETX(3, block_id)));
	printf("ASX%d_PRT_LOOP : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_PRT_LOOP(block_id)));
	printf("ASX%d_TX_CLK_SET0 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_TX_CLK_SETX(0, block_id)));
	printf("ASX%d_TX_CLK_SET1 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_TX_CLK_SETX(1, block_id)));
	printf("ASX%d_TX_CLK_SET2 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_TX_CLK_SETX(2, block_id)));
	printf("ASX%d_TX_CLK_SET3 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_TX_CLK_SETX(3, block_id)));
	printf("ASX%d_TX_COMP_BYP : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_TX_COMP_BYP(block_id)));
	printf("ASX%d_TX_HI_WATER0 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_TX_HI_WATERX(0, block_id)));
	printf("ASX%d_TX_HI_WATER1 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_TX_HI_WATERX(1, block_id)));
	printf("ASX%d_TX_HI_WATER2 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_TX_HI_WATERX(2, block_id)));
	printf("ASX%d_TX_HI_WATER3 : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_TX_HI_WATERX(3, block_id)));
	printf("ASX0_DBG_DATA_ENABLE : 0x%lx \n", cvmx_read_csr(CVMX_ASX0_DBG_DATA_ENABLE));
	printf("ASX0_DBG_DATA_DRV : 0x%lx \n", cvmx_read_csr(CVMX_ASX0_DBG_DATA_DRV));
	printf("ASX%d_RLD_DATA_DRV : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RLD_DATA_DRV(block_id)));
	printf("ASX%d_RLD_COMP : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RLD_COMP(block_id)));
	printf("ASX%d_RLD_PCTL_STRONG : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RLD_PCTL_STRONG(block_id)));
	printf("ASX%d_RLD_NCTL_STRONG : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RLD_NCTL_STRONG(block_id)));
	printf("ASX%d_RLD_PCTL_WEAK : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RLD_PCTL_WEAK(block_id)));
	printf("ASX%d_RLD_NCTL_WEAK : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RLD_NCTL_WEAK(block_id)));
	printf("ASX%d_RLD_BYPASS : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RLD_BYPASS(block_id)));
	printf("ASX%d_RLD_BYPASS_SETTING : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RLD_BYPASS_SETTING(block_id)));
	printf("ASX%d_RLD_SETTING : 0x%lx \n", block_id, cvmx_read_csr(CVMX_ASXX_RLD_SETTING(block_id)));
}

void show_spi_registers(uint16_t block_id, uint16_t offset)
{
	printf("SRX%d_SPI4_CAL%d : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_SRXX_SPI4_CALX(offset, block_id)));
	printf("SRX%d_COM_CTL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SRXX_COM_CTL(block_id)));
	printf("SRX%d_SPI4_STAT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SRXX_SPI4_STAT(block_id)));    
	printf("SRX%d_IGN_RX_FULL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SRXX_IGN_RX_FULL(block_id)));  
	printf("SRX%d_SW_TICK_CTL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SRXX_SW_TICK_CTL(block_id)));      
	printf("SRX%d_SW_TICK_DAT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SRXX_SW_TICK_DAT(block_id)));      
	printf("SPX%d_INT_REG : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_INT_REG(block_id)));      
	printf("SPX%d_INT_MSK : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_INT_MSK(block_id))); 
	printf("SPX%d_INT_SYNC : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_INT_SYNC(block_id))); 
	printf("SPX%d_INT_DAT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_INT_DAT(block_id))); 
	printf("SPX%d_ERR_CTL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_ERR_CTL(block_id))); 
	printf("SPX%d_TPA_SEL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_TPA_SEL(block_id)));
	printf("SPX%d_TPA_MAX : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_TPA_MAX(block_id)));
	printf("SPX%d_TPA_ACC : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_TPA_ACC(block_id)));
	printf("STX%d_BCKPRS_CNT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_BCKPRS_CNT(block_id)));
	printf("SPX%d_CLK_CTL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_CLK_CTL(block_id)));
	printf("SPX%d_CLK_STAT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_CLK_STAT(block_id)));
	printf("SPX%d_DRV_CTL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_DRV_CTL(block_id)));
	printf("SPX%d_TRN4_CTL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_TRN4_CTL(block_id)));
	printf("SPX%d_DBG_DESKEW_CTL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_DBG_DESKEW_CTL(block_id)));
	printf("SPX%d_DBG_DESKEW_STATE : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_DBG_DESKEW_STATE(block_id)));
	printf("STX%d_SPI4_CAL%d : 0x%lx \n", block_id, offset, cvmx_read_csr(CVMX_STXX_SPI4_CALX(offset, block_id)));
	printf("STX%d_COM_CTL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_COM_CTL(block_id)));
	printf("STX%d_ARB_CTL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_ARB_CTL(block_id)));
	printf("STX%d_IGN_CAL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_IGN_CAL(block_id)));
	printf("STX%d_MIN_BST : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_MIN_BST(block_id)));
	printf("STX%d_SPI4_DAT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_SPI4_DAT(block_id)));
	printf("STX%d_SPI4_STAT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_SPI4_STAT(block_id)));
	printf("STX%d_STAT_CTL : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_STAT_CTL(block_id)));
	printf("STX%d_STAT_PKT_XMT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_STAT_PKT_XMT(block_id)));
	printf("STX%d_STAT_BYTES_HI : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_STAT_BYTES_HI(block_id)));
	printf("STX%d_STAT_BYTES_LO : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_STAT_BYTES_LO(block_id)));
	printf("STX%d_BCKPRS_CNT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_BCKPRS_CNT(block_id)));
	printf("STX%d_DIP_CNT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_DIP_CNT(block_id)));
	printf("STX%d_INT_REG : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_INT_REG(block_id)));
	printf("STX%d_INT_MSK : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_INT_MSK(block_id)));
	printf("STX%d_INT_SYNC : 0x%lx \n", block_id, cvmx_read_csr(CVMX_STXX_INT_SYNC(block_id)));
	printf("SPX%d_BIST_STAT : 0x%lx \n", block_id, cvmx_read_csr(CVMX_SPXX_BIST_STAT(block_id)));
}

/*
   0 = stat registers hold value when read
   1 = stat registers are cleared when read
 */
void OcteonGmxTxDump(int block,int port_num, int clear)
{
	cvmx_agl_gmx_txx_stats_ctl_t gmx_stat_ctl;
	cvmx_gmxx_txx_stat0_t stat0;
	cvmx_gmxx_txx_stat1_t stat1;
	cvmx_gmxx_txx_stat2_t stat2;
	cvmx_gmxx_txx_stat3_t stat3;
	cvmx_gmxx_txx_stat4_t stat4;
	cvmx_gmxx_txx_stat5_t stat5;
	cvmx_gmxx_txx_stat6_t stat6;
	cvmx_gmxx_txx_stat7_t stat7;
	cvmx_gmxx_txx_stat8_t stat8;
	cvmx_gmxx_txx_stat9_t stat9;

	gmx_stat_ctl.u64 = 0;
	gmx_stat_ctl.s.rd_clr = clear;
	cvmx_write_csr(CVMX_AGL_GMX_TXX_STATS_CTL(port_num), gmx_stat_ctl.u64);

	stat0.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STAT0(port_num,block));
	stat1.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STAT1(port_num,block));
	stat2.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STAT2(port_num,block));
	stat3.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STAT3(port_num,block));
	stat4.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STAT4(port_num,block));
	stat5.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STAT5(port_num,block));
	stat6.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STAT6(port_num,block));
	stat7.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STAT7(port_num,block));
	stat8.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STAT8(port_num,block));
	stat9.u64 = cvmx_read_csr(CVMX_GMXX_TXX_STAT9(port_num,block));

	printf("\r\n-------------GMX TX Statistics Counters -------------- ");
	printf("\r\nNumber of packets dropped (never successfully sent) due to excessive deferal%#X", stat0.s.xsdef);
	printf("\r\nNumber of packets dropped (never successfully sent) due to excessive collision %#X", stat0.s.xscol);
	printf("\r\nNumber of packets sent with a single collision %#X", stat1.s.scol);         
	printf("\r\nNumber of packets sent with multiple collisions %#X", stat1.s.mcol);               
	printf("\r\nNumber of total octets sent on the interface. %#lX", (uint64_t)stat2.s.octs);  
	printf("\r\nNumber of total frames sent on the interface %#X", stat3.s.pkts);
	printf("\r\nNumber of packets sent with an octet count of 64 %#X", stat4.s.hist1);
	printf("\r\nNumber of packets sent with an octet count of < 64 %#X", stat4.s.hist0);
	printf("\r\nNumber of packets sent with an octet count of 128 - 255 %#X", stat5.s.hist3);
	printf("\r\nNumber of packets sent with an octet count of 65 - 127 %#X", stat5.s.hist2);
	printf("\r\nNumber of packets sent with an octet count of 512 - 1023 %#X", stat6.s.hist5);
	printf("\r\nNumber of packets sent with an octet count of 256 - 511 %#X", stat6.s.hist4);
	printf("\r\nNumber of packets sent with an octet count of > 1518 %#X", stat7.s.hist7);
	printf("\r\nNumber of packets sent with an octet count of1024 - 1518 %#X", stat7.s.hist6);     
	printf("\r\nNumber of packets sent to multicast DMAC %#X", stat8.s.mcst);
	printf("\r\nNumber of packets sent to broadcast DMAC %#X", stat8.s.bcst);
	printf("\r\nNumber of underflow packets %#X", stat9.s.undflw);
	printf("\r\nNumber of Control packets (PAUSE flow control) generated by GMX %#X", stat9.s.ctl);          

	printf("\r\n-----------------------------------------------\r\n");

}


/*
   0 = stat registers hold value when read
   1 = stat registers are cleared when read
 */
void OcteonGmxRxDump(int block, int port_num, int clear)
{
	cvmx_agl_gmx_rxx_stats_ctl_t gmx_stat_ctl;
	cvmx_gmxx_rxx_stats_pkts_t stat0;
	cvmx_gmxx_rxx_stats_octs_t stat1;
	cvmx_gmxx_rxx_stats_pkts_ctl_t stat2;
	cvmx_gmxx_rxx_stats_octs_ctl_t stat3;
	cvmx_gmxx_rxx_stats_pkts_dmac_t stat4;
	cvmx_gmxx_rxx_stats_octs_dmac_t stat5;
	cvmx_gmxx_rxx_stats_pkts_drp_t stat6;
	cvmx_gmxx_rxx_stats_octs_drp_t stat7;
	cvmx_gmxx_rxx_stats_pkts_bad_t stat8;

	gmx_stat_ctl.u64 = 0;
	gmx_stat_ctl.s.rd_clr = clear;
	cvmx_write_csr(CVMX_GMXX_RXX_STATS_CTL(port_num,block), gmx_stat_ctl.u64);

	stat0.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS(port_num,block));
	stat1.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_OCTS(port_num,block));
	stat2.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS_CTL(port_num,block));
	stat3.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_OCTS_CTL(port_num,block));
	stat4.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS_DMAC(port_num,block));
	stat5.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_OCTS_DMAC(port_num,block));
	stat6.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS_DRP(port_num,block));
	stat7.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_OCTS_DRP(port_num,block));
	stat8.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS_BAD(port_num,block));

	printf("\r\n--------------GMX RX Statistics Counters -------------- ");
	printf("\r\nCount of received good packets %#X", stat0.s.cnt);
	printf("\r\nOctet count of received good packets %#lX", (uint64_t)stat1.s.cnt);

	printf("\r\nCount of received pause packets %#X", stat2.s.cnt);
	printf("\r\nOctet count of received pause packets %#lX", (uint64_t)stat3.s.cnt);

	printf("\r\nCount of filtered dmac packets %#X", stat4.s.cnt);
	printf("\r\nOctet count of filtered dmac packets %#lX", (uint64_t)stat5.s.cnt);

	printf("\r\nCount of dropped packets %#X", stat6.s.cnt);
	printf("\r\nOctet count of dropped packets %#lX", (uint64_t)stat7.s.cnt);
	printf("\r\nCount of bad packets %#X", stat8.s.cnt);

	printf("\r\n---------------------------------------------------\r\n");

}

void OcteonPacketTruceDump(int block, int port_num)
{
	cvmx_gmxx_rxx_stats_pkts_t stat0;
	cvmx_gmxx_rxx_stats_pkts_drp_t stat6;

	cvmx_pip_stat0_prtx_t pipstat0;
	cvmx_pip_stat2_prtx_t stat2;
	cvmx_pip_stat_inb_pktsx_t pip_stat_inb_pktsx;
	cvmx_pip_stat_inb_octsx_t pip_stat_inb_octsx;
	cvmx_pip_stat_inb_errsx_t pip_stat_inb_errsx;

	cvmx_fpa_quex_available_t fapavail;

	stat0.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS(port_num,block));
	stat6.u64 = cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS_DRP(port_num,block));

	pipstat0.u64 = cvmx_read_csr(CVMX_PIP_STAT0_PRTX(port_num));
	stat2.u64 = cvmx_read_csr(CVMX_PIP_STAT2_PRTX(port_num));
	pip_stat_inb_pktsx.u64 = cvmx_read_csr(CVMX_PIP_STAT_INB_PKTSX(port_num));
	pip_stat_inb_octsx.u64 = cvmx_read_csr(CVMX_PIP_STAT_INB_OCTSX(port_num));
	pip_stat_inb_errsx.u64 = cvmx_read_csr(CVMX_PIP_STAT_INB_ERRSX(port_num));

	printf("\r\n-----------GMX RX Statistics Counters ------------- ");
	printf("\r\nCount of received good packets %#X", stat0.s.cnt);
	printf("\r\nCount of dropped packets %#X", stat6.s.cnt);
	printf("\r\n------------------------------------");


	printf("\r\n-------PIP Statistics Counters -------- ");
	printf("\r\nNumber of packets processed by PIP. %#X", stat2.s.pkts);
	printf("\r\nNumber of packets without GMX/SPX/PCI errors received by PIP  %#lX", pip_stat_inb_pktsx.u64);
	printf("\r\nTotal number of octets from all packets received by PIP  %#lX", pip_stat_inb_octsx.u64);
	printf("\r\nNumber of packets with GMX/SPX/PCI errors received by PIP %#lX", pip_stat_inb_errsx.u64);
	printf("\r\nInbound packets dropped by the IPD QOS widget per port %#X", pipstat0.s.drp_pkts);
	printf("\r\n-----------------------------------------------\r\n");


	printf("\r\n------------FPA Statistics Counters -------------- ");
	fapavail.u64 = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(0));
	printf("\r\nFPA POOL0 available %#X", fapavail.s.que_siz);
	fapavail.u64 = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(1));
	printf("\r\nFPA POOL1 available %#X", fapavail.s.que_siz);
	fapavail.u64 = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(2));
	printf("\r\nFPA POOL2 available %#X", fapavail.s.que_siz);
	printf("\r\n-----------------------------------------------\r\n");

}

int32_t dump_gmx(int argc, char *argv[])
{
	if(argc < 4)
	{
		printf("dump {rx $interface $port $clearflag(0,1)} | {tx $interface $port $clearflag(0,1)}|{truce $interface $port}\n");
		return CMD_ERR_TOO_FEW_ARGC;
	}

	if (argv[1] ==NULL)
		return CMD_ERR_NOT_MATCH;

	if(0 == strcmp(argv[1], "rx"))
	{
		if(NULL != argv[2] && NULL != argv[3] && NULL != argv[4])
		{
			OcteonGmxRxDump(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
		}
	}
	else if(0 == strcmp(argv[1], "tx"))
	{
		if(NULL != argv[2] && NULL != argv[3] && NULL != argv[4])
		{
			OcteonGmxTxDump(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
		}
	}
	else if(0 == strcmp(argv[1], "truce"))
	{
		if(NULL != argv[2] && NULL != argv[3])
		{
			OcteonPacketTruceDump(atoi(argv[2]), atoi(argv[3]));
		}
	}
	else
	{

		printf("dump {rx $interface $port $clearflag(0,1)} | {tx $interface $port $clearflag(0,1)}|{truce $interface $port}\n");
	}

	return CMD_EXEC_SUCCESS;
}

int32_t cmd_fastdisable(int argc, char *argv[])
{
    return disable_fastfwd();
}

int32_t cmd_fastenable(int argc, char *argv[])
{
    return enable_fastfwd();
}


int32_t cmd_show(int argc, char *argv[])
{	
	uint64_t loop = 0;

	if (argc < 2 )
	{				
		printf("show pip counter {$port [clear] | all} \n");
		printf("show fpa info {$pool | all}  \n");	
		printf("show fpa counter \n");
		printf("show fpa analyse {$pool | all}\n");
		printf("show fpa dump {$pool | all}\n");
		printf("show fau {64 | 32} \n");
		printf("show xaui statistics \n");
		printf("show pko port {$port} \n");
		printf("show dump pko \n");
		printf("show debug level \n");
		printf("show coremask \n");
		printf("show module info (show [shell|acl|main]module enable or disable \n");
		printf("show aging time \n");
		printf("show tag type \n");
		printf("show seq state\n");
		printf("show aging state\n");
		printf("show registers {pip $port}|{ipd $port}|{gmx $interface $port(0~3)}|{asx $interface} \n");
		printf("show icmp state\n");
		printf("show one-tuple state\n");
		printf("show car state\n");
		printf("show car table\n");
		printf("show meter\n");
		printf("show 5-tuple\n");
		printf("show passthough state\n");
		printf("show passthough vlan\n");	
		printf("show passthough port\n");
		printf("show ciu\n");
		printf("show basemac\n");
		return CMD_ERR_TOO_FEW_ARGC;
	}

	if (argv[1] ==NULL)
		return CMD_ERR_NOT_MATCH;

	if(strcmp(argv[1] , "pip") == 0)
	{			
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		else if(0 == strcmp(argv[2], "counter"))
		{
			if (argv[3] ==NULL)
			{
				printf("show pip counter {$port [clear] | all} \n");
				return CMD_ERR_NOT_MATCH;
			}
			else if(0 == strcmp(argv[3], "all"))
			{
				for(loop= 0; loop < CVMX_PIP_NUM_INPUT_PORTS; loop++)
				{
					printf("----------------------------------------------------------------------------\n");
					show_port_statistics(loop, 0);
				}				
			}
			else
			{
				uint16_t flag = 0;
				loop = atoi(argv[3]);
				if(NULL != argv[4] && 0 == strcmp(argv[4], "clear"))
					flag = 1;
				if(loop < CVMX_PIP_NUM_INPUT_PORTS)
					show_port_statistics(loop, flag);
				else
					return CMD_ERR_NOT_MATCH;
			}
		}
	}
	else if (strcmp(argv[1] , "basemac")== 0)
	{
		uint32_t i =0;
		int j = 0;
		uint8_t *tmp_base_table = base_mac_table;
		for(i=0;i<base_mac_size/6;i++)
		{
			tmp_base_table = base_mac_table + i*6;
			for(j=0;j<6;j++)
			{
				printf("base_mac[%d]= %02x\n",j,tmp_base_table[j]);
			}
		}
	}
	else if (strcmp(argv[1] , "fpa")== 0)
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;

		if (strcmp(argv[2] , "info") == 0)
		{
			if (argv[3] ==NULL)
				return CMD_ERR_NOT_MATCH;
			else 
			{
				if (!strcmp(argv[3] , "all"))
				{
					cvm_common_fpa_display_all_pool_info();
				}
				else
				{
					loop = atoi(argv[3]);
					FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_SHELL,FASTFWD_COMMON_DBG_LVL_DEBUG, 
							"Show the FPA info: Pool = %ld \r\n", loop);
					if((loop < CVMX_FPA_NUM_POOLS) && (loop >0))
						cvm_common_fpa_display_pool_info(loop);
					else
						return CMD_ERR_NOT_MATCH;

				}
			}
		}
		else if(strcmp(argv[2] , "counter") == 0)
		{
			cvm_common_display_fpa_buffer_count();
		}
		else if(strcmp(argv[2] , "analyse") == 0)
		{
			if (argv[3] ==NULL)
				return CMD_ERR_NOT_MATCH;
			else 
			{
				if (!strcmp(argv[3] , "all"))
				{
					int i = 0;
					for (i=0; i<CVMX_FPA_NUM_POOLS; i++)
					{
						if (cvm_common_fpa_info[i].count)
						{
							cvm_common_analyze_fpa(i);
						}
					}
				}
				else
				{
					loop = atoi(argv[3]);
					if((loop <CVMX_FPA_NUM_POOLS) && (loop >0))
						cvm_common_analyze_fpa(loop);
					else
						return CMD_ERR_NOT_MATCH;
				}
			}				
		}
		else if(strcmp(argv[2] , "dump") == 0)
		{
			if (argv[3] ==NULL)
				return CMD_ERR_NOT_MATCH;
			else 
			{
				if (!strcmp(argv[3] , "all"))
				{
					int i = 0;
					for (i=0; i<CVMX_FPA_NUM_POOLS; i++)
					{
						if (cvm_common_fpa_info[i].count)
						{
							cvm_common_dump_fpa(i);
						}
					}
				}
				else
				{
					loop = atoi(argv[3]);
					if((loop <CVMX_FPA_NUM_POOLS-1) && (loop < CVMX_FPA_NUM_POOLS))
						cvm_common_dump_fpa(loop);
					else
						return CMD_ERR_NOT_MATCH;
				}
			}				
		}
		else
			return CMD_ERR_NOT_MATCH;
	}
	else if (strcmp(argv[1] , "acl")== 0)
	{

	}
	else if (0 == strcmp(argv[1], "fau"))
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(0 == strcmp(argv[2], "64"))
		{
			show_fau_dump_64();
			printf("fwd eth cnt : %lu\n", rule_cnt_info.fwd_eth_cnt);
			printf("fwd capwap 802.11 cnt : %lu\n", rule_cnt_info.fwd_cw_cnt);
			printf("fwd capwap 802.3 cnt : %lu\n", rule_cnt_info.fwd_cw_802_3_cnt);
		}
		else if(0 == strcmp(argv[2], "32"))
		{
			show_fau_dump_32();
		}
	}
	else if (0 == strcmp(argv[1], "pko"))
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(0 == strcmp(argv[2], "port"))
		{
			if(NULL != argv[3])
			{
				show_pko_port_statistics(atoi(argv[3]));
			}
		}
	}
	else if(0 == strcmp(argv[1], "xaui"))
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(0 == strcmp(argv[2], "statistics"))
		{
			show_xaui_statistics();
		}
	}
	else if(0 == strcmp(argv[1], "dump"))
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(0 == strcmp(argv[2], "pko"))
		{
			cvm_common_dump_pko();
		}
	}
	else if(0 == strcmp(argv[1], "debug"))
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(0 == strcmp(argv[2], "level"))
		{
			cvm_common_get_debug_level();
		}
	}
	else if(0 == strcmp(argv[1], "coremask"))
	{
		printf("\n Core mask : 0x%lx \n", core_mask);	
	}
	else if(0 == strcmp(argv[1], "aging"))
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(0 == strcmp(argv[2], "time"))
		{
			printf("aging time : %d (second) \n", acl_aging_timer);
		}
		else if(0 == strcmp(argv[2], "state"))
		{
			if(acl_aging_enable == FUNC_ENABLE)
			{
				printf("acl aging state: enable\n");
			}
			else
			{
				printf("acl aging state: disable\n");
			}
		}
	}
	else if(0 == strcmp(argv[1], "module"))
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(0 == strcmp(argv[2], "info"))
		{
			if(1 == module_print[FASTFWD_COMMON_MOUDLE_MAIN])
			{
				printf("module main enable \n");
			}
			else
			{
				printf("module main disable \n");
			}

			if(1 == module_print[FASTFWD_COMMON_MOUDLE_SHELL])
			{
				printf("module shell enable \n");
			}
			else
			{
				printf("module shell disable \n");
			}

			if(1 == module_print[FASTFWD_COMMON_MOUDLE_FLOWTABLE])
			{
				printf("module acl enable \n");
			}
			else
			{
				printf("module acl disable \n");
			}
		}
	}
	else if(0 == strcmp(argv[1], "tag"))
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(0 == strcmp(argv[2], "type"))
		{
			get_tag_type();
		}
	}
	else if(strcmp(argv[1] , "seq") == 0)
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(0 == strcmp(argv[2], "state"))
		{
			if(flow_sequence_enable == FUNC_ENABLE)
			{
				printf("flow sequence state: enable\n");
			}
			else
			{
				printf("flow sequence state: disable\n");
			}
		}
	}
	else if(0 == strcmp(argv[1], "registers"))
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(0 == strcmp(argv[2], "pip"))
		{
			if(NULL != argv[3])
			{
				show_pip_registers(atoi(argv[3]));
			}
		}
		else if(0 == strcmp(argv[2], "ipd"))
		{
			if(NULL != argv[3])
			{
				show_ipd_registers(atoi(argv[3]));
			}
		}
		else if(0 == strcmp(argv[2], "gmx"))
		{
			if((NULL != argv[3])&&(NULL != argv[4]))
			{
				show_gmx_registers(atoi(argv[3]), atoi(argv[4]));
			}
		}
		else if(0 == strcmp(argv[2], "asx"))
		{
			if(NULL != argv[3])
			{
				show_asx_registers(atoi(argv[3]));
			}
		}
		else if(0 == strcmp(argv[2], "spi"))
		{
			if((NULL != argv[3])&&(NULL != argv[4]))
			{
				show_spi_registers(atoi(argv[3]), atoi(argv[4]));
			}
		}
		else
		{
			printf("show registers {pip|ipd|gmx|asx|spi} \n");
			return CMD_ERR_NOT_MATCH;
		}
	}
	else if(strcmp(argv[1] , "icmp") == 0)
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(strcmp(argv[2] , "state") == 0)
		{
			if(cvm_ip_icmp_enable == FUNC_ENABLE)
			{
				printf("icmp fast-fwd is enable\n");
			}
			else
			{
				printf("icmp fast-fwd is disable\n");
			}
		}
	}
	else if(strcmp(argv[1] , "one-tuple") == 0)
	{
		if (argv[2] ==NULL)
			return CMD_ERR_NOT_MATCH;
		if(strcmp(argv[2] , "state") == 0)
		{
			if(cvm_ip_only_enable == FUNC_ENABLE)
			{
				printf("ip-only rule is enable\n");
			}
			else
			{
				printf("ip-only rule is disable\n");
			}
		}
	}
	else if(strcmp(argv[1] , "car") == 0)
	{
		if(argv[2] == NULL)
		{
			printf("show car {state}\n");
			return CMD_ERR_NOT_MATCH;
		}
		if(strcmp(argv[2] , "state") == 0)
		{
			if(cvm_car_enable == FUNC_ENABLE)
			{
				printf("car is enable\n");
			}
			else
			{
				printf("car is disable\n");
			}
		}
	}
	else if(strcmp(argv[1] , "meter") == 0)
	{
		cvm_car_show_template();
	}
	else if(strcmp(argv[1] , "5-tuple") == 0)
	{
		printf("gl_five_tuple: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d    %s\n",
				IP_FMT(gl_five_tuple.ip_src), gl_five_tuple.th_sport,
				IP_FMT(gl_five_tuple.ip_dst), gl_five_tuple.th_dport, PROTO_STR(gl_five_tuple.ip_p));
	}
	else if(strcmp(argv[1] , "passthough") == 0)
	{
		if(argv[2] == NULL)
		{
			printf("show passthough state\n");
			printf("show passthough vlan\n");
			printf("show passthough port\n");
			return CMD_ERR_NOT_MATCH;
		}
		if(strcmp(argv[2] , "state") == 0)
		{
			if(cvm_passthough_enable == FUNC_ENABLE)
			{
				printf("passthough test is enable\n");
			}
			else
			{
				printf("passthough test is disable\n");
			}
		}
		if(strcmp(argv[2] , "vlan") == 0)
		{
			printf("passthough vlan id = %d\n", passthough_vlan_id);
		}
		if(strcmp(argv[2] , "port") == 0)
		{
			printf("passthough vlan id = %d\n", passthough_vlan_id);
		}
	}
	else if(strcmp(argv[1] , "ciu") == 0)
	{
		show_ciu_stat();
	}
	else
	{
		printf("show pip counter {$port [clear] | all} \n");
		printf("show fpa info {$pool | all}  \n");	
		printf("show fpa counter \n");
		printf("show fpa analyse {$pool | all}\n");
		printf("show fau dump {64 | 32} \n");
		printf("show xaui statistics \n");
		printf("show pko port {$port} \n");
		printf("show dump pko \n");
		printf("show debug level \n");
		printf("show coremask \n");
		printf("show module info (show [shell|acl|main]module enable or disable \n");
		printf("show aging time \n");
		printf("show tag type \n");
		printf("show seq state\n");
		printf("show aging state\n");
		printf("show registers {pip $port}|{ipd $port}|{gmx $interface $port(0~3)}|{asx $interface} \n");
		printf("show icmp state\n");
		printf("show one-tuple state\n");
		printf("show car state\n");
		printf("show car table\n");
		printf("show meter\n");
		printf("show 5-tuple\n");
		printf("show passthough state\n");
		printf("show passthough vlan\n");
		printf("show passthough port\n");
		printf("show ciu\n");
		return CMD_ERR_NOT_MATCH;
	}
	return CMD_EXEC_SUCCESS;
}

/*
 * check ip format
 * ip:  ip string
 *  0: invalid
 *  1: valid
 */
static int is_valid_ip(const char *ip) 
{ 
	int section = 0;  //每一节的十进制值 
	int dot = 0;       //几个点分隔符 
	int prev = -1;     //每一节中上一个字符 

	if(ip == NULL)
	{
		return 0;
	}

	while(*ip){ 
		if(*ip == '.'){ 
			dot++; 
			if(dot > 3){ 
				return 0; 
			}
			if(prev == '.')
			{
				return 0;
			}
			if(section >= 0 && section <=255){ 
				section = 0; 
			}else{ 
				return 0; 
			} 
		}else if(*ip >= '0' && *ip <= '9'){ 
			if((*ip == '0') && ((prev == '.') || (prev == -1)) && (*(ip+1)!='.') && (*(ip+1)!= '\0'))
			{
				return 0;
			}
			section = section * 10 + *ip - '0';  
		}else{ 
			return 0; 
		} 
		prev = *ip; 
		ip++;        
	}

	if(dot != 3)
		return 0;

	if(section >= 0 && section <=255){ 
		return 1; 
	} 
	return 0; 
}

static void print_rpa_head(rule_item_t* rule)
{
	printf("rap header :\n");
	printf("rpa_type = %u,rpa_d_s_slotNum = %u\n",rule->rules.rpa_header.rpa_type,\
		rule->rules.rpa_header.rpa_d_s_slotNum);
	printf("rpa_dnetdevNum = %u,rpa_snetdevNum = %u\n",rule->rules.rpa_header.rpa_dnetdevNum,\
		rule->rules.rpa_header.rpa_snetdevNum);
}
static void print_icmp_rule(rule_item_t* rule)
{
	if(rule == NULL)
		return;

	if(rule->rules.protocol != 1)
	{
		printf("print_icmp_rule error: not icmp rule!\n");
		return;
	}
	printf("  five tuple: %d.%d.%d.%d => %d.%d.%d.%d    icmp\n",
			IP_FMT(rule->rules.sip), IP_FMT(rule->rules.dip));

	switch(rule->rules.action_type)
	{
		case FLOW_ACTION_ICMP:
			printf("  action_type = FLOW_ACTION_ICMP\n");
			break;
		case FLOW_ACTION_CAPWAP_802_11_ICMP:
			printf("  action_type = FLOW_ACTION_CAPWAP_802_11_ICMP\n");
			break;
		case FLOW_ACTION_CAPWAP_802_3_ICMP:
			printf("  action_type = FLOW_ACTION_CAPWAP_802_3_ICMP\n");
			break;
		case FLOW_ACTION_RPA_ICMP:
			printf("  action_type = FLOW_ACTION_RPA_ICMP\n");
			print_rpa_head(rule);
			break;
		case FLOW_ACTION_RPA_CAPWAP_802_11_ICMP:
			printf("  action_type = FLOW_ACTION_RPA_CAPWAP_802_11_ICMP\n");
			print_rpa_head(rule);
			break;
		case FLOW_ACTION_RPA_CAPWAP_802_3_ICMP:
			printf("  action_type = FLOW_ACTION_RPA_CAPWAP_802_3_ICMP\n");
			print_rpa_head(rule);
			break;
		default:
			printf("  action_type = unknow\n");
			break;
	}
	printf("  forward port = %d\n", rule->rules.forward_port);
}

static void print_rule(rule_item_t* rule)
{
	if(rule == NULL)
		return;

	if((rule->rules.action_type == FLOW_ACTION_ICMP) ||
			(rule->rules.action_type == FLOW_ACTION_CAPWAP_802_11_ICMP) ||
			(rule->rules.action_type == FLOW_ACTION_CAPWAP_802_3_ICMP) ||
			(rule->rules.action_type == FLOW_ACTION_RPA_ICMP) ||
			(rule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_802_11_ICMP) ||
			(rule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_802_3_ICMP))
	{
		print_icmp_rule(rule);
		return;
	}

	if(cvm_ip_only_enable == FUNC_ENABLE)
	{
		printf("  five tuple: any-ip:any-port => %d.%d.%d.%d:any-port   any(tcp|udp)\n",
				IP_FMT(rule->rules.dip));
	}
	else
	{
		printf("  five tuple: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d    %s\n",
				IP_FMT(rule->rules.sip), rule->rules.sport,
				IP_FMT(rule->rules.dip), rule->rules.dport, PROTO_STR(rule->rules.protocol));	
	}

	if(rule->rules.rule_state == RULE_IS_LEARNING)
	{
		printf("  rule_state = LEARNING\t");
		if(acl_aging_check(&(rule->rules)) > 0)
			printf("age\n");
		else
			printf("new\n");
		return;	
	}

	if(rule->rules.action_type == FLOW_ACTION_DROP)
	{
		printf("  action_type = FLOW_ACTION_DROP\n");
		return;
	}
	if(rule->rules.action_type == FLOW_ACTION_TOLINUX)
	{
		printf("  action_type = FLOW_ACTION_TOLINUX\n");
		return;
	}

	printf("  smac: %02x-%02x-%02x-%02x-%02x-%02x", MAC_FMT(rule->rules.ether_shost));
	printf("  dmac: %02x-%02x-%02x-%02x-%02x-%02x\n", MAC_FMT(rule->rules.ether_dhost));
	printf("  eth protocol: %04x\n", rule->rules.ether_type);

	switch(rule->rules.action_type)
	{
		case FLOW_ACTION_ETH_FORWARD:
			printf("  action_type = FLOW_ACTION_ETH_FORWARD\n");
			break;
		case FLOW_ACTION_RPA_ETH_FORWARD:
			printf("  action_type = FLOW_ACTION_RPA_ETH_FORWARD\n");
			print_rpa_head(rule);
			break;
		case FLOW_ACTION_CAP802_3_FORWARD:
		case FLOW_ACTION_RPA_CAP802_3_FORWARD:
			printf("    tunnel_index: %d\n", rule->rules.tunnel_index);
			printf("    capwap use_num = %d\n", capwap_cache_bl[rule->rules.tunnel_index].use_num);
			printf("    capwap tunnel: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d  tos = 0x%02x\n",
					IP_FMT(capwap_cache_bl[rule->rules.tunnel_index].sip), capwap_cache_bl[rule->rules.tunnel_index].sport,
					IP_FMT(capwap_cache_bl[rule->rules.tunnel_index].dip), capwap_cache_bl[rule->rules.tunnel_index].dport,  
					capwap_cache_bl[rule->rules.tunnel_index].tos);
			if(rule->rules.action_type == FLOW_ACTION_CAP802_3_FORWARD)
			{
				printf("  action_type = FLOW_ACTION_CAP802_3_FORWARD\n");
			}
			else if(rule->rules.action_type == FLOW_ACTION_RPA_CAP802_3_FORWARD)
			{
				printf("  action_type = FLOW_ACTION_RPA_CAP802_3_FORWARD\n");
				print_rpa_head(rule);
			}
			break;
		case FLOW_ACTION_CAPWAP_FORWARD:
		case FLOW_ACTION_RPA_CAPWAP_FORWARD:
			
			printf("    tunnel_index: %d\n", rule->rules.tunnel_index);
			printf("    capwap use_num = %d\n", capwap_cache_bl[rule->rules.tunnel_index].use_num);
			printf("    capwap tunnel: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d  tos = 0x%02x\n",
					IP_FMT(capwap_cache_bl[rule->rules.tunnel_index].sip), capwap_cache_bl[rule->rules.tunnel_index].sport,
					IP_FMT(capwap_cache_bl[rule->rules.tunnel_index].dip), capwap_cache_bl[rule->rules.tunnel_index].dport,  
					capwap_cache_bl[rule->rules.tunnel_index].tos);
			if(rule->rules.action_type == FLOW_ACTION_CAPWAP_FORWARD)
			{
				printf("  action_type = FLOW_ACTION_CAPWAP_FORWARD\n");
			}
			else if(rule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_FORWARD)
			{
				printf("  action_type = FLOW_ACTION_RPA_CAPWAP_FORWARD\n");
				print_rpa_head(rule);
			}
			break;
		default:
			printf("  action_type = UNKNOWN\n");
			break;
	}
	printf("  forward port = %d\n", rule->rules.forward_port);
	printf("  time stamp: %lu\n", rule->rules.time_stamp);
	if(rule->rules.rule_state == RULE_IS_STATIC)
	{
		printf("  rule_state = STATIC\n");
	}
	else
	{
		if(rule->rules.rule_state == RULE_IS_LEARNED)
		{
			printf("  rule_state = LEARNED\t");
		}
		if(acl_aging_check(&(rule->rules)) > 0)
			printf("age\n");
		else
			printf("new\n");
	}
	printf("dsa_info: 0x%08lx\n", rule->rules.dsa_info);
	printf("out_type:0x%02x   out_tag:0x%02x   in_type:0x%02x   in_tag:0x%02x\n", rule->rules.out_ether_type, rule->rules.out_tag, rule->rules.in_ether_type, rule->rules.in_tag);
	printf("packet_wait = %d\n", rule->rules.packet_wait);
	printf("extend_index = 0x%lx\n", (int64_t)(rule->rules.extend_index));
	printf("action mask = 0x%x\n", rule->rules.action_mask);
	switch (rule->rules.direct_flag)
	{
		case DIRECTION_UP:
			printf("direct :up\n");
			break;
		case DIRECTION_DOWN:
			printf("direct :down\n");
			break;
		case DIRECTION_UP_DOWN:
			printf("direct :up-down\n");
			break;
		default:
			printf("direct :undefine\n");
			break;
	}

	if(rule->rules.nat_flag)
	{
        printf("nat_flag=%d\n", rule->rules.nat_flag);
        printf("nat_sip=0x%x\n", rule->rules.nat_sip);
        printf("nat_dip=0x%x\n", rule->rules.nat_dip);
        printf("nat_sport=0x%x\n", rule->rules.nat_sport);
        printf("nat_dport=0x%x\n", rule->rules.nat_dport);
	}

	/*add by wangjian for support pppoe 2013-3-15*/
	if (rule->rules.pppoe_flag == 1)
	{
		printf("pppoe_info:    session_id:%u \n", rule->rules.pppoe_session_id);
	}

	// TODO: print user info
}




/* show all none-empty rules in table */
int32_t cmd_acl_show_rule()
{
	rule_item_t  *rule ;
	uint32_t i;
	uint32_t static_rule_cnt = 0;
	uint32_t learning_rule_cnt = 0;
	uint32_t learned_rule_cnt = 0;
	uint32_t total_rule_cnt = 0;
	uint32_t quit_flag = 0;
	uint32_t page_flag = 0;


	for(i = 0; i < acl_static_tbl_size; i++)
	{
		rule = &acl_bucket_tbl[i];
		if(rule->rules.rule_state == RULE_IS_EMPTY)
		{
			if(rule->valid_entries == 0)
			{
				continue;
			}
			rule = rule->next;
		}
		else
		{
			if(rule->rules.rule_state == RULE_IS_STATIC)
			{
				static_rule_cnt++;
			}
			else if(rule->rules.rule_state == RULE_IS_LEARNED)
			{
				learned_rule_cnt++;
			}
			else if(rule->rules.rule_state == RULE_IS_LEARNING)
			{
				learning_rule_cnt++;
			}
			total_rule_cnt++;
			page_flag++;
			if(quit_flag == 0)
			{
				printf("----------------------------------------------------------------------------\n");
				printf("bucket idx = %d, list head, valid_entries = %d\n", i, rule->valid_entries);
				print_rule(rule);    
			}
			rule = rule->next;
		}

		while(rule != NULL)
		{
			if(rule->rules.rule_state == RULE_IS_STATIC)
			{
				static_rule_cnt++;
			}
			else if(rule->rules.rule_state == RULE_IS_LEARNED)
			{
				learned_rule_cnt++;
			}
			else if(rule->rules.rule_state == RULE_IS_LEARNING)
			{
				learning_rule_cnt++;
			}
			total_rule_cnt++;
			page_flag++;
			if(quit_flag == 0)
			{			
				printf("----------------------------------------------------------------------------\n");
				printf("bucket idx = %d, dynamic list node\n", i);
				print_rule(rule);
			}
			rule = rule->next;
		}

		if((quit_flag == 0) && (page_flag > 5))                  /*add by chengjing*/
		{
			page_flag = 0;
			uart_write_string(uart_index,"[C]ontinue, [Q]uit:");
			while(1)
			{
				int ch;

				ch = uart_read_byte_wait(uart_index);

				if(ch == 'c')
				{
					printf("\n");
					break;
				}
				else if(ch == 'q')
				{
					quit_flag=1;
					break;
				}
			}
		}
	}

	printf("----------------------------------------------------------------------------\n");
	printf("static : %d    ", static_rule_cnt);
	printf("learned : %d    ", learned_rule_cnt);
	printf("learning : %d\n", learning_rule_cnt);
	printf("total : %d\n", total_rule_cnt);

	return RETURN_OK;
}


/* show all none-empty rules in table */
int32_t cmd_acl_show_match_rule(		
		uint32_t ip_src, 
		uint32_t ip_dst, 
		uint16_t th_sport, 
		uint16_t th_dport, 
		uint8_t ip_p)
{
	rule_item_t  *rule ;
	uint32_t i;

	for(i = 0; i < acl_static_tbl_size; i++)
	{
		rule = &acl_bucket_tbl[i];
		if(rule->rules.rule_state == RULE_IS_EMPTY)
		{
			if(rule->valid_entries == 0)
			{
				continue;
			}
			rule = rule->next;
		}
		else
		{
			if((rule->rules.sip == ip_src) && 
					(rule->rules.dip == ip_dst) &&
					(rule->rules.sport == th_sport) && 
					(rule->rules.dport == th_dport) &&
					(rule->rules.protocol == ip_p))
			{
				printf("----------------------------------------------------------------------------\n");
				printf("bucket idx = %d, list head\n", i);
				print_rule(rule);
				return RETURN_OK;
			}
			rule = rule->next;				    
		}

		while(rule != NULL)
		{
			if((rule->rules.sip == ip_src) && 
					(rule->rules.dip == ip_dst) &&
					(rule->rules.sport == th_sport) && 
					(rule->rules.dport == th_dport) &&
					(rule->rules.protocol == ip_p))
			{
				printf("----------------------------------------------------------------------------\n");
				printf("bucket idx = %d, dynamic list node\n", i);
				print_rule(rule);
				return RETURN_OK;
			}
			rule = rule->next;
		}             		
	}

	printf("not find match rule!\n");
	return RETURN_OK;
}

int32_t cmd_acl_rule_set_car(		
		uint32_t ip_src, 
		uint32_t ip_dst, 
		uint16_t th_sport, 
		uint16_t th_dport, 
		uint8_t ip_p,
		uint8_t car_index)
{
#if 0
	rule_item_t  *rule ;
	uint32_t i;

	for(i = 0; i < acl_static_tbl_size; i++)
	{
		rule = &acl_bucket_tbl[i];
		if(rule->rules.rule_state == RULE_IS_EMPTY)
		{
			if(rule->valid_entries == 0)
			{
				continue;
			}
			rule = rule->next;
		}
		else
		{
			if(cvm_ip_only_enable == FUNC_ENABLE)
			{
				if(rule->rules.dip == ip_dst)
				{
					rule->rules.action_mask = FLOW_ACTION_METER;
					rule->rules.reserved1 = car_index;
					return RETURN_OK;
				}
			}
			else if((rule->rules.sip == ip_src) && 
					(rule->rules.dip == ip_dst) &&
					(rule->rules.sport == th_sport) && 
					(rule->rules.dport == th_dport) &&
					(rule->rules.protocol == ip_p))
			{
				rule->rules.action_mask = FLOW_ACTION_METER;
				rule->rules.reserved1 = car_index;
				return RETURN_OK;
			}
			else
				rule = rule->next;                  
		}

		while(rule != NULL)
		{
			if(cvm_ip_only_enable == FUNC_ENABLE)
			{
				if(rule->rules.dip == ip_dst)
				{
					rule->rules.action_mask = FLOW_ACTION_METER;
					rule->rules.reserved1 = car_index;
					return RETURN_OK;
				}
			}
			else if((rule->rules.sip == ip_src) && 
					(rule->rules.dip == ip_dst) &&
					(rule->rules.sport == th_sport) && 
					(rule->rules.dport == th_dport) &&
					(rule->rules.protocol == ip_p))
			{
				rule->rules.action_mask = FLOW_ACTION_METER;
				rule->rules.reserved1 = car_index;
				return RETURN_OK;
			}
			else
				rule = rule->next;
		}          
	}

	printf("no match rule\n");
	return RETURN_ERROR;
#endif
	return RETURN_OK;
}





/* show all none-empty rules in table */
void cmd_acl_show_learned_rule()
{
	rule_item_t  *rule ;
	uint32_t i;
	uint32_t learned_rule_cnt = 0;
	uint32_t quit_flag = 0; 
	uint32_t page_flag = 0;

	for(i = 0; i < acl_static_tbl_size; i++)
	{
		rule = &acl_bucket_tbl[i];
		if(rule->rules.rule_state == RULE_IS_EMPTY)
		{
			if(rule->valid_entries == 0)
			{
				continue;
			}
			rule = rule->next;
		}
		else
		{
			if(rule->rules.rule_state == RULE_IS_LEARNED)
			{
				learned_rule_cnt++;
				page_flag++;
				if(quit_flag == 0)
				{
					printf("----------------------------------------------------------------------------\n");
					printf("bucket idx = %d, list head\n", i);
					print_rule(rule);
				}
			}
			rule = rule->next;
		}

		while(rule != NULL)
		{
			if(rule->rules.rule_state == RULE_IS_LEARNED)
			{
				learned_rule_cnt++;
				page_flag++;
				if(quit_flag == 0)
				{
					printf("----------------------------------------------------------------------------\n");
					printf("bucket idx = %d, dynamic list node\n", i);
					print_rule(rule);
				}
			}
			rule = rule->next;
		}
		if((quit_flag == 0) && (page_flag > 5))                 
		{
			page_flag = 0;
			uart_write_string(uart_index,"[C]ontinue, [Q]uit:");
			while(1)
			{
				int ch;
				ch = uart_read_byte_wait(uart_index);
				if(ch == 'c')
				{
					printf("\n");
					break;
				}
				else if(ch == 'q')
				{
					quit_flag=1;
					break;
				}
			}
		}
	}

	printf("----------------------------------------------------------------------------\n");
	printf("learned : %d\n", learned_rule_cnt);
}


/* show all none-empty rules in table */
void cmd_acl_show_static_rule()
{
	rule_item_t  *rule ;
	uint32_t i;
	uint32_t static_rule_cnt = 0;
	uint32_t quit_flag=0; 
	uint32_t page_flag = 0;

	for(i = 0; i < acl_static_tbl_size; i++)
	{
		rule = &acl_bucket_tbl[i];
		if(rule->rules.rule_state == RULE_IS_EMPTY)
		{
			if(rule->valid_entries == 0)
			{
				continue;
			}
			rule = rule->next;
		}
		else
		{
			if(rule->rules.rule_state == RULE_IS_STATIC)
			{
				static_rule_cnt++;
				page_flag++;
				if(quit_flag == 0)
				{
					printf("----------------------------------------------------------------------------\n");
					printf("bucket idx = %d, list head\n", i);
					print_rule(rule);
				}
			}
			rule = rule->next;
		}

		while(rule != NULL)
		{
			if(rule->rules.rule_state == RULE_IS_STATIC)
			{
				static_rule_cnt++;
				page_flag++;
				if(quit_flag == 0)
				{
					printf("----------------------------------------------------------------------------\n");
					printf("bucket idx = %d, dynamic list node\n", i);
					print_rule(rule);
				}
			}
			rule = rule->next;
		}

		if((quit_flag == 0) && (page_flag > 5))                 
		{
			page_flag = 0;
			uart_write_string(uart_index,"[C]ontinue, [Q]uit:");
			while(1)
			{
				int ch;
				ch = uart_read_byte_wait(uart_index);
				if(ch == 'c')
				{
					printf("\n");
					break;
				}
				else if(ch == 'q')
				{
					quit_flag=1;
					break;
				}
			}
		}
	}

	printf("----------------------------------------------------------------------------\n");
	printf("static : %d\n", static_rule_cnt);
}


static int32_t cmd_acl_show_rule_sum()
{
	uint64_t s_tbl_used_rule = 0;
	uint64_t s_tbl_aged_rule = 0;
	uint64_t s_tbl_learned_rule = 0;
	uint64_t s_tbl_learning_rule = 0;
	uint64_t s_tbl_static_rule = 0;

	uint64_t d_tbl_used_rule = 0;
	uint64_t d_tbl_aged_rule = 0;
	uint64_t d_tbl_learned_rule = 0;
	uint64_t d_tbl_learning_rule = 0;
	uint64_t d_tbl_static_rule = 0;

	uint64_t i = 0;
	rule_item_t * s_tbl_rule = NULL;
	rule_item_t * d_tbl_rule = NULL;

	for(i = 0; i < acl_static_tbl_size; i++)
	{
		s_tbl_rule = (rule_item_t *)acl_bucket_tbl+i;

		if(s_tbl_rule->rules.rule_state == RULE_IS_EMPTY)
		{
			if(NULL == s_tbl_rule->next)
				continue;             
		}
		else
		{
			switch(s_tbl_rule->rules.rule_state)
			{
				case RULE_IS_LEARNED:
					s_tbl_learned_rule++;
					break;
				case RULE_IS_LEARNING:
					s_tbl_learning_rule++;
					break;
				case RULE_IS_STATIC:
					s_tbl_static_rule++;
					break;    
				default:
					printf("error rule state %d!\n", s_tbl_rule->rules.rule_state);
					return RETURN_ERROR;
			}
			s_tbl_used_rule++;
			if(acl_aging_check(&(s_tbl_rule->rules)) > 0)
			{
				s_tbl_aged_rule++;
			}
		}

		/* dynamic info */
		d_tbl_rule = s_tbl_rule->next;
		while(d_tbl_rule != NULL)
		{
			switch(d_tbl_rule->rules.rule_state)
			{
				case RULE_IS_LEARNED:
					d_tbl_learned_rule++;
					break;
				case RULE_IS_LEARNING:
					d_tbl_learning_rule++;
					break;
				case RULE_IS_STATIC:
					d_tbl_static_rule++;
					break;    
				default:
					printf("error rule state %d!\n", d_tbl_rule->rules.rule_state);
					return RETURN_ERROR;
			}
			d_tbl_used_rule++;
			if(acl_aging_check(&(d_tbl_rule->rules)) > 0)
			{
				d_tbl_aged_rule++;
			}

			d_tbl_rule = d_tbl_rule->next;
		}
	}


	/* capwap table info */
	uint32_t cw_tbl_used = 0;
	uint32_t cw_tbl_802_3_num = 0;
	uint32_t cw_tbl_802_11_num = 0;
	union capwap_hd* cw_hdr;

	for(i = 0; i < MAX_CAPWAP_CACHE_NUM; i++)
	{ 
		if(capwap_cache_bl[i].use_num != 0)
		{
			cw_tbl_used++;
			cw_hdr = (union capwap_hd*)(capwap_cache_bl[i].cw_hd);
			if(cw_hdr->m_t == 0)
			{
				cw_tbl_802_3_num++;
			}            
			else if(cw_hdr->m_wbid == 1)
			{
				cw_tbl_802_11_num++;
			}
		}
	}

	/* print result */
	printf("==============================================\n");
	printf("acl_bucket_tbl count:\n");
	printf("  entry num: %d\n", acl_static_tbl_size);
	printf("  used num: %lu\n", s_tbl_used_rule);
	printf("  free num: %lu\n", acl_static_tbl_size - s_tbl_used_rule);
	printf("  use rate: %02f%%\n", (float)s_tbl_used_rule / (float)acl_static_tbl_size *100);
	printf("  new rules num: %lu\n", s_tbl_used_rule - s_tbl_aged_rule);
	printf("  aged rules num: %lu\n", s_tbl_aged_rule);
	printf("  learned rules num: %lu\n", s_tbl_learned_rule);
	printf("  learning rules num: %lu\n", s_tbl_learning_rule);
	printf("  static insert rules num: %lu\n", s_tbl_static_rule);

	printf("==============================================\n");
	printf("acl_dynamic_tbl count:\n");
	printf("  entry num: %d\n", acl_dynamic_tbl_size);
	printf("  used num: %lu, \t compare used num: %lu\n", d_tbl_used_rule, rule_cnt_info.dynamic_rule_cnt);
	printf("  free num: %lu\n", acl_dynamic_tbl_size - d_tbl_used_rule);
	printf("  use rate: %02f%%\n", (float)d_tbl_used_rule / (float)acl_dynamic_tbl_size *100);
	printf("  new rules num: %lu\n", d_tbl_used_rule - d_tbl_aged_rule);
	printf("  aged rules num: %lu\n", d_tbl_aged_rule);
	printf("  learned rules num: %lu\n", d_tbl_learned_rule);
	printf("  learning rules num: %lu\n", d_tbl_learning_rule);
	printf("  static insert rules num: %lu\n", d_tbl_static_rule);

	printf("==============================================\n");
	printf("capwap cache table:\n");
	printf("  capwap table entry num: %d\n", MAX_CAPWAP_CACHE_NUM);
	printf("  capwap table used entry num: %d\n", cw_tbl_used);
	printf("  capwap table 802.3 entry num: %d\n", cw_tbl_802_3_num);
	printf("  capwap table 802.11 entry num: %d\n", cw_tbl_802_11_num);
	printf("==============================================\n");

	return RETURN_OK;
}

/* show capwap table */
static void cmd_acl_show_cw()
{
	uint32_t i=0, j=0;
	uint32_t total_count = 0;
	uint32_t quit_flag=0; 
	uint32_t page_flag = 0;

	printf("\ncapwap cache table:\n");
	printf("==============================================\n");

	for(i = 0; i < MAX_CAPWAP_CACHE_NUM; i++)
	{ 
		if(capwap_cache_bl[i].use_num != 0)
		{
			total_count++;
			page_flag++;
			if(quit_flag == 0)
			{
				printf("idx = %d    use_number = %d\n", i,  capwap_cache_bl[i].use_num);
				printf("sip=0x%x, dip=0x%x, sport=0x%x, dport=0x%x, tos=0x%x\n", 
						capwap_cache_bl[i].sip, 
						capwap_cache_bl[i].dip,
						capwap_cache_bl[i].sport,
						capwap_cache_bl[i].dport,
						capwap_cache_bl[i].tos);

				printf("capwap header:\n");
				for(j = 0; j < CW_H_LEN; j++)
				{
					printf("0x%x  ", capwap_cache_bl[i].cw_hd[j]);
				}
				printf("\n\n");
			}
		}

		if((quit_flag == 0) && (page_flag > 5))                 
		{
			page_flag = 0;
			uart_write_string(uart_index,"[C]ontinue, [Q]uit:");
			while(1)
			{
				int ch;
				ch = uart_read_byte_wait(uart_index);
				if(ch == 'c')
				{
					printf("\n");
					break;
				}
				else if(ch == 'q')
				{
					quit_flag=1;
					break;
				}
			}
		}
	}

	printf("==============================================\n");
	printf("total count : %d\n", total_count);
}


/**
 * Description:
 *  parse a packet, and fill the rule, this func is just for testing.
 *
 * Parameter:
 *  pkt_ptr: a packet made by construct_pkt()
 *  rule: rule
 *  pkt_type: packet type
 *  action_type: rule action type
 *  is_static: static insert or self-learning
 *
 * Return:
 *  0: Successfully
 *  -1: Failed
 *
 */
int32_t fill_rule(
		rule_item_t* rule, 
		five_tuple_t* tuple,
		uint8_t action_type,
		uint16_t forward_port,
		uint8_t rule_state)
{
	uint16_t cw_idx = 0;

	memset(&rule->rules, 0, sizeof(rule_param_t));

	if((action_type != FLOW_ACTION_TOLINUX) &&
			(action_type != FLOW_ACTION_ETH_FORWARD) &&
			(action_type != FLOW_ACTION_CAP802_3_FORWARD) &&
			(action_type != FLOW_ACTION_CAPWAP_FORWARD) &&
			(action_type != FLOW_ACTION_DROP))
	{
		printf("action_type error\n");
		return RETURN_ERROR;
	}

	/* L3-4 header, the HASH key */
	rule->rules.dip = tuple->ip_dst;
	rule->rules.sip = tuple->ip_src;
	rule->rules.protocol= tuple->ip_p;
	rule->rules.dport= tuple->th_dport;
	rule->rules.sport= tuple->th_sport;

	/* rule information */
	rule->rules.action_type = action_type;
	rule->rules.rule_state = rule_state;
	rule->rules.time_stamp = get_sec();

	if((action_type == FLOW_ACTION_TOLINUX) || (action_type == FLOW_ACTION_DROP))
	{
		return RETURN_OK;
	}

	/* L2_header information */
	rule->rules.ether_dhost[0] = 0xbc; rule->rules.ether_dhost[1] = 0x30; rule->rules.ether_dhost[2] = 0x5b; 
	rule->rules.ether_dhost[3] = 0xc1; rule->rules.ether_dhost[4] = 0xc5; rule->rules.ether_dhost[5] = 0xac;
	rule->rules.ether_shost[0] = 0x00; rule->rules.ether_shost[1] = 0x1f; rule->rules.ether_shost[2] = 0x64; 
	rule->rules.ether_shost[3] = 0x12; rule->rules.ether_shost[4] = 0x02; rule->rules.ether_shost[5] = 0xb0;
	rule->rules.ether_type = 0x0800;

	if(action_type == FLOW_ACTION_CAP802_3_FORWARD)
	{
		cvmx_spinlock_lock(&capwap_cache_bl_lock);
		for(cw_idx = 0; cw_idx < MAX_CAPWAP_CACHE_NUM; cw_idx++)
		{
			if(capwap_cache_bl[cw_idx].use_num == 0)
			{
				capwap_cache_bl[cw_idx].use_num++;
				rule->rules.tunnel_index = cw_idx;
				// TODO: insert capwap table
				break;
			}
		}
		cvmx_spinlock_unlock(&capwap_cache_bl_lock);

		if(cw_idx == MAX_CAPWAP_CACHE_NUM)
		{
			printf("capwap table is full!\n");
			return RETURN_ERROR;
		}

		/* tunnel internal L2 header */
		rule->rules.acl_tunnel_eth_header_dmac[0] = 0xbc; rule->rules.acl_tunnel_eth_header_dmac[1] = 0x30; 
		rule->rules.acl_tunnel_eth_header_dmac[2] = 0x5b; rule->rules.acl_tunnel_eth_header_dmac[3] = 0xc1; 
		rule->rules.acl_tunnel_eth_header_dmac[4] = 0xc5; rule->rules.acl_tunnel_eth_header_dmac[5] = 0xac;
		rule->rules.acl_tunnel_eth_header_smac[0] = 0x00; rule->rules.acl_tunnel_eth_header_smac[1] = 0x1f; 
		rule->rules.acl_tunnel_eth_header_smac[2] = 0x64; rule->rules.acl_tunnel_eth_header_smac[3] = 0x12; 
		rule->rules.acl_tunnel_eth_header_smac[4] = 0x02; rule->rules.acl_tunnel_eth_header_smac[5] = 0xb0;
		rule->rules.acl_tunnel_eth_header_ether = 0x0800;

	}
	else if(action_type == FLOW_ACTION_CAPWAP_FORWARD)
	{
		uint8_t * wifi_addr = NULL;
		uint8_t * cw_hd = NULL;

		cvmx_spinlock_lock(&capwap_cache_bl_lock);
		for(cw_idx = 0; cw_idx < MAX_CAPWAP_CACHE_NUM; cw_idx++)
		{
			if(capwap_cache_bl[cw_idx].use_num == 0)
			{
				capwap_cache_bl[cw_idx].use_num++;
				rule->rules.tunnel_index = cw_idx;

				capwap_cache_bl[cw_idx].dip = 0x0a01496f;
				capwap_cache_bl[cw_idx].sip = 0x0a014964;
				capwap_cache_bl[cw_idx].dport = 0x8001;
				capwap_cache_bl[cw_idx].sport = 0x147f;
				capwap_cache_bl[cw_idx].tos = 0x00;

				cw_hd = capwap_cache_bl[cw_idx].cw_hd;
				cw_hd[0] = 0x00;cw_hd[1] = 0x60;cw_hd[2] = 0x03;cw_hd[3] = 0x20;
				cw_hd[4] = 0x00;cw_hd[5] = 0x00;cw_hd[6] = 0x00;cw_hd[7] = 0x00;
				cw_hd[8] = 0x01;cw_hd[9] = 0x00;cw_hd[10] = 0x00;cw_hd[11] = 0x00;
				cw_hd[12] = 0x37;cw_hd[13] = 0x00;cw_hd[14] = 0x00;cw_hd[15] = 0x00;
				break;
			}
		}
		cvmx_spinlock_unlock(&capwap_cache_bl_lock);

		if(cw_idx == MAX_CAPWAP_CACHE_NUM)
		{
			printf("capwap table is full!\n");
			return RETURN_ERROR;
		}

		/* tunnel 802.11 header */
		rule->rules.acl_tunnel_wifi_header_fc[0] = 0x08;
		rule->rules.acl_tunnel_wifi_header_fc[1] = 0x02;
		rule->rules.acl_tunnel_wifi_header_qos[0] = 0;
		rule->rules.acl_tunnel_wifi_header_qos[1] = 0;
		wifi_addr = rule->rules.acl_tunnel_wifi_header_addr;
		wifi_addr[0] = 0x00;wifi_addr[1] = 0x1e;wifi_addr[2] = 0x4c;wifi_addr[3] = 0x62;wifi_addr[4] = 0xcf;wifi_addr[5] = 0x31;
		wifi_addr[6] = 0x00;wifi_addr[7] = 0x1f;wifi_addr[8] = 0x64;wifi_addr[9] = 0xe2;wifi_addr[10] = 0x57;wifi_addr[11] = 0x62;
		wifi_addr[12] = 0x00;wifi_addr[13] = 0x1f;wifi_addr[14] = 0x64;wifi_addr[15] = 0x12;wifi_addr[16] = 0x02;wifi_addr[17] = 0xb0;

	}

	rule->rules.forward_port = forward_port;
	return RETURN_OK;
}


/* insert a rule by uart command, just for debug and test */
int32_t cmd_acl_insert_rule(
		uint32_t ip_src, 
		uint32_t ip_dst, 
		uint16_t th_sport, 
		uint16_t th_dport, 
		uint8_t ip_p,
		uint8_t action_type,
		uint16_t forward_port,
		uint8_t rule_state)
{
	five_tuple_t tuple;
	rule_item_t  *rule ;
	rule_item_t  *head_rule=NULL ;
	rule_item_t  *free_rule  = NULL; /*the first free bucket position*/
	cvmx_spinlock_t          *head_lock;

	tuple.ip_src = ip_src;
	tuple.ip_dst = ip_dst;
	tuple.th_sport = th_sport;
	tuple.th_dport = th_dport;
	tuple.ip_p = ip_p;

	/*look up ACL Table and get the bucket*/
	hash(ip_dst, ip_src, ip_p, th_dport, th_sport);
	rule = CASTPTR(rule_item_t, cvmx_scratch_read64(CVM_SCR_ACL_CACHE_PTR));

	head_rule = rule;
	cvmx_spinlock_lock(&rule->lock);
	head_lock = &rule->lock;


	/*if the first bucket is empty and there are no more buckets, then insert current flow*/
	if(head_rule->rules.rule_state == RULE_IS_EMPTY)
	{
		if(head_rule->valid_entries == 0)
		{
			/*表项为空，直接插入表项*/
			if(fill_rule(head_rule, &tuple, action_type, forward_port, rule_state) == RETURN_ERROR)
			{
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;
			}
			cvmx_spinlock_unlock(head_lock);
			return RETURN_OK;
		}
		else /*first bucket is empty but with other buckets*/
		{
			free_rule = head_rule; /*record current free bucket position*/

			if(head_rule->next != NULL)
			{
				rule = head_rule->next;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_insert_static_rule: Should never come to here file%s, line %d, rule=0x%p, num=0x%d,next=0x%p.\r\n",__FILE__, __LINE__,rule,rule->valid_entries,rule->next);	
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;
			}
		}
	}


	while(1)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
				"acl_insert_static_rule: lookup the table, rule=0x%p.\r\n",rule);	

		if ((rule->rules.dip == ip_dst) && 
				(rule->rules.sip == ip_src) &&
				(rule->rules.dport == th_dport) &&
				(rule->rules.sport == th_sport) &&
				(rule->rules.protocol == ip_p)) 
		{
#if 0		
			if((rule->rules.rule_state == RULE_IS_STATIC) || (rule->rules.rule_state == RULE_IS_CMD_SET))
			{

				/*策略相同，重复插入*/
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;
			}
			else
			{
				/*插入的静态表与自学习表项冲突*/
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;    
			}
#endif
			/* replace old rule */
			if(fill_rule(rule, &tuple, action_type, forward_port, rule_state) == RETURN_ERROR)
			{
				if(rule != head_rule)
				{
					cvmx_free(rule);
					rule = NULL;
					atomic_dec64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  /* -1, add by zhaohan */
				}
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;
			}
			++head_rule->valid_entries;
			cvmx_spinlock_unlock(head_lock);
			return RETURN_OK;
		}
		if(rule->next != NULL)
			rule = rule->next;
		else
			break;
	}

	if (free_rule ==  NULL) 
	{
		/*没找到空白处，新申请一条*/
		rule->next = (rule_item_t *)cvmx_malloc(rule_arena, sizeof(rule_item_t));

		if(rule->next == NULL)
		{
			cvmx_fau_atomic_add64(CVM_FAU_ALLOC_RULE_FAIL, 1);	
			cvmx_spinlock_unlock(head_lock);
			return RETURN_ERROR;
		}
		memset(rule->next, 0, sizeof(rule_item_t));
		free_rule= rule->next;
		atomic_add64_nosync(&(rule_cnt_info.dynamic_rule_cnt)); // add by zhaohan
	}

	/*插入静态表项到free_rule处*/
	if(fill_rule(free_rule, &tuple, action_type, forward_port, rule_state) == RETURN_ERROR)
	{
		if(free_rule != head_rule)
		{
			cvmx_free(free_rule);
			free_rule = NULL;
			atomic_dec64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  /* -1, add by zhaohan */
		}
		cvmx_spinlock_unlock(head_lock);
		return RETURN_ERROR;
	}
	++head_rule->valid_entries;

	cvmx_spinlock_unlock(head_lock);
	return RETURN_OK;
}


/* delete a rule by uart command, just for debug and test */
int32_t cmd_acl_delete_rule(uint32_t ip_src, uint32_t ip_dst, uint16_t th_sport, uint16_t th_dport, uint8_t ip_p)
{
	rule_item_t  *rule ;
	rule_item_t  *head_rule=NULL ;
	rule_item_t  *pre_rule=NULL ;	
	//rule_item_t  *free_rule  = NULL; /*the first free bucket position*/
	cvmx_spinlock_t          *head_lock;


	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_cmd_delete_rule: packet-fiveTuple----dip=0x%x, sip=0x%x,dport=%d, sport=%d,proto=%d.  \r\n",
			ip_dst,ip_src,th_dport,th_sport,ip_p);

	/*look up ACL Table and get the bucket*/
	hash(ip_dst, ip_src, ip_p, th_dport, th_sport);
	rule = CASTPTR(rule_item_t, cvmx_scratch_read64(CVM_SCR_ACL_CACHE_PTR));

	head_rule = rule;
	pre_rule = rule;
	head_lock = &rule->lock;

	if(head_rule->valid_entries== 0)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_ERROR,
				"acl_cmd_delete_rule: ERROR, the bucket is empty, can not delete\n");
		return RETURN_ERROR;
	}

	cvmx_spinlock_lock(&rule->lock);

	while(1)
	{		   
		if((rule->rules.dip == ip_dst) && (rule->rules.sip == ip_src) &&
				(rule->rules.dport == th_dport) &&(rule->rules.sport == th_sport) &&(rule->rules.protocol == ip_p)) 
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"acl_cmd_delete_rule: find the rule,rule=0x%p\n",rule);									

			/*不是首条规则，则释放*/
			if (head_rule != rule)
			{		
				pre_rule->next = rule->next;	
				cvmx_free(rule);
				rule = NULL;
				atomic_dec64_nosync(&(rule_cnt_info.dynamic_rule_cnt));
			}
			else
			{
				int tmp = 0;
				uint64_t *ptmp = (uint64_t *)head_rule;
				/*首条rule，清零*/
				while(tmp < 16) /*128 Bytes*/
				{
					*ptmp = 0;
					ptmp = ptmp + 1;
					tmp++;
				}	
			}
			cvmx_spinlock_unlock(head_lock);
			return RETURN_OK;
		}			 

		if(rule->next != NULL)
		{
			pre_rule = rule;
			rule = rule->next;
		}
		else
			break;
	}

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_ERROR,
			"acl_cmd_delete_rule: ERROR, the bucket is empty, can not delete\n");
	cvmx_spinlock_unlock(head_lock);
	return RETURN_ERROR;
}



/* acl command handler */
int32_t cmd_acl(int argc, char *argv[])
{
	uint32_t ip[4];
	uint8_t sip[4];
	uint8_t dip[4];
	uint16_t sport = 0;
	uint16_t dport = 0;
	uint8_t protocol = 0;
	uint16_t forward_port = 0;
	uint8_t rule_state = RULE_IS_STATIC;
	uint8_t action_type = FLOW_ACTION_ETH_FORWARD;

	if (argc < 2 )
	{	
		printf("acl insert {$sip $dip $sport $dport $protocol $action_type ($fwd_port)} \n");
		printf("acl delete {$sip $dip $sport $dport $protocol} \n");
		printf("acl show {rule | cw | cnt | sum}\n");
		printf("acl show match {$sip $dip $sport $dport $protocol} \n");
		printf("acl cnt {show | clear} \n");
		printf("acl insall \n");
		printf("acl set 5-tuple\n");
		return CMD_ERR_TOO_FEW_ARGC;
	}
	if (argv[1] ==NULL)
		return CMD_ERR_NOT_MATCH;

	/* insert acl static rule by 5-tuple */
	if(strcmp(argv[1] , "insert") == 0)
	{
		if(argc < 8)
		{
			printf("acl insert {$sip $dip $sport $dport $protocol $action_type ($fwd_port)} \n");
			return CMD_ERR_TOO_FEW_ARGC;
		}
		if((argv[2] == NULL) || (argv[3] == NULL) || (argv[4] == NULL) || (argv[5] == NULL) || 
				(argv[6] == NULL) || (argv[7] == NULL))
		{
			return CMD_ERR_NOT_MATCH;
		}

		/* src ip */
		if(strcmp(argv[2] , "*") == 0)
		{
			*(uint32_t *)sip = gl_five_tuple.ip_src;
		}
		else
		{
			/* check ip format*/
			if(!is_valid_ip(argv[2]))
			{
				printf("src ip format error\n");
				return CMD_ERR_NOT_MATCH;
			}
			if(sscanf(argv[2], "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == -1)
			{
				return CMD_ERR_NOT_MATCH;
			}
			sip[0] = (uint8_t)ip[0];
			sip[1] = (uint8_t)ip[1];
			sip[2] = (uint8_t)ip[2];
			sip[3] = (uint8_t)ip[3];
		}

		/* dest ip */
		if(strcmp(argv[3] , "*") == 0)
		{
			*(uint32_t *)dip = gl_five_tuple.ip_dst;
		}
		else
		{
			if(!is_valid_ip(argv[3]))
			{
				printf("dst ip format error\n");
				return CMD_ERR_NOT_MATCH;
			}		
			if(sscanf(argv[3], "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == -1)
			{
				return CMD_ERR_NOT_MATCH;
			}
			dip[0] = (uint8_t)ip[0];
			dip[1] = (uint8_t)ip[1];
			dip[2] = (uint8_t)ip[2];
			dip[3] = (uint8_t)ip[3];
		}

		/* src port */
		if(strcmp(argv[4] , "*") == 0)
			sport = gl_five_tuple.th_sport;
		else
			sport = (uint16_t)atoi(argv[4]);

		/* dest port */
		if(strcmp(argv[5] , "*") == 0)
			dport = gl_five_tuple.th_dport;
		else
			dport = (uint16_t)atoi(argv[5]);

		/* protocol */
		if(strcmp(argv[6] , "*") == 0)
			protocol = gl_five_tuple.ip_p;
		else if(strcmp(argv[6] , "tcp") == 0)
			protocol = 6;
		else if(strcmp(argv[6] , "udp") == 0)
			protocol = 17;
		else
		{
			printf("protocol %s unknow! {tcp | udp}\n", argv[6]);
			return CMD_ERR_NOT_MATCH;
		}

		if(strcmp(argv[7], "drop") == 0)
		{
			action_type= FLOW_ACTION_DROP;
		}
		else if(strcmp(argv[7], "control") == 0)
		{
			action_type= FLOW_ACTION_TOLINUX;
		}
		else if(strcmp(argv[7], "eth") == 0)
		{
			action_type= FLOW_ACTION_ETH_FORWARD;
		}
		else if((strcmp(argv[7], "cw") == 0) || (strcmp(argv[8], "cw802.11") == 0))
		{
			action_type= FLOW_ACTION_CAPWAP_FORWARD;
		}
		else if(strcmp(argv[7], "cw802.3") == 0)
		{
			action_type= FLOW_ACTION_CAP802_3_FORWARD;
		}
		else
		{
			printf("action_type = %s  is error! {eth | cw802.11 | cw802.3 | drop | control}\n", argv[7]);
			return CMD_ERR_NOT_MATCH;
		}

		if((action_type == FLOW_ACTION_ETH_FORWARD) ||
				(action_type == FLOW_ACTION_CAPWAP_FORWARD) ||
				(action_type == FLOW_ACTION_CAPWAP_FORWARD))
		{
			if(argv[8] == NULL)
			{
				printf("acl insert {$sip $dip $sport $dport $protocol $action_type ($fwd_port)} \n");
				return CMD_ERR_NOT_MATCH;
			}
			forward_port = (uint16_t)atoi(argv[8]);
		}

		if(cmd_acl_insert_rule(
					*((uint32_t*)sip), 
					*((uint32_t*)dip), 
					sport, 
					dport, 
					protocol, 
					action_type,
					forward_port,
					rule_state
				      )
				== RETURN_ERROR)
		{
			printf("cmd_acl_insert_rule fail\n");
			return CMD_EXEC_ERROR;
		}

	}
	/* delete acl rule by 5-tuple */
	else if((strcmp(argv[1], "delete") == 0) || (strcmp(argv[1], "del") == 0))
	{
		if(argc < 7)
		{
			printf("acl insert {$sip $dip $sport $dport $protocol} \n");
			return CMD_ERR_TOO_FEW_ARGC;
		}
		if((argv[2] == NULL) || (argv[3] == NULL) || (argv[4] == NULL) || (argv[5] == NULL) || (argv[6] == NULL))
		{
			return CMD_ERR_NOT_MATCH;
		}

		/* check ip format*/
		if(!is_valid_ip(argv[2]))
		{
			printf("src ip format error\n");
			return CMD_ERR_NOT_MATCH;
		}
		if(!is_valid_ip(argv[3]))
		{
			printf("dst ip format error\n");
			return CMD_ERR_NOT_MATCH;
		}

		if(sscanf(argv[2], "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == -1)
		{
			return CMD_ERR_NOT_MATCH;
		}
		sip[0] = (uint8_t)ip[0];
		sip[1] = (uint8_t)ip[1];
		sip[2] = (uint8_t)ip[2];
		sip[3] = (uint8_t)ip[3];

		if(sscanf(argv[3], "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == -1)
		{
			return CMD_ERR_NOT_MATCH;
		}
		dip[0] = (uint8_t)ip[0];
		dip[1] = (uint8_t)ip[1];
		dip[2] = (uint8_t)ip[2];
		dip[3] = (uint8_t)ip[3];

		sport = (uint16_t)atoi(argv[4]);
		dport = (uint16_t)atoi(argv[5]);

		/* check protocol */
		if(strcmp(argv[6] , "tcp") == 0)
			protocol = 6;
		else if(strcmp(argv[6] , "udp") == 0)
			protocol = 17;
		else
		{
			printf("protocol %s unknow! {tcp | udp}\n", argv[6]);
			return CMD_ERR_NOT_MATCH;
		}

		printf("src ip = %d.%d.%d.%d\n", sip[0], sip[1], sip[2], sip[3]);
		printf("dst ip = %d.%d.%d.%d\n", dip[0], dip[1], dip[2], dip[3]);
		printf("src port = %d\n", sport);
		printf("dst port = %d\n", dport);
		printf("protocol = %s\n", PROTO_STR(protocol));

		if(cmd_acl_delete_rule(*((uint32_t*)sip), *((uint32_t*)dip), sport, dport, protocol) == RETURN_ERROR)
		{
			printf("acl_cmd_delete_static_rule fail\n");
			return CMD_EXEC_ERROR;
		}		
	}

	/* show all none-empty rules in table */
	else if(strcmp(argv[1] , "show") == 0)
	{
		if(argc < 3)
		{
			printf("acl show {rule | cw | cnt | sum | static | learned} \n");
			return CMD_ERR_TOO_FEW_ARGC;
		}
		if(strcmp(argv[2], "rule") == 0)
		{
			cmd_acl_show_rule();  
		}
		else if(strcmp(argv[2], "static") == 0)
		{
			cmd_acl_show_static_rule();
		}
		else if(strcmp(argv[2], "learned") == 0)
		{
			cmd_acl_show_learned_rule();
		}
		else if(strcmp(argv[2], "cw") == 0)
		{
			cmd_acl_show_cw();  
		}
		else if(strcmp(argv[2], "cnt") == 0)
		{
			printf("---------------------------------------\n");
			printf("acl rule statistics information:\n");
			printf("rule age number = %lu\n", rule_cnt_info.rule_age_cnt);
			printf("rule hit number = %lu\n", rule_cnt_info.rule_hit_cnt);
			printf("rule miss number = %lu\n", rule_cnt_info.rule_miss_cnt);
			printf("fwd eth cnt : %lu\n", rule_cnt_info.fwd_eth_cnt);
			printf("fwd capwap 802.11 cnt : %lu\n", rule_cnt_info.fwd_cw_cnt);
			printf("fwd capwap 802.3 cnt : %lu\n", rule_cnt_info.fwd_cw_802_3_cnt);

			printf("fpa pool 0 available: %lu\n", cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(0)));
			printf("fpa pool 1 available: %lu\n", cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(1)));
			printf("fpa pool 2 available: %lu\n", cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(2)));
			printf("fpa pool 3 available: %lu\n", cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(3)));
			printf("fpa pool 4 available: %lu\n", cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(4)));
			printf("fpa pool 5 available: %lu\n", cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(5)));
			printf("fpa pool 6 available: %lu\n", cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(6)));
			printf("fpa pool 7 available: %lu\n", cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(7)));
			printf("\n");  
		}
		else if(strcmp(argv[2], "sum") == 0)
		{
			cmd_acl_show_rule_sum();
		}
		else if(strcmp(argv[2], "match") == 0)
		{
			if(argc < 7)
			{
				printf("acl show match {$sip $dip $sport $dport $protocol} \n");
				return CMD_ERR_TOO_FEW_ARGC;
			}
			if((argv[3] == NULL) || (argv[4] == NULL) || (argv[5] == NULL) || 
					(argv[6] == NULL) || (argv[7] == NULL))
			{
				return CMD_ERR_NOT_MATCH;
			}
			if(!is_valid_ip(argv[3]))
			{
				printf("src ip format error\n");
				return CMD_ERR_NOT_MATCH;
			}
			if(!is_valid_ip(argv[4]))
			{
				printf("dst ip format error\n");
				return CMD_ERR_NOT_MATCH;
			}		

			if(sscanf(argv[3], "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == -1)
			{
				return CMD_ERR_NOT_MATCH;
			}
			sip[0] = (uint8_t)ip[0];
			sip[1] = (uint8_t)ip[1];
			sip[2] = (uint8_t)ip[2];
			sip[3] = (uint8_t)ip[3];

			if(sscanf(argv[4], "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == -1)
			{
				return CMD_ERR_NOT_MATCH;
			}
			dip[0] = (uint8_t)ip[0];
			dip[1] = (uint8_t)ip[1];
			dip[2] = (uint8_t)ip[2];
			dip[3] = (uint8_t)ip[3];

			sport = (uint16_t)atoi(argv[5]);
			dport = (uint16_t)atoi(argv[6]);

			/* check protocol */
			if(strcmp(argv[7] , "tcp") == 0)
				protocol = 6;
			else if(strcmp(argv[7] , "udp") == 0)
				protocol = 17;
			else
			{
				printf("protocol %s unknow! {tcp | udp}\n", argv[7]);
				return CMD_ERR_NOT_MATCH;
			}

			cmd_acl_show_match_rule(*((uint32_t*)sip), *((uint32_t*)dip), sport, dport, protocol);
		}
		else if(strcmp(argv[2], "max-entry") == 0)
		{
			printf("acl_bucket_max_entries = %d\n", acl_bucket_max_entries);
		}
	}

	/* acl count info clear */
	else if(strcmp(argv[1], "clear") == 0)
	{
		if(argc <3)
		{
			printf("acl clear cnt\n");
			printf("acl clear rule\n");
			printf("acl clear aging\n");
			return CMD_ERR_TOO_FEW_ARGC;
		}
		/* clear */
		if(strcmp(argv[2], "cnt") == 0)
		{
			cvmx_atomic_set64(&rule_cnt_info.rule_age_cnt, 0);
			cvmx_atomic_set64(&rule_cnt_info.rule_hit_cnt, 0);
			cvmx_atomic_set64(&rule_cnt_info.rule_miss_cnt, 0);
			cvmx_atomic_set64(&rule_cnt_info.extern_cnt, 0);
			cvmx_atomic_set64(&rule_cnt_info.fwd_eth_cnt, 0);
			cvmx_atomic_set64(&rule_cnt_info.fwd_cw_cnt, 0);
			cvmx_atomic_set64(&rule_cnt_info.fwd_cw_802_3_cnt, 0);
			printf("all acl rule statistics information is clear!\n\n");
		}
		if(strcmp(argv[2], "rule") == 0)
		{
			acl_clear_rule();
		}
		if(strcmp(argv[2], "aging") == 0)
		{
			acl_clear_aging_rule();
		}
	}
	else if(strcmp(argv[1], "set") == 0)
	{
		if(argv[2] == NULL)
		{
			printf("acl set 5-tuple\n");
			printf("acl set car {$sip $dip $sport $dport $protocol} $car_index\n");
			return CMD_ERR_TOO_FEW_ARGC;
		}
		if(strcmp(argv[2], "5-tuple") == 0)
		{
			if(argc < 8)
			{
				printf("acl set 5-tuple {$sip $dip $sport $dport $protocol}\n");
				return CMD_ERR_TOO_FEW_ARGC;
			}
			if((argv[3] == NULL) || (argv[4] == NULL) || (argv[5] == NULL) || (argv[6] == NULL) || (argv[7] == NULL))
			{
				printf("arg is null\n");
				return CMD_ERR_NOT_MATCH;
			}

			/* check ip format*/
			if(!is_valid_ip(argv[3]))
			{
				printf("src ip format error\n");
				return CMD_ERR_NOT_MATCH;
			}
			if(!is_valid_ip(argv[4]))
			{
				printf("dst ip format error\n");
				return CMD_ERR_NOT_MATCH;
			}		

			if(sscanf(argv[3], "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == -1)
			{
				printf("src ip error\n");
				return CMD_ERR_NOT_MATCH;
			}
			sip[0] = (uint8_t)ip[0];
			sip[1] = (uint8_t)ip[1];
			sip[2] = (uint8_t)ip[2];
			sip[3] = (uint8_t)ip[3];

			if(sscanf(argv[4], "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == -1)
			{
				printf("dest ip error\n");
				return CMD_ERR_NOT_MATCH;
			}
			dip[0] = (uint8_t)ip[0];
			dip[1] = (uint8_t)ip[1];
			dip[2] = (uint8_t)ip[2];
			dip[3] = (uint8_t)ip[3];

			/* check protocol */
			if(strcmp(argv[7] , "tcp") == 0)
				protocol = 6;
			else if(strcmp(argv[7] , "udp") == 0)
				protocol = 17;
			else
			{
				printf("protocol %s unknow! {tcp | udp}\n", argv[7]);
				return CMD_ERR_NOT_MATCH;
			}

			/* set value */
			gl_five_tuple.ip_src = *((uint32_t*)sip);
			gl_five_tuple.ip_dst= *((uint32_t*)dip);
			gl_five_tuple.th_sport = (uint16_t)atoi(argv[5]);
			gl_five_tuple.th_dport = (uint16_t)atoi(argv[6]);
			gl_five_tuple.ip_p = protocol;
		}
		else if(strcmp(argv[2], "car") == 0)
		{
			int8_t car_index = 0;
			if(argc < 9)
			{
				printf("acl set car {$sip $dip $sport $dport $protocol} $car_index\n");
				return CMD_ERR_TOO_FEW_ARGC;
			}
			if((argv[3] == NULL) || (argv[4] == NULL) || (argv[5] == NULL) || (argv[6] == NULL) || 
					(argv[7] == NULL) || (argv[8] == NULL))
			{
				return CMD_ERR_NOT_MATCH;
			}
			/* src ip */
			if(strcmp(argv[3] , "*") == 0)
			{
				*(uint32_t *)sip = gl_five_tuple.ip_src;
			}
			else
			{
				/* check ip format*/
				if(!is_valid_ip(argv[3]))
				{
					printf("src ip format error\n");
					return CMD_ERR_NOT_MATCH;
				}
				if(sscanf(argv[3], "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == -1)
				{
					return CMD_ERR_NOT_MATCH;
				}
				sip[0] = (uint8_t)ip[0];
				sip[1] = (uint8_t)ip[1];
				sip[2] = (uint8_t)ip[2];
				sip[3] = (uint8_t)ip[3];
			}

			/* dest ip */
			if(strcmp(argv[4] , "*") == 0)
			{
				*(uint32_t *)dip = gl_five_tuple.ip_dst;
			}
			else
			{
				if(!is_valid_ip(argv[4]))
				{
					printf("dst ip format error\n");
					return CMD_ERR_NOT_MATCH;
				}       
				if(sscanf(argv[4], "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == -1)
				{
					return CMD_ERR_NOT_MATCH;
				}
				dip[0] = (uint8_t)ip[0];
				dip[1] = (uint8_t)ip[1];
				dip[2] = (uint8_t)ip[2];
				dip[3] = (uint8_t)ip[3];
			}

			/* src port */
			if(strcmp(argv[5] , "*") == 0)
				sport = gl_five_tuple.th_sport;
			else
				sport = (uint16_t)atoi(argv[5]);

			/* dest port */
			if(strcmp(argv[6] , "*") == 0)
				dport = gl_five_tuple.th_dport;
			else
				dport = (uint16_t)atoi(argv[6]);

			/* protocol */
			if(strcmp(argv[7] , "*") == 0)
				protocol = gl_five_tuple.ip_p;
			else if(strcmp(argv[7] , "tcp") == 0)
				protocol = 6;
			else if(strcmp(argv[7] , "udp") == 0)
				protocol = 17;
			else
			{
				printf("protocol %s unknow! {tcp | udp}\n", argv[7]);
				return CMD_ERR_NOT_MATCH;
			}

			car_index = (uint8_t)atoi(argv[8]);
			cmd_acl_rule_set_car(*((uint32_t*)sip), *((uint32_t*)dip), sport, dport, protocol, car_index);
		}
		else if(strcmp(argv[2], "max-entry") == 0)
		{
			uint32_t entry_num = 0;
			if(argv[3] == NULL)
			{
				printf("acl set max-entry $num\n");
				return CMD_ERR_NOT_MATCH;
			}
			entry_num = atoi(argv[3]);
			if(entry_num > 2)
			{
				acl_bucket_max_entries = entry_num;
			}
			else
			{
				printf("acl set max-entry $num\n");
				return CMD_ERR_NOT_MATCH;
			}
		}
		else
		{
			printf("acl set 5-tuple\n");
			printf("acl set car {$sip $dip $sport $dport $protocol} $car_index\n");
			printf("acl set max-entry $num\n");
			return CMD_ERR_NOT_MATCH;
		}
	}
	return CMD_EXEC_SUCCESS;

}

int32_t cmd_read_memory(uint64_t addr)
{
	uint8_t ch;
	uint64_t i, flag = 0;
    int64_t *p = (int64_t *)(addr);

	while(1)
	{
		if(0 == flag)
		{
			for(i = 0; i < 5; i++)
			{
				if((int64_t)p > 0x7ffffffd) break;
				
				uint64_t val = *p;
				printf("0x%llX : ", CAST64(p));
				printf("\t%02llX %02llX %02llX %02llX - %02llX %02llX %02llX %02llX\r\n",
						CAST64((uint32_t)( (val >> 56) & 0xff)),
						CAST64((uint32_t)( (val >> 48) & 0xff)),
						CAST64((uint32_t)( (val >> 40) & 0xff)),
						CAST64((uint32_t)( (val >> 32) & 0xff)),
						CAST64((uint32_t)( (val >> 24) & 0xff)),
						CAST64((uint32_t)( (val >> 16) & 0xff)),
						CAST64((uint32_t)( (val >> 8) & 0xff)),
						CAST64((uint32_t)( (val >> 0) & 0xff))
				      );   
				p+=1;
			}
			flag = 1;
		}
		printf("d: continue q: quint \n");	
		ch = uart_read_byte_wait(uart_index);
		if(ch == 'q')
		{
			break;
		}
		else if(ch == 'd')
		{
			flag = 0;
		}
	}

	return RETURN_OK;
}

int32_t cmd_read(int argc, char *argv[])
{
	uint64_t addr;
	if (argc < 2 )
	{	
		printf("read csr {addr} (addr is decimal or hex) \n");
		printf("read memory {addr} (addr is decimal or hex) \n");
		return CMD_ERR_TOO_FEW_ARGC;
	}
	if(0 == strcmp(argv[1], "csr"))
	{
		if(NULL != argv[2])
		{
			if ((!strncmp(argv[2], "0x", 2)) || (!strncmp(argv[2], "0X", 2)))
			{
				addr = strtol(argv[2], NULL, 16);
			}
			else
			{
				addr = atol(argv[2]);
			}

			if(0 != (addr % 8))
			{
				printf("Register address is not correct, 8 bytes alignment \n");
				return CMD_ERR_NOT_MATCH;
			}
			cvm_common_read_csr(addr);
			return CMD_EXEC_SUCCESS;
		}
	}
	else if(0 == strcmp(argv[1], "memory"))
	{
		if(NULL != argv[2])
		{
			if ((!strncmp(argv[2], "0x", 2)) || (!strncmp(argv[2], "0X", 2)))
			{
				addr = strtol(argv[2], NULL, 16);
			}
			else
			{
				addr = atol(argv[2]);
			}

			if((addr < 0x100000) || (addr > 0x7ffffffd))
			{
				printf("Memory do not read. [0x100000~0x7ffffffd]\n");
				return CMD_ERR_NOT_MATCH;
			}
			cmd_read_memory(addr);
			return CMD_EXEC_SUCCESS;
		}
	}

	printf("read csr {addr} (addr is decimal or hex) \n");
	printf("read memory {addr} (addr is decimal or hex) \n");
	return CMD_ERR_NOT_MATCH;

}

void cmd_write_memory(uint64_t addr, uint64_t val)
{
	int8_t *p = (int8_t *)(addr);
	*p = val;
	printf("0x%llX : ", CAST64(p));
	printf("\t%02llX %02llX %02llX %02llX - %02llX %02llX %02llX %02llX\r\n",
			CAST64((uint32_t)( (val >> 56) & 0xff)),
			CAST64((uint32_t)( (val >> 48) & 0xff)),
			CAST64((uint32_t)( (val >> 40) & 0xff)),
			CAST64((uint32_t)( (val >> 32) & 0xff)),
			CAST64((uint32_t)( (val >> 24) & 0xff)),
			CAST64((uint32_t)( (val >> 16) & 0xff)),
			CAST64((uint32_t)( (val >> 8) & 0xff)),
			CAST64((uint32_t)( (val >> 0) & 0xff))
	      );
}

int32_t cmd_write(int argc, char *argv[])
{
	uint64_t addr,val;
	if (argc < 2 )
	{	
		printf("write csr {addr} {val} (addr is decimal or hex) \n");
		printf("write memory {addr} {val-high(hex)} {val-low(hex)} \n");
		return CMD_ERR_TOO_FEW_ARGC;
	}
	if(0 == strcmp(argv[1], "csr"))
	{
		if((NULL != argv[2]) &&(NULL != argv[3]))
		{
			if ((!strncmp(argv[2], "0x", 2)) || (!strncmp(argv[2], "0X", 2)))
			{
				addr = strtol(argv[2], NULL, 16);
			}
			else
			{
				addr = atol(argv[2]);
			}

			if ((!strncmp(argv[3], "0x", 2)) || (!strncmp(argv[3], "0X", 2)))
			{
				val = strtol(argv[3], NULL, 16);
			}
			else
			{
				val = atol(argv[3]);
			}

			if(0 != (addr % 8))
			{
				printf("Register address is not correct, 8 bytes alignment \n");
				return CMD_ERR_NOT_MATCH;
			}

			cvm_common_write_csr(addr, val);
			return CMD_EXEC_SUCCESS;
		}
	}
	else if(0 == strcmp(argv[1], "memory"))
	{
		if((NULL != argv[2]) && (NULL != argv[3]) && (NULL != argv[4]))
		{
			if ((!strncmp(argv[2], "0x", 2)) || (!strncmp(argv[2], "0X", 2)))
			{
				addr = strtol(argv[2], NULL, 16);
			}
			else
			{
				addr = atol(argv[2]);
			}

			if((addr < 0x100000) || (addr > 0x7ffffffd))
			{
				printf("Memory do not write. [0x100000~0x7ffffffd]\n");
				return CMD_ERR_NOT_MATCH;
			}

			val = (uint64_t)(strtol(argv[3], NULL, 16))*0x100000000 + strtol(argv[4], NULL, 16);
			cmd_write_memory(addr, val);
			return CMD_EXEC_SUCCESS;			
		}
	}

	printf("write csr {addr} {val} (addr is decimal or hex) \n");
	printf("write memory {addr} {val-high(hex)} {val-low(hex)} \n");
	return CMD_ERR_NOT_MATCH;

}



int32_t cmd_exit()
{
	switch_uart_to_linux();
	return 0;
}

#ifdef OCTEON_DEBUG_LEVEL
/* for debug */
CVMX_SHARED uint32_t debug_tag_val = 0;
int32_t cmd_show_tag()
{
	printf("tag_val = 0x%x\n", debug_tag_val);
	return 0;
}
#endif

int32_t cmd_check(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("check rule entry\n");
		printf("check pfast entry\n");
		return RETURN_ERROR;
	}

	if(strcmp(argv[1], "rule") == 0)
	{
		if(strcmp(argv[2], "entry") == 0)
		{
			check_rule_entry_num();
		}
	}else if(strcmp(argv[1], "pfast") == 0)
	{
	    if(strcmp(argv[2], "entry") == 0)
		{
			check_pfast_entry_num();
		}
	}
	return RETURN_OK;
}

void show_user_stats()
{

}

#define USER_STATE(t)  ((t) == USER_IS_ONLINE ? "online" : ((t) == USER_IS_OFFLINE ? "offline" : "empty"))     

void print_user(user_item_t* user)
{
    if(user == NULL)
        return;
        
    printf(" IP: %d.%d.%d.%d    ", IP_FMT(user->user_info.user_ip));
    printf("%s\r\n", USER_STATE(user->user_info.user_state));
    printf(" forward_up_bytes: %ld\n", user->user_info.forward_up_bytes);
    printf(" forward_up_packet: %ld\n", user->user_info.forward_up_packet);
    printf(" forward_down_bytes: %ld\n", user->user_info.forward_down_bytes);
    printf(" forward_down_packet: %ld\n", user->user_info.forward_down_packet);
}

void show_user_all()
{
	user_item_t *user;
	uint64_t i = 0;
	uint32_t online_user_cnt = 0;
	uint32_t offline_user_cnt = 0;
	uint32_t total_user_cnt = 0;
	uint32_t quit_flag = 0;
	uint32_t page_flag = 0;

	for(i = 0; i < user_static_tbl_size; i++)
	{
		user = &user_bucket_tbl[i];
		if(user->user_info.user_state == USER_IS_EMPTY)
		{
			if(user->valid_entries == 0)
			{
				continue;
			}
			user = user->next;
		}
		else
		{
			if(user->user_info.user_state == USER_IS_ONLINE)
			{
				online_user_cnt++;
			}
			else if(user->user_info.user_state == USER_IS_OFFLINE)
			{
				offline_user_cnt++;
			}
			total_user_cnt++;
			page_flag++;
			if(quit_flag == 0)
			{
				printf("----------------------------------------------------------------------------\n");
				printf("user bucket idx = %ld, list head\n", i);
				print_user(user);    
			}
			user = user->next;
		}

		while(user != NULL)
		{
			if(user->user_info.user_state == USER_IS_ONLINE)
			{
				online_user_cnt++;
			}
			else if(user->user_info.user_state == USER_IS_OFFLINE)
			{
				offline_user_cnt++;
			}
			total_user_cnt++;
			page_flag++;
			if(quit_flag == 0)
			{
				printf("----------------------------------------------------------------------------\n");
				printf("user bucket idx = %ld, list head\n", i);
				print_user(user);    
			}
			user = user->next;
		}

		if((quit_flag == 0) && (page_flag > 5))                  
		{
			page_flag = 0;
			uart_write_string(uart_index,"[C]ontinue, [Q]uit:");
			while(1)
			{
				int ch;

				ch = uart_read_byte_wait(uart_index);

				if(ch == 'c')
				{
					printf("\n");
					break;
				}
				else if(ch == 'q')
				{
					quit_flag=1;
					break;
				}
			}
		}
	}

	printf("----------------------------------------------------------------------------\n");
	printf("online : %d    ", online_user_cnt);
	printf("offline : %d    ", offline_user_cnt);
	printf("total : %d\n", total_user_cnt);
}

int32_t cmd_user(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("user show stats\n");
		printf("user show all\n");
		return RETURN_ERROR;
	}

	if(strcmp(argv[1], "show") == 0)
	{
		if(strcmp(argv[2], "stats") == 0)
		{
			show_user_stats();
		}
		else if(strcmp(argv[2], "all") == 0)
		{
			show_user_all();
		}
		else
		{
			printf("user show stats\n");
			printf("user show all\n");
		}
	}

	return RETURN_OK;
}


extern CVMX_SHARED int cvm_rate_limit_enabled;
extern CVMX_SHARED uint64_t cvm_rate_limit_drop_counter;
extern CVMX_SHARED uint64_t cvm_rate_limit_pass_counter;
extern CVMX_SHARED int cvm_rate_limit_debug;
extern CVMX_SHARED int ratelimit_flag;

extern CVMX_SHARED global_various_t * global_various_ptr;

extern void dump_ratelimit_table(void);

int32_t cmd_limit(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("limit enable\n");
        printf("limit disable\n");
        printf("limit show\n");
        printf("limit clear\n");
        printf("limit debug_on\n");
        printf("limit debug_off\n"); 
        printf("limit dump\n"); 
        printf("limit bps\n"); 
        printf("limit pps\n");         
        return RETURN_ERROR;
    }

    if(strcmp(argv[1], "enable") == 0)
	{
	    global_various_ptr->cvm_rate_limit_enabled = FUNC_ENABLE;
	}
	else if(strcmp(argv[1], "disable") == 0)
	{
        global_various_ptr->cvm_rate_limit_enabled = FUNC_DISABLE;
	}
    else if(strcmp(argv[1], "show") == 0)
	{    
	    printf("limit %s\n", (global_various_ptr->cvm_rate_limit_enabled == FUNC_ENABLE) ? "enable":"disable");
        printf("limit drop: %lu\n", global_various_ptr->cvm_rate_limit_drop_counter);
        printf("limit pass: %lu\n", global_various_ptr->cvm_rate_limit_pass_counter);
    }
    else if(strcmp(argv[1], "clear") == 0)
	{
        global_various_ptr->cvm_rate_limit_drop_counter = 0;
        global_various_ptr->cvm_rate_limit_pass_counter = 0;
    }
    else if(strcmp(argv[1], "debug_on") == 0)
    {
        cvm_rate_limit_debug = 1;
    }
    else if(strcmp(argv[1], "debug_off") == 0)
    {
        cvm_rate_limit_debug = 0;
    }  
    else if(strcmp(argv[1], "dump") == 0)
    {
        dump_ratelimit_table();
    }
    else if(strcmp(argv[1], "bps") == 0)
    {
        ratelimit_flag = 0;
    } 
    else if(strcmp(argv[1], "pps") == 0)
    {
        ratelimit_flag = 1;
    }     
    else
    {
        printf("limit enable\n");
        printf("limit disable\n");
        printf("limit show\n");
        printf("limit clear\n");
        printf("limit debug_on\n");
        printf("limit debug_off\n");
        printf("limit dump\n"); 
        printf("limit bps\n"); 
        printf("limit pps\n"); 
        return RETURN_ERROR;
    }

    return RETURN_OK;
}

/* show pow state. */
CVMX_SHARED uint8_t pow_cap_buf[512*1024];
int32_t cmd_pow(int argc, char *argv[])
{
	if(argc < 2)
	{
	    printf("pow dump\n");
	    printf("pow tolinuxgrp N\n");
	    printf("pow tolinuxfccpgrp N\n");
	    printf("pow fromlinuxgrp N\n");
		return RETURN_ERROR;
	}

	if(strcmp(argv[1], "dump") == 0)
	{
	    memset(pow_cap_buf, 0, sizeof(pow_cap_buf));
        cvmx_pow_capture(pow_cap_buf, sizeof(pow_cap_buf));
        cvmx_pow_display(pow_cap_buf, sizeof(pow_cap_buf));
	}
    else if(strcmp(argv[1], "tolinuxgrp") == 0)
    {
        if(argv[2])
        {
            product_info.to_linux_group = atoi(argv[2]);
            printf("to_linux_group change to %d\n", atoi(argv[2]));
        }
    }
    else if(strcmp(argv[1], "tolinuxfccpgrp") == 0)
    {
        if(argv[2])
        {
            product_info.to_linux_fccp_group = atoi(argv[2]);
            printf("to_linux_fccp_group change to %d\n", atoi(argv[2]));
        }
    }
    else if(strcmp(argv[1], "fromlinuxgrp") == 0)
    {
        if(argv[2])
        {
            product_info.from_linux_group = atoi(argv[2]);
            printf("from_linux_group change to %d\n", atoi(argv[2]));
        }
    }    
	else
	{
        printf("pow dump\n");
        return RETURN_ERROR;
	}
	return RETURN_OK;
}	




int32_t cmd_pko(int argc, char *argv[])
{
    int i = 0;
    int queue = 0;
    cvmx_pko_mem_debug8_t debug8;
    cvmx_pko_mem_queue_ptrs_t queue_ptr;
    if(argc < 2)
	{
	    printf("pko dump\n");
	    printf("pko free_queue $port\n");
		printf("pko queue_ptr\n");
		return RETURN_ERROR;
	}

	if(strcmp(argv[1], "dump") == 0)
	{
        for(i = 0; i < 256; i++)
        {
            cvmx_write_csr(CVMX_PKO_REG_READ_IDX, i);
            debug8.u64 = cvmx_read_csr(CVMX_PKO_MEM_DEBUG8);
            printf("queue %d, debug8 = 0x%lx, doorbell=%d\n", 
                i, 
                debug8.u64,
                debug8.cn58xx.doorbell
                );
        }
	}
    else if(strcmp(argv[1], "queue_ptr") == 0)
	{
        for(i = 0; i < 256; i++)
        {
            cvmx_write_csr(CVMX_PKO_REG_READ_IDX, i);
            queue_ptr.u64 = cvmx_read_csr(CVMX_PKO_MEM_QUEUE_PTRS);
            printf("queue %d, queue_ptr = 0x%lu, buf_ptr=0x%lu, vaddr=0x%p, queue=%lu, port=%lu\n", 
                i, 
                queue_ptr.u64,
                (uint64_t)queue_ptr.cn58xx.buf_ptr,
                cvmx_phys_to_ptr(queue_ptr.cn58xx.buf_ptr),
                (uint64_t)queue_ptr.cn58xx.queue,
                (uint64_t)queue_ptr.cn58xx.port
                );
        }
	}
	else if(strcmp(argv[1], "free_queue")==0)
	{
	    if(argv[2] != NULL)
	    {
            /* Enable packet output by enabling all queues for this port */
            queue = cvmx_pko_get_base_queue(atoi(argv[2]));
            int stop_queue = queue + cvmx_pko_get_num_queues(atoi(argv[2]));
            while (queue < stop_queue)
            {
                cvmx_pko_mem_queue_qos_t pko_qos;
                pko_qos.u64 = 0;
                pko_qos.s.pid = atoi(argv[2]);
                pko_qos.s.qid = queue;
                pko_qos.s.qos_mask = 0xff;
                cvmx_write_csr(CVMX_PKO_REG_READ_IDX, queue);
                cvmx_write_csr(CVMX_PKO_MEM_QUEUE_QOS, pko_qos.u64);
                queue++;
            }
        }
	}

	return RETURN_OK;
}
/**
 * initilize the shell
 * param @uart_i in-and-out to the uart
 */
void shell_common_register(void)
{
	register_shell_cmd("help", cmd_help, "print the help information", "help [cmd]", CMD_DISPLAY_VIEW);
	register_shell_cmd("show", cmd_show, "show debug information", "show [cmd]", CMD_DISPLAY_VIEW);
	register_shell_cmd("reboot", cmd_reboot, "reboot",  "reboot",  CMD_DISPLAY_VIEW);
	register_shell_cmd("exit", cmd_exit, "exit", "back to linux", CMD_DISPLAY_VIEW);
	register_shell_cmd("clear", cmd_clear, "clear debug information", "clear [cmd]", CMD_DISPLAY_VIEW);
	register_shell_cmd("set", cmd_set, "set debug information", "set [cmd]", CMD_DISPLAY_VIEW);
	register_shell_cmd("acl", cmd_acl, "acl table operation", "acl [cmd]", CMD_DISPLAY_VIEW);
	register_shell_cmd("read", cmd_read, "read", "read [cmd]", CMD_DISPLAY_VIEW);
	register_shell_cmd("write", cmd_write, "write", "write [cmd]", CMD_DISPLAY_VIEW);
	register_shell_cmd("dump", dump_gmx, "dump", "dump [cmd]", CMD_DISPLAY_VIEW);
	register_shell_cmd("fastdisable", cmd_fastdisable, "fastdisable", "fastdisable [cmd]", CMD_DISPLAY_VIEW);
	register_shell_cmd("fastenable", cmd_fastenable, "fastenable", "fastenable [cmd]", CMD_DISPLAY_VIEW);
	register_shell_cmd("check", cmd_check, "check", "check [cmd]", CMD_DISPLAY_VIEW);
	register_shell_cmd("user", cmd_user, "user", "user [cmd]", CMD_DISPLAY_VIEW);
    register_shell_cmd("pow", cmd_pow, "pow", "pow [cmd]", CMD_DISPLAY_VIEW);
    register_shell_cmd("limit", cmd_limit, "limit", "limit [cmd]", CMD_DISPLAY_VIEW);

    register_shell_cmd("pko", cmd_pko, "pko", "pko [cmd]", CMD_DISPLAY_VIEW);
#ifdef OCTEON_DEBUG_LEVEL
	register_shell_cmd("tag", cmd_show_tag, "cmd_show_tag", "cmd_show_tag", CMD_DISPLAY_VIEW);
#endif
}

