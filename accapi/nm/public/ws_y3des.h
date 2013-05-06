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
* ws_y3des.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for des3  
*
*
*******************************************************************************/
#ifndef _WS_Y3DES_H                            
#define _WS_Y3DES_H

#include <string.h>


//类构造函数
extern void ymDES2_Initialize(); 

//功能:产生16个28位的key
//参数:源8位的字符串(key),存放key的序号0-1
//结果:函数将调用private CreateSubKey将结果存于char SubKeys[keyN][16][48]
extern void ymDES2_InitializeKey(char* srcBytes,unsigned int keyN);

//功能:加密8位字符串
//参数:8位字符串,使用Key的序号0-1
//结果:函数将加密后结果存放于private szCiphertext[16]
//      用户通过属性Ciphertext得到
extern void ymDES2_EncryptData(char* _srcBytes,unsigned int keyN);

//功能:解密16位十六进制字符串
//参数:16位十六进制字符串,使用Key的序号0-1
//结果:函数将解密候结果存放于private szPlaintext[8]
//      用户通过属性Plaintext得到
extern void ymDES2_DecryptData(char* _srcBytes,unsigned int keyN);

//功能:加密任意长度字符串
//参数:任意长度字符串,长度,使用Key的序号0-1
//结果:函数将加密后结果存放于private szFCiphertextAnyLength[8192]
//      用户通过属性CiphertextAnyLength得到
extern void ymDES2_EncryptAnyLength(char* _srcBytes,unsigned int _bytesLength,unsigned int keyN);

//功能:解密任意长度十六进制字符串
//参数:任意长度字符串,长度,使用Key的序号0-1
//结果:函数将加密后结果存放于private szFPlaintextAnyLength[8192]
//      用户通过属性PlaintextAnyLength得到
extern void ymDES2_DecryptAnyLength(char* _srcBytes,unsigned int _bytesLength, unsigned int keyN);

//功能:Bytes到Bits的转换,
//参数:待变换字符串,处理后结果存放缓冲区指针,Bits缓冲区大小
extern void ymDES2_Bytes2Bits(char *srcBytes, char* dstBits, unsigned int sizeBits);

//功能:Bits到Bytes的转换,
//参数:待变换字符串,处理后结果存放缓冲区指针,Bits缓冲区大小
extern void ymDES2_Bits2Bytes(char *dstBytes, char* srcBits, unsigned int sizeBits);

//功能:Int到Bits的转换,
//参数:待变换字符串,处理后结果存放缓冲区指针
extern void ymDES2_Int2Bits(unsigned int srcByte, char* dstBits);
		
//功能:Bits到Hex的转换
//参数:待变换字符串,处理后结果存放缓冲区指针,Bits缓冲区大小
extern void ymDES2_Bits2Hex(char *dstHex, char* srcBits, unsigned int sizeBits);
		
//功能:Bits到Hex的转换
//参数:待变换字符串,处理后结果存放缓冲区指针,Bits缓冲区大小
extern void ymDES2_Hex2Bits(char *srcHex, char* dstBits, unsigned int sizeBits);

//szCiphertextInBinary的get函数
extern char* ymDES2_GetCiphertextInBinary();

//szCiphertextInHex的get函数
extern char* ymDES2_GetCiphertextInHex();

//Ciphertext的get函数
extern char* ymDES2_GetCiphertextInBytes();

//Plaintext的get函数
extern char* ymDES2_GetPlaintext();

//CiphertextAnyLength的get函数
extern char* ymDES2_GetCiphertextAnyLength();

//PlaintextAnyLength的get函数
extern char* ymDES2_GetPlaintextAnyLength();

//功能:生成子密钥
//参数:经过PC1变换的56位二进制字符串,生成的szSubKeys编号0-1
//结果:将保存于char szSubKeys[16][48]
extern void ymDES2_CreateSubKey(char* sz_56key,unsigned int keyN);

//功能:DES中的F函数,
//参数:左32位,右32位,key序号(0-15),使用的szSubKeys编号0-1
//结果:均在变换左右32位
extern void ymDES2_FunctionF(char* sz_Li,char* sz_Ri,unsigned int iKey,unsigned int keyN);

//功能:IP变换
//参数:待处理字符串,处理后结果存放指针
//结果:函数改变第二个参数的内容
extern void ymDES2_InitialPermuteData(char* _src,char* _dst);

//功能:将右32位进行扩展位48位,
//参数:原32位字符串,扩展后结果存放指针
//结果:函数改变第二个参数的内容
extern void ymDES2_ExpansionR(char* _src,char* _dst);

//功能:异或函数,
//参数:待异或的操作字符串1,字符串2,操作数长度,处理后结果存放指针
//结果: 函数改变第四个参数的内容
extern void ymDES2_XOR(char* szParam1,char* szParam2, unsigned int uiParamLength, char* szReturnValueBuffer);

//功能:S-BOX , 数据压缩,
//参数:48位二进制字符串,
//结果:返回结果:32位字符串
extern void ymDES2_CompressFuncS(char* _src48, char* _dst32);

//功能:IP逆变换,
//参数:待变换字符串,处理后结果存放指针
//结果:函数改变第二个参数的内容
extern void ymDES2_PermutationP(char* _src,char* _dst);

                         
#endif
 
