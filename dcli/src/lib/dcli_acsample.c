#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include "ac_sample_def.h"
#include "ac_sample_err.h"
#include "ws_dbus_def.h"
#include "ac_sample_dbus.h"
#include "dcli_acsample.h"

#include "command.h"
#include "dcli_main.h"

#define DCLI_SAMPLE_SET_STR  "set acsample info\n"

static unsigned int show_running_flag = 0; 

struct cmd_node acsample_node = 
{
	ACSAMPLE_NODE,
	"%s(config-acsample)# "
};


DEFUN(conf_acsample_func,
	conf_acsample_cmd,
	"config acsample",
	CONFIG_STR
	"Config acsample\n"
)
{
	if(CONFIG_NODE == vty->node)
	{
		vty->node = ACSAMPLE_NODE;
		vty->index = NULL;	
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}


DEFUN(conf_acsample_service_enable_func,
	conf_acsample_service_enable_cmd,
	"service (enable|disable)",
	"config acsample enable|disable\n"
	"config acsample enable\n"
	"config acsample disable\n"
)
{    
	int sample_state = 0;
    if(0 == strncmp(argv[0], "enable", strlen(argv[0]))) {
        sample_state = SAMPLE_SERVICE_STATE_ON;
    }
    else if(0 == strncmp(argv[0], "disable", strlen(argv[0]))) {
        sample_state = SAMPLE_SERVICE_STATE_OFF;
    }
    else {
        return CMD_WARNING;
    }

    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
            int iRet = AS_RTN_OK;
    		iRet = dbus_set_sample_service_state(dbus_connection_dcli[i]->dcli_dbus_connection, sample_state);
    		if(AS_RTN_OK != iRet) {
                vty_out(vty, "Slot(%d) :acsample service %s failed!\n", i, SAMPLE_SERVICE_STATE_ON == sample_state ? "enable" : "disable");
    		}
        }
    }  
    
    return CMD_SUCCESS;
}



DEFUN(set_acsample_params_func,
	set_acsample_params_cmd,
	"set acsample params ("SE_XML_ACSAMPLE_INTERVAL"|"SE_XML_ACSAMPLE_STATIC_TIME") <5-3600>",
	DCLI_SAMPLE_SET_STR
	"set acsample params\n"
	"set acsample params\n"
	"set acsample sample (cpu memory temperatrue) "SE_XML_ACSAMPLE_INTERVAL" \n"
	"set acsample statistic_time (cpu memory temperatrue) "SE_XML_ACSAMPLE_STATIC_TIME"\n"
	"set acsample param to value. seconds!\n"
)
{
	const char *conf_key;
	const char *conf_value;
	unsigned int value=10;
	int type = AC_SAMPLE_PARAM_TYPE_INTERVAL;

	conf_key = argv[0];
	conf_value = argv[1];
	value = strtoul( conf_value, NULL, 10 );

	if(0 == strncmp(conf_key,SE_XML_ACSAMPLE_INTERVAL, strlen(conf_key))) {
		type = AC_SAMPLE_PARAM_TYPE_INTERVAL;
	}
	else if(0 == strncmp(conf_key,SE_XML_ACSAMPLE_STATIC_TIME, strlen(conf_key))) {
		type = AC_SAMPLE_PARAM_TYPE_STATISTICS;
	}
	else{
		vty_out(vty, "%s : Do not has this param!\n", conf_value);
		return CMD_FAILURE;
	}
    
    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
            int iRet = AS_RTN_OK;
            iRet = dbus_set_sample_param(dbus_connection_dcli[i]->dcli_dbus_connection, type, value );
            if (AS_RTN_OK != iRet){
                vty_out(vty, "Slot(%d) :set %s to %d faild!\n", i, conf_key, value );
            }
        }
    }  

	return CMD_SUCCESS;
}

DEFUN(set_acsample_resend_func,
			set_acsample_resend_cmd, 
			"set acsample params "SE_XML_ACSAMPLE_RESEND_INTERVAL" <0-3600>",
			DCLI_SAMPLE_SET_STR
			"set acsample params\n"
			"set acsample params\n"
			"set acsample (cpu memory temperature ) "SE_XML_ACSAMPLE_RESEND_INTERVAL"\n"
			"set acsample param to value. seconds!\n")
{
	const char *conf_value=NULL;
	const char *conf_key=NULL;
	unsigned int value=10;
	int type = AC_SAMPLE_PARAM_TYPE_RESEND_INTERVAL;
	
#if 0	
	conf_key = argv[0];
	conf_value = argv[1];
	value = strtoul(conf_value, NULL, 10);
	if(0 == strncmp(conf_key,SE_XML_ACSAMPLE_RESEND_INTERVAL, strlen(conf_key))) {	
		type = AC_SAMPLE_PARAM_TYPE_RESEND_INTERVAL;
	}
	else {
		vty_out(vty, "%s : Do not has this param!\n", conf_value);
		return CMD_FAILURE;
	}
#endif	
	conf_value = argv[0];
	value = strtoul(conf_value, NULL, 10);

	int i = 1;
	for(i = 1; i < MAX_SLOT; i++) {
		if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
			int iRet = AS_RTN_OK;
			iRet = dbus_set_sample_param(dbus_connection_dcli[i]->dcli_dbus_connection, type, value );
			if (AS_RTN_OK != iRet) {
			    vty_out(vty, "Slot(%d) :set %s to %d faild!\n", i, "resend time", value );
			}
		}
	} 

	return CMD_SUCCESS;
}


DEFUN(show_acsample_conf_func,
	show_acsample_conf_cmd,
	"show acsample configuration",
	"show acsample configuration\n"
	"show acsample configuration\n"
	"show acsample sample ( cpu memory temperature ) sample_interval,statistic_time,trap resend interval and threshold configuration\n"
)
{
    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {  
            vty_out(vty,"------------------------------------------------------------------------\n");
            vty_out(vty, "AcSample Slot             : %d \n", i ); 
            
        	int iRet = 0;
        	
        	char *samples[] = AC_SAMPLE_ARRAY;
        	int samples_num = sizeof(samples)/sizeof(samples[0]);

        	unsigned int sample_interval;
        	unsigned int statistics_time;
        	unsigned int resend_interval;
        	unsigned int threshold;

        	
        	iRet = dbus_get_sample_param(dbus_connection_dcli[i]->dcli_dbus_connection, 
        										&sample_interval,
        										&statistics_time,
        										&resend_interval );
        	if(AS_RTN_OK != iRet) {
        		vty_out(vty, "get acsample configuration failed!\n");
        	}
        	else
        	{
        		vty_out(vty,"The acsample configuration:\n");
        		vty_out(vty,"   sample interval      	:  %d\n", sample_interval);
        		vty_out(vty,"   statistic time      		:  %d\n", statistics_time);
        		vty_out(vty,"   trap resend interval 	:  %d\n", resend_interval);		
        	}
        	
        	int j = 0;
        	for(j = 0; j < samples_num; j++)
        	{
        		iRet = dbus_get_signal_threshold(dbus_connection_dcli[i]->dcli_dbus_connection, 
        											samples[j],
        											&threshold );
        		if(AS_RTN_OK != iRet) {
        			vty_out(vty,"   %11s threshold:  %s\n", samples[j], "error" );
        		}
        		else {
        			vty_out(vty,"   %11s threshold:  %d\n", samples[j], threshold );
        		}
        	}
            vty_out(vty,"------------------------------------------------------------------------\n");
        }    	
    }

	return CMD_SUCCESS;
}

/*added by stephen, for the cpu info show out */

DEFUN(show_acsample_cpu_stat_func,
	show_acsample_cpu_stat_cmd,
	"show cpu stat",
	"show cpu stat\n"
	"show cpu stat\n"
	"show cpu stat\n"

)
{
	int i = 0;
	int m = 0; 
	int k = 0;

	cpu_in_t cpu_data;

	for(i = 1; i < MAX_SLOT; i++){
		k = 0;
		if(dbus_connection_dcli[i]->dcli_dbus_connection){
			
			vty_out(vty, "gamestarted\n");
			if(AS_RTN_OK != dbus_get_cpu_stat(dbus_connection_dcli[i]->dcli_dbus_connection, &cpu_data)){
				vty_out(vty, "get cpu info failed!\n");

			}
			vty_out(vty, "SLOT %d:\n", i);
			for(m = 1; m <= cpu_data.cpu_cont; m++){
				vty_out(vty, "core%d\t\t", m);
				if(0 == m % 7){
					vty_out(vty, "\n");
					for(; k < cpu_data.cpu_cont; k++){
						vty_out(vty, "%f\t", cpu_data.cpu_info[k]);
						if(0 == (k + 1) % 7){
							vty_out(vty, "\n");
							k++;
							break;
						}

					}

				}
			}

			vty_out(vty, "average\t\n");
			for(k; k < cpu_data.cpu_cont; k++){
				vty_out(vty, "%f\t", cpu_data.cpu_info[k]);
			}
			vty_out(vty, "%f\t\n", cpu_data.cpu_average);

		}
		//print out the data as the special rule

		memset(&cpu_data, 0, sizeof(cpu_in_t));

	}

	vty_out(vty, "gamestopedd\n");


	return CMD_SUCCESS;


}


