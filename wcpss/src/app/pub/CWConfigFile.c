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
* CWConfigFile.c
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/

 
#include "CWCommon.h"
#include "wcpss/wid/WID.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

FILE *gCWConfigFile = NULL;

CWConfigValue *gConfigValues;
int gConfigValuesCount;
int gModelCount;
//for auto upgrade
CWConfigVersionInfo *gConfigVersionInfo;
CWConfigVersionInfo_new *gConfigVerInfo = NULL;

struct n_rate_table g11nRateTable[NRATE_TABLE_LEN];

//for batchlly upgrade
CWConfigVersionInfo *gConfigVersionUpdateInfo[BATCH_UPGRADE_AP_NUM];


CWOUIInfo                 *gBlackOuiInfoList = NULL;
CWOUIInfo                 *gWhiteOuiInfoList = NULL;
int                        gblackOuiNum = 0;
int                        gWhiteOuiNum = 0;
int                        gOuiListType = 1;/*0---none,1---use black oui list ,2---use white oui list*/
unsigned char gupdateCountOneTime = 5;
unsigned char gupdateControl = 0;

int ApimgModelCount;


// replacement for std fgets which seems to dislike windows return character
char *CWFgets(char *buf, int bufSize, FILE *f) {
	int i = -1;
	
	if(buf == NULL || f == NULL || bufSize <= 0) return NULL;
	
	CW_ZERO_MEMORY(buf, bufSize);
	
	do {
		i++;
		buf[i] = getc(f);
		if(buf[i] == EOF) {
			i--;
			break;
		}
	} while (i < bufSize && buf[i] != '\n' && buf[i] != '\r');
	
	if(i == -1) return NULL;
	i++;
	buf[i] = '\0';
	
	return buf;
}

//get one "useful" (not a comment, not blank) line from the config file
char * CWGetCommand(FILE *configFile) {
	char *buff = NULL;
	char *command = NULL;
	char *ret = NULL;
	CW_CREATE_STRING_ERR_WID(buff, CW_BUFFER_SIZE, return NULL;);
	
	while (((ret = CWFgets(buff, CW_BUFFER_SIZE, configFile)) != NULL) && (buff[0] == '\n' || buff[0] == '\r' || buff[0] == '#')); // skip comments and blank lines
	
	if(buff != NULL && ret != NULL) {
		int len = strlen(buff);
		buff[len-1] = '\0'; // remove new line
		
		CW_CREATE_STRING_ERR_WID(command, len, CW_FREE_OBJECT_WID(buff); return NULL;);
		memset(command,0,len);
		strcpy(command, buff);
	}
	
	CW_FREE_OBJECT_WID(buff);
	
	return command;
}

// *********************************************************************************************************************************
// *** Function Description ********************************************************************************************************
// *********************************************************************************************************************************
// Parses the configuration file
// *********************************************************************************************************************************
// *** Function Parameters *********************************************************************************************************
// *********************************************************************************************************************************
// CWBool isCount	CW_TRUE ot just count ACAddresses and paths, CW_FALSE to actually parse them
// *********************************************************************************************************************************
// *** Returns *********************************************************************************************************************
// *********************************************************************************************************************************
// CWBool:		CW_TRUE: if the operation is succesful, CW_FALSE otherwise
CWBool CWParseTheFile(CWBool isCount) {
	char *line = NULL;
	int i;
	
	if(!isCount) {
		for(i = 0; i < gConfigValuesCount; i++) {
			if(gConfigValues[i].type == CW_STRING_ARRAY) {
				CW_CREATE_ARRAY_ERR((gConfigValues[i].value.str_array_value), gConfigValues[i].count, char*, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			}
		}
	} else {
		for(i = 0; i < gConfigValuesCount; i++) {
			if(gConfigValues[i].type == CW_STRING_ARRAY) {
				gConfigValues[i].count = 0;
			}
		}
	}
	
	gCWConfigFile = fopen (CW_CONFIG_FILE, "rb");
	if (gCWConfigFile == NULL) {
		CWErrorRaiseSystemError(CW_ERROR_GENERAL);
	}
	
	while((line = CWGetCommand(gCWConfigFile)) != NULL) {
		int i, j;
		CWDebugLog("*** Parsing (%s) ***", line);
		
		for(i = 0; i < gConfigValuesCount; i++) {
			if(!strncmp(line, gConfigValues[i].code, strlen(gConfigValues[i].code))) {
				char *myLine = line + strlen(gConfigValues[i].code);
				
				switch(gConfigValues[i].type) {
					case CW_INTEGER:
						gConfigValues[i].value.int_value = atoi(myLine);
						break;
					case CW_STRING:
						CW_CREATE_STRING_FROM_STRING_ERR(gConfigValues[i].value.str_value, myLine, CW_FREE_OBJECT_WID(line); return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
						break;
					case CW_STRING_ARRAY:
						#ifdef CW_DEBUGGING
							CWDebugLog("*** Parsing String Array... *** \n");
						#endif
						j = 0;
						CW_FREE_OBJECT_WID(line);
						while((line = CWGetCommand(gCWConfigFile)) != NULL && strcmp(line, gConfigValues[i].endCode)) {
							#ifdef CW_DEBUGGING
								CWDebugLog("*** Parsing String (%s) *** \n", line);
							#endif
							
							if(isCount) gConfigValues[i].count++;
							else {
								CW_CREATE_STRING_FROM_STRING_ERR((gConfigValues[i].value.str_array_value)[j], line, CW_FREE_OBJECT_WID(line); return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
								j++;
							}
							CW_FREE_OBJECT_WID(line);
						}
						break;
				}
				
				break;
			}
		}

		
		CW_FREE_OBJECT_WID(line);
	}
	
	CWDebugLog("*** Config File Parsed ***");

	fclose(gCWConfigFile);
	
	return CW_TRUE;
}

// parses the configuration file
CWBool CWParseConfigFile() {
	if(!(CWConfigFileInitLib())) return CW_FALSE;
	
	if(!CWParseTheFile(CW_TRUE // just count the objects
			)) return CW_FALSE;
	
	
	if(!CWParseTheFile(CW_FALSE // actually parse
			)) return CW_FALSE;
	
	#ifdef CW_DEBUGGING
		{
			int i;
			for(i = 0; i < gConfigValuesCount; i++) {
				if(gConfigValues[i].type == CW_INTEGER) {
					CWDebugLog("%s%d", gConfigValues[i].code, gConfigValues[i].value.int_value);
				}
			}
		}
		CWDebugLog("*** Config File END ***");
	#endif
	
	return CWConfigFileDestroyLib();
}





