#ifndef __STP_BITMAP_H
#define __STP_BITMAP_H

#define NUMBER_OF_PORTS 51
#define NUMBER_OF_BYTES 9
#define BIT_OF_BYTE 8

#define STP_OK 0
#define STP_ERR (STP_OK + 1)

/* For distributed system  2011-09-09 */
#define STP_NOT_DISTRIBUTED  0
#define STP_IS_DISTRIBUTED   1


typedef struct tagBITMAP
{
  unsigned char part[9];
} BITMAP_T;

void stp_bitmap_clear(BITMAP_T*  BitmapPtr);
 

void  stp_bitmap_set_allbits(BITMAP_T* BitmapPtr);

/* Bit range [0 .. 63] */
void  stp_bitmap_set_bit(BITMAP_T* BitmapPtr,int Bit); 

    //(BitmapPtr)->part0 |= (1 << (Bit)); }

void stp_bitmap_clear_bit(BITMAP_T* BitmapPtr, int Bit); 
    //(BitmapPtr)->part0 &= ~(1 << (Bit));

int stp_bitmap_get_bit(BITMAP_T* BitmapPtr, int Bit);

int stp_bitmap_portbmp_add(BITMAP_T* desPtr,BITMAP_T* souPtr);

int stp_bitmap_get_portindex_from_bmp(BITMAP_T* BitmapPtr,int Bytes,int Bit);

         //((BitmapPtr)->part0 & (1 << (Bit)))
/*#define BitmapCopy(BitmapDstPtr,BitmapSrcPtr) \
        { (BitmapDstPtr)->part0 = (BitmapSrcPtr)->part0; }

#define BitmapXor(ResultPtr,BitmapPtr1,BitmapPtr2) \
        { (ResultPtr)->part0 = (BitmapPtr1)->part0 ^ (BitmapPtr2)->part0; }

#define BitmapClearBits(BitmapPtr,BitmapBitsPtr) \
        { (BitmapPtr)->part0 &= ~((BitmapBitsPtr)->part0); }

#define BitmapSetBits(BitmapPtr,BitmapBitsPtr) \
        { (BitmapPtr)->part0 |= ((BitmapBitsPtr)->part0); }

#define BitmapOr(ResultPtr,BitmapPtr1,BitmapPtr2) \
        { (ResultPtr)->part0 = (BitmapPtr1)->part0 | (BitmapPtr2)->part0; }

#define BitmapAnd(ResultPtr,BitmapPtr1,BitmapPtr2) \
        { (ResultPtr)->part0 = (BitmapPtr1)->part0 & (BitmapPtr2)->part0; }

#define BitmapNot(ResultPtr,BitmapPtr) \
        { (ResultPtr)->part0 = ~((BitmapPtr)->part0); }


 Return zero if identical 
#define BitmapCmp(BitmapPtr1,BitmapPtr2) \
        ((BitmapPtr1)->part0 != (BitmapPtr2)->part0) 

#define BitmapIsZero(BitmapPtr) \
        (!(BitmapPtr)->part0)

#define BitmapIsAllOnes(BitmapPtr) \
        ((BitmapPtr)->part0 == 0xF)*/


#endif /* __STP_BITMAP_H */

