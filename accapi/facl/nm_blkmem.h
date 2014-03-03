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
* nm_blkmem.h
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*
*******************************************************************************/

#ifndef _NM_BLKMEM_H
#define _NM_BLKMEM_H

#define MAX_BLK_NUM			256

typedef struct nm_blk_mem nm_blk_mem_t;

int 
nm_blkmem_create(nm_blk_mem_t ** nm_blk_mem,
		      char *name,
		      unsigned int item_size, unsigned int blk_item_num, 
		      unsigned int max_blk_num);
int 
nm_blkmem_destroy(nm_blk_mem_t ** nm_blk_mem);
void *
nm_blkmem_malloc_item(nm_blk_mem_t * nm_blk_mem);
int 
nm_blkmem_free_item(nm_blk_mem_t * nm_blk_mem, void *buff);

void
nm_blkmem_log_all_blkmem( char *blkmem_name );

#endif

