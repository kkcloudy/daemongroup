#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#ifndef __SCCP_APPL_H__
#define __SCCP_APPL_H__

int 
ranap_send_msg2sccp
(
	unsigned int type, 
	unsigned int class, 
	unsigned int des_loc_ref, 
	unsigned int sou_loc_ref,
	char *data, 
	int data_len
);

#endif	/*__SCCP_APPL_H__*/

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

