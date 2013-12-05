#ifndef _TIPC_ZCLIENT_H_
#define _TIPC_ZCLIENT_H_


#include "zebra.h"

#define SERVER_NAME  17777

#define SERVER_TYPE  0x2000
#define SERVER_INST  (1000+product->board_id)

#define MAX_CONNECT   5

#define TIPC_PACKET_HEADER_SIZE					6
#define TIPC_PACKET_HEADER_MARKER              255
#define TIPC_PACKET_VERSION						1


#if 0
#ifndef TIPC_ZCLIENT_DEBUG
#define TIPC_ZCLIENT_DEBUG
#endif
#endif

/*gujd: 2012-02-08: pm 5:20 . In order to decrease the warning when make img . For declaration  func  in .h file .*/
extern struct interface *check_interface_exist_by_name(const char * name);
extern struct interface *check_interface_exist_by_name_len(const char * name,size_t namelen);

/*gujd: 2012-06-06, am 11:03. Change the ve sub (like ve1.100) to support multi-core (like ve01f1.100).
	When sync interface info , the ve parent interface is same too. */
#if 0
#define DISABLE_REDISTRIBUTE_INTERFACE(X,R) \
do {\
	if(judge_eth_debug_interface(X)==ETH_DEBUG_INTERFACE\
	 ||judge_obc_interface(X)==OBC_INTERFACE\
	 ||judge_mng_interface(X)==MNG_INTERFACE\
	 ||judge_ve_interface(X)==VE_INTERFACE\
	 ||judge_loop_interface(X)==LOOP_INTERFACE\
   ||judge_pimreg_interface(X)==PIMREG_INTERFACE\
   ||judge_gre_interface(X)==GRE_INTERFACE\
	 ||judge_sit0_interface(X)==SIT0_INTERFACE\
	 ||judge_pppoe_interface(X)==PPPOE_INTERFACE\
	 ||judge_oct_interface(X)==OCT_INTERFACE\
	 ||judge_radio_interface(X)==DISABLE_RADIO_INTERFACE)\
	  R=1;\
	 else\
	  R=0;\
} while(0)
#else
#define DISABLE_REDISTRIBUTE_INTERFACE(X,R) \
do {\
	if(judge_eth_debug_interface(X)==ETH_DEBUG_INTERFACE\
	 ||judge_obc_interface(X)==OBC_INTERFACE\
	 ||judge_mng_interface(X)==MNG_INTERFACE\
	 ||judge_loop_interface(X)==LOOP_INTERFACE\
   ||judge_pimreg_interface(X)==PIMREG_INTERFACE\
   ||judge_gre_interface(X)==GRE_INTERFACE\
	 ||judge_sit0_interface(X)==SIT0_INTERFACE\
	 ||judge_pppoe_interface(X)==PPPOE_INTERFACE\
	 ||judge_oct_interface(X)==OCT_INTERFACE\
	 ||judge_radio_interface(X)==DISABLE_RADIO_INTERFACE)\
	  R=1;\
	 else\
	  R=0;\
} while(0)

#endif
#define DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_VEXX(X,R) \
do {\
	if(judge_eth_debug_interface(X)==ETH_DEBUG_INTERFACE\
	 ||judge_obc_interface(X)==OBC_INTERFACE\
	 ||judge_mng_interface(X)==MNG_INTERFACE\
	||judge_ve_interface(X)==VE_INTERFACE\
	 ||judge_loop_interface(X)==LOOP_INTERFACE\
   ||judge_pimreg_interface(X)==PIMREG_INTERFACE\
   ||judge_gre_interface(X)==GRE_INTERFACE\
	 ||judge_sit0_interface(X)==SIT0_INTERFACE\
	 ||judge_pppoe_interface(X)==PPPOE_INTERFACE\
	 ||judge_oct_interface(X)==OCT_INTERFACE\
	 ||judge_radio_interface(X)==DISABLE_RADIO_INTERFACE)\
	  R=1;\
	 else\
	  R=0;\
} while(0)

