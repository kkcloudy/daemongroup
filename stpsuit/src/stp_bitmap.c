/*******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology
********************************************************************************

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* stp_bitmap.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for bitmap op in stp module.
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.2 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "sysdef/npd_sysdef.h"
#include "stp_bitmap.h"

extern unsigned int productId;
extern unsigned int stp_slot_id;
extern unsigned int stp_is_distributed;



void stp_bitmap_clear(BITMAP_T* BitmapPtr)
{  
    int i=0;
	for(i = 0;i<9;i++) {
		BitmapPtr->part[i]=0;
	}	
}

void  stp_bitmap_set_allbits(BITMAP_T* BitmapPtr)
{ 
    int i=0;
	for(i=0;i<9;i++) {
		
		BitmapPtr->part[i]=0xff; 
	}	
}

void  stp_bitmap_set_bit(BITMAP_T*BitmapPtr,int Bit)
{ 
  	int index = 0;
	int byte = 0;
	if(STP_IS_DISTRIBUTED == stp_is_distributed)
	{
		Bit = Bit%64;  /* get port index */		
		index = (Bit/8) + 1;
		byte = Bit%8;		
	}
	else if(PRODUCT_ID_AX7K == productId){
		index = (Bit) / 64;
		byte = (Bit) % 64 + 1;
	}//非7u系列的端口portindex 范围是0-23，为了和7U生成的enabled_ports内容一致
	//使用以下的转换过程
	
	else if(PRODUCT_ID_AU3K == productId ||PRODUCT_ID_AU3K_BCM == productId ||PRODUCT_ID_AU3K_BCAT == productId ||PRODUCT_ID_AU2K_TCAT == productId){
		index = (Bit) / 8 + 1;
		byte = (Bit) % 8 ;
	}
	//5612,5612i,4626 portindex range 64-88, so make change below
	else if(PRODUCT_ID_AX5K == productId || PRODUCT_ID_AU4K == productId || PRODUCT_ID_AX5K_I == productId || PRODUCT_ID_AX5K_E == productId || PRODUCT_ID_AX5608 == productId) {
		if(Bit >= 64) {
			Bit = Bit - 64;
			index = (Bit) / 8 + 1;
			byte = (Bit) % 8 ;
		}
		else {
			index = (Bit) / 8 + 1;
			byte = (Bit) % 8 ;
		}
	}
	stp_syslog_dbg("stp_bitmap_set_bit BitmapPtr->part[%d] 0x%x ,byte 0x%x\n",index, BitmapPtr->part[index],byte);
	if(byte <= 7)
		BitmapPtr->part[index]|= (1 << byte);
}


void stp_bitmap_clear_bit(BITMAP_T* BitmapPtr,int Bit)
{
	int index = 0;
	int byte = 0;

	if(STP_IS_DISTRIBUTED == stp_is_distributed)
	{
		Bit = Bit%64;  /* get port index */		
		index = (Bit/8) + 1;
		byte = Bit%8;		
	}
	else if(PRODUCT_ID_AX7K == productId){
		index = (Bit) / 64;
		byte = (Bit) % 64 + 1;
	}
	
	else if(PRODUCT_ID_AU3K == productId ||PRODUCT_ID_AU3K_BCM == productId ||PRODUCT_ID_AU3K_BCAT == productId ||PRODUCT_ID_AU2K_TCAT == productId){
		index = (Bit) / 8 + 1;
		byte = (Bit) % 8 ;
	}
	else if(PRODUCT_ID_AX5K == productId || PRODUCT_ID_AU4K == productId || PRODUCT_ID_AX5K_I == productId || PRODUCT_ID_AX5K_E == productId || PRODUCT_ID_AX5608 == productId) {
		if(Bit >= 64) {
			Bit = Bit - 64;
			index = (Bit) / 8 + 1;
			byte = (Bit) % 8 ;
		}
		else {
			index = (Bit) / 8 + 1;
			byte = (Bit) % 8 ;
		}
	}
	if(byte <= 7) 
		BitmapPtr->part[index]&= ~(1 << byte);
}

