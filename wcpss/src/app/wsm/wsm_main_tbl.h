/******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
*******************************************************************************

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
*******************************************************************************
* wsm_main_tbl.h
*
*
* DESCRIPTION:
*  The functions about WTP-BSS-STA tables, Add, delete, or modify.There tables 
*  will be used when parse frame.  
*
* DATE:
*  2008-07-11
*
* CREATOR:
*  guoxb@autelan.com
*
* CHANGE LOG:
*  2009-12-14 <guoxb> Add Inter-AC roaming table operation functions.
*  2009-12-29 <guoxb> Review source code, almost re-write all of table 
*                                operation function.
*  2010-02-09 <guoxb> Modified files name, copyright etc.
*
******************************************************************************/

#ifndef _WSM_MAIN_TBL_H
#define _WSM_MAIN_TBL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include<netinet/in.h> 

#include "wcpss/waw.h"
#include "wsm_wifi_bind.h"


void wsm_tbl_init(void);
int wsm_tbl_add(int type,  void *pdata);
int wsm_tbl_del(int type,  void *pdata);
int wsm_tbl_modify(int type,  void *pdata);
void* wsm_tbl_search(int type,  unsigned char *pdata,unsigned int wtpid);
unsigned char wsm_tbl_mac_key(unsigned char *pMAC);
void wsm_tbl_element_print(int type, void *pdata);
void wsm_tbl_print(int type);

#endif
