#ifdef CVM_CAR

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-spinlock.h"
#include "cvmx-fpa.h"
#include "cvmx-pip.h"
#include "cvmx-ipd.h"
#include "cvmx-pko.h"
#include "cvmx-dfa.h"
#include "cvmx-pow.h"
#include "cvmx-gmx.h"
#include "cvmx-sysinfo.h"
#include "cvmx-coremask.h"
#include "cvmx-malloc.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper.h"

#include "acl.h"
#include "car.h"


CVMX_SHARED   cvm_car_tb_t  *car_tbl;
CVMX_SHARED uint64_t cvm_car_rate_const_scaled;

#ifdef USER_TABLE_FUNCTION
extern CVMX_SHARED user_item_t *user_bucket_tbl;
extern CVMX_SHARED uint32_t user_static_tbl_size;
#endif

/*支持的Meter模板,需根据需求修改*/
uint32_t cbs[METER_TEMPLATE_NUM] = {10000,10000,10000,10000,10000,10000,10000,10000}; // in bytes
//uint32_t cir[METER_NUMBER] = {10,100,3000,4000,50000,700000,1000000,9000000}; // in kbps
uint32_t cir[METER_TEMPLATE_NUM] = {4096,8192,16384,32768,65536,131072,262144,524288}; // in kbps


int car_init(void)
{
	cvmx_sysinfo_t    *sys_info_ptr  = cvmx_sysinfo_get();
	uint64_t           cpu_clock_hz  = sys_info_ptr->cpu_clock_hz;
	cvm_car_rate_const_scaled = ((cpu_clock_hz/1024) * (CVM_CAR_SCALE_FACTOR * 8));
	return 0;
}

inline void prefetch_car_table(user_item_t *user)
{
	CVMX_PREFETCH0(user);
}


int cvm_car_tb_set(user_item_t *user, int rate)
{
	if((rate <0) || (user == NULL))
		return -1;

	user->user_info.rate = rate;
	user->user_info.depth = 10000;

	if (user->user_info.rate == 0)
	{
		user->user_info.rate_in_cycles_per_byte = 0x0FFFFFFFFFFFFFFFUL;
		user->user_info.depth_in_cycles         = 0;
	}
	else
	{
		user->user_info.rate_in_cycles_per_byte = (cvm_car_rate_const_scaled/user->user_info.rate);
		user->user_info.depth_in_cycles         = (user->user_info.depth * user->user_info.rate_in_cycles_per_byte);
	}

	return 0;
}


inline int cvm_car_result(int bytes, uint32_t usr_idx, uint16_t usr_link_idx)
{
	uint64_t cycles_curr    = 0;
	uint64_t cycles_elapsed = 0;
	uint64_t cycles_reqd    = 0;
	int      retval         = CVM_CAR_DROP_PKT;
	user_item_t *user = NULL;
	
	if((user = get_user_item(usr_idx, usr_link_idx)) == NULL)
		return -1;

	cycles_curr    = cvmx_get_cycle();
	cycles_reqd    = (user->user_info.rate_in_cycles_per_byte * bytes);
	cycles_elapsed = ((cycles_curr - user->user_info.cycles_prev) << CVM_CAR_SCALE_FACTOR_BIT_SHIFT);

	if (cvmx_unlikely(cycles_elapsed > user->user_info.depth_in_cycles))
	{
		cycles_elapsed  = user->user_info.depth_in_cycles;
		user->user_info.cycles_prev = cycles_curr - (user->user_info.depth_in_cycles >> CVM_CAR_SCALE_FACTOR_BIT_SHIFT);
	}

	if (cycles_elapsed < cycles_reqd)
	{
		retval = CVM_CAR_DROP_PKT;
	}
	else
	{
		user->user_info.cycles_prev = user->user_info.cycles_prev + (cycles_reqd >> CVM_CAR_SCALE_FACTOR_BIT_SHIFT);
		retval = CVM_CAR_PASS_PKT;
	}

	return retval;
}

int cvm_car_set_template(uint32_t index, uint32_t cir_val, uint32_t cbs_val)
{
	if(index > METER_TEMPLATE_NUM - 1)
	{
		printf("invalid meter index %d\n", index);
		return -1;
	}

	cir[index] = cir_val;
	cbs[index] = cbs_val;
	return 0;
}

void cvm_car_show_template(void)
{
	int i;

	printf("\n");
	printf("======================================================\n");
	printf("METER Template:\n");
	printf("======================================================\n");
	printf("index   cir(kbps)       cbs(bytes)\n");
	for(i = 0; i < 8; i++)
	{
		printf("%2d      %8d        %8d\n", i, cir[i], cbs[i]);
	}
	printf("\n");
	printf("======================================================\n");
}

#endif