DEFUN(show_acsample_cpu_real_time_stat_func,
	show_acsample_cpu_real_time_stat_cmd,
	"show cpu real_time stat",
	"show cpu real_time stat\n"
	"show cpu real_time stat\n"
	"show cpu real_time stat\n"

)
{
	int retu = 0;
	int state = 0;
	int i = 1;
	int m = 0;
	int k = 0;
	cpu_in_t cpu_rt_data;
	

		syslog(LOG_DEBUG, "1111111");
	retu = dbus_get_sample_service_state(dcli_dbus_connection, &state);
		syslog(LOG_DEBUG, "2222222");
	if(AS_RTN_OK != retu)
	{
		vty_out(vty,"ac sample error!\n" );
		return CMD_FAILURE;
	}
	else
	{
		if( SAMPLE_SERVICE_STATE_OFF == state )
		{
			vty_out( vty, "acsample service is disable!\n" );
		}
		else
		{
			vty_out( vty, "acsample service is enable!\n" );
		}
	}   


	for(i = 1; i < MAX_SLOT; i++){
		k = 0;
		syslog(LOG_DEBUG, "3333333");
		if(dbus_connection_dcli[i]->dcli_dbus_connection){ 
			syslog(LOG_DEBUG, "zzzzzzz%p\n", dbus_connection_dcli[i]->dcli_dbus_connection);
			syslog(LOG_DEBUG, "yyyyyyy%d\n", dbus_connection_dcli[i]->dcli_dbus_connection);
			
			syslog(LOG_DEBUG, "4444444");
			if(AS_RTN_OK != dbus_get_sample_state(dbus_connection_dcli[i]->dcli_dbus_connection, SAMPLE_NAME_CPURT, &state))
			{
        		vty_out(vty,"ac sample error!" );
        		continue ;
        	}
			else{
				if( SAMPLE_SERVICE_STATE_OFF == state )
        		{
        			vty_out( vty, "slot %d cpu rt service is disable!\n", i );
        			continue ;
        		}
        		else
        		{
        			//vty_out( vty, "slot %d cpu rt service is enable!\n", i);

				}

		
			}
			syslog(LOG_DEBUG, "5555555");

		if(AS_RTN_OK != dbus_get_cpu_rt_stat(dbus_connection_dcli[i]->dcli_dbus_connection, &cpu_rt_data)){
						vty_out(vty, "get cpu info failed!\n");
		}
		syslog(LOG_DEBUG, "6666666");
		if(1 == i)
		{
		vty_out(vty, "%-4s  %-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s\n", "Slot",
				"ave", "cpu0", "cpu1", "cpu2", "cpu3", "cpu4", "cpu5", "cpu6", "cpu7", "cpu8", "cpu9", "cpu10", 
				"cpu11", "cpu12", "cpu13","cpu14", "cpu15");
		}
		vty_out(vty, "%-4d  ", i);
		vty_out(vty, "%-6.1f", cpu_rt_data.cpu_average);
		for(k = 0; k < cpu_rt_data.cpu_cont; k++){
			vty_out(vty, "%-6.1f", cpu_rt_data.cpu_info[k]);
		}
		vty_out(vty, "\n");
		}
		
		/*
		vty_out(vty, "SLOT %d:\n", i);
		for(m = 1; m <= cpu_rt_data.cpu_cont; m++){
			vty_out(vty, "core%d\t\t", m);
			if(0 == m % 7){
				vty_out(vty, "\n");
				for(; k < cpu_rt_data.cpu_cont; k++){
					vty_out(vty, "%-8f\t", cpu_rt_data.cpu_info[k]);
					if(0 == (k + 1) % 7){
						vty_out(vty, "\n");
						k++;
						break;
					}

				}

			}
		}

		syslog(LOG_DEBUG, "7777777");
		vty_out(vty, "%-8s\t\n", "average");
		for(k; k < cpu_rt_data.cpu_cont; k++){
			vty_out(vty, "%-8f\t", cpu_rt_data.cpu_info[k]);
		}
		vty_out(vty, "%-8f\t\n", cpu_rt_data.cpu_average);		
		}
		*/
		memset(&cpu_rt_data, 0, sizeof(cpu_in_t));

	}



	return CMD_SUCCESS;


}








DEFUN(show_acsample_info_func,
	show_acsample_info_cmd,
	"show acsample info",
	"show acsample info\n"
	"show acsample info\n"
	"show acsample info, cpu¡¢memery and temperature infomation\n"
)
{
	int i = 1;
	int j = 0;
	int retu = 0;
	//dcli_dbus_connection
    vty_out(vty,"------------------------------------------------------------------------\n");
    vty_out(vty, "AcSample AllSlot  dhcpusage  \n" );             
	char *samples[] = AC_SAMPLE_ARRAY;
	int samples_num = sizeof(samples)/sizeof(samples[0]);
	unsigned int latest=0;
	unsigned int average=0;
	unsigned int max=0;
	int state=0;

	/*service state*/
	retu = dbus_get_sample_service_state(dcli_dbus_connection, &state);
	if(AS_RTN_OK != retu)
	{
		vty_out(vty,"ac sample error!\n" );
	}
	else
	{
		if( SAMPLE_SERVICE_STATE_OFF == state )
		{
			vty_out( vty, "acsample service is disable!\n" );
		}
		else
		{
			vty_out( vty, "acsample service is enable!\n" );
		}
	}        	
    /*sample info   just cpu  memusage  temperature */     
	j = samples_num -1;
	vty_out(vty, "%s infomation:\n", samples[j] );
	if( AS_RTN_OK != dbus_get_sample_info(dcli_dbus_connection,
											samples[j],
											&latest,
											&average,
											&max ) )
	{
		vty_out(vty, "get %s infomation failed!\n", samples[j] );
	}
	else 
	{
		vty_out(vty, "latest:    %d\n", latest );
		vty_out(vty, "average:   %d\n", average );
		vty_out(vty, "max:       %d\n", max );
	}
	/////////////////////////////////////
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {  
            vty_out(vty,"------------------------------------------------------------------------\n");
            vty_out(vty, "AcSample Slot     : %d \n", i ); 
        	/*service state*/
        	if(AS_RTN_OK != dbus_get_sample_service_state(dbus_connection_dcli[i]->dcli_dbus_connection, &state))
        	{
        		vty_out(vty,"ac sample error!" );
        		continue ;
        	}
        	else
        	{
        		if( SAMPLE_SERVICE_STATE_OFF == state )
        		{
        			vty_out( vty, "acsample service is disable!\n" );
        			continue ;
        		}
        		else
        		{
        			vty_out( vty, "acsample service is enable!\n" );
        		}
        	}
        	
        	/*sample info   just cpu  memusage  temperature */        	
        	for(j = 0; j < samples_num; j++) {
        		if(3 != j)
    			{
	        		vty_out(vty, "%s infomation:\n", samples[j] );
	        		if( AS_RTN_OK != dbus_get_sample_info(dbus_connection_dcli[i]->dcli_dbus_connection,
	        												samples[j],
	        												&latest,
	        												&average,
	        												&max ) )
	        		{
	        			vty_out(vty, "get %s infomation failed!\n", samples[j] );
	        		}
	        		else {
	        			vty_out(vty, "latest:    %d\n", latest );
	        			vty_out(vty, "average:   %d\n", average );
	        			vty_out(vty, "max:       %d\n", max );
	        		}
    			}
        	}
        }
    }
    
	return CMD_SUCCESS;
}

DEFUN(set_acsample_threshold_maxuser_func,
		set_acsample_threshold_maxuser_cmd,
		"set acsample threshold "SAMPLE_NAME_MAX_USER" <1-10000>",
		DCLI_SAMPLE_SET_STR
		"set acsample threshold\n"
		"set acsample threshold\n"
		"set acsample "SAMPLE_NAME_MAX_USER" threshold\n"
		"set acsample threshold to value\n")
{
	const char *conf_key;
	const char *conf_value;
	unsigned int value=10;

	conf_key = SAMPLE_NAME_MAX_USER;
	conf_value = argv[0];
	value = strtoul( conf_value, NULL, 10 );
	
    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
            int iRet = AS_RTN_OK;
            iRet = dbus_set_signal_threshold(dbus_connection_dcli[i]->dcli_dbus_connection, 
                                                conf_key,
                                                value );
            if(AS_RTN_DBUS_ERR == iRet) {
                vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
            }
            else if(AS_RTN_OK != iRet) {
                vty_out(vty, "Slot(%d) :set %s threshold failed!\n", i, conf_key);
            }
        }
    } 

	return CMD_SUCCESS;

}


