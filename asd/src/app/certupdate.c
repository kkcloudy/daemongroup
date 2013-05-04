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
* AsdCertupdate.c
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
#include <stdlib.h>
#include "include/proc.h"
#include "include/certupdate.h"
#include "include/debug.h"

/*更新AP证书的状态*/
int update_cert_status(char *fileconfig)
{
	prop_data properties[KEYS_MAX];
	int prop_count=0;
	char get_cert_status[255] ={0,};
	int res = 0;

	/*从配置文件中读取证书状态*/
	prop_count=load_prop(SEP_EQUAL,fileconfig,properties);
	get_prop("CERT_STATUS", get_cert_status, properties, prop_count);
	free_prop(properties,prop_count);

	/*判断证书状态是否是无效*/
	if(atoi(get_cert_status) != 1)
	{
		/*将状态修改为有效*/
		res = !save_cert_status(fileconfig, "1");
	}
	return res;
}

/*保存证书状态*/
int save_cert_status(char *fileconfig, char *cert_flag)
{
	int res = 0;
	res = !save_global_conf(SEP_EQUAL,fileconfig,"",  "CERT_STATUS",cert_flag);
	if(res != 0)
	{
		DPrintf("open file %s error\n", fileconfig);
	}
	return res;
}

