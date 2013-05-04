/***********************license start************************************
 * File name: fwd_main.c 
 * Auther     : lutao
 * 
 * Copyright (c) Autelan . All rights reserved.
 * 
 **********************license end**************************************/

#include "cvmx.h"
#include "cvmx-config.h"
#include "autelan_product_info.h"


/*
  *	product_num				module_num				   product_product_type
  *		5						3					    AU_5612I
  *		5						X(don't care, but 3)	    AX_7605
  */
void cvmx_oct_set_product_id(product_info_t *product_info)
{
	unsigned char product_num = 0;
	unsigned char module_num = 0;
	
	unsigned char product_type = 0;
	product_num = cvmx_read64_uint8(CPLD1_BASE_ADDR+CPLD_PRODUCT_CTL+ (1ull<<63));
	module_num = cvmx_read64_uint8(CPLD1_BASE_ADDR+CPLD_MODULE_CTL+ (1ull<<63));
	product_type = cvmx_read64_uint8(CPLD1_BASE_ADDR + CPLD_PRODUCT_TYPE + (1ull<<63));
	
	product_num &= 0xf;
	product_type = (product_type >> 2) & PRODUCT_TYPE_MASK;
	printf(" product_num=%d,\n module_num=%d,\n product_type = %d \n", product_num, module_num,product_type);
	
	if(product_num == 3)
	{
		if (module_num == 0)
		{
			product_info->product_type = AU_3524;
		}
		else if (module_num == 1)
		{
			product_info->product_type = AU_3052;	
		}
		else if (module_num == 2)
		{
			product_info->product_type = AU_3028;
		}
		else if (module_num == 3)
		{
			product_info->product_type = AU_3052_P;
		}
		else if (module_num == 4)
		{
			product_info->product_type = AU_3028_P;
		}
		else if (module_num == 5)
		{
			product_info->product_type = AU_3524_P;
		}
		else
		{
			product_info->product_type = UNKNOWN_PRODUCT_ID;
		}
	}
	else if (product_num == 4)
	{
		if (module_num == 1)
		{
			product_info->product_type = AU_4624;
		}
		else if (module_num == 2)
		{
			product_info->product_type = AU_4524;	
		}
		else if (module_num == 3)
		{
			product_info->product_type = AU_4626_P;
		}
		else if (module_num == 4)
		{
			product_info->product_type = AU_4524_P;
		}
		else
		{
			product_info->product_type = UNKNOWN_PRODUCT_ID;
		}
	}
	else if (product_num == 5)
	{
		if (module_num == 0)
		{
			product_info->product_type = AX_5612;
		}
		else if (module_num == 3)
		{
			product_info->product_type = AX_5612I;	
		}
		else if (module_num == 0x61)
		{
			product_info->product_type = AX_5612E;	
		}
		else if (module_num == 0x62)
		{
			product_info->product_type = AX_5608;	
		}
		else
		{
			product_info->product_type = UNKNOWN_PRODUCT_ID;
		}
	}
	else if (product_num == 7)
	{
	
	    product_info->product_type = AX_7605;
		
		if (product_type == 0x1)
			product_info->board_type = module_num;

		
	}
	else
	{
		/*wrong product product_type*/
		product_info->product_type = UNKNOWN_PRODUCT_ID;	
	}
		
}

