#ifndef __NAM_ETH_API_H__
#define __NAM_ETH_API_H__

#define NAM_ERR_HW 7
#define BUFFER_MODE_SHARED 0
#define BUFFER_MODE_DIVIDED 1

/*
  APIs exported to be used by NPD module
*/

extern int nam_read_eth_port_info(unsigned int modue_type, unsigned char slotno,  unsigned char portno, struct eth_port_s *ethport);


#endif

