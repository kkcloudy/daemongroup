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
* ACSettingsFile.c
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

#define CW_SETTINGS_FILE 	"settings.ac.txt"

#define CWMIN_DEFAULT	3
#define CWMAX_DEFAULT	10
#define AIFS_DEFAULT	1


FILE* gSettingsFile=NULL;
WTPQosValues* gDefaultQosValues=NULL;

void CWExtractValue(char* start, char** startValue, char** endValue, int* offset)
{
	*offset=strspn (start+1, " \t\n\r");
	*startValue = start +1+ *offset;

	*offset=strcspn (*startValue, " \t\n\r");
	*endValue = *startValue + *offset -1;
}

CWBool CWParseSettingsFile()
{
	char *line = NULL;
	gSettingsFile = fopen (CW_SETTINGS_FILE, "rb");
	if (gSettingsFile == NULL) {
		CWErrorRaiseSystemError(CW_ERROR_GENERAL);
	}
	
	CW_CREATE_ARRAY_ERR(gDefaultQosValues, NUM_QOS_PROFILES, WTPQosValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	while((line = (char*)CWGetCommand(gSettingsFile)) != NULL) 
	{
		char* startTag=NULL;
		char* endTag=NULL;
		
		if((startTag=strchr (line, '<'))==NULL) 
		{
			CW_FREE_OBJECT(line);
			continue;
		}

		if((endTag=strchr (line, '>'))==NULL) 
		{
			CW_FREE_OBJECT(line);
			continue;
		}
		
		if (!strncmp(startTag+1, "CWMIN_VOICE", endTag-startTag-1))
		{
			int value = atoi(endTag+1);
			
			if(value==0) value=CWMIN_DEFAULT;
			gDefaultQosValues[VOICE_QUEUE_INDEX].cwMin = value;
			wid_syslog_debug_debug(WID_DEFAULT,"CWMIN_VOICE: %d", gDefaultQosValues[VOICE_QUEUE_INDEX].cwMin);
			CW_FREE_OBJECT(line);
			continue;	
		}
		if (!strncmp(startTag+1, "CWMAX_VOICE", endTag-startTag-1))
		{
			int value = atoi(endTag+1);
			
			if(value==0) value=CWMAX_DEFAULT;
			gDefaultQosValues[VOICE_QUEUE_INDEX].cwMax = value;
			wid_syslog_debug_debug(WID_DEFAULT,"CWMAX_VOICE: %d", gDefaultQosValues[VOICE_QUEUE_INDEX].cwMax);
			CW_FREE_OBJECT(line);
			continue;	
		}
		if (!strncmp(startTag+1, "AIFS_VOICE", endTag-startTag-1))
		{
			int value = atoi(endTag+1);
			
			if(value==0) value=AIFS_DEFAULT;
			gDefaultQosValues[VOICE_QUEUE_INDEX].AIFS = value;
			wid_syslog_debug_debug(WID_DEFAULT,"AIFS_VOICE: %d", gDefaultQosValues[VOICE_QUEUE_INDEX].AIFS);
			CW_FREE_OBJECT(line);
			continue;	
		}


		if (!strncmp(startTag+1, "CWMIN_VIDEO", endTag-startTag-1))
		{
			int value = atoi(endTag+1);
			
			if(value==0) value=CWMIN_DEFAULT;
			gDefaultQosValues[VIDEO_QUEUE_INDEX].cwMin = value;
			wid_syslog_debug_debug(WID_DEFAULT,"CWMIN_VIDEO: %d", gDefaultQosValues[VIDEO_QUEUE_INDEX].cwMin);
			CW_FREE_OBJECT(line);
			continue;	
		}
		if (!strncmp(startTag+1, "CWMAX_VIDEO", endTag-startTag-1))
		{
			int value = atoi(endTag+1);
			
			if(value==0) value=CWMAX_DEFAULT;
			gDefaultQosValues[VIDEO_QUEUE_INDEX].cwMax = value;
			wid_syslog_debug_debug(WID_DEFAULT,"CWMAX_VIDEO: %d", gDefaultQosValues[VIDEO_QUEUE_INDEX].cwMax);
			CW_FREE_OBJECT(line);
			continue;	
		}
		if (!strncmp(startTag+1, "AIFS_VIDEO", endTag-startTag-1))
		{
			int value = atoi(endTag+1);
			
			if(value==0) value=AIFS_DEFAULT;
			gDefaultQosValues[VIDEO_QUEUE_INDEX].AIFS = value;
			wid_syslog_debug_debug(WID_DEFAULT,"AIFS_VIDEO: %d", gDefaultQosValues[VIDEO_QUEUE_INDEX].AIFS);
			CW_FREE_OBJECT(line);
			continue;	
		}
		

		if (!strncmp(startTag+1, "CWMIN_BEST_EFFORT", endTag-startTag-1))
		{
			int value = atoi(endTag+1);
			
			if(value==0) value=CWMIN_DEFAULT;
			gDefaultQosValues[BESTEFFORT_QUEUE_INDEX].cwMin = value;
			wid_syslog_debug_debug(WID_DEFAULT,"CWMIN_BEST_EFFORT: %d", gDefaultQosValues[BESTEFFORT_QUEUE_INDEX].cwMin);
			CW_FREE_OBJECT(line);
			continue;	
		}
		if (!strncmp(startTag+1, "CWMAX_BEST_EFFORT", endTag-startTag-1))
		{
			int value = atoi(endTag+1);
			
			if(value==0) value=CWMAX_DEFAULT;
			gDefaultQosValues[BESTEFFORT_QUEUE_INDEX].cwMax = value;
			wid_syslog_debug_debug(WID_DEFAULT,"CWMAX_BEST_EFFORT: %d", gDefaultQosValues[BESTEFFORT_QUEUE_INDEX].cwMax);
			CW_FREE_OBJECT(line);
			continue;	
		}
		if (!strncmp(startTag+1, "AIFS_BEST_EFFORT", endTag-startTag-1))
		{
			int value = atoi(endTag+1);
			
			if(value==0) value=AIFS_DEFAULT;
			gDefaultQosValues[BESTEFFORT_QUEUE_INDEX].AIFS = value;
			wid_syslog_debug_debug(WID_DEFAULT,"AIFS_BEST_EFFORT: %d", gDefaultQosValues[BESTEFFORT_QUEUE_INDEX].AIFS);
			CW_FREE_OBJECT(line);
			continue;	
		}


		if (!strncmp(startTag+1, "CWMIN_BACKGROUND", endTag-startTag-1))
		{
			int value = atoi(endTag+1);

			if(value==0) value=CWMIN_DEFAULT;
			gDefaultQosValues[BACKGROUND_QUEUE_INDEX].cwMin = value;
			wid_syslog_debug_debug(WID_DEFAULT,"CWMIN_BACKGROUND: %d", gDefaultQosValues[BACKGROUND_QUEUE_INDEX].cwMin);
			CW_FREE_OBJECT(line);
			continue;	
		}
		if (!strncmp(startTag+1, "CWMAX_BACKGROUND", endTag-startTag-1))
		{
			int value = atoi(endTag+1);
			
			if(value==0) value=CWMAX_DEFAULT;
			gDefaultQosValues[BACKGROUND_QUEUE_INDEX].cwMax = value;
			wid_syslog_debug_debug(WID_DEFAULT,"CWMAX_BACKGROUND: %d", gDefaultQosValues[BACKGROUND_QUEUE_INDEX].cwMax);
			CW_FREE_OBJECT(line);
			continue;	
		}
		if (!strncmp(startTag+1, "AIFS_BACKGROUND", endTag-startTag-1))
		{
			int value = atoi(endTag+1);
			
			if(value==0) value=AIFS_DEFAULT;
			gDefaultQosValues[BACKGROUND_QUEUE_INDEX].AIFS = value;
			wid_syslog_debug_debug(WID_DEFAULT,"AIFS_BACKGROUND: %d", gDefaultQosValues[BACKGROUND_QUEUE_INDEX].AIFS);
			CW_FREE_OBJECT(line);
			continue;	
		}
		CW_FREE_OBJECT(line);
	}
	return CW_TRUE;
}
