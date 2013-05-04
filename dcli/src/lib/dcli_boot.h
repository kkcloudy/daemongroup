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
* dcli_boot.h
*
* MODIFY:
*		
*
* CREATOR:
*		shancx@autelan.com
*
* DESCRIPTION:
*		CLI definition for bootfile 
*
* DATE:
*		10/27/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.11 $	
*******************************************************************************/
#ifndef __DCLI_BOOT_H__
#define __DCLI_BOOT_H__

#define BM_IOC_MAGIC 0xEC
#define BM_IOC_ENV_EXCH		_IOWR(BM_IOC_MAGIC,10,boot_env_t)
#define SAVE_BOOT_ENV 	1
#define GET_BOOT_ENV	0
#define VTYSH_MAX_FILE_LEN 32
#define INTERFACE_NAMSIZ 20
#define FILE_NAMESIZ 20

/*added by zhaocg for using sor.sh script */
#define SOR_COMMAND_SIZE 512 
#define SOR_OPFILENAME_SIZE 256


#define SEM_SLOT_COUNT_PATH       "/dbm/product/slotcount"

typedef struct boot_env
{	
	char name[64];	
	char value[128];	
	int operation;
}boot_env_t;

#define BOOT_ENV_VALUE_LEN 128

#define BM_IOC_BOOTROM_EXCH    _IOWR(BM_IOC_MAGIC, 17,bootrom_file)/*gxd update bootrom based on cli*/
typedef struct bootrom_file
{
	char path_name[PATH_MAX];
}bootrom_file;
extern int sor_exec(struct vty* vty,char* cmd,char* filename,int time);

#define SOR_VAR_PATH "/var/run/sad/"
#define SOR_RESULT_FILE_PRIX "sor_result"
extern void dcli_boot_init();
extern int is_distributed;
extern int is_active_master;

#define FILE_SYSTEM_READ_USER "/var/run/file_system_read_user"
#define FILE_SYSTEM_WRITE_USER "/var/run/file_system_write_user"
#define FILE_SYSTEM_EXEC_USER "/var/run/file_system_exec_user"
#define FILE_SYSTEM_READ_USERBK "/var/run/file_system_read_userbk"
#define FILE_SYSTEM_WRITE_USERBK "/var/run/file_system_write_userbk"
#define FILE_SYSTEM_EXEC_USERBK "/var/run/file_system_exec_userbk"
#define PERMIT_READ 1
#define PERMIT_WRITE 2
#define PERMIT_EXEC 3


#define BOARD_81SMU_PATH "/dbm/local_board/board_code"
#define AX81SMU 0

#endif /*__DCLI_BOOT_H__*/
