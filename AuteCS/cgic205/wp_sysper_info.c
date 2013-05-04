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
* wp_sysper_info.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system infos 
*
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include "cgic.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_nm_status.h"
#include "ws_sysinfo.h"
#if 0
#ifndef MIN
#define MIN(a,b) ( (a)<(b) ? (a):(b) ) 
#endif

#define MAX_CPU 16
#define CPU_STATUS 8
#define DEFAULT_REFRESH 1
#define CPU_USAGE_IDEL 3

#define MAX_LEN 2048 
#define VALUE_LEN 32
#define M_SIZE 1024

#define SUCCESS 0
#define SYS_ERR 1
#define PARA_ERR 2

static char* cp_cpu_info = "/proc/stat";
static char* cp_mem_info = "/proc/meminfo";
//static char* LANGUAGE_PATH = "../htdocs/text/en-ch.txt";

typedef struct st_Cpus_Status
{   
    unsigned int cpu_no;
    float ar_cpu_usage[MAX_CPU][CPU_STATUS];
}CPUS_STU,*P_CPUS_STU;

typedef struct st_Cpu_Temperature
{    
    unsigned int un_tem_core;    
    unsigned int un_tem_sface;
}CPU_TEM,*P_CPU_TEM;

typedef struct st_Mem_Status
{    
    unsigned int un_mem_total;    
    unsigned int un_mem_used;    
    unsigned int un_mem_free;    
    unsigned int un_mem_buffers;
}MEM_STU,*P_MEM_STU;

typedef struct st_Sys_Info
{    
    P_CPUS_STU pst_cpus_status;    
    P_CPU_TEM pst_cpu_temperature;    
    P_MEM_STU pst_mem_status;
}SYS_INFO,*P_SYS_INFO;
 
static CPUS_STU st_cpus_status;
static CPU_TEM st_cpu_temperature;
static MEM_STU st_mem_status;
static SYS_INFO st_sys_info={ &st_cpus_status, 
                              &st_cpu_temperature,  
                              &st_mem_status};
                             

static char* get_token(char *str_tok, unsigned int *pos)
{
    unsigned int temp = *pos;
    while(isspace(*(str_tok+temp)) && (*(str_tok+temp) != '\0'))
    {
        temp++;
    }
    while(!isspace(*(str_tok+temp)) && (*(str_tok+temp) != '\0'))
    {
        temp++;
    }
    while(isspace(*(str_tok+temp)) && (*(str_tok+temp) != '\0'))
    {
        temp++;
    }
    *pos = temp;
    return (char *)(str_tok+temp);
}

static int get_cpu_status(unsigned int *cpu_times, unsigned int *count)
{
    FILE *fp = NULL;
    char buffer[BUF_LEN];
    char *iter = NULL;
    unsigned int begin = 0;
    unsigned int cpu_no = 0;
    unsigned int cpu_status = 0;
    (*count) = 0;
    
    memset(buffer, 0, sizeof(buffer));
    memset(cpu_times, 0, sizeof(unsigned int)*MAX_CPU*CPU_STATUS);
    
    if(NULL != (fp = fopen(cp_cpu_info,"r")))
    {  
        cpu_no = 0;
        cpu_status = 0;
        while( (NULL != fgets(buffer, BUF_LEN, fp)) && cpu_no < MAX_CPU )
        {    
            iter = buffer;
            begin = 0;
            cpu_status = 0;
            if(buffer[begin]=='c' && buffer[begin+1]=='p' && buffer[begin+2]=='u' )
            {
              if( buffer[begin+3]!=' ')
              {   
                  (*count)++;
                  get_token(buffer, &begin);
                  iter += begin;
                  while(cpu_status < CPU_STATUS)
                  {
                      *(cpu_times+cpu_no*CPU_STATUS+(cpu_status++)) = strtoul(iter, &iter, 0);
                  }
                  cpu_no ++;
              }
            }
            else
            {
                break;
            }
        }
        fclose(fp);
        return SUCCESS;
    }
    return SYS_ERR;
}

