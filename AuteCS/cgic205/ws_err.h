/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* ws_err.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
* tangsq@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#ifndef _WS_ERR_H
#define _WS_ERR_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define _release	1//when you commint this file, please sure this macro is set to 1

#if _release
#define ASSERT(sure)
#define DEBUG_PRINT( format... ) 
#else
#define ASSERT(sure) if(!(sure)){fprintf( stderr, "ASSERT ERR  FILE : %s ; LINE : %d;", __FILE__,__LINE__ );exit(0);}
#define DEBUG_PRINT( format... ) {fprintf( stderr, "--> FILE: %s ; LINE: %d ;", __FILE__, __LINE__ );fprintf( stderr, format );}		
#endif


void ShowErrorPage(char *message); /*输出警告框，警告信息为message，点击返回按钮返回登陆界面*/
void ShowAlert(char *message);       /*输出提示框，点击确定继续其它操作*/
extern void LogoffPage(char *username);

#endif

