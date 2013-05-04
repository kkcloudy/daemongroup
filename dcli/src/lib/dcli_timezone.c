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
* dcli_timezone.c
*
* CREATOR:
* 		zhouym@autelan.com
*
* MODIFY:
* 		zengxx@autelan.com
*
* DESCRIPTION:
*		CLI definition for syslog module.
*
* DATE:
*		2010-03-17 12:00:11
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.10 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h> 
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <zebra.h>
#include <sys/wait.h>
#include <syslog.h>

#include "command.h"
#include "ws_log_conf.h"
#include "ws_dbus_list.h"
#include "dcli_main.h"
#include "ac_manage_def.h"
#include "ac_manage_extend_interface.h"
#include "ac_manage_ntpsyslog_interface.h"

#define TMP_TIME "/var/run/timezone.txt"

struct cmd_node timezone_node = 
{
	TIME_ZONE_NODE,
	"%s(config-timezone)# "
};
void process_str(char *s)
{
	int i = 0;
	for(i; '\0' != s[i]; i++)
	{
		if(s[i] >= 'A' && s[i] <= 'Z')
		s[i] += 32;
	}
}


int write_timezone_xml(char *xmlpath, char *area, char *city)
{
	//fprintf(stderr,"write_timezone_xml:area=%s,city=%s\n",area,city);
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp,rootnode;
    int conflag=0;	
	doc = xmlReadFile(xmlpath, "utf-8", 256); 
	if (doc == NULL ) 
	{
		return -1;
	}
	rootnode = xmlDocGetRootElement(doc); 
	if (rootnode == NULL)
	{
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(rootnode->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);
		return -1;
	}

	cur = rootnode->xmlChildrenNode;	
	while(cur !=NULL)
	{
	    tmp = cur->xmlChildrenNode;
		if (xmlStrcmp(cur->name, BAD_CAST "timezone") == 0)           
		{
            conflag++;//表示找到节点
			while(tmp !=NULL)
			{	 
				if ((!xmlStrcmp(tmp->name, BAD_CAST "area")))
				{
					xmlNodeSetContent(tmp, BAD_CAST  area); 
				}
				if ((!xmlStrcmp(tmp->name, BAD_CAST "city")))
				{
					xmlNodeSetContent(tmp, BAD_CAST  city);
				}
				tmp = tmp->next;
			}
		}
	   cur = cur->next;
	}
	if(0 == conflag)//新建
	{
		
		xmlNodePtr node = xmlNewNode(NULL,BAD_CAST "timezone");  
		xmlAddChild(rootnode,node);	
		
		xmlNewChild( node, NULL, BAD_CAST "area", BAD_CAST (area));
		xmlNewChild( node, NULL, BAD_CAST "city", BAD_CAST (city));
			
	}
	xmlSaveFile(xmlpath,doc); 
	xmlFreeDoc(doc);
	return 0;
}

int read_timezone_xml(char *xmlpath, char *area, char *city)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
	doc = xmlReadFile(xmlpath, "utf-8", 256); 
	if (doc == NULL ) 
	{
		return -1;
	}
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL)
	{
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);
		return -1;
	}

	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST "timezone"))           
		{
			xmlChar *value=NULL;			
			while(tmp !=NULL)
			{	 
				if ((!xmlStrcmp(tmp->name, BAD_CAST "area")))
				{
					value=xmlNodeGetContent(tmp);
					strcpy(area,(char *)value);	
					xmlFree(value);
				}
				if ((!xmlStrcmp(tmp->name, BAD_CAST "city")))
				{
					value=xmlNodeGetContent(tmp);
					strcpy(city,(char *)value);	
					xmlFree(value);
				}
				tmp = tmp->next;
			}            
		}      
	   cur = cur->next;
	}
	xmlFreeDoc(doc);
	return 0;
}
int dcli_set_timezone(char *area,char *city)
{
	int i = 0;
	int p_masterid = 0;	
	int localsid = 0;
	p_masterid = get_product_info(PRODUCT_ACTIVE_MASTER);
	localsid = get_product_info(PRODUCT_LOCAL_SLOTID);
	if(p_masterid == localsid)
	{
		for(i = 1; i < MAX_SLOT; i++)
		{
			if(dbus_connection_dcli[i]->dcli_dbus_connection) 
			{
				if(p_masterid != i)
				{
					ac_manage_set_timezone(dbus_connection_dcli[i]->dcli_dbus_connection, area, city);
				}
			}
		}	
	}
	return 0;
}