#if 0
#define DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO(X,R) \
	 do {\
		 if(judge_eth_debug_interface(X)==ETH_DEBUG_INTERFACE\
		  ||judge_obc_interface(X)==OBC_INTERFACE\
		  ||judge_mng_interface(X)==MNG_INTERFACE\
		  ||judge_ve_interface(X)==VE_INTERFACE\
		  ||judge_loop_interface(X)==LOOP_INTERFACE\
		  ||judge_pimreg_interface(X)==PIMREG_INTERFACE\
		  ||judge_pppoe_interface(X)==PPPOE_INTERFACE\
			||judge_gre_interface(X)==GRE_INTERFACE\
		  ||judge_oct_interface(X)==OCT_INTERFACE\
		  ||judge_sit0_interface(X)==SIT0_INTERFACE)\
		   R=1;\
		  else\
		   R=0;\
	 } while(0)
#else
#define DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO(X,R) \
	 do {\
		 if(judge_eth_debug_interface(X)==ETH_DEBUG_INTERFACE\
		  ||judge_obc_interface(X)==OBC_INTERFACE\
		  ||judge_mng_interface(X)==MNG_INTERFACE\
		  ||judge_loop_interface(X)==LOOP_INTERFACE\
		  ||judge_pimreg_interface(X)==PIMREG_INTERFACE\
		  ||judge_pppoe_interface(X)==PPPOE_INTERFACE\
			||judge_gre_interface(X)==GRE_INTERFACE\
		  ||judge_oct_interface(X)==OCT_INTERFACE\
		  ||judge_sit0_interface(X)==SIT0_INTERFACE)\
		   R=1;\
		  else\
		   R=0;\
	 } while(0)

#endif
/*not include raido and ve01f1(veXX)*/
#define DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO_VEXX(X,R) \
	 do {\
		 if(judge_eth_debug_interface(X)==ETH_DEBUG_INTERFACE\
		  ||judge_obc_interface(X)==OBC_INTERFACE\
		  ||judge_mng_interface(X)==MNG_INTERFACE\
		  ||judge_ve_interface(X)==VE_INTERFACE\
		  ||judge_loop_interface(X)==LOOP_INTERFACE\
		  ||judge_pimreg_interface(X)==PIMREG_INTERFACE\
		  ||judge_pppoe_interface(X)==PPPOE_INTERFACE\
			||judge_gre_interface(X)==GRE_INTERFACE\
		  ||judge_oct_interface(X)==OCT_INTERFACE\
		  ||judge_sit0_interface(X)==SIT0_INTERFACE)\
		   R=1;\
		  else\
		   R=0;\
	 } while(0)


#define DISABLE_REDISTRIBUTE_INTERFACE_7605I(X,R) \
do {\
	if(judge_eth_debug_interface(X)==ETH_DEBUG_INTERFACE\
	 ||judge_obc_interface(X)==OBC_INTERFACE\
	 ||judge_mng_interface(X)==MNG_INTERFACE\
	 ||judge_ve_interface(X)==VE_INTERFACE\
	 ||judge_loop_interface(X)==LOOP_INTERFACE\
   ||judge_pimreg_interface(X)==PIMREG_INTERFACE\
   ||judge_gre_interface(X)==GRE_INTERFACE\
	 ||judge_sit0_interface(X)==SIT0_INTERFACE\
	 ||judge_pppoe_interface(X)==PPPOE_INTERFACE\
	 ||judge_oct_interface(X)==OCT_INTERFACE\
	 ||judge_radio_interface(X)==DISABLE_RADIO_INTERFACE)\
	  R=1;\
	 else\
	  R=0;\
} while(0)

#define DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO_7605I(X,R) \
		 do {\
			 if(judge_eth_debug_interface(X)==ETH_DEBUG_INTERFACE\
			  ||judge_obc_interface(X)==OBC_INTERFACE\
			  ||judge_mng_interface(X)==MNG_INTERFACE\
			  ||judge_ve_interface(X)==VE_INTERFACE\
			  ||judge_loop_interface(X)==LOOP_INTERFACE\
			  ||judge_pimreg_interface(X)==PIMREG_INTERFACE\
			  ||judge_pppoe_interface(X)==PPPOE_INTERFACE\
				||judge_gre_interface(X)==GRE_INTERFACE\
			  ||judge_oct_interface(X)==OCT_INTERFACE\
			  ||judge_sit0_interface(X)==SIT0_INTERFACE)\
			   R=1;\
			  else\
			   R=0;\
		 } while(0)
		 
#define DISABLE_LOCAL_INTERFACE_VE(X,R) \
		  do {\
			  if(judge_ve_interface(X)==VE_INTERFACE\
			   &&judge_real_local_interface(X)==LOCAL_BOARD_INTERFACE)\
				R=1;\
			   else\
				R=0;\
		  } while(0)

#endif
