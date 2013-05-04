#ifndef  _CAR_H_
#define   _CAR_H_

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

#define CVM_CAR_SCALE_FACTOR_BIT_SHIFT  16
#define CVM_CAR_SCALE_FACTOR            (1 << CVM_CAR_SCALE_FACTOR_BIT_SHIFT)
#define CVM_CAR_CRC_LEN                 4
#define CVM_CAR_MTU                     1500
#define CVM_CAR_CBS                     (3 * CVM_CAR_MTU)   /* Max Token Bucket depth = 3 * MTU */

#define CVM_CAR_PASS_PKT             1
#define CVM_CAR_DROP_PKT             (! CVM_CAR_PASS_PKT)

#define METER_TEMPLATE_NUM    64 /*CAR模板数量*/
/*
令牌桶的算法基于RFC2697（单速率三色标记）实现。
允许速率CIR  （Committed Information Rate）：允许情况下的最小带宽。
允许突发流量CBS（Committed Burst Size）：单位为字节，这是瞬间可以超过CIR的流量；
过度突发流量EBS（Excess Burst Size）       ：单位为字节，在允许突发流量CBS上额外允许的突发量。
以上三个值通过命令行配置，推荐配置参考如下：
CBS = CIR * (1 byte)/(8 bits) * 1.5 seconds
EBS = 2 * CBS
TC(t)：t时刻C桶的剩余量。t=0时，TC(0)＝CBS
TP(t)：t时刻P桶的剩余量。t=0时，TP(0)＝EBS
其中CIR是每秒IP报文的字节数，包括IP头部，不包括特殊的头部（but not link specific headers）。
CBS和EBS单位也是字节数。CBS和EBS至少有一个配置需大于0。推荐配置：
当CBS和EBS配置大于0时，最好配置为大于或等于IP报文的最大包长。

注: EBS目前没有实现
*/
typedef struct _cvm_car_tb
{
    uint32_t   depth;       /*CBS in bytes*/
    uint32_t   rate;        /* CIR in kbps*/

    uint64_t   rate_in_cycles_per_byte;
    uint64_t   depth_in_cycles;
    uint64_t   cycles_prev;

} cvm_car_tb_t; 

int car_init(void);
int cvm_car_tb_set(user_item_t *user, int speed);
inline int cvm_car_result(int bytes, uint32_t usr_idx, uint16_t usr_link_idx);
void cvm_car_show(uint32_t car_index);
void cvm_car_dump(void);
void cvm_car_show_template(void);
int cvm_car_set_template(uint32_t index, uint32_t cir_val, uint32_t cbs_val);


#endif 