DEFUN(conf_acsample_threshold_func,
	conf_acsample_threshold_cmd,
	"set acsample threshold ("SAMPLE_NAME_CPU"|"SAMPLE_NAME_MEMUSAGE"|"SAMPLE_NAME_TMP")  <1-100>",
	DCLI_SAMPLE_SET_STR
	"set acsample threshold\n"
	"set acsample threshold\n"
	"set acsample "SAMPLE_NAME_CPU"threshold\n"
	"set acsample "SAMPLE_NAME_MEMUSAGE"threshold\n"
	"set acsample "SAMPLE_NAME_TMP"threshold\n"
	"set acsample threshold to value\n"
)
{
	const char *conf_key;
	const char *conf_value;
	unsigned int value = 10;
	
	conf_key = argv[0];
	conf_value = argv[1];
	value = strtoul( conf_value, NULL, 10 );


	if( 0 == strncmp(conf_key, SAMPLE_NAME_CPU, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_CPU;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_MEMUSAGE, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_MEMUSAGE;
	}
	else if (0 == strncmp(conf_key, SAMPLE_NAME_TMP, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_TMP;
	}
	else if (0 == strncmp(conf_key, SAMPLE_NAME_DHCPUSE, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_DHCPUSE;
	}
	else
	{
		vty_out(vty,"input error!");
		return CMD_FAILURE;
	}

    
    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
            int iRet = AS_RTN_OK;
            iRet = dbus_set_signal_threshold( dbus_connection_dcli[i]->dcli_dbus_connection, 
                                                conf_key,
                                                value );
            if(AS_RTN_DBUS_ERR == iRet) {
                vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
            }
            else if(AS_RTN_OK != iRet) {
                vty_out(vty, "Slot(%d) :set %s threshold failed!\n", i, conf_key);
            }
        }
    } 

	return CMD_SUCCESS;
}


DEFUN(set_acsample_conf_int_sti_by_name_func,
	set_acsample_conf_int_sti_by_name_cmd,
	"set acsample sample ("SAMPLE_NAME_DROPRATE""\
							"|"SAMPLE_NAME_MAX_USER""\
							"|"SAMPLE_NAME_RADIUS_REQ_SUC""\
							"|"SAMPLE_NAME_PORTER_NOT_REACH""\
							"|"SAMPLE_NAME_RADIUS_AUTH""\
							"|"SAMPLE_NAME_RADIUS_COUNT""\
							"|"SAMPLE_NAME_FINE_SYN_ATTACK""\
							"|"SAMPLE_NAME_INTERFACE_FLOW") param ("SE_XML_ACSAMPLE_STATIC_TIME""\
							"|"SE_XML_ACSAMPLE_INTERVAL") <5-3600>",
			"set acsample sample\n"
			"set acsample sample\n"
			"set acsample sample\n"
			""SAMPLE_NAME_DROPRATE""
			""SAMPLE_NAME_MAX_USER""
			""SAMPLE_NAME_RADIUS_REQ_SUC""
			""SAMPLE_NAME_PORTER_NOT_REACH""
			""SAMPLE_NAME_RADIUS_AUTH""
			""SAMPLE_NAME_RADIUS_COUNT""
			""SAMPLE_NAME_FINE_SYN_ATTACK""
			""SAMPLE_NAME_INTERFACE_FLOW""
	"param"
	""SE_XML_ACSAMPLE_STATIC_TIME""
	""SE_XML_ACSAMPLE_INTERVAL""
	"param to value. \n"
)
{
    
	unsigned int value=0;
	int type = 0;

	const char *conf_key=argv[0];
	const char *conf_key_2=argv[1];
	const char *conf_value=argv[2];	

	value = strtoul( conf_value, NULL, 10 );


#if 0
	if( 0 == strncmp(conf_key, SAMPLE_NAME_CPU, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_CPU;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_MEMUSAGE, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_MEMUSAGE;
	}
	else if (0 == strncmp(conf_key, SAMPLE_NAME_TMP, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_TMP;
	}
	else 
#endif
    if( 0 == strncmp(conf_key, SAMPLE_NAME_DROPRATE , strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_DROPRATE ;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_MAX_USER, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_MAX_USER;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_REQ_SUC, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_REQ_SUC;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_IP_POOL, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_IP_POOL;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_PORTER_NOT_REACH, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_PORTER_NOT_REACH;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_AUTH, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_AUTH;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_COUNT, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_COUNT;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_DHCP_IPPOOL, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_DHCP_IPPOOL;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_FINE_SYN_ATTACK, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_FINE_SYN_ATTACK;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_INTERFACE_FLOW, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_INTERFACE_FLOW;
	}
	else
	{
		vty_out(vty,"sample name input error!");
		return CMD_FAILURE;
	}

#if 0
	if( 0 == strncmp(conf_key,"switch", strlen(conf_key)) )
	{
		type = AC_SAMPLE_PARAM_TYPE_SWITCH;
	}
	else 
#endif 

	if( 0 == strncmp(conf_key_2,SE_XML_ACSAMPLE_STATIC_TIME, strlen(conf_key_2) ) )
	{	
		conf_key_2=SE_XML_ACSAMPLE_STATIC_TIME;
		type = AC_SAMPLE_PARAM_TYPE_STATISTICS;
	}
	else if( 0 == strncmp(conf_key_2,SE_XML_ACSAMPLE_INTERVAL, strlen(conf_key_2) ) )
	{
		conf_key_2=SE_XML_ACSAMPLE_INTERVAL;
		type = AC_SAMPLE_PARAM_TYPE_INTERVAL;
	}

#if 0
	else if( 0 == strncmp(conf_key,"resend_interval", strlen(conf_key) ) )
	{
		type = AC_SAMPLE_PARAM_TYPE_RESEND_INTERVAL;
		
	}else if( 0 == strncmp(conf_key,"config_reload_interval", strlen(conf_key) ) )
	{
		type = AC_SAMPLE_PARAM_TYPE_CONFIG_RELOAD_INTERVAL;
		
	}else if( 0 == strncmp(conf_key,"threshold", strlen(conf_key) ) )
	{
		type = AC_SAMPLE_PARAM_TYPE_THRESHOLD;
	}
#endif

	else
	{
		vty_out(vty, "%s : Do not has this param!\n", conf_value);
		return CMD_FAILURE;
	}

    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
            int iRet = AS_RTN_OK;
            iRet = dbus_set_sample_param_by_name(dbus_connection_dcli[i]->dcli_dbus_connection, conf_key, type, value );
            if(AS_RTN_OK != iRet)
            {
                switch (iRet)
                {
                    case AS_RTN_STATICS_LETTER_INTERVAL:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d sample statistic time less than sample interval error!\n", i, conf_key, conf_key_2,  value );
                        break;
                    case AS_RTN_NULL_POINTER:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d sample not exist!\n", i, conf_key, conf_key_2,  value );
                        break;
                    case AS_RTN_PARAM_ERR:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d param value error!\n", i, conf_key, conf_key_2,  value );
                        break;
                    case AS_RTN_SAMPLE_STATE_ERROR:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d sample state error!\n", i, conf_key, conf_key_2,  value );
                        break;
                    case AS_RTN_INTERVAL_LETTER_MIN:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d sample interval less than min value error!\n", i, conf_key, conf_key_2,  value );
                        break;
                    case AS_RTN_INTERVAL_LAGER_STATICS:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d sample interval greater than statistic time error!\n", i, conf_key, conf_key_2,  value );
                        break;
                            
                    default:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d Unknown error!\n", i, conf_key, conf_key_2,  value );
                }
            }
        }
    }
    
	return CMD_SUCCESS;
}


DEFUN(set_acsample_res_rel_conf_by_name_func,
	set_acsample_res_rel_conf_by_name_cmd,
	"set acsample sample ("SAMPLE_NAME_DROPRATE""\
							"|"SAMPLE_NAME_MAX_USER""\
							"|"SAMPLE_NAME_RADIUS_REQ_SUC""\
							"|"SAMPLE_NAME_PORTER_NOT_REACH""\
							"|"SAMPLE_NAME_RADIUS_AUTH""\
							"|"SAMPLE_NAME_RADIUS_COUNT""\
							"|"SAMPLE_NAME_FINE_SYN_ATTACK""\
							"|"SAMPLE_NAME_INTERFACE_FLOW") param ("SE_XML_ACSAMPLE_RESEND_INTERVAL""\
							"|"SE_XML_ACSAMPLE_RELOAD_INTERVAL") <0-3600>",
			DCLI_SAMPLE_SET_STR
			"set acsample sample\n"
			"set acsample sample\n"
			"set acsample sample\n"
			""SAMPLE_NAME_DROPRATE""
			""SAMPLE_NAME_MAX_USER""
			""SAMPLE_NAME_RADIUS_REQ_SUC""
			""SAMPLE_NAME_PORTER_NOT_REACH""
			""SAMPLE_NAME_RADIUS_AUTH""
			""SAMPLE_NAME_RADIUS_COUNT""
			""SAMPLE_NAME_FINE_SYN_ATTACK""
			""SAMPLE_NAME_INTERFACE_FLOW""
	"param"
	""SE_XML_ACSAMPLE_RESEND_INTERVAL""
	""SE_XML_ACSAMPLE_RELOAD_INTERVAL""
	"param to value. \n"
)
{
	unsigned int value = 0;
	int type = 0;

	const char *conf_key=argv[0];
	const char *conf_key_2=argv[1];
	const char *conf_value=argv[2];	

	value = strtoul( conf_value, NULL, 10 );
	
#if 0
	if( 0 == strncmp(conf_key, SAMPLE_NAME_CPU, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_CPU;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_MEMUSAGE, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_MEMUSAGE;
	}
	else if (0 == strncmp(conf_key, SAMPLE_NAME_TMP, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_TMP;
	}
	else 
#endif
	if( 0 == strncmp(conf_key, SAMPLE_NAME_DROPRATE , strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_DROPRATE ;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_MAX_USER, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_MAX_USER;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_REQ_SUC, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_REQ_SUC;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_IP_POOL, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_IP_POOL;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_PORTER_NOT_REACH , strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_PORTER_NOT_REACH ;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_AUTH, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_AUTH;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_COUNT, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_COUNT;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_DHCP_IPPOOL, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_DHCP_IPPOOL;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_FINE_SYN_ATTACK, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_FINE_SYN_ATTACK;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_INTERFACE_FLOW, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_INTERFACE_FLOW;
	}
	else
	{
		vty_out(vty,"sample name input error!");
		return CMD_FAILURE;
	}

#if 0
	if( 0 == strncmp(conf_key,"switch", strlen(conf_key)) )
	{
		type = AC_SAMPLE_PARAM_TYPE_SWITCH;
	}
	else if( 0 == strncmp(conf_key,"statistic_time", strlen(conf_key) ) )
	{
		type = AC_SAMPLE_PARAM_TYPE_STATISTICS;
	}
	else if( 0 == strncmp(conf_key,"sample_interval", strlen(conf_key) ) )
	{
		type = AC_SAMPLE_PARAM_TYPE_INTERVAL;
		
	}else 
#endif
	if( 0 == strncmp(conf_key_2,SE_XML_ACSAMPLE_RESEND_INTERVAL, strlen(conf_key_2) ) )
	{
		conf_key_2=SE_XML_ACSAMPLE_RESEND_INTERVAL;
		type = AC_SAMPLE_PARAM_TYPE_RESEND_INTERVAL;
		
	}else if( 0 == strncmp(conf_key_2,SE_XML_ACSAMPLE_RELOAD_INTERVAL, strlen(conf_key_2) ) )
	{
		conf_key_2=SE_XML_ACSAMPLE_RELOAD_INTERVAL;
		type = AC_SAMPLE_PARAM_TYPE_CONFIG_RELOAD_INTERVAL;
		
	}
#if 0
	else if( 0 == strncmp(conf_key,"threshold", strlen(conf_key) ) )
	{
		type = AC_SAMPLE_PARAM_TYPE_THRESHOLD;
	}
#endif
	else
	{
		vty_out(vty, "%s : Do not has this param!\n", conf_value);
		return CMD_FAILURE;
	}

    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
            int iRet = AS_RTN_OK;
            iRet = dbus_set_sample_param_by_name(dbus_connection_dcli[i]->dcli_dbus_connection, conf_key, type, value);
            if(AS_RTN_OK != iRet)
            {
                switch (iRet)
                {
                    case AS_RTN_STATICS_LETTER_INTERVAL:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d sample statistic time less than sample interval error!\n", i, conf_key, conf_key_2,  value );
                        break;
                    case AS_RTN_NULL_POINTER:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d sample not exist!\n", i, conf_key, conf_key_2,  value );
                        break;
                    case AS_RTN_PARAM_ERR:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d param value error!\n", i, conf_key, conf_key_2,  value );
                        break;
                    case AS_RTN_SAMPLE_STATE_ERROR:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d sample state error!\n", i, conf_key, conf_key_2,  value );
                        break;
                    case AS_RTN_INTERVAL_LETTER_MIN:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d sample interval less than min value error!\n", i, conf_key, conf_key_2,  value );
                        break;
                    case AS_RTN_INTERVAL_LAGER_STATICS:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d sample interval greater than statistic time error!\n", i, conf_key, conf_key_2,  value );
                        break;
                            
                    default:
                        vty_out(vty, "Slot(%d) :set sampe %s %s to %d Unknown error!\n", i, conf_key, conf_key_2,  value );
                }
            }
        }
    }    

	return CMD_SUCCESS;
}



DEFUN(set_acsample_conf_thr_by_name_func,
	set_acsample_conf_thr_threshold_cmd,
	"set acsample sample ("SAMPLE_NAME_DROPRATE""\
							"|"SAMPLE_NAME_MAX_USER""\
							"|"SAMPLE_NAME_RADIUS_REQ_SUC""\
							"|"SAMPLE_NAME_PORTER_NOT_REACH""\
							"|"SAMPLE_NAME_RADIUS_AUTH""\
							"|"SAMPLE_NAME_RADIUS_COUNT""\
							"|"SAMPLE_NAME_FINE_SYN_ATTACK""\
							"|"SAMPLE_NAME_INTERFACE_FLOW") param "SE_XML_ACSAMPLE_THRESHOLD" <1-100>",
			DCLI_SAMPLE_SET_STR
			"set acsample sample\n"
			"set acsample sample\n"
			"set acsample sample\n"
			""SAMPLE_NAME_DROPRATE""
			""SAMPLE_NAME_MAX_USER""
			""SAMPLE_NAME_RADIUS_REQ_SUC""
			""SAMPLE_NAME_PORTER_NOT_REACH""
			""SAMPLE_NAME_RADIUS_AUTH""
			""SAMPLE_NAME_RADIUS_COUNT""
			""SAMPLE_NAME_FINE_SYN_ATTACK""
			""SAMPLE_NAME_INTERFACE_FLOW""
	"param"
	""SE_XML_ACSAMPLE_THRESHOLD""
	"param to value. \n"
)
{
	const char *conf_key;
	const char *conf_value;
	int value=0;
	
	conf_key = argv[0];
	conf_value = argv[1];
	value = strtoul( conf_value, NULL, 10 );

#if 0
	if( 0 == strncmp(conf_key, SAMPLE_NAME_CPU, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_CPU;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_MEMUSAGE, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_MEMUSAGE;
	}
	else if (0 == strncmp(conf_key, SAMPLE_NAME_TMP, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_TMP;
	}
	else 
#endif
	if( 0 == strncmp(conf_key, SAMPLE_NAME_DROPRATE , strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_DROPRATE ;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_MAX_USER, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_MAX_USER;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_REQ_SUC, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_REQ_SUC;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_IP_POOL, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_IP_POOL;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_PORTER_NOT_REACH , strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_PORTER_NOT_REACH ;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_AUTH, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_AUTH;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_COUNT, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_COUNT;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_DHCP_IPPOOL, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_DHCP_IPPOOL;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_FINE_SYN_ATTACK, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_FINE_SYN_ATTACK;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_INTERFACE_FLOW, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_INTERFACE_FLOW;
	}
	else
	{
		vty_out(vty,"sample name input error!");
		return CMD_FAILURE;
	}


    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
            int iRet = AS_RTN_OK;
            iRet = dbus_set_signal_threshold(dbus_connection_dcli[i]->dcli_dbus_connection, conf_key, value );
            if(AS_RTN_DBUS_ERR == iRet) {
                vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
            }
            else if(AS_RTN_OK != iRet) {
                vty_out(vty,"Slot(%d) :set %s threshold failed!\n", i, conf_key);
            }
            
        }
    } 
	
	return CMD_SUCCESS;
}


DEFUN(show_acsample_conf_by_name_func,
		show_acsample_conf_by_name_cmd,
		"show acsample sample ("SAMPLE_NAME_CPU""\
							"|"SAMPLE_NAME_MEMUSAGE""\
							"|"SAMPLE_NAME_TMP""\
							"|"SAMPLE_NAME_DROPRATE""\
							"|"SAMPLE_NAME_MAX_USER""\
							"|"SAMPLE_NAME_RADIUS_REQ_SUC""\
							"|"SAMPLE_NAME_PORTER_NOT_REACH""\
							"|"SAMPLE_NAME_RADIUS_AUTH""\
							"|"SAMPLE_NAME_RADIUS_COUNT""\
							"|"SAMPLE_NAME_FINE_SYN_ATTACK""\
							"|"SAMPLE_NAME_INTERFACE_FLOW")  configuration",
			"show acsample sample\n"
			"show acsample sample\n"
			"show acsample sample\n"
			""SAMPLE_NAME_CPU""
			""SAMPLE_NAME_MEMUSAGE""
			""SAMPLE_NAME_TMP""
			""SAMPLE_NAME_DROPRATE""
			""SAMPLE_NAME_MAX_USER""
			""SAMPLE_NAME_RADIUS_REQ_SUC""
			""SAMPLE_NAME_PORTER_NOT_REACH""
			""SAMPLE_NAME_RADIUS_AUTH""
			""SAMPLE_NAME_RADIUS_COUNT""
			""SAMPLE_NAME_FINE_SYN_ATTACK""
			""SAMPLE_NAME_INTERFACE_FLOW""
			"sample switch, interval,statistic time,resend interval,reload config file interval and threshold configuration\n"
)
{
	int iRet = 0;
	
	const char *conf_key = argv[0];

	int				sample_on=0;
	unsigned int		statistics_time=0;
	unsigned int		sample_interval=0;
	unsigned long	resend_interval=0;
	unsigned int		reload_config_interval=0;
	//int				over_threshold_check_type;	//not use
	//int				clear_check_type;				//not use
	int				threshold=0;


	if( 0 == strncmp(conf_key, SAMPLE_NAME_CPU, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_CPU;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_MEMUSAGE, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_MEMUSAGE;
	}
	else if (0 == strncmp(conf_key, SAMPLE_NAME_TMP, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_TMP;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_DROPRATE , strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_DROPRATE ;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_MAX_USER, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_MAX_USER;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_REQ_SUC, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_REQ_SUC;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_IP_POOL, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_IP_POOL;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_PORTER_NOT_REACH , strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_PORTER_NOT_REACH ;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_AUTH, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_AUTH;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_COUNT, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_COUNT;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_DHCP_IPPOOL, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_DHCP_IPPOOL;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_FINE_SYN_ATTACK, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_FINE_SYN_ATTACK;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_INTERFACE_FLOW, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_INTERFACE_FLOW;
	}
	else
	{
		vty_out(vty,"sample name input error!");
		return CMD_FAILURE;
	}


    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) { 
            vty_out(vty,"------------------------------------------------------------------------\n");
            vty_out(vty, "AcSample Slot             : %d \n", i );  
            int iRet = AS_RTN_OK;
            iRet = dbus_get_sample_params_info_by_name(dbus_connection_dcli[i]->dcli_dbus_connection,
                                                        conf_key,
                                                        &sample_on,
                                                        &statistics_time,
                                                        &sample_interval,
                                                        &resend_interval,
                                                        &reload_config_interval,
                                                        &threshold );
            if(AS_RTN_OK != iRet) {
                vty_out(vty, "get acsample %s configuration failed!\n",conf_key );
            }
            else {
                vty_out(vty,"%s sample configuration:\n", conf_key );
                vty_out(vty,"   %-30s :  %s\n", "sample state", SAMPLE_ON==sample_on?"on":"off");
                vty_out(vty,"   %-30s :  %u\n", "sample statistic time", statistics_time);
                vty_out(vty,"   %-30s :  %u\n", "sample interval", sample_interval);
                vty_out(vty,"   %-30s :  %u\n", "sample resend interval", resend_interval);
                vty_out(vty,"   %-30s :  %u\n", "sample reload config interval", reload_config_interval);
                vty_out(vty,"   %-30s :  %d\n", "sample threshold", threshold);     
            }
            vty_out(vty,"------------------------------------------------------------------------\n");
        }
    }
    
	return CMD_SUCCESS;

}

//modified by stephen 
DEFUN(set_acsample_state_func,
			set_acsample_state_cmd,
			"set acsample sample ("SAMPLE_NAME_DROPRATE""\
								"|"SAMPLE_NAME_MAX_USER""\
								"|"SAMPLE_NAME_RADIUS_REQ_SUC""\
								"|"SAMPLE_NAME_FINE_SYN_ATTACK""\
								"|"SAMPLE_NAME_CPURT""\
								"|"SAMPLE_NAME_INTERFACE_FLOW") param "SE_XML_ACSAMPLE_SWITCH" (on|off)",
			"set acsample sample\n"
			"set acsample sample \n"
			"set acsample sample \n"
			""SAMPLE_NAME_DROPRATE"\n"
			""SAMPLE_NAME_MAX_USER"\n"
			""SAMPLE_NAME_RADIUS_REQ_SUC"\n"
			""SAMPLE_NAME_FINE_SYN_ATTACK"\n"
			""SAMPLE_NAME_CPURT"\n"
			""SAMPLE_NAME_INTERFACE_FLOW"\n"
			"parameter\n"
			"swithch on|off\n"
			"switch on\n"
			"switch off\n"
		)
{    
	const char *leveltype = LOG_LEVEL_NAME_DEBUG;
	int state = SAMPLE_ON;

	const char *conf_key = NULL;
	const char *conf_value = NULL;
	conf_key = argv[0];
	conf_value = argv[1];

	unsigned int slot_id = 0;
	if(3 == argc) {
        slot_id = atoi(argv[2]);
        if(0 == slot_id || slot_id > SLOT_MAX_NUM) {
            return CMD_WARNING;
        }
    }
    

#if 0
	if( 0 == strncmp(conf_key, SAMPLE_NAME_CPU, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_CPU;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_MEMUSAGE, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_MEMUSAGE;
	}
	else if (0 == strncmp(conf_key, SAMPLE_NAME_TMP, strlen(conf_key)) )
	{
		conf_key = SAMPLE_NAME_TMP;
	}
	else 
#endif

	if( 0 == strncmp(conf_key, SAMPLE_NAME_DROPRATE , strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_DROPRATE ;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_MAX_USER, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_MAX_USER;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_REQ_SUC, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_REQ_SUC;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_PORTER_NOT_REACH, strlen(conf_key)))
	{
        conf_key = SAMPLE_NAME_PORTER_NOT_REACH ;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_AUTH, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_AUTH;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_RADIUS_COUNT, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_RADIUS_COUNT;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_IP_POOL, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_IP_POOL;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_DHCP_IPPOOL, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_DHCP_IPPOOL;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_FINE_SYN_ATTACK, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_FINE_SYN_ATTACK;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_CPURT, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_CPURT;
	}
	else if( 0 == strncmp(conf_key, SAMPLE_NAME_INTERFACE_FLOW, strlen(conf_key)))
	{
		conf_key = SAMPLE_NAME_INTERFACE_FLOW;
	}	
	else
	{
		vty_out(vty,"sample name input error!");
		return CMD_FAILURE;
	}
	
	if (0 == strncmp( conf_value, "off",strlen(conf_value)) )
	{
		state = SAMPLE_OFF;
	}

    if(3 == argc) {
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {                
            int iRet = AS_RTN_OK;
            iRet = dbus_set_sample_state(dbus_connection_dcli[slot_id]->dcli_dbus_connection, conf_key, state);
            if(AS_RTN_DBUS_ERR == iRet) {
                vty_out(vty, "Slot(%d) :<error> failed get reply.\n", slot_id);
            }
            else if(AS_RTN_OK != iRet) {
                vty_out(vty,"Slot(%d) :set acsample config %s switch error!\n", slot_id, conf_key);
            }
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
        }
    }
    else {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
                int iRet = AS_RTN_OK;
                iRet = dbus_set_sample_state(dbus_connection_dcli[i]->dcli_dbus_connection, conf_key, state);
                if(AS_RTN_DBUS_ERR == iRet) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else if(AS_RTN_OK != iRet) {
                    vty_out(vty,"Slot(%d) :set acsample config %s switch error!\n", i, conf_key);
                }
            }
        }
    }
    
	return CMD_SUCCESS;
}

ALIAS(set_acsample_state_func,
	set_acsample_slot_state_cmd,
	"set acsample sample ("SAMPLE_NAME_PORTER_NOT_REACH""\
						"|"SAMPLE_NAME_RADIUS_AUTH""\
						"|"SAMPLE_NAME_RADIUS_COUNT") param "SE_XML_ACSAMPLE_SWITCH" (on|off) <1-16>",
	"set acsample sample\n"
	"set acsample sample \n"
	"set acsample sample \n"
	""SAMPLE_NAME_PORTER_NOT_REACH"\n"
	""SAMPLE_NAME_RADIUS_AUTH"\n"
	""SAMPLE_NAME_RADIUS_COUNT"\n"
	"parameter\n"
	"switch on|off\n"
	"switch on\n"
	"switch off\n"
	"slot_id <1-16>"
)




DEFUN(set_acsample_socket_timeout_func,
			set_acsample_socket_timeout_cmd,
			"set acsample params "SE_XML_ACSAMPLE_TIMEOUT" <0-75000>",
			DCLI_SAMPLE_SET_STR
			"set acsample params "SE_XML_ACSAMPLE_TIMEOUT" \n"
			"set acsample params "SE_XML_ACSAMPLE_TIMEOUT" \n"
			"set acsample param "SE_XML_ACSAMPLE_TIMEOUT" milliseconds\n"
			"set acsample "SE_XML_ACSAMPLE_TIMEOUT" to value.\n"
		)
{
	const char *conf_value = NULL;
	conf_value = argv[0];	
	unsigned int value = DEFAULT_TIMEOUT;
	value = strtoul( conf_value, NULL, 10 );

    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
            int iRet = AS_RTN_OK;
            iRet = dbus_set_sample_param_timeout(dbus_connection_dcli[i]->dcli_dbus_connection, value);
            if(AS_RTN_DBUS_ERR == iRet) {
                vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
            }
            else if(AS_RTN_OK != iRet) {
                vty_out(vty,"Slot(%d) :set acsample timeout %d failed!\n", i, value);
            }
            
        }
    }

	return CMD_SUCCESS;
}

DEFUN(show_acsample_socket_timeout_func,
			show_acsample_socket_timeout_cmd,
			"show acsample configuration "SE_XML_ACSAMPLE_TIMEOUT"",
			"show acsample configuration "SE_XML_ACSAMPLE_TIMEOUT" \n"
			"show acsample configuration "SE_XML_ACSAMPLE_TIMEOUT" \n"
			"show acsample configuration "SE_XML_ACSAMPLE_TIMEOUT" milliseconds\n"
		)
{
    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
            int iRet = AS_RTN_OK;
            unsigned int value = 0;
            iRet = dbus_get_sample_param_timeout(dbus_connection_dcli[i]->dcli_dbus_connection, &value);
            if(AS_RTN_OK == iRet) {
                vty_out(vty,"Slot(%d) :acsample timeout %d milinseconds.\n", i, value);
            }
            if(AS_RTN_DBUS_ERR == iRet) {
                vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
            }
            else if(AS_RTN_OK != iRet) {
                vty_out(vty,"Slot(%d) :show acsample timeout failed: return %d\n", i, iRet);
            }
            
        }
    }

	return CMD_SUCCESS;
}