static int get_cpu_usage(P_SYS_INFO pst_sys_info, unsigned int refresh )
{
    unsigned int cpu_no = 0;
    unsigned int cpu_stu = 0;
    int total_time = 0;
    unsigned int *cpu_time_old = NULL;
    unsigned int *cpu_time_now = NULL; 
    float *cpu_usage = NULL;
    int *diffs_temp = NULL;
    unsigned int time_old[MAX_CPU][CPU_STATUS];
    unsigned int time_cur[MAX_CPU][CPU_STATUS];
    int time_diffs[CPU_STATUS];
    unsigned int count_1 = 0;
    unsigned int count_2 = 0;

    memset((void *)time_old, 0, MAX_CPU*CPU_STATUS*sizeof(unsigned int));
    memset((void *)time_cur, 0, MAX_CPU*CPU_STATUS*sizeof(unsigned int));
    memset((void *)time_diffs, 0, CPU_STATUS*sizeof(unsigned int));
    
    if(SUCCESS == get_cpu_status((unsigned int *)time_old, &count_1))
    {   
        sleep(refresh);
        if(SUCCESS == get_cpu_status((unsigned int *)time_cur, &count_2))
        {
            pst_sys_info->pst_cpus_status->cpu_no = MIN(count_1, count_2);
            for(;cpu_no<(pst_sys_info->pst_cpus_status->cpu_no);cpu_no++)
            {
                cpu_stu = 0;
                total_time = 0;
                diffs_temp = time_diffs;
                cpu_time_old = (unsigned int *)time_old + CPU_STATUS*cpu_no;
                cpu_time_now = (unsigned int *)time_cur + CPU_STATUS*cpu_no;
                cpu_usage = (float *)pst_sys_info->pst_cpus_status->ar_cpu_usage + CPU_STATUS*cpu_no;
                memset(diffs_temp, 0, sizeof(int)*CPU_STATUS);
                for(;cpu_stu<CPU_STATUS;cpu_stu++)
                {
                    *diffs_temp = (int)((*cpu_time_now++)-(*cpu_time_old++));
                    total_time += *diffs_temp++;
                }
                total_time = (total_time == 0)?1:total_time;
                cpu_stu = 0;
                diffs_temp = time_diffs;
                for(;cpu_stu<CPU_STATUS;cpu_stu++)
                {
                    *cpu_usage++ = (*diffs_temp++)*100.0/total_time;
                }
            }
        }
        return SYS_ERR;
    }
    return SYS_ERR;
}

static int get_mem_usage( P_SYS_INFO pst_sys_info )
{
    FILE *fp = NULL;
    char buffer[BUF_LEN];
    char *iter = NULL;
    unsigned int begin = 0;
    
    memset(buffer, 0, sizeof(buffer));
    if(NULL != (fp = fopen(cp_mem_info,"r")))
    {  
        if(NULL == fgets(buffer, BUF_LEN, fp))
        {
        	fclose(fp);
            return SYS_ERR;
        }
        get_token(buffer, &begin);
        iter = buffer + begin;
        pst_sys_info->pst_mem_status->un_mem_total = strtoul(iter, &iter, 0);

        if(NULL == fgets(buffer, BUF_LEN, fp))
        {
        	fclose(fp);
            return SYS_ERR;
        }
        begin = 0;
        get_token(buffer, &begin);
        iter = buffer + begin;
        pst_sys_info->pst_mem_status->un_mem_free = strtoul(iter, &iter, 0);

        if(NULL == fgets(buffer, BUF_LEN, fp))
        {
        	fclose(fp);
            return SYS_ERR;
        }
        begin = 0;
        get_token(buffer, &begin);
        iter = buffer + begin;
        pst_sys_info->pst_mem_status->un_mem_buffers = strtoul(iter, &iter, 0);

        pst_sys_info->pst_mem_status->un_mem_used = pst_sys_info->pst_mem_status->un_mem_total - pst_sys_info->pst_mem_status->un_mem_free;
        
        fclose(fp);
        return SUCCESS;
    }
    return SYS_ERR;

}

static int get_cpu_temperature( P_SYS_INFO pst_sys_info )
{
    struct sys_envir sys;
    memset((struct sys_envir *)&sys, 0, sizeof(struct sys_envir));
    
    ccgi_dbus_init();  //用此函数了，从底层取值了啊 
	if( show_sys_envir(&sys) == CCGI_SUCCESS)
	{
	    st_sys_info.pst_cpu_temperature->un_tem_core = (unsigned int)sys.core_tmprt;
	    st_sys_info.pst_cpu_temperature->un_tem_sface = (unsigned int)sys.surface_tmprt;

	    return SUCCESS;
	}
    return SYS_ERR;
}
#endif
 CPUS_STU st_cpus_status;
 CPU_TEM st_cpu_temperature;
 MEM_STU st_mem_status;
 SYS_INFO st_sys_info={ &st_cpus_status, 
                              &st_cpu_temperature,  
                              &st_mem_status};


 

