#ifndef _SE_AGENT_OCTNIC_H_
#define _SE_AGENT_OCTNIC_H_

/* pcie interface function */
int32_t se_agent_octeon_init();
int32_t se_agent_send_fccp_from_pci(uint8_t* fccp_send, int32_t slen, uint8_t* fccp_rcv, int32_t rlen);

#endif