DEFUN(set_acsample_log_level_func,
			set_acsample_log_level_cmd,
			"set acsample log-debug (on|off)",
			DCLI_SAMPLE_SET_STR
			"set acsample log-debug \n"
			"set acsample log-debug \n"
			"set acsample log-debug on\n"
			"set acsample log-debug off\n"
		)
{
	const char *leveltype = LOG_LEVEL_NAME_DEBUG;
	int nlevel = LOG_AT_LEAST_INFO;

	const char *conf_value = NULL;
	conf_value = argv[0];	
	
	if(0 == strcmp( conf_value, "on")) {
		nlevel = LOG_AT_LEAST_DEBUG;
	}

    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {                
            int iRet = AS_RTN_OK;
            iRet = dbus_set_debug_level(dbus_connection_dcli[i]->dcli_dbus_connection, nlevel);
            if(AS_RTN_DBUS_ERR == iRet) {
                vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
            }
            else if(AS_RTN_OK != iRet) {
                vty_out(vty,"Slot(%d) :set log level %s failed!\n", i, leveltype);
            }
            
        }
    }

	return CMD_SUCCESS;
}

int get_default_sample_config ( const char *sample_name, acsample_conf *sample_conf)
{
	if ( NULL == sample_name || NULL == sample_conf )
	{
		return -1;
	}

	if ( !strcmp(sample_name, SAMPLE_NAME_DROPRATE))
	{
		sample_conf->sample_on=SAMPLE_OFF;
		sample_conf->statistics_time=DEFAULT_STATISTICS_TIME;
		sample_conf->sample_interval=DEFAULT_SAMPLE_INTERVAL;
		sample_conf->resend_interval=DEFAULT_RESEND_INTERVAL;
		sample_conf->reload_config_interval=DEFAULT_CONFIG_RELOAD_INTERVAL;
		//sample_conf->over_threshold_check_type
		//sample_conf->clear_check_type
		sample_conf->threshold=DEFAULT_DROP_THRESHOLD;

	}else if ( !strcmp(sample_name, SAMPLE_NAME_MAX_USER))
	{
		sample_conf->sample_on=SAMPLE_OFF;
		sample_conf->statistics_time=DEFAULT_STATISTICS_TIME;
		sample_conf->sample_interval=DEFAULT_SAMPLE_INTERVAL;
		sample_conf->resend_interval=DEFAULT_RESEND_INTERVAL;
		sample_conf->reload_config_interval=DEFAULT_CONFIG_RELOAD_INTERVAL;
		//sample_conf->over_threshold_check_type
		//sample_conf->clear_check_type
		sample_conf->threshold=DEFAULT_ONLINE_USER_THRESHOLD;
		
	}else if ( !strcmp(sample_name, SAMPLE_NAME_RADIUS_REQ_SUC))
	{
		sample_conf->sample_on=SAMPLE_OFF;
		sample_conf->statistics_time=DEFAULT_STATISTICS_TIME;
		sample_conf->sample_interval=DEFAULT_SAMPLE_INTERVAL;
		sample_conf->resend_interval=DEFAULT_RESEND_INTERVAL;
		sample_conf->reload_config_interval=DEFAULT_CONFIG_RELOAD_INTERVAL;
		//sample_conf->over_threshold_check_type
		//sample_conf->clear_check_type
		sample_conf->threshold=DEFAULT_RADIUS_REQ_SUC_THRESHOLD;
		
	}else if ( !strcmp(sample_name, SAMPLE_NAME_PORTER_NOT_REACH))
	{
		sample_conf->sample_on=SAMPLE_OFF;
		sample_conf->statistics_time=DEFAULT_PORTAL_STATISTIC_TIME;
		sample_conf->sample_interval=DEFAULT_PORTAL_SAMPLE_INTERVAL;
		sample_conf->resend_interval=DEFAULT_RESEND_INTERVAL;
		sample_conf->reload_config_interval=DEFAULT_CONFIG_RELOAD_INTERVAL;
		//sample_conf->over_threshold_check_type
		//sample_conf->clear_check_type
		sample_conf->threshold=DEFAULT_SAMPLE_SERVER_THRESHOLD;
		
	}else if ( !strcmp(sample_name, SAMPLE_NAME_RADIUS_AUTH))
	{
		sample_conf->sample_on=SAMPLE_OFF;
		sample_conf->statistics_time=DEFAULT_RADIUS_AUTH_STATISTICS_TIME;
		sample_conf->sample_interval=DEFAULT_RADIUS_AUTH_SAMPLE_INTERVAL;
		sample_conf->resend_interval=DEFAULT_RESEND_INTERVAL;
		sample_conf->reload_config_interval=DEFAULT_CONFIG_RELOAD_INTERVAL;
		//sample_conf->over_threshold_check_type
		//sample_conf->clear_check_type
		sample_conf->threshold=DEFAULT_SAMPLE_SERVER_THRESHOLD;
		
	}else if ( !strcmp(sample_name, SAMPLE_NAME_RADIUS_COUNT))
	{
		sample_conf->sample_on=SAMPLE_OFF;
		sample_conf->statistics_time=DEFAULT_RADIUS_ACCT_STATISTICS_TIME;
		sample_conf->sample_interval=DEFAULT_RADIUS_AUTH_SAMPLE_INTERVAL;
		sample_conf->resend_interval=DEFAULT_RESEND_INTERVAL;
		sample_conf->reload_config_interval=DEFAULT_CONFIG_RELOAD_INTERVAL;
		//sample_conf->over_threshold_check_type
		//sample_conf->clear_check_type
		sample_conf->threshold=DEFAULT_SAMPLE_SERVER_THRESHOLD;
		
	}else if ( !strcmp(sample_name, SAMPLE_NAME_FINE_SYN_ATTACK))
	{
		sample_conf->sample_on=SAMPLE_OFF;
		sample_conf->statistics_time=DEFAULT_STATISTICS_TIME;
		sample_conf->sample_interval=DEFAULT_SAMPLE_INTERVAL;
		sample_conf->resend_interval=DEFAULT_RESEND_INTERVAL;
		sample_conf->reload_config_interval=DEFAULT_CONFIG_RELOAD_INTERVAL;
		//sample_conf->over_threshold_check_type
		//sample_conf->clear_check_type
		sample_conf->threshold=NORM_STAT_THRESHOLD;
		
	}else if ( !strcmp(sample_name, SAMPLE_NAME_INTERFACE_FLOW))
	{
		sample_conf->sample_on=SAMPLE_OFF;
		sample_conf->statistics_time=DEFAULT_STATISTICS_TIME;
		sample_conf->sample_interval=DEFAULT_SAMPLE_INTERVAL;
		sample_conf->resend_interval=DEFAULT_RESEND_INTERVAL;
		sample_conf->reload_config_interval=DEFAULT_CONFIG_RELOAD_INTERVAL;
		//sample_conf->over_threshold_check_type
		//sample_conf->clear_check_type
		sample_conf->threshold=DEFAULT_THRESHOLD;
		
	}else 
	{
		return -2;
	}

	return 0;

}