static void get_xml_mem(char *str_xml)
{
    unsigned int mem_used = st_sys_info.pst_mem_status->un_mem_used/M_SIZE;
    unsigned int mem_total = st_sys_info.pst_mem_status->un_mem_total/M_SIZE;
    
    if( NULL == str_xml )
    {
        return;
    }
    strcat(str_xml, "<ems>");
    strcat(str_xml, "<used>");
    sprintf(str_xml+strlen(str_xml), "%d", mem_used);
    strcat(str_xml, "</used>");
    strcat(str_xml, "<max>");
    sprintf(str_xml+strlen(str_xml), "%d", mem_total);
    strcat(str_xml, "</max>");
    strcat(str_xml, "</ems>");
    
}

static void get_xml_temperature(char *str_xml)
{
    unsigned int cpu_tem = st_sys_info.pst_cpu_temperature->un_tem_core;
    
    if( NULL == str_xml )
    {
        return;
    }
    strcat(str_xml, "<tem>");
    sprintf(str_xml+strlen(str_xml), "%d", cpu_tem);
    strcat(str_xml, "</tem>");
}

static void get_xml_cpuinfo(char *str_xml)
{
    unsigned int cpu_seq = 0;
    unsigned int cpu_usage = 0;
	unsigned int total_cpu_usage = 0;	
	
    for(;cpu_seq<st_sys_info.pst_cpus_status->cpu_no;cpu_seq++)
    {
    /*
        strcat(str_xml, "<cpu>");
        strcat(str_xml, "<id>");
        sprintf(str_xml+strlen(str_xml), "%d", cpu_seq+1);
        strcat(str_xml, "</id>");    
        strcat(str_xml, "<rate>");
        cpu_usage = (unsigned int)(100 - st_sys_info.pst_cpus_status->ar_cpu_usage[cpu_seq][CPU_USAGE_IDEL]);
        sprintf(str_xml+strlen(str_xml), "%d", cpu_usage);
        strcat(str_xml, "</rate>");   
        strcat(str_xml, "</cpu>");
   */
        cpu_usage = (unsigned int)(100 - st_sys_info.pst_cpus_status->ar_cpu_usage[cpu_seq][CPU_USAGE_IDEL]);
		total_cpu_usage+=cpu_usage;

		
    }

	
	
	strcat(str_xml, "<cpu>");
	sprintf(str_xml+strlen(str_xml), "%d", (unsigned int)(total_cpu_usage/st_sys_info.pst_cpus_status->cpu_no));	
	strcat(str_xml, "</cpu>");

    
	
	
}

static void create_xml(char *str_xml)
{
    if(NULL == str_xml)
    {
        return;
    }
    strcpy(str_xml, "<?xml version=\"1.0\" encoding=\"utf-8\"?>"\
                    "<root>");
                    
    get_xml_mem(str_xml);
    get_xml_temperature(str_xml);
    get_xml_cpuinfo(str_xml);
    
    strcat(str_xml, "</root>");
    
}

static int create_sys_info(char *str_xml)
{
    memset(str_xml, 0, sizeof(char)*MAX_LEN);
    get_cpu_usage(&st_sys_info, DEFAULT_REFRESH);
    get_mem_usage(&st_sys_info);
    get_cpu_temperature(&st_sys_info);
    create_xml(str_xml);
    return 0;
}

int cgiMain()
{
  char *str_xml = NULL;
  if(NULL == (str_xml = (char *)malloc(sizeof(char)*MAX_LEN)))
  {
      return 1;
  }
  memset(str_xml, 0, sizeof(char)*MAX_LEN);
  cgiHeaderContentType("text/xml");
  
  create_sys_info(str_xml);

  fprintf(cgiOut,"%s",str_xml);
  
  free(str_xml);
  return 0;
}

#ifdef __cplusplus
}
#endif
