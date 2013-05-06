#ifndef __NAM_MAIN_API_H__
#define __NAM_MAIN_API_H__
/*
  APIs to STP exported to be used by NPD module
*/

extern int nam_asic_init(enum product_id_e product_id);
extern int nam_asic_stp_init(void);
extern int nam_asic_stp_port_enable(unsigned char devNum, unsigned int portId, unsigned char stp_enable);
extern int nam_asic_stp_port_state_update(unsigned char devNum, unsigned int portId, unsigned char stpState);
#endif