#define VTYSH_CMD_LINE_LEN	1024
#define vtysh_add_show_string_format(format,...)\
{\
	int cmd_line_len=0;					\
	int cmd_buff_len=VTYSH_CMD_LINE_LEN;		\
	int cmd_buff_count=1;		\
	char *vtysh_cmd_tempstr = NULL;			\
	vtysh_cmd_tempstr=(char *)malloc(cmd_buff_len*sizeof(char));		\
	while( (cmd_line_len = snprintf(vtysh_cmd_tempstr,cmd_buff_len-1,format,__VA_ARGS__))>= cmd_buff_len-2)		\
	{\
		if(cmd_buff_count>3)\
		{\
			break;\
		}\
		cmd_buff_len *=(1<<cmd_buff_count);\
		cmd_buff_count++;\
		vtysh_cmd_tempstr = (char *)realloc(vtysh_cmd_tempstr,cmd_buff_len*sizeof(char));\
	}\
	if( cmd_buff_count<=3)\
	{\
		vtysh_add_show_string(vtysh_cmd_tempstr);\
	}\
	free(vtysh_cmd_tempstr);\
}

static int dcli_acsample_show_running_config(struct vty *vty)
{
	char *samples[] = AC_SAMPLE_ARRAY;
	char *samples_second[] = AC_SAMPLE_SECOND_ARRAY;
	int i=0;
	int samples_num = sizeof(samples)/sizeof(samples[0]);
	int samples_second_num = sizeof(samples_second)/sizeof(samples_second[0]);
	int type;
	unsigned int sample_interval;
	unsigned int statistics_time;
	unsigned int resend_interval;
	unsigned int threshold;
	int iRet;
	int state;
	int				sample_on;
	unsigned int		reload_config_interval;
	//int				over_threshold_check_type;	//not use
	//int				clear_check_type;				//not use


	#if 0
	do {
		char _tmpstr[64];
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "acsample");
		vtysh_add_show_string(_tmpstr);
	} while(0);
	#endif
	
	/*param*/
	iRet = dbus_get_sample_param( dcli_dbus_connection,
								  &sample_interval,
								  &statistics_time,
								  &resend_interval);
	if( AS_RTN_DBUS_ERR == iRet )
	{
		return CMD_SUCCESS;
	}
	
	if( AS_RTN_OK != iRet )
	{
		sample_interval = DEFAULT_SAMPLE_INTERVAL;
		statistics_time = DEFAULT_STATISTICS_TIME;
		resend_interval = DEFAULT_RESEND_INTERVAL;
	}
	
    if(show_running_flag) {
        vty_out(vty, "config acsample\n");
    }else {
	    vtysh_add_show_string("config acsample");
	}    

	if( DEFAULT_SAMPLE_INTERVAL != sample_interval )
	{
	    if(show_running_flag) {
           vty_out(vty, " set acsample params "SE_XML_ACSAMPLE_INTERVAL" %d\n", sample_interval );
	    }else {
            vtysh_add_show_string_format(" set acsample params "SE_XML_ACSAMPLE_INTERVAL" %d", sample_interval );
	    }
	}
	if (DEFAULT_STATISTICS_TIME != statistics_time )
	{
	    if(show_running_flag) {
           vty_out(vty, " set acsample params "SE_XML_ACSAMPLE_STATIC_TIME" %d\n", statistics_time );
	    }else {
		    vtysh_add_show_string_format(" set acsample params "SE_XML_ACSAMPLE_STATIC_TIME" %d", statistics_time );
		}    
	}
	if ( DEFAULT_RESEND_INTERVAL != resend_interval)
	{   
	    if(show_running_flag) {
            vty_out(vty, " set acsample params "SE_XML_ACSAMPLE_RESEND_INTERVAL" %d\n", resend_interval );
	    }else {
            vtysh_add_show_string_format(" set acsample params "SE_XML_ACSAMPLE_RESEND_INTERVAL" %d", resend_interval );
	    }
	}
	
	/*threshold*/
	for( i = 0; i < samples_num; i++ )
	{
		iRet = dbus_get_signal_threshold( dcli_dbus_connection, 
											samples[i],
											&threshold );
		if( AS_RTN_OK != iRet )
		{
			if( 0 == strcmp( samples[i], SAMPLE_NAME_CPU ) )
			{
				threshold = DEFAULT_CPU_THRESHOLD;
			}
			else if( 0 == strcmp( samples[i], SAMPLE_NAME_MEMUSAGE ) )
			{
				threshold = DEFAULT_MEM_THRESHOLD;
			}
			else if( 0 == strcmp( samples[i], SAMPLE_NAME_TMP ) )
			{
				threshold = DEFAULT_TMP_THRESHOLD;
			}
			else if( 0 == strcmp( samples[i], SAMPLE_NAME_DHCPUSE ) )
			{
				threshold = DEFAULT_THRESHOLD;
			}
		}
		else
		{
			if( 0 == strcmp( samples[i], SAMPLE_NAME_CPU ) && 
				DEFAULT_CPU_THRESHOLD == threshold)
			{
				continue;
			}
			else if( 0 == strcmp( samples[i], SAMPLE_NAME_MEMUSAGE ) &&
					DEFAULT_MEM_THRESHOLD==threshold )
			{
				continue;
			}
			else if( 0 == strcmp( samples[i], SAMPLE_NAME_TMP ) &&
					DEFAULT_TMP_THRESHOLD==threshold )
			{
				continue;
			}
			else if( 0 == strcmp( samples[i], SAMPLE_NAME_DHCPUSE ) &&
					DEFAULT_THRESHOLD==threshold )
			{
				continue;
			}
		}

	    if(show_running_flag) {
            vty_out(vty, " set acsample threshold %s %d\n", samples[i], threshold);
	    }else {
            vtysh_add_show_string_format(" set acsample threshold %s %d", samples[i], threshold );
	    }

	}

	unsigned int timeout=0;
	
	iRet = dbus_get_sample_param_timeout(dcli_dbus_connection, &timeout);
	
	if ( AS_RTN_OK==iRet && timeout != DEFAULT_TIMEOUT)
	{
	    if(show_running_flag) {
            vty_out(vty, " set acsample params "SE_XML_ACSAMPLE_TIMEOUT" %u\n", timeout );
	    }else {
            vtysh_add_show_string_format(" set acsample params "SE_XML_ACSAMPLE_TIMEOUT" %u", timeout );
	    }
	}

    
    
	acsample_conf sample_conf;
	int ret=0;
	for ( i = 0; i < samples_second_num; i++ )
	{
		memset (&sample_conf, 0, sizeof (acsample_conf));
		
		sample_on               = SAMPLE_ON;
		statistics_time         = DEFAULT_STATISTICS_TIME;
		sample_interval         = DEFAULT_SAMPLE_INTERVAL;
		resend_interval         = DEFAULT_RESEND_INTERVAL;
		reload_config_interval  = DEFAULT_CONFIG_RELOAD_INTERVAL;
		//int				over_threshold_check_type;	//not use
		//int				clear_check_type;				//not use
		threshold               = DEFAULT_THRESHOLD;

        
        iRet = dbus_get_sample_params_info_by_name( dcli_dbus_connection,
                                                    samples_second[i],
                                                    &sample_on,
                                                    &statistics_time,
                                                    &sample_interval,
                                                    &resend_interval,
                                                    &reload_config_interval,
                                                    &threshold );
        if( AS_RTN_OK != iRet ) {
            continue;
        }
        
        ret = get_default_sample_config ( samples_second[i], &sample_conf );
        
        if(0 == strcmp(samples_second[i], SAMPLE_NAME_PORTER_NOT_REACH) 
            || 0 == strcmp(samples_second[i], SAMPLE_NAME_RADIUS_AUTH)
            || 0 == strcmp(samples_second[i], SAMPLE_NAME_RADIUS_COUNT)) {

            unsigned int slot_index = 1;
            for(slot_index = 1; slot_index < MAX_SLOT; slot_index++) {
                if(dbus_connection_dcli[slot_index]->dcli_dbus_connection) {  

                    int temp_sample_on                        = SAMPLE_OFF;
                    unsigned int temp_statistics_time         = DEFAULT_STATISTICS_TIME;
                    unsigned int temp_sample_interval         = DEFAULT_SAMPLE_INTERVAL;
                    unsigned int temp_resend_interval         = DEFAULT_RESEND_INTERVAL;
                    unsigned int temp_reload_config_interval  = DEFAULT_CONFIG_RELOAD_INTERVAL;
                    unsigned int temp_threshold               = DEFAULT_THRESHOLD;
                    
                    iRet = dbus_get_sample_params_info_by_name(dbus_connection_dcli[slot_index]->dcli_dbus_connection,
                                                                samples_second[i],
                                                                &temp_sample_on,
                                                                &temp_statistics_time,
                                                                &temp_sample_interval,
                                                                &temp_resend_interval,
                                                                &temp_reload_config_interval,
                                                                &temp_threshold );
                    if(AS_RTN_OK != iRet) {
                        continue;
                    }

                    if (temp_sample_on != sample_conf.sample_on) {  //is default config
                        if(show_running_flag) {
                            vty_out(vty, " set acsample sample %s param %s %s %d\n", samples_second[i], SE_XML_ACSAMPLE_SWITCH ,SAMPLE_ON == temp_sample_on ? "on" : "off", slot_index);
                        }else {
                            vtysh_add_show_string_format(" set acsample sample %s param %s %s %d", samples_second[i], SE_XML_ACSAMPLE_SWITCH ,SAMPLE_ON == temp_sample_on ? "on" : "off", slot_index);                    
                        }
                    }
                }                                                    
            }
            
        }
		else {
            if (sample_on != sample_conf.sample_on) {  //is default config
                if(show_running_flag) {
                    vty_out(vty, " set acsample sample %s param %s %s\n",samples_second[i], SE_XML_ACSAMPLE_SWITCH ,SAMPLE_ON == sample_on ? "on" : "off");
                }else {
                    vtysh_add_show_string_format(" set acsample sample %s param %s %s",samples_second[i], SE_XML_ACSAMPLE_SWITCH ,SAMPLE_ON==sample_on?"on":"off");
                }
            }
		}
		
        if (sample_interval != sample_conf.sample_interval) {
            if(show_running_flag) {
                vty_out(vty, " set acsample sample %s param %s %u\n",samples_second[i], SE_XML_ACSAMPLE_INTERVAL ,sample_interval);
            }else {
                vtysh_add_show_string_format(" set acsample sample %s param %s %u",samples_second[i], SE_XML_ACSAMPLE_INTERVAL ,sample_interval);
            }
        }    
        
        if (statistics_time != sample_conf.statistics_time) {
            if(show_running_flag) {
                vty_out(vty, " set acsample sample %s param %s %u\n",samples_second[i], SE_XML_ACSAMPLE_STATIC_TIME ,statistics_time);
            }else {
                vtysh_add_show_string_format(" set acsample sample %s param %s %u",samples_second[i], SE_XML_ACSAMPLE_STATIC_TIME ,statistics_time);
            }
        }
        
        if (resend_interval != sample_conf.resend_interval) {
            if(show_running_flag) {
                vty_out(vty, " set acsample sample %s param %s %u\n",samples_second[i], SE_XML_ACSAMPLE_RESEND_INTERVAL ,resend_interval);
            }else {
                vtysh_add_show_string_format(" set acsample sample %s param %s %u",samples_second[i], SE_XML_ACSAMPLE_RESEND_INTERVAL ,resend_interval);
            }
        }
        
        if (reload_config_interval != sample_conf.reload_config_interval) {
            if(show_running_flag) {
                vty_out(vty, " set acsample sample %s param %s %u\n",samples_second[i], SE_XML_ACSAMPLE_RELOAD_INTERVAL ,reload_config_interval);
            }else {
                vtysh_add_show_string_format(" set acsample sample %s param %s %u",samples_second[i], SE_XML_ACSAMPLE_RELOAD_INTERVAL ,reload_config_interval);
            }
        }
        
        if (threshold != sample_conf.threshold) {
            if(show_running_flag) {
                vty_out(vty, " set acsample sample %s param %s %d\n",samples_second[i], SE_XML_ACSAMPLE_THRESHOLD ,threshold);
            }else {
                vtysh_add_show_string_format(" set acsample sample %s param %s %d",samples_second[i], SE_XML_ACSAMPLE_THRESHOLD ,threshold);
            }
		}
	}
	

	/*service state*/
	iRet = dbus_get_sample_service_state( dcli_dbus_connection, &state );
	if( AS_RTN_OK != iRet )
	{
		return CMD_SUCCESS;
	}
	else
	{
		if( DEFAULT_SERVER_STATE != state )
		{		    
            if(show_running_flag) {
                vty_out(vty, " service %s\n", (SAMPLE_SERVICE_STATE_ON == state) ? "enable" : "disable");
            }else {
                vtysh_add_show_string_format(" service %s", (SAMPLE_SERVICE_STATE_ON == state) ? "enable" : "disable");
            }
		}
	}

    if(show_running_flag) {
        vty_out(vty, " exit\n");
    }else {
        vtysh_add_show_string(" exit");
    }
	
	return CMD_SUCCESS;
}

