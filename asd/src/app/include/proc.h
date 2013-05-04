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
* proc.h
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

#ifndef _PROP_H
#define _PROP_H

/* Separators */
#define SEP_EQUAL '='
#define SEP_SPACE ' '
#define SEP_Q_MARK	'"'
#define SEP_TAB (char)0x9

#define ENABLE 		1
#define DISABLE 		2
#define MYVERIFY 		3
#define LENGTH		255
#define KEYS_MAX 	255


typedef struct prop_data
{
  char *key;
  char *value;
} prop_data;

/**
 Split a string into a key and value pair. String must be in one of the
 following formats:
 <key><separator><value> or setenv <key><separator><value>
 @param type  Type of character separating key and value in the string.
 Available types are: SEP_EQUAL - "=", SEP_SPACE - " ", SEP_TAB "\t".
 @param line  String.
 @param prop  Pointer to the structure with keys and values.
 @return 0 - if there was an error, 1 - otherwise.
*/
int get_pr (char type, char *line, prop_data *prop);

/**
 Split data read from a file into key and value pairs and save them in
 an array of structures. Each string read from the file is searched for
 key and value pair.
 @param type  Type of character separating key and value in the string.
 Available types are: SEP_EQUAL - "=", SEP_SPACE - " ", SEP_TAB "\t".
 @param file_name  String with file name.
 @param properties  Array of structures with keys and values.
 @return Count of extracted key and value pairs.
*/
int load_prop (char type, char *file_name, prop_data properties[]);


/**
 Search for a value in an array of key and value structures.
 @param key  String with key.
 @param result  String with value if it is found, NULL - otherwise.
 @param properties  Array of structures with keys and values.
 @param count  Count of array elements.
 @return String with value if it is found, NULL - otherwise.
*/
char *get_prop (char *key, char *result, prop_data properties[], int count);

/**
 Save a key and value pair to a file. Saves a line with structure:
 <command><key><separator><value>
 @param type  Type of character separating key and value. Available
 types are: SEP_EQUAL - "=", SEP_SPACE - " ", SEP_TAB "\t".
 @param filename  String with file name.
 @param command  String with command.
 @param key  String with key.
 @param value  String with value.
 @return 0 - on error, 1 - otherwise.
*/

int save_global_conf(char type, char* filename, char *command, char *key, char *value);

/**
 Free memory used by an array of structures with keys and values.
 @param properties  String with key.
 @param result  String with value if it is found, NULL - otherwise.
 @param properties  Array of structures with keys and values.
 @param count  Count of array elements.
 @return Always returns 1.
*/
int free_prop (prop_data properties[], int count);
int hex2int(char c);
//int macstr2byte(char *macstr, unsigned char *des_byte, int byte_len);

void *get_buffer(int len);
void *free_buffer(void *buffer, int len);
int str2byte( char *str, int len,  unsigned char *byte_out);
FILE *lock_file(char* filename);
int unlock_file(FILE* fidIn );

//@}
#endif