int dcli_do_tzconfig_sh(char *area_tmp,char *city_tmp)
{
	char city[50]={0};
	char area[50]={0};
	char cmd[256]={0};
	int sys_ret = -1,w_ret = -1;
	strcpy(city,city_tmp);
	strcpy(area,area_tmp);
	FILE *fp = NULL;
	/*命令行参数为小写是为了能够自动补全参数，
	此处做大小写转换是为脚本执行提供正确参数*/
	if(strcmp(area,"us") == 0)
		strcpy(area,"US");
	else
		area[0] -= 32;
	if((strncmp(city,"gmt",3) == 0)||(strncmp(city,"uct",3) == 0)||(strncmp(city,"utc",3) == 0))
	{
		city[0] -= 32;
		city[1] -= 32;
		city[2] -= 32;
	}
	else
	{
		city[0] -= 32;
	}
	fp = fopen(TMP_TIME,"w");
	if(NULL != fp)
	{
		fclose(fp);
	}
	snprintf(cmd,sizeof(cmd)-1,"/usr/sbin/tzconfig %s %s",area,city);		
	sys_ret = system(cmd);
	dcli_set_timezone(area, city);
	if(sys_ret == 0)
	{		
		return CMD_SUCCESS;
	}
	else
	{		
		return CMD_WARNING;
	}

}
DEFUN(conf_timezone_func,
	conf_timezone_cmd,
	"config timezone",
	CONFIG_STR
	"config timezone!\n"
)
{

	if (CONFIG_NODE == vty->node) {
		vty->node = TIME_ZONE_NODE;
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(set_timezone_africa_func,
	set_timezone_africa_cmd,
	"set timezone africa (abidjan|accra|addis_Ababa|algiers|asmara|asmera|bamako|bangui|banjul|\
	 bissau|blantyre|brazzaville|bujumbura|cairo|casablanca|ceuta|conakry|dakar|dar_es_Salaam|\
	 djibouti|douala|el_Aaiun|freetown|gaborone|harare|johannesburg|kampala|khartoum|kigali|kinshasa|\
	 lagos|libreville|lome|luanda|lubumbashi|lusaka|malabo|maputo|maseru|mbabane|mogadishu|monrovia|\
	 nairobi|ndjamena|niamey|nouakchott|ouagadougou|porto-Novo|sao_Tome|timbuktu|tripoli|tunis|windhoek)",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{	
	int sh_ret = -1;
	//if_ntp_exist();
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else if(NULL != argv[0])
	{
		sh_ret = dcli_do_tzconfig_sh("africa", argv[0]);
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	
	return CMD_SUCCESS;
}


DEFUN(set_timezone_america_func,
	set_timezone_america_cmd,
	"set timezone america (adak|anchorage|anguilla|antigua|araguaina|argentina/Buenos_Aires|argentina/Catamarca|\
	argentina/ComodRivadavia|argentina/Cordoba|argentina/Jujuy|argentina/La_Rioja|argentina/Mendoza|argentina/Rio_Gallegos|\
	argentina/San_Juan|argentina/San_Luis|argentina/Tucuman|argentina/Ushuaia|aruba|asuncion|atikokan|atka|bahia|barbados|\
	belem|belize|blanc-Sablon|boa_Vista|bogota|boise|buenos_Aires|cambridge_Bay|campo_Grande|cancun|caracas|catamarca|\
	cayenne|cayman|chicago|chihuahua|coral_Harbour|cordoba|costa_Rica|cuiaba|curacao|danmarkshavn|dawson|dawson_Creek|\
	denver|detroit|dominica|edmonton|eirunepe|el_Salvador|ensenada|fort_Wayne|fortaleza|glace_Bay|godthab|goose_Bay|\
	grand_Turk|grenada|guadeloupe|guatemala|guayaquil|guyana|halifax|havana|hermosillo|indiana/Indianapolis|indiana/Knox|\
	indiana/Marengo|indiana/Petersburg|indiana/Tell_City|indiana/Vevay|indiana/Vincennes|indiana/Winamac|indianapolis|\
	inuvik|iqaluit|jamaica|jujuy|juneau|kentucky/Louisville|kentucky/Monticello|knox_IN|la_Paz|lima|los_Angeles|louisville|\
	maceio|managua|manaus|marigot|martinique|mazatlan|mendoza|menominee|merida|mexico_City|miquelon|moncton|monterrey|\
	montevideo|montreal|montserrat|massau|new_York|nipigon|nome|noronha|north_Dakota/Center|north_Dakota/New_Salem|panama|\
	pangnirtung|paramaribo|phoenix|port-au-Prince|port_of_Spain|porto_Acre|porto_Velho|puerto_Rico|rainy_River|rankin_Inlet|\
	recife|regina|resolute|rio_Branco|rosario|santarem|santiago|santo_Domingo|sao_Paulo|scoresbysund|shiprock|st_Barthelemy|\
	st_Johns|st_Kitts|st_Lucia|st_Thomas|st_Vincent|swift_Current|tegucigalpa|thule|thunder_Bay|tijuana|toronto|tortola|\
	vancouver|virgin|whitehorse|winnipeg|yakutat|yellowknife)",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{	
	int sh_ret = -1;
	//if_ntp_exist();
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else if(NULL != argv[0])
	{
		sh_ret = dcli_do_tzconfig_sh("america", argv[0]);
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	
	return CMD_SUCCESS;
}

DEFUN(set_timezone_us_func,
	set_timezone_us_cmd,
	"set timezone us (alaska|aleutian|arizona|central|east-Indiana|eastern|hawaii|indiana-Starke|michigan|mountain|pacific|samoa)",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{	
	int sh_ret = -1;
	//if_ntp_exist();
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else if(NULL != argv[0])
	{
		sh_ret = dcli_do_tzconfig_sh("us", argv[0]);
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	
	return CMD_SUCCESS;
}

DEFUN(set_timezone_canada_func,
	set_timezone_canada_cmd,
	"set timezone canada (atlantic|central|east-Saskatchewan|eastern|mountain|newfoundland|pacific|saskatchewan|yukon)",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{	
	int sh_ret = -1;
	//if_ntp_exist();
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else if(NULL != argv[0])
	{
		sh_ret = dcli_do_tzconfig_sh("canada", argv[0]);
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	
	return CMD_SUCCESS;
}

DEFUN(set_timezone_asia_func,
	set_timezone_asia_cmd,
	"set timezone asia (aden|almaty|amman|anadyr|aqtau|aqtobe|ashgabat|ashkhabad|baghdad|bahrain|baku|bangkok|beirut|bishkek|brunei|\
	calcutta|choibalsan|chongqing|chungking|colombo|dacca|damascus|dhaka|dili|dubai|dushanbe|gaza|harbin|\
	ho_Chi_Minh|hong_Kong|hovd|irkutsk|istanbul|jakarta|jayapura|jerusalem|kabul|kamchatka|karachi|kashgar|\
	katmandu|kolkata|krasnoyarsk|kuala_Lumpur|kuching|kuwait|macao|macau|magadan|makassar|manila|muscat|nicosia|\
	novosibirsk|omsk|oral|phnom_Penh|pontianak|pyongyang|qatar|qyzylorda|rangoon|riyadh|riyadh87|riyadh88|riyadh89|\
	saigon|sakhalin|samarkand|seoul|shanghai|singapore|taipei|tashkent|tbilisi|tehran|tel_Aviv|thimbu|thimphu|tokyo|\
	ujung_Pandang|ulaanbaatar|ulan_Bator|urumqi|vientiane|vladivostok|yakutsk|yekaterinburg|yerevan)",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{	
	int sh_ret = -1;
	//if_ntp_exist();
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else if(NULL != argv[0])
	{
		sh_ret = dcli_do_tzconfig_sh("asia", argv[0]);
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	
	return CMD_SUCCESS;
}

DEFUN(set_timezone_atlantic_func,
	set_timezone_atlantic_cmd,
	"set timezone atlantic (azores|bermuda|canary|cape_Verde|faeroe|faroe|jan_Mayen|madeira|reykjavik|south_Georgia|st_Helena|stanley)",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{	
	int sh_ret = -1;
	//if_ntp_exist();
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else if(NULL != argv[0])
	{
		sh_ret = dcli_do_tzconfig_sh("atlantic", argv[0]);
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	
	return CMD_SUCCESS;
}

DEFUN(set_timezone_australia_func,
	set_timezone_australia_cmd,
	"set timezone australia (aCT|adelaide|brisbane|broken_Hill|canberra|currie|darwin|eucla|hobart|lHI|lindeman|lord_Howe|melbourne|\
	nSW|north|perth|queensland|south|sydney|tasmania|victoria|west|yancowinna)",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{	
	int sh_ret = -1;
	//if_ntp_exist();
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else if(NULL != argv[0])
	{
		sh_ret = dcli_do_tzconfig_sh("australia", argv[0]);
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	
	return CMD_SUCCESS;
}


DEFUN(set_timezone_europe_func,
	set_timezone_europe_cmd,
	"set timezone europe (amsterdam|andorra|athens|belfast|belgrade|berlin|bratislava|brussels|bucharest|budapest|chisinau|copenhagen|\
	dublin|gibraltar|guernsey|helsinki|isle_of_Man|istanbul|jersey|kaliningrad|kiev|lisbon|ljubljana|london|luxembourg|madrid|malta|\	
	mariehamn|minsk|monaco|moscow|nicosia|oslo|paris|podgorica|prague|riga|rome|samara|san_Marino|sarajevo|simferopol|skopje|sofia|\
	stockholm|tallinn|tirane|tiraspol|uzhgorod|vaduz|vatican|vienna|vilnius|volgograd|warsaw|zagreb|zaporozhye|zurich)",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{	
	int sh_ret = -1;
	//if_ntp_exist();
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else if(NULL != argv[0])
	{
		sh_ret = dcli_do_tzconfig_sh("europe", argv[0]);
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	
	return CMD_SUCCESS;
}

DEFUN(set_timezone_indian_func,
	set_timezone_indian_cmd,
	"set timezone indian (antananarivo|chagos|christmas|cocos|comoro|kerguelen|mahe|maldives|mauritius|mayotte|reunion)",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{	
	int sh_ret = -1;
	//if_ntp_exist();
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else if(NULL != argv[0])
	{
		sh_ret = dcli_do_tzconfig_sh("indian", argv[0]);
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	
	return CMD_SUCCESS;
}


DEFUN(set_timezone_pacific_func,
	set_timezone_pacific_cmd,
	"set timezone pacific (apia|auckland|chatham|easter|efate|enderbury|fakaofo|fiji|funafuti|galapagos|gambier|guadalcanal|guam|\
	honolulu|johnston|kiritimati|kosrae|kwajalein|majuro|marquesas|midway|nauru|niue|norfolk|noumea|pago_Pago|palau|pitcairn|ponape|\
	port_Moresby|rarotonga|saipan|samoa|tahiti|tarawa|tongatapu|truk|wake|wallis|yap)",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{	
	int sh_ret = -1;
	//if_ntp_exist();
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else if(NULL != argv[0])
	{
		sh_ret = dcli_do_tzconfig_sh("pacific", argv[0]);
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	
	return CMD_SUCCESS;
}

DEFUN(set_timezone_cst_func,
	set_timezone_cst_cmd,
	"set timezone cst",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{
	int sh_ret = -1;
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else
	{
		sh_ret = dcli_do_tzconfig_sh("asia", "shanghai");
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	return CMD_SUCCESS;
}

DEFUN(set_timezone_utc_func,
	set_timezone_utc_cmd,
	"set timezone utc",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{
	int sh_ret = -1;
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else
	{
		sh_ret = dcli_do_tzconfig_sh("etc", "utc");
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	return CMD_SUCCESS;
}

DEFUN(set_timezone_etc_func,
	set_timezone_etc_cmd,
	"set timezone etc (gmt|gmt+0|gmt+1|gmt+10|gmt+11|gmt+12|gmt+2|gmt+3|gmt+4|gmt+5|gmt+6|gmt+7|gmt+8|gmt+9|gmt-0|gmt-1|gmt-10|\
	gmt-11|gmt-12|gmt-13|gmt-14|gmt-2|gmt-3|gmt-4|gmt-5|gmt-6|gmt-7|gmt-8|gmt-9|gmt0|greenwich|uct|utc|universal|zulu)",
	"set\n"
	"set timezone\n"
	"Enter the area \n"
	)
{
	#if 0 
	int sh_ret = -1;
	//if_ntp_exist();
	if (access("/usr/sbin/tzconfig", F_OK|X_OK) != 0)
	{
		vty_out (vty, "Set timezone error:can't find tzconfig!\n");
		return CMD_WARNING;		
	}
	else if(NULL != argv[0])
	{
		sh_ret = dcli_do_tzconfig_sh("etc", argv[0]);
		if(sh_ret == CMD_SUCCESS)
		{
			vty_out (vty, "Set timezone successful!\n");
			return CMD_SUCCESS;
		}
		else
		{
			vty_out (vty, "Set timezone error!\n");
			return CMD_WARNING;
		}
	}	
	#endif
	return CMD_SUCCESS;
}

	

DEFUN(show_timezone_func,
	show_timezone_cmd,
	"show timezone\n",
	"show timezone and the time adjustment\n"
	)
{	
	char cmdstr[128] = {0};
	char result[128]={0};
	char result_str[128]={0};
	char result_tmp[128]={0};
	FILE *fp = 	NULL;
	
	sprintf(cmdstr,"sudo date +%%Z");
	fp = popen( cmdstr, "r" );
	if(fp)
	{
		fgets(result_str, sizeof(result_str), fp );
		snprintf(result, sizeof(result_str)-1, "Timezone  : %s", result_str);
		pclose(fp);

		sprintf(cmdstr,"sudo date +%%:z");		
		fp = popen( cmdstr, "r" );		
		if(fp)
		{			
			fgets( result_tmp, sizeof(result_tmp), fp);
			strncat(result, "Adjustment: ", sizeof(result)-strlen(result)-1);
			strncat(result, result_tmp, sizeof(result)-strlen(result)-1);
			vty_out (vty, result);
			pclose(fp);
		}
	}
	return CMD_SUCCESS;
}

DEFUN(del_timezone_func,
	del_timezone_cmd,
	"delete timezone\n",
	"delete timezone and the time adjustment\n"
	)
{
	int sh_ret = -1;
	sh_ret = dcli_do_tzconfig_sh("delete", "delete");
	if(sh_ret == CMD_SUCCESS)
	{
		vty_out (vty, "Delete timezone successful!\n");
		return CMD_SUCCESS;
	}
	else
	{
		vty_out (vty, "Delete timezone error!\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


int  dcli_timezone_show_running_config(struct vty* vty)
{

	char showStr[256]={0};
	char buff[128] = {0};
	int ret = -1;
	FILE *fp = NULL;
	char *p = NULL;
	char area[30] = {0};
	char city[30] = {0};
	vtysh_add_show_string("!timezone section \n");
	/////////////////////////////////////////////////
	if (access(TMP_TIME, F_OK) == 0)
	{
		fp=fopen("/etc/timezone","r");
		if (fp != NULL)
		{
			vtysh_add_show_string("config timezone \n");
			fgets(buff,sizeof(buff),fp);
			p = strtok(buff,"/");
			if(NULL != p)
			{
				strncpy(area,p,sizeof(area)-1);
				p = strtok(NULL, "/");
				strncpy(city,p,sizeof(area)-1);
				process_str(area);
				process_str(city);
				if((0 == strncmp(area,"asia",4))&&(0 == strncmp(city,"shanghai",8)))
				{
					snprintf(showStr,sizeof(showStr)-1," set timezone cst \n");
				}
				else if((0 == strncmp(area,"etc",3))&&(0 == strncmp(city,"utc",3)))
				{
					snprintf(showStr,sizeof(showStr)-1," set timezone utc \n");
				}
				else
				{
					snprintf(showStr,sizeof(showStr)-1," set timezone %s %s \n",area,city);
				}
				vtysh_add_show_string(showStr);
			}
			fclose(fp);	
			vtysh_add_show_string(" exit\n");	
		}
	}
    return CMD_SUCCESS;

}

void dcli_timezone_init
(
	void) 
{
	install_node(&timezone_node, dcli_timezone_show_running_config, "TIME_ZONE_NODE");
	install_default(TIME_ZONE_NODE);
	install_element(CONFIG_NODE,&conf_timezone_cmd);	
	//install_element(TIME_ZONE_NODE,&set_timezone_africa_cmd);
	//install_element(TIME_ZONE_NODE,&set_timezone_america_cmd);
	//install_element(TIME_ZONE_NODE,&set_timezone_us_cmd);
	//install_element(TIME_ZONE_NODE,&set_timezone_canada_cmd);
	install_element(TIME_ZONE_NODE,&set_timezone_asia_cmd);
	//install_element(TIME_ZONE_NODE,&set_timezone_atlantic_cmd);
	//install_element(TIME_ZONE_NODE,&set_timezone_australia_cmd);
	//install_element(TIME_ZONE_NODE,&set_timezone_europe_cmd);
	//install_element(TIME_ZONE_NODE,&set_timezone_indian_cmd);
	//install_element(TIME_ZONE_NODE,&set_timezone_pacific_cmd);
	//install_element(TIME_ZONE_NODE,&set_timezone_systemV_cmd);
	//install_element(TIME_ZONE_NODE,&set_timezone_etc_cmd);
	install_element(TIME_ZONE_NODE,&show_timezone_cmd);
	//install_element(TIME_ZONE_NODE,&del_timezone_cmd);
	install_element(TIME_ZONE_NODE,&set_timezone_cst_cmd);
	install_element(TIME_ZONE_NODE,&set_timezone_utc_cmd);
}


///////////////////////////////////////////
#ifdef __cplusplus
}
#endif