int stp_bitmap_get_bit(BITMAP_T* BitmapPtr, int Bit)
{	
	int index = 0;
	int byte = 0;

	if(STP_IS_DISTRIBUTED == stp_is_distributed)
	{
		Bit = Bit%64;  /* get port index */		
		index = (Bit/8) + 1;
		byte = Bit%8;		
	}
	else if(PRODUCT_ID_AX7K == productId){
		index = (Bit) / 64;
		byte = (Bit) % 64 + 1;
	}
	
	else if(PRODUCT_ID_AU3K == productId ||PRODUCT_ID_AU3K_BCM == productId ||PRODUCT_ID_AU3K_BCAT == productId ||PRODUCT_ID_AU2K_TCAT == productId){
		index = (Bit) / 8 + 1;
		byte = (Bit) % 8 ;
	}
	else if(PRODUCT_ID_AX5K == productId || PRODUCT_ID_AU4K == productId || PRODUCT_ID_AX5K_I == productId || PRODUCT_ID_AX5K_E == productId || PRODUCT_ID_AX5608 == productId) {
		if(Bit >= 64) {
			Bit = Bit - 64;
			index = (Bit) / 8 + 1;
			byte = (Bit) % 8 ;
		}
		else {
			index = (Bit) / 8 + 1;
			byte = (Bit) % 8 ;
		}
	}
	if(byte <= 7)
	 	return (BitmapPtr->part[index] & (1 << byte));
	else
		return -1;
}

int stp_bitmap_portbmp_add(BITMAP_T* desPtr,BITMAP_T* souPtr)
{
	int i;
	for(i = 0; i < 9 ; i++) {
		desPtr->part[i] |= souPtr->part[i];
	}
}

int stp_bitmap_get_portindex_from_bmp(BITMAP_T* BitmapPtr,int Bytes,int Bit)
{
	unsigned int port_index = 0;
	unsigned int slot_no = 0;
	if(0 ==  BitmapPtr->part[Bytes]) {
		return -2;
	} 
	else {
		if(BitmapPtr->part[Bytes] & (1 << Bit)) {
			if(STP_IS_DISTRIBUTED == stp_is_distributed)
			{
				//port_index = 8 * (Bytes - 1) + (Bit);   /*zhengcaisheng changes*/
				#if 1
				slot_no = stp_slot_id;
                port_index = 64*(slot_no -1) + 8*(Bytes - 1) + Bit;
				return port_index;
                #else
                port_index = 8*(Bytes - 1) + Bit;
				return port_index;
				#endif
			
			}
			else if(PRODUCT_ID_AU3K == productId ||
				PRODUCT_ID_AU3K_BCM == productId ||
				PRODUCT_ID_AU3K_BCAT == productId ||
				PRODUCT_ID_AU2K_TCAT == productId){
				if(Bytes >0){
					port_index = 8 * (Bytes - 1) + (Bit-1);/*zhengcaisheng changes*/
					return port_index;
				}
				else {
					return -2;
				}
			}
			else if(PRODUCT_ID_AX5K_E == productId ||
					PRODUCT_ID_AX5608== productId) {
				if(Bytes >0){
					port_index = 8 * (Bytes - 1) + Bit;
					return port_index;
				}
				else {
					return -2;
				}
			}
			/*
 			else if(PRODUCT_ID_AX5K == productId || PRODUCT_ID_AU4K == productId || PRODUCT_ID_AX5K_I == productId) {
				if(Bytes >0){
					port_index = 64 + 8 * (Bytes - 1) + (Bit-1);//zhengcaisheng changes
					return port_index;
				}
				else {
					return -2;
				}
			}*/
			else if(PRODUCT_ID_AX5K_I== productId) {
				if(Bytes >0){
					port_index = 56 + 8 * (Bytes - 1) + Bit;/*zhengcaisheng changes*/
					return port_index;
				}
				else {
					return -2;
				}
			}
			else if(PRODUCT_ID_AU4K == productId || PRODUCT_ID_AX5K == productId) {
				if(Bytes >0){
					port_index = 64 + 8 * (Bytes - 1) + (Bit-1);/*zhengcaisheng changes*/
					return port_index;
				}
				else {
					return -2;
				}
			}
			else if(PRODUCT_ID_AX7K == productId){
				port_index = 64*Bytes + Bit-1;
				return port_index;
				
			}
			else {
				return -2;
			}
		}
		else
			return -1;
	}
}