DEFUN(show_acsample_running_config_func,
	show_acsample_running_config_cmd,
	"show acsample running config",
	SHOW_STR
	"show acsample running config\n" 
)
{
    vty_out(vty, "============================================================================\n");

    show_running_flag = 1;
    
    dcli_acsample_show_running_config(vty);

    show_running_flag = 0;

    vty_out(vty, "============================================================================\n");

	return CMD_SUCCESS;
}


void dcli_acsample_init()
{
	install_node(&acsample_node, dcli_acsample_show_running_config, "ACSAMPLE_NODE");
	install_default(ACSAMPLE_NODE);
	install_element(CONFIG_NODE, &conf_acsample_cmd);
	
	install_element(CONFIG_NODE, &show_acsample_running_config_cmd);
	install_element(ACSAMPLE_NODE, &show_acsample_running_config_cmd);
	
	install_element(ACSAMPLE_NODE, &conf_acsample_service_enable_cmd);
	
	install_element(ACSAMPLE_NODE, &show_acsample_socket_timeout_cmd);
	install_element(ACSAMPLE_NODE, &set_acsample_socket_timeout_cmd);
	
	install_element(ACSAMPLE_NODE, &conf_acsample_threshold_cmd);
//	install_element(ACSAMPLE_NODE, &set_acsample_threshold_maxuser_cmd);
	install_element(ACSAMPLE_NODE, &set_acsample_params_cmd);
	install_element(ACSAMPLE_NODE, &set_acsample_resend_cmd);

	install_element(ACSAMPLE_NODE, &set_acsample_state_cmd);	
	install_element(ACSAMPLE_NODE, &set_acsample_slot_state_cmd);
	install_element(ACSAMPLE_NODE, &set_acsample_conf_int_sti_by_name_cmd);
	install_element(ACSAMPLE_NODE, &set_acsample_res_rel_conf_by_name_cmd);
	install_element(ACSAMPLE_NODE, &set_acsample_conf_thr_threshold_cmd);
	install_element(ACSAMPLE_NODE, &show_acsample_conf_by_name_cmd);
	
	install_element(ACSAMPLE_NODE, &set_acsample_log_level_cmd);

	install_element(ACSAMPLE_NODE, &show_acsample_conf_cmd);

	//install_element(ACSAMPLE_NODE, &show_acsample_cpu_stat_cmd);
	install_element(ACSAMPLE_NODE, &show_acsample_cpu_real_time_stat_cmd);
	install_element(ACSAMPLE_NODE, &show_acsample_info_cmd);
	install_element(VIEW_NODE, &show_acsample_info_cmd);
	install_element(ENABLE_NODE, &show_acsample_info_cmd);
	
}



