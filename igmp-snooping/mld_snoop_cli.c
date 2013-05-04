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
* mld_snoop_cli.c
*
*
* CREATOR:
* 		yangxs@autelan.com
*
* DESCRIPTION:
* 		mld inter source to handle orders or configure file
*
* DATE:
*		4/20/2010
*
* FILE REVISION NUMBER:
*  		$Revision: 1.1 $
*
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "igmp_snoop_com.h"
#include "igmp_snoop_inter.h"
#include "igmp_snoop.h"
#include "igmp_snp_log.h"
#include "sysdef/returncode.h"
#include "mld_snoop_main.h"
#include "mld_snoop_inter.h"

INT mld_set_enable(LONG flag);

INT mld_snp_set_enable_dbus();
INT mld_snp_set_disable_dbus();

extern LONG	mld_snoop_enable;	/*MLD snoop enable or not*/
/*******************************************************************************
 * mld_set_enable
 *
 * DESCRIPTION:
 *   		begin to set the igmp enable or disable.
 *
 * INPUTS:
 * 		flag - a flag to define enable or disable mld snooping
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set endis mld snoop done
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT mld_set_enable(LONG flag)
{

	int ret = IGMPSNP_RETURN_CODE_OK;

	if(ENABLE == igmp_snoop_enable)
	{
   		igmp_snp_syslog_err(" igmp snoop is enable, can not start mld snoop!\n");	
    	return IGMPSNP_RETURN_CODE_ERROR_SW;		
	}
	else
	{
    	if( flag )
		{
    		ret = mld_enable_init();
    	}
    	else
		{
    		ret = mld_snp_stop();
    	}

   		igmp_snp_syslog_dbg(" mld snoop is set flag %d ret %d.\n",flag,ret);				
		if(IGMPSNP_RETURN_CODE_OK == ret)
		{
    	    mld_snoop_enable = flag;		
		}
	}
	return IGMPSNP_RETURN_CODE_OK;
}


/*******************************************************************************
 * mld_snp_set_enable_dbus
 *
 * DESCRIPTION:
 *   		set the enable mld snooping flag .
 *
 * INPUTS:
 * 		null	
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set success
 *		IGMPSNP_RETURN_CODE_ALREADY_SET - the enable already set
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT mld_snp_set_enable_dbus()
{
	LONG flag = 0;
	int ret = IGMPSNP_RETURN_CODE_OK;
	
	if(!mld_snoop_enable){
		flag = MLD_SNOOP_YES;
		ret = mld_set_enable(flag);
	}
	else{
		igmp_snp_syslog_err("mld snp has been enable!\n");
		return IGMPSNP_RETURN_CODE_ALREADY_SET;/*error*/
	}
	return ret;/*success*/
}

/*******************************************************************************
 * mld_snp_set_disable_dbus
 *
 * DESCRIPTION:
 *   		set the disable igmp snooping flag .
 *
 * INPUTS:
 * 		null	
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set success
 *		IGMPSNP_RETURN_CODE_ALREADY_SET - the disable already set
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT mld_snp_set_disable_dbus()
{
	LONG flag = 0;
	int ret = IGMPSNP_RETURN_CODE_OK;
	
	if(mld_snoop_enable){
		flag = MLD_SNOOP_NO;
		ret = mld_set_enable(flag);
	}
	else{
		igmp_snp_syslog_err("mld snp has been Disable!\n");
		return IGMPSNP_RETURN_CODE_ALREADY_SET;
	}
	return ret;
}


#ifdef __cplusplus
}
#endif