unsigned int  stp_bitmap_get_panelport_from_portindex
(
    unsigned int eth_g_index,
    unsigned char *slot_no,
    unsigned char *port_no
)
{
	unsigned char slot_index = 0,port_index = 0;
	
	slot_index = ((eth_g_index & 0x000007C0) >> 6);
	port_index = (eth_g_index & 0x0000003F);

	if(stp_is_distributed == STP_IS_DISTRIBUTED){
		*slot_no = slot_index + 1;/*slot_start_no is 1*/
		*port_no = port_index + 1;/*port start no is 1*/
	}
	else if(PRODUCT_ID_AX7K == productId){
		*slot_no = slot_index;//AU7K,AU3K,slot_start_no is 0
		*port_no = port_index + 1;//AU7K port start no is 1
		if((*slot_no > 5)||(*port_no > 6)){
			return STP_ERR;
		}
		// printf("slot_no %d,port_no %d\n",*slot_no,*port_no);
	}
	else if(PRODUCT_ID_AX5K == productId){
		*slot_no = slot_index;//AX5K,4K.slot start no is 0
		*port_no = port_index + 1;//AX5K port start no is 1
		if((*slot_no > 1)||(*port_no > 28)){
			return STP_ERR;
		}
		// printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}
	else if(PRODUCT_ID_AX5K_I == productId){
		*slot_no = slot_index;//AX5K,4K.slot start no is 0
		*port_no = port_index + 1;//AX5K port start no is 1
		if((*slot_no > 1)||(*port_no > 28)){
			return STP_ERR;
		}
		// printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}
	else if(PRODUCT_ID_AU4K == productId){//AX5K,4K.slot start no is 0
       *slot_no= slot_index;
	   *port_no = port_index + 1;//AU4K port start no is 1
	   if((*slot_no > 1)||(*port_no > 28)){
         return STP_ERR;
	   }
	  //printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}
	else if(PRODUCT_ID_AU3K == productId){
       *slot_no = slot_index + 1;//AU3K,slot_start_no is 0
	   *port_no = port_index + 1 ;//AU3K port start no is 0
	   //::TODO　restrict the slot,port !!!
	   
	 // printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}
	else if(PRODUCT_ID_AU3K_BCM == productId){
	   *slot_no = slot_index + 1;//AU3K,slot_start_no is 0
	   *port_no = port_index + 1 ;//AU3K port start no is 0
	   //::TODO　restrict the slot,port !!!
		   
		 // printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}	 
	else if(PRODUCT_ID_AU3K_BCAT == productId){
	   *slot_no = slot_index + 1;//AU3K,slot_start_no is 0
	   *port_no = port_index + 1 ;//AU3K port start no is 0	
	}
   	else if(PRODUCT_ID_AU2K_TCAT == productId){
	   *slot_no = slot_index + 1;//AU3K,slot_start_no is 0
	   *port_no = port_index + 1 ;//AU3K port start no is 0
	}
	return STP_OK;
   /**********************
	*enum product_id_e {
	*	PRODUCT_ID_NONE,
	*	PRODUCT_ID_AX7K,
	**	PRODUCT_ID_AX5K,
	*	PRODUCT_ID_AU4K,
	*	PRODUCT_ID_AU3K,
	*	PRODUCT_ID_MAX
	*};
       **********************/
	
	/*************************************  
	*chassis_slot_count, chassis_slot_start_no
	*{0,0},  // PRODUCT_ID_NONE
	*{5,0},	// PRODUCT_ID_AX7K
	*{1,1},	// PRODUCT_ID_AX5K
	*{1,1},	// PRODUCT_ID_AU4K
	*{0,0}	// PRODUCT_ID_AU3K
	*************************************/
	/* ***********************************
	*ext_slot_count,  eth_port_1no, eth_port_count	
	*{0,0,0},   	//  MODULE_ID_NONE
       * {1,1,4},   	//  MODULE_ID_AX7_CRSMU
	*{0,1,6},	// MODULE_ID_AX7_6GTX
	*{0,1,6},    // MODULE_ID_AX7_6GE_SFP
	*{0,1,1},	// MODULE_ID_AX7_XFP
	*{0,1,6},	// MODULE_ID_AX7_6GTX_POE
	*{0,0,0},	// MODULE_ID_AX5
       ****************************************/
}
#ifdef __cplusplus
}
#endif

