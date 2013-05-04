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
* AsdDebug.c
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "include/debug.h"
#include "include/structure.h"

char* SPrintBuffer(const void *start, int offset, int len)
{
	const int max_len = 32;
	static char s_out[32*3+3], s_tmp[8];
	int i = 0;
	int before = 0, after = 0;
	unsigned char *buf = NULL;

	if (len > max_len) len = max_len;
	if (offset < len)  len = offset;

	before = len/2 + len%2;
	after  = len - before;

	s_out[0] = '\0';
	buf = (unsigned char *)start;
	for (i=0; i<before; i++) {
		sprintf(s_tmp, "%02X ", buf[i]);
		strcat(s_out, s_tmp);
	}

	if (len < offset) strcat(s_out, "... ");

	buf = (unsigned char *)start + offset - after;
	for (i=0; i<after; i++) {
		sprintf(s_tmp, "%02X ", buf[i]);
		strcat(s_out, s_tmp);
	}
	
	return s_out;	
}
void PrintBuffer(const void* start, int offset)
{
	int i, j, m, n;
	char *  tmpstr;
	tmpstr = (char *)start;
	
	if(start == 0 || start == (void *)0xFFFFFFFF)
		return;
	
	m = offset / 16;
	n = offset % 16;
	
	for(i = 0; i < m + (n == 0 ? 0 : 1); i++)
	{
		printf("%04X:  ", i * 16);
		for (j = 0; j < (i < m ? 16 : n); j ++)
		{
			printf("%02X ", tmpstr[(i * 16) + j] & 0x00FF);
		}
		for(; j < 16; j++) printf("   ");
		for (j = 0; j < (i < m ? 16 : n); j ++)
		{
			if(tmpstr[(i * 16) + j] >= 32 )
				printf("%c", tmpstr[(i * 16) + j]);
			else
				printf(".");
		}
		
		printf("\n");
	}
}
void print_hex_string(void *_buf, int len)
{
    int i;
    char *buf = (char *)_buf;
    if (len==0) { printf("<empty string>"); return; }
    if (len>=40) {
        for (i = 0; i < 10; i++)
             printf("%02X ", *((unsigned char *)buf + i));
        printf(" ... ");
        for (i = len-10; i < len; i++)
             printf("%02X ", *((unsigned char *)buf + i));
        printf(" [%d bytes]", len);
        return;
    }
    for (i = 0; i < len; i++)
        printf("%02X", *((unsigned char *)buf + i));
}


void print_to_string(char* buf, int len)
{
    int i;

    if (len==0) { printf("<empty string>"); return; }
    if (len>=40) {
        for (i = 0; i < 20; i++)
             printf("%c", *((unsigned char *)buf + i));
        printf(" ... ");
        for (i = len-20; i < len; i++)
             printf("%c", *((unsigned char *)buf + i));
        printf(" [%d bytes]", len);
        return;
    }
    for (i = 0; i < len; i++)
        printf("%c", *((unsigned char *)buf + i));
}
	
void print_string(void *_str,int len)
{
	int i;
	unsigned char *str = (unsigned char *)_str;
	
    	for(i=0;i<len;i++)
	{
		printf("%02X ",*str++);
		if((i+1)%16==0) printf("\n");
	}
	printf("\n");
}

void print_string_array(char *name, void *_str,int len)
{
	int i;
	unsigned char *str = (unsigned char *)_str;

	if(name != NULL)
		printf("%s(%d) :\n", name, len);
	for(i=0;i<len;i++)
	{
		printf("%02X ",*str++);
		if((i+1)%16==0) printf("\n");
	}
	printf("\n");
}

