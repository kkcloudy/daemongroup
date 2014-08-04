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
* dcli_user.c
*
* MODIFY:
*
* CREATOR:
*		shancx@autelan.com
*
* DESCRIPTION:
*		CLI definition for user module.
*
* DATE:
*		04/22/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.50 $	
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif


#include <stdio.h>
#include <stdlib.h>
#include "dcli_user.h"
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <stdarg.h>
#include <errno.h>

/*for BSD*/
#include <dbus/dbus.h>
#include "bsd/bsdpub.h"
#include "bsd_bsd.h"

#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include "memory.h"
#include "command.h"

#include <shadow.h>
#include <unistd.h>
#include <dbus/sem/sem_dbus_def.h>
#include "dcli_main.h"
#include "linklist.h"
#include "memory.h"

#if 1 //added by houxx
#include "dcli_md5.h"
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_modules.h>
#include <termios.h>


#include <pwd.h>;
#include <sys/types.h>;
#include <unistd.h>

#endif


#if 1 //added by houxx

#include <signal.h>
#include <string.h>
#include <time.h>


#define PAM_PROMPT_ECHO_OFF	1
#define PAM_PROMPT_ECHO_ON	2
#define PAM_ERROR_MSG		3
#define PAM_TEXT_INFO		4

/* Linux-PAM specific types */

#define PAM_RADIO_TYPE          5        /* yes/no/maybe conditionals */

/* This is for server client non-human interaction.. these are NOT
   part of the X/Open PAM specification. */

#define PAM_BINARY_PROMPT       7

/* maximum size of messages/responses etc.. (these are mostly
   arbitrary so Linux-PAM should handle longer values). */

#define PAM_MAX_NUM_MSG       32
#define PAM_MAX_MSG_SIZE      512
#define PAM_MAX_RESP_SIZE     512


#define INPUTSIZE PAM_MAX_MSG_SIZE           /* maximum length of input+1 */
#define CONV_ECHO_ON  1                            /* types of echo state */
#define CONV_ECHO_OFF 0
#define HTDIGEST "/etc/htdigest" 

int Htdigest(const char *name,const char *passwd,int flag){
	FILE *fp;	
	if(NULL == (fp = fopen(HTDIGEST,"r+"))){
		return -1;
	}

	char buf[1024] = {0};
	char user_name[32] = {0};
	char user_passwd[32 + 1] = {0};

	char value[1024] = {0}; 
	char md5[32 + 1] = {0};
	
	if(flag){
		sprintf(value,"%s:AuteLAN:%s",name,passwd);
		Md5(value,md5); 
	}
	else{
		sprintf(md5,"%s",passwd);
	}
	
	while(NULL != fgets(buf,1024,fp)){
		sscanf(buf,"%[^:]:%*[^:]:%[^\n]",user_name,user_passwd);	
		if( !strcmp(user_name,name)){
			if(fseek(fp,-33,SEEK_CUR)!= 0)
			{
				fclose(fp);
				return -1;

			}
			fprintf(fp,"%s",md5);
			fclose(fp);
			return 1;
		}
		memset(buf,0,1024); 
		memset(user_name,0,32);
		memset(user_passwd,0,32);
	}
	fprintf(fp,"%s:AuteLAN:%s\n",name,md5);
	fclose(fp);
	return 1;
}



/*
 * external timeout definitions - these can be overriden by the
 * application.
 */

time_t pam_misc_conv_warn_time = 0;                  /* time when we warn */
time_t pam_misc_conv_die_time  = 0;               /* time when we timeout */

const char *pam_misc_conv_warn_line = "..\a.Time is running out...\n";
const char *pam_misc_conv_die_line  = "..\a.Sorry, your time is up!\n";

char input_passwd[33];

int pam_misc_conv_died=0;       /* application can probe this for timeout */

/*
 * These functions are for binary prompt manipulation.
 * The manner in which a binary prompt is processed is application
 * specific, so these function pointers are provided and can be
 * initialized by the application prior to the conversation function
 * being used.
 */

static void pam_misc_conv_delete_binary(void *appdata,
					pamc_bp_t *delete_me)
{
    PAM_BP_RENEW(delete_me, 0, 0);
}

int (*pam_binary_handler_fn)(void *appdata, pamc_bp_t *prompt_p) = NULL;
void (*pam_binary_handler_free)(void *appdata, pamc_bp_t *prompt_p)
      = pam_misc_conv_delete_binary;

/* the following code is used to get text input */

static volatile int expired=0;

/* return to the previous signal handling */
static void reset_alarm(struct sigaction *o_ptr)
{
    (void) alarm(0);                 /* stop alarm clock - if still ticking */
    (void) sigaction(SIGALRM, o_ptr, NULL);
}

/* this is where we intercept the alarm signal */
static void time_is_up(int ignore)
{
    expired = 1;
}

/* set the new alarm to hit the time_is_up() function */
static int set_alarm(int delay, struct sigaction *o_ptr)
{
    struct sigaction new_sig;

    sigemptyset(&new_sig.sa_mask);
    new_sig.sa_flags = 0;
    new_sig.sa_handler = time_is_up;
    if ( sigaction(SIGALRM, &new_sig, o_ptr) ) {
	return 1;         /* setting signal failed */
    }
    if ( alarm(delay) ) {
	(void) sigaction(SIGALRM, o_ptr, NULL);
	return 1;         /* failed to set alarm */
    }
    return 0;             /* all seems to have worked */
}

/* return the number of seconds to next alarm. 0 = no delay, -1 = expired */
static int get_delay(void)
{
    time_t now;

    expired = 0;                                        /* reset flag */
    (void) time(&now);

    /* has the quit time past? */
    if (pam_misc_conv_die_time && now >= pam_misc_conv_die_time) {
	fprintf(stderr,"%s",pam_misc_conv_die_line);

	pam_misc_conv_died = 1;       /* note we do not reset the die_time */
	return -1;                                           /* time is up */
    }

    /* has the warning time past? */
    if (pam_misc_conv_warn_time && now >= pam_misc_conv_warn_time) {
	fprintf(stderr, "%s", pam_misc_conv_warn_line);
	pam_misc_conv_warn_time = 0;                    /* reset warn_time */

	/* indicate remaining delay - if any */

	return (pam_misc_conv_die_time ? pam_misc_conv_die_time - now:0 );
    }

    /* indicate possible warning delay */

    if (pam_misc_conv_warn_time)
	return (pam_misc_conv_warn_time - now);
    else if (pam_misc_conv_die_time)
	return (pam_misc_conv_die_time - now);
    else
	return 0;
}

/* read a line of input string, giving prompt when appropriate */
static int read_string(int echo, const char *prompt, char **retstr)
{
    struct termios term_before, term_tmp;
    char line[INPUTSIZE];
    struct sigaction old_sig;
    int delay, nc = -1, have_term = 0;
    sigset_t oset, nset;



	//fflush(stdin);

    D(("called with echo='%s', prompt='%s'.", echo ? "ON":"OFF" , prompt));

    if (isatty(STDIN_FILENO)) {                      /* terminal state */

	/* is a terminal so record settings and flush it */
	if ( tcgetattr(STDIN_FILENO, &term_before) != 0 ) {
	    D(("<error: failed to get terminal settings>"));
	    *retstr = NULL;
	    return -1;
	}
	memcpy(&term_tmp, &term_before, sizeof(term_tmp));
	if (!echo) {
	    term_tmp.c_lflag &= ~(ECHO);
	}
	have_term = 1;

	/*
	 * We make a simple attempt to block TTY signals from terminating
	 * the conversation without giving PAM a chance to clean up.
	 */

	sigemptyset(&nset); 
	sigaddset(&nset, SIGINT); 
	sigaddset(&nset, SIGTSTP); 
	(void) sigprocmask(SIG_BLOCK, &nset, &oset);

    } else if (!echo) {
	D(("<warning: cannot turn echo off>"));
    }

    /* set up the signal handling */
    delay = get_delay();

    /* reading the line */
	while (delay >= 0)
	{

		fprintf(stderr, "%s", prompt);
		/* this may, or may not set echo off -- drop pending input */
		if (have_term)
		    (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_tmp);

		if ( delay > 0 && set_alarm(delay, &old_sig) ) {
		    D(("<failed to set alarm>"));
		    break;
		} else {
			int input;
			int i = 0;

#if 0
		    nc = read(STDIN_FILENO, line, INPUTSIZE-1);
#endif

			while((input = getchar()) != '\n' && i <= 32)
			{
				line[i++] = (char)input;			
			}
			line[i] = '\0';
			nc = i+1;


		    if (have_term) {
			(void) tcsetattr(STDIN_FILENO, TCSADRAIN, &term_before);
			if (!echo || expired)             /* do we need a newline? */
			    fprintf(stderr,"\n");
		    }
		    if ( delay > 0 ) {
			reset_alarm(&old_sig);
		    }
		    if (expired) {
			delay = get_delay();
		    } else if (nc > 0) {                 /* we got some user input */
			D(("we got some user input"));

			if (nc > 0 && line[nc-1] == '\n') {     /* <NUL> terminate */
			    line[--nc] = '\0';
			} else {
			    if (echo) {
				fprintf(stderr, "\n");
			    }
			    line[nc] = '\0';
			}
			
			if(i < 32)
				strcpy(input_passwd,line);
			*retstr = x_strdup(line);
			_pam_overwrite(line);

			goto cleanexit;                /* return malloc()ed string */

		    } else if (nc == 0) {                                /* Ctrl-D */
			D(("user did not want to type anything"));

			*retstr = NULL;
			if (echo) {
			    fprintf(stderr, "\n");
			}
			goto cleanexit;                /* return malloc()ed "" */
		    } else if (nc == -1) {
			/* Don't loop forever if read() returns -1. */
			D(("error reading input from the user: %s", strerror(errno)));
			if (echo) {
			    fprintf(stderr, "\n");
			}
			*retstr = NULL;
			goto cleanexit;                /* return NULL */
		    }
		}
	}

    /* getting here implies that the timer expired */

    D(("the timer appears to have expired"));

    *retstr = NULL;
    _pam_overwrite(line);

 cleanexit:

    if (have_term) {
	(void) sigprocmask(SIG_SETMASK, &oset, NULL);
	(void) tcsetattr(STDIN_FILENO, TCSADRAIN, &term_before);
    }

    return nc;
}

/* end of read_string functions */

/*
 * This conversation function is supposed to be a generic PAM one.
 * Unfortunately, it is _not_ completely compatible with the Solaris PAM
 * codebase.
 *
 * Namely, for msgm's that contain multiple prompts, this function
 * interprets "const struct pam_message **msgm" as equivalent to
 * "const struct pam_message *msgm[]". The Solaris module
 * implementation interprets the **msgm object as a pointer to a
 * pointer to an array of "struct pam_message" objects (that is, a
 * confusing amount of pointer indirection).
 */

int pass_misc_conv(int num_msg, const struct pam_message **msgm,
	      struct pam_response **response, void *appdata_ptr)
{
    int count=0;
    struct pam_response *reply;

    if (num_msg <= 0)
	return PAM_CONV_ERR;

    D(("allocating empty response structure array."));

    reply = (struct pam_response *) calloc(num_msg,
					   sizeof(struct pam_response));
    if (reply == NULL) {
	D(("no memory for responses"));
	return PAM_CONV_ERR;
    }

    D(("entering conversation function."));

    for (count=0; count < num_msg; ++count) {
	char *string=NULL;
	int nc;

	switch (msgm[count]->msg_style) {
	case PAM_PROMPT_ECHO_OFF:
	    nc = read_string(CONV_ECHO_OFF,msgm[count]->msg, &string);
	    if (nc < 0) {
		goto failed_conversation;
	    }
	    break;
	case PAM_PROMPT_ECHO_ON:
	    nc = read_string(CONV_ECHO_ON,msgm[count]->msg, &string);
	    if (nc < 0) {
		goto failed_conversation;
	    }
	    break;
	case PAM_ERROR_MSG:
	    if (fprintf(stderr,"%s\n",msgm[count]->msg) < 0) {
		goto failed_conversation;
	    }
	    break;
	case PAM_TEXT_INFO:
	    if (fprintf(stdout,"%s\n",msgm[count]->msg) < 0) {
		goto failed_conversation;
	    }
	    break;
	case PAM_BINARY_PROMPT:
	{
	    pamc_bp_t binary_prompt = NULL;

	    if (!msgm[count]->msg || !pam_binary_handler_fn) {
		goto failed_conversation;
	    }

	    PAM_BP_RENEW(&binary_prompt,
			 PAM_BP_RCONTROL(msgm[count]->msg),
			 PAM_BP_LENGTH(msgm[count]->msg));
	    PAM_BP_FILL(binary_prompt, 0, PAM_BP_LENGTH(msgm[count]->msg),
			PAM_BP_RDATA(msgm[count]->msg));

	    if (pam_binary_handler_fn(appdata_ptr,
				      &binary_prompt) != PAM_SUCCESS
		|| (binary_prompt == NULL)) {
		goto failed_conversation;
	    }
	    string = (char *) binary_prompt;
	    binary_prompt = NULL;

	    break;
	}
	default:
	    fprintf(stderr, "erroneous conversation (%d)\n"
		    ,msgm[count]->msg_style);
	    goto failed_conversation;
	}

	if (string) {                         /* must add to reply array */
	    /* add string to list of responses */

	    reply[count].resp_retcode = 0;
	    reply[count].resp = string;
	    string = NULL;
	}
    }

    *response = reply;
    reply = NULL;

    return PAM_SUCCESS;

failed_conversation:

    D(("the conversation failed"));

    if (reply) {
	for (count=0; count<num_msg; ++count) {
	    if (reply[count].resp == NULL) {
		continue;
	    }
	    switch (msgm[count]->msg_style) {
	    case PAM_PROMPT_ECHO_ON:
	    case PAM_PROMPT_ECHO_OFF:
		_pam_overwrite(reply[count].resp);
		free(reply[count].resp);
		break;
	    case PAM_BINARY_PROMPT:
		pam_binary_handler_free(appdata_ptr,
					(pamc_bp_t *) &reply[count].resp);
		break;
	    case PAM_ERROR_MSG:
	    case PAM_TEXT_INFO:
		/* should not actually be able to get here... */
		free(reply[count].resp);
	    }                                            
	    reply[count].resp = NULL;
	}
	/* forget reply too */
	free(reply);
	reply = NULL;
    }

    return PAM_CONV_ERR;
}

#endif




extern int boot_flag;	


/* Execute command in child process. */
static int
execute_dcli_shell (const char *command)
{
#if 0
	pid_t pid;
	int ret,status;
	pid = fork ();
	if (pid < 0)
    {
      /* Failure of fork(). */
      fprintf (stderr, "Can't fork: %s\n", safe_strerror (errno));
      exit (1);
    }
  	else if (pid == 0)
    {
   
		system(command);
	/*
		system("/opt/bin/username/useradd.sh aaa  abc vtyadmin");
*/
	      /* When execlp suceed, this part is not executed. */
		  printf("syetem\n");
      	fprintf (stderr, "Can't execute %s: %s\n", command, safe_strerror (errno));
      	exit (1);
    }
  	else
    {
      /* This is parent. */
      ret = wait4 (pid, &status, 0, NULL);
	  printf("aaaa syetem\n");
    }
#else
		
	return system(command);
#endif
  return 0;
}

int is_admin_user(char* name)
{
	struct passwd *pwd = NULL;

	pwd = getpwuid(getuid());
	if(!pwd)
		return -1;
	if(strcmp(pwd->pw_name,"admin")) 
		return -1;
	return 1;
}
int is_user_exsit(char* name)
{
	struct passwd *passwd;

	passwd = getpwnam(name);
	if(passwd)
		return 1;
	else
		return 0;

}

int is_user_self(char* name)
{
	int uid;
	struct passwd *passwd;

	uid = geteuid();
	passwd = getpwnam(name);
	if(!passwd)
		return -1;
	if(uid == passwd->pw_uid)
		return 1;
	else
		return 0;
}

int get_user_role(char* name)
{
	int uid,i;
	struct passwd *passwd;
	struct group *group=NULL;

	group = getgrnam(ADMINGROUP);
	if(group && group->gr_mem )
	{
		for(i=0;group->gr_mem[i];i++)
			if(!strcmp(group->gr_mem[i],name))
				return 1;

	}
	
	group = getgrnam(VIEWGROUP);
	if(group && group->gr_mem )
	{
		for(i=0;group->gr_mem[i];i++)
			if(!strcmp(group->gr_mem[i],name))
				return 0;

	}
	return -1;

}

int get_self_role()
{
	int uid,i;
	struct passwd *passwd = NULL;
	struct group *group=NULL;
	char *name;

	passwd = getpwuid(getuid());
	if(passwd)
		name = passwd->pw_name;
	else
		return -1;
	group = getgrnam(ADMINGROUP);
	if(group && group->gr_mem )
	{
		for(i=0;group->gr_mem[i];i++)
			if(!strcmp(group->gr_mem[i],name))
				return 1;

	}
	
	group = getgrnam(VIEWGROUP);
	if(group && group->gr_mem )
	{
		for(i=0;group->gr_mem[i];i++)
			if(!strcmp(group->gr_mem[i],name))
				return 0;

	}
	return -1;

}

/****************************************
*added user to system
*input 
*       name: user name
*	  passwd: user password
*output 
* 	  none
*return
*       0     OK
*       -1   the user is exist
*       -2   system error
*
*****************************************/

int dcli_user_add_sh(const char* name,const char* password,char* enable,char* sec)
{
	char command[128];
	if(!name || !password || !enable || !sec)
		return -3;
	sprintf(command,"useradd.sh %s \'%s\' %s %s",name,password,enable,sec);
	return execute_dcli_shell(command);

}

/**add by gjd : used for check the register users number , the max num is 32**/
int
dcli_user_number_check()

{
	int i=0,j=0;
	struct group *grentry = NULL;
	char *ptr;

	grentry = getgrnam(ADMINGROUP);
	if (grentry)
	{
		for(i=0;ptr=grentry->gr_mem[i];i++)
			;				
	}
	
	grentry = getgrnam(VIEWGROUP);
	if (grentry)
	{
		for(j=0;ptr=grentry->gr_mem[j];j++)
			;	
	}

	return (i+j);
	
}
/**2011-03-18: pm 4:00**/

int dcli_user_name_check(char* name)
{
	int i = 0;
	int len ;
	char tmp;
	if(!name)
		return 1;
	len = strlen(name);
	if(len < 4 || len > 32)
		return 2;
	tmp = *name;
	if(!(tmp>='a' && tmp<='z')&&!(tmp>='A' &&tmp<='Z'))
		return 3;
	for(i = 0;i<len ;i++)
	{
		tmp = *(name+i);
		if(tmp == '_' || (tmp>='0' && tmp<='9')||(tmp>='a' &&tmp<='z')||(tmp>='A' &&tmp<='Z'))
		{
			continue;
		}
		else
		{
			return 1;
		}
	}
	return 0;
	
}
 int passwdlen = 4;
 int passwdmaxlen = 32;
 int strongpasswd = 0;
 int passwdalivetime = 90;
 int passwmaxerrtimes = 3;
 int passwdunreplytimes = 3;

 
#if 1 //added by houxx
 
#define  x_strdup(s)  ( (s) ? strdup(s):NULL )
#define LENGTH_PASS 64
 
 static struct pam_conv pass_conv = {
	pass_misc_conv ,//misc_conv
	 NULL
 };

	 
	 
 struct cracklib_options {
	 int min_length;
	 int dig_credit;
	 int up_credit;
	 int low_credit;
	 int oth_credit;
 };
 
 
 static char * str_lower(char *string)
 {
	 char *cp;
 
	 for (cp = string; *cp; cp++)
		 *cp = tolower(*cp);
	 return string;
 }
 
 static int palindrome(const char *new)
 {
	 int i, j;
 
	 i = strlen (new);
 
	 for (j = 0;j < i;j++)
		 if (new[i - j - 1] != new[j])
			 return 0;
 
	 return 1;
 }
 
 
 static int simple(struct cracklib_options *opt,const char *new)
 {
	 int digits = 0;
	 int uppers = 0;
	 int lowers = 0;
	 int others = 0;
	 int size;
	 int i;
 
	 for (i = 0;new[i];i++) {
	 if (isdigit (new[i]))
		 digits++;
	 else if (isupper (new[i]))
		 uppers++;
	 else if (islower (new[i]))
		 lowers++;
	 else
		 others++;
	 }

	 if ((opt->dig_credit >= 0) && (digits > opt->dig_credit))
	 digits = opt->dig_credit;
 
	 if ((opt->up_credit >= 0) && (uppers > opt->up_credit))
	 uppers = opt->up_credit;
 
	 if ((opt->low_credit >= 0) && (lowers > opt->low_credit))
	 lowers = opt->low_credit;
 
	 if ((opt->oth_credit >= 0) && (others > opt->oth_credit))
	 others = opt->oth_credit;
 
	 size = opt->min_length;
 
	 if (opt->dig_credit >= 0)
	 size -= digits;
	 else if (digits < opt->dig_credit * -1)
	 return 1;
 
	 if (opt->up_credit >= 0)
	 size -= uppers;
	 else if (uppers < opt->up_credit * -1)
	 return 1;
 
	 if (opt->low_credit >= 0)
	 size -= lowers;
	 else if (lowers < opt->low_credit * -1)
	 return 1;
 
	 if (opt->oth_credit >= 0)
	 size -= others;
	 else if (others < opt->oth_credit * -1)
	 return 1;
 
	 if (size <= i)
	 return 0;
 
	 return 1;
 }
 
 
#endif


 #if 0 //added by houxx
int dcli_user_passwd_check(char* username,char* passwd)
{
	int i = 0;
	int len ;
	char tmp;
	int hasword=0,hasbigword=0,hasnumber=0;
	if(!passwd)
		return 1;
	len = strlen(passwd);
	if(len < passwdlen || len > passwdmaxlen)
		return 2;
	for(i = 0;i<len ;i++)
	{
		tmp = *(passwd+i);
		if((tmp>='0' && tmp<='9'))
		{
			hasnumber = 1;
			continue;
		}
		else if(tmp>='a' &&tmp<='z') 
		{
			hasword =1;
			continue;

		}
		else if(tmp>='A' &&tmp<='Z')
		{
			hasbigword = 1;
			continue;

		}			
		else if(tmp == '_')
			continue;
	}
	
	if(strongpasswd && !boot_flag)
	{
		if(pwd_reply_check(username,passwd)>0)
			return 5;
		
	}
	if(strongpasswd && hasword && hasbigword && hasnumber)
	{

		if(!username||strcmp(username,passwd))
			return 0;
		else
			return 4;
	}
	else if(!strongpasswd)
		return 0;
	else
		return 3;
 }
#else
int dcli_user_passwd_check(char* username,char* passwd)
{
	
	int ret = 0;	
	int len ;
	struct cracklib_options opt;
	char *newmono = NULL;

	if(!passwd)
		return 6;
	
	get_global_variable_status();
/*	newmono = (char *)malloc(LENGTH_PASS);*/
	newmono = str_lower(x_strdup(passwd));
	opt.min_length = passwdlen;	

	len = strlen(passwd);
	if(len < passwdlen || len > passwdmaxlen)
	{
		free(newmono);
		return 4;
	}
	if(strongpasswd)
	{
		opt.up_credit = -1;
		opt.low_credit = -1;
		opt.dig_credit = -1;
		if(username && (strcmp(username,passwd) == 0))
		{
			free(newmono);
			return 3;
		}		
		if (simple(&opt, passwd))
		{
			free(newmono);
			return 2;
		}	

		if (palindrome(newmono))
		{	
			free(newmono);
			return 1;
		}
	}


#if 0	
	if(strongpasswd && !boot_flag)
	{
		if(pwd_reply_check(username,passwd)>0)
		{
			free(newmono);
			return 5;		
		}
	}
#endif	
	free(newmono);
	return 0;

	
	
}

int dcli_user_change_passwd_check(char* username)
{
	
	pam_handle_t *pamh = NULL;
	int ret = 0;
	int	result = 1;
	char *oldtoken;	
	char* loginname=NULL;
	
	loginname = getenv("USER");
	
	if(!strcmp(loginname,"admin")&&strcmp(username,"admin"))	
	{
		pam_start("adminchkpw", username, &pass_conv, &pamh);
	}
	else
	{
		pam_start("passwd", username, &pass_conv, &pamh);
	}

	ret = pam_chauthtok(pamh, PAM_CHANGE_EXPIRED_AUTHTOK); 
	if (ret == PAM_SUCCESS)
	{	
		result = 0;
	}	
	if (pam_end (pamh, ret) != PAM_SUCCESS) 
	{
		 pamh = NULL;
		 return 1;
	}

	pamh = NULL;
	return result;
}


#define OLD_PASSWORDS_FILE	"/etc/security/opasswd"
#define OLD_PASSWORDS_FILE_BK	"/etc/security/opasswd.bak"

static int i64c(int i)
{
	if (i <= 0)
		return ('.');
	if (i == 1)
		return ('/');
	if (i >= 2 && i < 12)
		return ('0' - 2 + i);
	if (i >= 12 && i < 38)
		return ('A' - 12 + i);
	if (i >= 38 && i < 63)
		return ('a' - 38 + i);
	return ('z');
}

static char *crypt_make_salt(void)
{
	time_t now;
	static unsigned long x;
	static char result[3];

	time(&now);
	x += now + getpid() + clock();
	result[0] = i64c(((x >> 18) ^ (x >> 6)) & 077);
	result[1] = i64c(((x >> 12) ^ x) & 077);
	result[2] = '\0';
	return result;
}

extern int get_pwd_unrepeat_setting();

int write_old_password(const char *forwho, const char *newpass)
{
	static char buf[16384] = {0},t_buf1[16384] = {0};
	char *s_luser, *s_uid, *s_npas, *s_pas;
	const char *msg = NULL;
	FILE *opwfile,*opwfilebk;
	int n_pas = 0,get = 0;
	char salt[12];

	set_file_attr(OLD_PASSWORDS_FILE_BK);

	opwfile = fopen(OLD_PASSWORDS_FILE, "r");
	
	if((NULL == opwfile))
	{
		return -1;
	}
	opwfilebk = fopen(OLD_PASSWORDS_FILE_BK, "w");

	if(NULL == opwfilebk) 
	{
		if(opwfile)
			fclose(opwfile);
		return -1;

	}

	{
		char t_buf2[16384]= {0};
		int length = 0;
		char *p;
		int i = 0;
		memset(salt, 0, sizeof(salt));

		get_pwd_unrepeat_setting();
	
		strcpy(salt, "$1$");
		strcat(salt, crypt_make_salt());
		strcat(salt, crypt_make_salt());
		strcat(salt, crypt_make_salt());
		strcat(salt, crypt_make_salt());
		while (fgets(buf, 16380, opwfile)) {
			p = buf;
			p = strchr(buf,':');
			length = strlen(forwho) > (p - buf) ? strlen(forwho):(p-buf);
			if (!strncmp(buf, forwho, length)) {
				get = 1;
				buf[strlen(buf)-1] = '\0';
				s_luser = strtok(buf, ":,");
				s_uid   = strtok(NULL, ":,");
				s_npas  = strtok(NULL, ":,");
				s_pas   = strtok(NULL, ":,");
				n_pas   = atoi(s_npas);

				if(n_pas == passwdunreplytimes+1)
					sprintf(t_buf1,"%s:%s:%d:",s_luser,s_uid,n_pas);
				else
					sprintf(t_buf1,"%s:%s:%d:",s_luser,s_uid,n_pas+1);

				while (s_pas != NULL) {
					if (!strcmp(crypt(newpass, s_pas), s_pas)) {						
						fclose(opwfile);
						fclose(opwfilebk);
						return 1;
					}
					else
					{	
						if(n_pas != passwdunreplytimes+1)
						{
							sprintf(t_buf2,"%s,",s_pas);
							strcat(t_buf1,t_buf2);
						}
						else
						{
							if(i != 0)
							{
								sprintf(t_buf2,"%s,",s_pas);
								strcat(t_buf1,t_buf2);
							}
						}
					}
					s_pas = strtok(NULL, ":,");
					i++;
				}	
				sprintf(t_buf2,"%s\n",crypt(newpass,salt));
				strcat(t_buf1,t_buf2);	
				fprintf(opwfilebk,t_buf1);
				continue;
			}
			else
				fprintf(opwfilebk,buf);
		}
		if(get != 1)
		{	
			int uid;
			uid = getuid();

			
			sprintf(t_buf1,"%s:%d:1:%s\n",forwho,uid,crypt(newpass,salt));
			fprintf(opwfilebk,t_buf1);
		}
	
		fclose(opwfile);
		fclose(opwfilebk);

		
		unlink (OLD_PASSWORDS_FILE);
		rename (OLD_PASSWORDS_FILE_BK,OLD_PASSWORDS_FILE);
	}
	return 0;
}

int delete_old_password(const char *forwho)
{
	static char buf[16384] = {0};
	FILE *opwfile,*opwfilebk;

	set_file_attr(OLD_PASSWORDS_FILE_BK);

	opwfile = fopen(OLD_PASSWORDS_FILE, "r");
	
	if((NULL == opwfile))
	{
		return -1;
	}
	opwfilebk = fopen(OLD_PASSWORDS_FILE_BK, "w");

	if(NULL == opwfilebk) 
	{
		if(opwfile)
			fclose(opwfile);
		return -1;

	}

	{
		while (fgets(buf, 16380, opwfile)) {
			if (!strncmp(buf, forwho, strlen(forwho))) {
				continue;
			}
			else
				fprintf(opwfilebk,buf);
		}

	
		fclose(opwfile);
		fclose(opwfilebk);

		
		unlink (OLD_PASSWORDS_FILE);
		rename (OLD_PASSWORDS_FILE_BK,OLD_PASSWORDS_FILE);
	}
	return 0;
}


#endif

/*gujd : 2013-03-06, pm  2:46. Add code for user authentication file sync to  other boards.*/
extern char BSD_DBUS_BUSNAME[PATH_LEN];
extern char BSD_DBUS_OBJPATH[PATH_LEN];
extern char BSD_DBUS_INTERFACE[PATH_LEN];
extern char BSD_COPY_FILES_BETEWEEN_BORADS[PATH_LEN];
extern int distributFag ;


int 
check_file_exist(const char *filename)
{
	int ret = -1;	
	ret = access(filename, 0);/*to check the file is exist or not ?*/
	if(ret < 0)
		syslog(LOG_NOTICE,"access file (%s) failed.\n",filename);
	return ret;
}

int
sync_dir_from_master_to_other_by_bsd(const char *src_path, const char *des_path)
{
	int ret = 0;
	int i;
	unsigned int src_slotid = 0;
	unsigned int des_slotid = 0;
	int fd;		
	int op = BSD_TYPE_NORMAL;
	int count = 0;
	int ID[MAX_SLOT_NUM] = {0};
		
	fd = fopen("/dbm/product/master_slot_id", "r");
	if(!fd)
	{
		syslog(LOG_NOTICE,"fopen file /dbm/product/master_slot_id failed.\n");
		return CMD_WARNING;
	}
	fscanf(fd, "%d", &src_slotid);
	fclose(fd);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,src_slotid,distributFag);
	
	count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,op);
	if(count != 0)
	{
		for(i = 0; i < count; i++)
		{
    		des_slotid = ID[i];
		    
			ret = dcli_bsd_copy_file_to_board_v2(dcli_dbus_connection,des_slotid,src_path,des_path,1,op);
			if(ret < 0)
			{
				syslog(LOG_NOTICE,"copy dir(%s) to slot[%d](%s) failed.\n",src_path,des_slotid,des_path);
			}
		}
	}
	
	return CMD_SUCCESS;
	
}

int
sync_file_from_master_to_other_by_bsd(const char *src_path, const char *des_path)
{
	int i;
	DBusMessage *query = NULL;		
	DBusMessage *reply = NULL;	
	DBusError err = {0};
	unsigned int result = 0;
	unsigned int src_slotid = 0;
	unsigned int des_slotid = 0;
	
	char *src_path_temp = src_path;
	char *des_path_temp = des_path;
	int fd;		
	int tar_switch = 0;
	/*int op = BSD_TYPE_CORE;*/
	int op = BSD_TYPE_NORMAL;
	int count = 0;
	int ID[MAX_SLOT_NUM] = {0};
		
	fd = fopen("/dbm/product/master_slot_id", "r");
	if(!fd)
	{
		syslog(LOG_NOTICE,"fopen file /dbm/product/master_slot_id failed.\n");
		return CMD_WARNING;
	}
	fscanf(fd, "%d", &src_slotid);
	fclose(fd);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,src_slotid,distributFag);
	
	count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,op);
	if(count != 0)
	{
		for(i = 0; i < count; i++)
		{
    		des_slotid = ID[i];
		    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
								BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
			dbus_error_init(&err);
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&des_slotid,
									 DBUS_TYPE_STRING,&src_path_temp,
									 DBUS_TYPE_STRING,&des_path_temp,
									 DBUS_TYPE_UINT32,&tar_switch,
									 DBUS_TYPE_UINT32,&op,
									 DBUS_TYPE_INVALID);
		
			reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			dbus_message_unref(query);
			if (NULL == reply) {
				fprintf(stderr,"<error> failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					fprintf(stderr,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
			dbus_message_unref(reply);
			
		}
	}
	
	return CMD_SUCCESS;
	
}

int
write_word_to_file(const char *word, const char *file_path)
{
	int ret = 0;
	FILE *fp = NULL;
	int length = strlen(word);
	char str[HOSTNAME_MAX_LEN+1] = {0};	
		
	fp = fopen(file_path,"w+");
	if(!fp)
	{
		syslog(LOG_NOTICE,"fopen file (%s) failed.\n",file_path);
		return -1;
	}
	/*length + 1 is '\0' */
	memcpy(str,word,length);
	str[length]= '\n';
	fwrite(str, length+1, 1,fp);
	fclose(fp);
	
	return 0;
}

int
read_int_vlaue_from_file(const char *file_path)
{
	FILE *fp = NULL;
	int value = -1;
		
	 if(file_path== NULL)
	 {
		return -1;
	 }
	 fp = fopen(file_path,"r");
	 if(fp == NULL)
	 {
		 syslog(LOG_NOTICE,"fopen file (%s) failed.\n",file_path);
		 return -1;
	 }
	 fscanf(fp ,"%d",&value);
	 fclose(fp);
	 return value;
	
}


DEFUN (user_authentication_file_sync,
	   user_authentication_file_sync_cmd,
	   "user authentication file sync",	   
	   "User command\n"
	   "User authentication\n"
	   "Authentication file\n"
	   "Synchronization to other board\n"
	   )
{
	int ret = -1;
	
	/*check the file : /etc/shadow; /etc/passwd .*/
	ret = check_file_exist(USER_NAME_AUTHEN_FILE);
	if(ret < 0)
	{
		vty_out(vty,"The file %s is not exist\n",USER_NAME_AUTHEN_FILE);
		return CMD_WARNING;
	  }
	ret = check_file_exist(USER_PASSWD_AUTHEN_FILE);
	if(ret < 0)
	{
		vty_out(vty,"The file %s is not exist\n",USER_PASSWD_AUTHEN_FILE);
		return CMD_WARNING;
	  }
		
	/*sync these file to other board by bsd api.*/
	ret = sync_file_from_master_to_other_by_bsd(USER_PASSWD_AUTHEN_FILE,USER_PASSWD_AUTHEN_FILE);
	if(ret < 0)
	{
		vty_out(vty,"Sync file %s failed .\n",USER_PASSWD_AUTHEN_FILE);
		return CMD_WARNING;
	  }
	
	ret = sync_file_from_master_to_other_by_bsd(USER_NAME_AUTHEN_FILE,USER_NAME_AUTHEN_FILE);
	if(ret < 0)
	{
		vty_out(vty,"Sync file %s failed .\n",USER_NAME_AUTHEN_FILE);
		return CMD_WARNING;
	}
	/**wangchao add**/
	ret = sync_file_from_master_to_other_by_bsd(USER_GROUP_AUTHEN_FILE,USER_GROUP_AUTHEN_FILE);
	if(ret < 0)
	{
		vty_out(vty,"Sync file %s failed .\n",USER_GROUP_AUTHEN_FILE);
		return CMD_WARNING;
	}

	
	
	/*sync home dir*/
	ret = sync_dir_from_master_to_other_by_bsd(HOME_DIR,HOME_DIR);
	if(ret < 0)
	{
		vty_out(vty,"Sync dir %s failed .\n",USER_NAME_AUTHEN_FILE);
		return CMD_WARNING;
	  }
	
	/*write a flag to file /var/run/user_auth_sync, for show running.*/
	ret = write_word_to_file("1",USER_AUTHEN_FILE_SYNC_FLAG);
	if(ret < 0)
	{
		vty_out(vty,"Write file %s failed .\n",USER_AUTHEN_FILE_SYNC_FLAG);
		return CMD_WARNING;
	  }
	
	return CMD_SUCCESS;
}


DEFUN (no_user_authentication_file_sync,
	   no_user_authentication_file_sync_cmd,
	   "no user authentication file sync",
	   "Set its default\n"
	   "User command\n"
	   "User authentication\n"
	   "Authentication file\n"
	   "Synchronization to other board\n"
	   )
{
	int ret = -1;
	
	/*write a flag to file /var/run/user_auth_sync, for show running.*/
	ret = write_word_to_file("0",USER_AUTHEN_FILE_SYNC_FLAG);
	if(ret < 0)
	{
		vty_out(vty,"Write file %s failed .\n",USER_AUTHEN_FILE_SYNC_FLAG);
		return CMD_WARNING;
	  }
	
	return CMD_SUCCESS;
}

void
dcli_user_authentication_file_sync_show_running(void)
{
	int value = 0;
	char buf[128]={0};
	int ret = -1;

	
	ret = check_file_exist(USER_AUTHEN_FILE_SYNC_FLAG);
	if(ret < 0)
	{
		/*if not exist, creat is and default value is 0 (disable state).*/
		ret = write_word_to_file("0",USER_AUTHEN_FILE_SYNC_FLAG);
		if(ret < 0)
		{
			fprintf(stderr,"Write file %s failed .\n",USER_AUTHEN_FILE_SYNC_FLAG);
			return ;
		  }
	  }

	/*1 is enable , 0 is disable*/
	value = read_int_vlaue_from_file(USER_AUTHEN_FILE_SYNC_FLAG);
	if(value == 1)
	{
		
		sprintf(buf,"user authentication file sync");
		vtysh_add_show_string(buf);
		}
	return ;
	
	
}
/************ Code for user authentication file sync to  other boards.(END)*************/

DEFUN (user_add,
	   user_add_cmd,
	   "user add USERNAME PASSWD (view|enable)",
	   "User command\n"
	   "Add user\n"
	   "The new user name;user name length should be >=4 & <=32\n"
	   "The user password;user password length should be >=4 & <=32,and in passwd strong state,it should be >=6 & <=32\n"
	   "The user of view\n"
	   "The user of admin\n")
{
	int ret;
	char passwd[32];
	char userrole[32];
	int num;

	ret = dcli_user_name_check((char*)argv[0]);
	
	if(is_user_exsit(argv[0]))		
	{
		vty_out(vty,"the user %s is exist\n",argv[0]);
		return CMD_WARNING;
	}
	
	if(ret == 1)
	{
		vty_out(vty,"the user name should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'\n");
		return CMD_WARNING;
	}
	else if(ret == 2)
	{
		vty_out(vty,"the user name length should be >=4 & <=32\n");
		return CMD_WARNING;
	}
	else if(ret == 3)
	{
		vty_out(vty,"the user name first char  should be 'A'-'Z' or 'a'-'z'\n");
		return CMD_WARNING;
	}
#if 0
	if(!strncmp("normal",argv[3],strlen(argv[3])))
#else
	if(!boot_flag)
#endif
	{
		sprintf(passwd,"normal");
		ret = dcli_user_passwd_check(argv[0],argv[1]);
		if(ret == 1)
		{
			 vty_out(vty,"the user password is a palindrome \n");
			return CMD_WARNING;
		}	
		else if(ret == 2)
		{
			 vty_out(vty,"the user password is too simple \n");
			return CMD_WARNING;
		}
		else if(ret == 3)
		{
			 vty_out(vty,"the user password should be not same as username\n");
			 return CMD_WARNING;
		 }		 
		 else if(ret == 4)
		 {
			 vty_out(vty,"the user password too short or too long\n");
			 return CMD_WARNING;
		 }		 
		 else if(ret == 5)
		 {
			 vty_out(vty,"the user password length should be >= %d && <=32\n",passwdlen);
			 return CMD_WARNING;
		 }
	 }
	else
	{
		sprintf(passwd,"security");
	}

	 ret = write_old_password(argv[0], argv[1]);

	/**add by gjd: check the register the amount of users**/
	num = dcli_user_number_check();
	if(num >= 32)
	{
		vty_out(vty,"Can't add new user %s,execeed the max user number of system . The system max user number is 32 \n",argv[0]);
		return CMD_WARNING;
	}
		
	if(!strncmp("enable",argv[2],strlen(argv[2])))
		sprintf(userrole,"enable");
	else
		sprintf(userrole,"view");
		ret = dcli_user_add_sh(argv[0],argv[1],userrole,passwd);
	if(0 != ret)
	{
		vty_out(vty,"Add user %s error\n",argv[0]);
		return CMD_WARNING;
	}
#if 0	 
	add_pwd_reply_times(argv[0],argv[1]);
#endif

	Htdigest(argv[0],argv[1],1);
	return CMD_SUCCESS;
}

DEFUN (user_del,
	   user_del_cmd,
	   "user delete USERNAME",
	   "User command\n"
	   "Delete user\n"
	   "The deleted user name\n")
{
	int ret;
	char command[64];

	
	if(!is_user_exsit(argv[0]))		
	{
		vty_out(vty,"the user %s is not exist\n",argv[0]);
		return CMD_WARNING;
	}
	ret = is_user_self(argv[0]);
	if(ret == 1)
	{
		vty_out(vty,"Can't del user %s self\n",argv[0]);
		return CMD_WARNING;
	}
	else if(ret == -1)
	{
		vty_out(vty,"the user %s is not exist\n",argv[0]);
		return CMD_WARNING;
	}
	
	if(get_user_role((char *)argv[0]) == 1)
	{
		vty_out(vty,"Can't del user %s which belong admin group\n",argv[0]);
		return CMD_WARNING;
	}
	delete_old_password(argv[0]);	
	if(ret != 0)
	{
		vty_out(vty,"error to delete passwd from opasswd file\n");
		return CMD_WARNING;
	}
	sprintf(command,"userdel.sh %s",argv[0]);
	ret = execute_dcli_shell(command);
	if(ret != 0){
		vty_out(vty,"Delete user %s error\n",argv[0]);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN (user_role_change,
	   user_role_change_cmd,
	   "user role USERNAME (view|enable)",
	   "User command\n"
	   "User role\n"
	   "User name\n"
	   "View user\n"
	   "Admin user\n")
{
	int ret;
	char command[64];
	char userrole[8];

	/*Prevent array subscript beyond the bounds --added by zhaocg*/
	int len = strlen(argv[0]);
	if(len<4||len>32)
	{
		vty_out(vty,"the user name length should be >=4 & <=32\n");
		return CMD_WARNING;
	}
	if(!strcmp(argv[0],"admin"))
	{
		vty_out(vty,"can't change user 'ADMIN' role \n");
		return CMD_WARNING;
	}
	if(!strncmp("enable",argv[1],strlen(argv[1])))
		sprintf(userrole,"enable");
	else
		sprintf(userrole,"view");
	sprintf(command,"userrole.sh %s %s",argv[0],userrole);
    ret = execute_dcli_shell(command);
	if(ret != 0){
		vty_out(vty,"Change user %s role error\n",argv[0]);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}
DEFUN (show_user_all,
	   show_user_all_cmd,
	   "show user",
	   "Show system infomation\n"
	   "System usr\n")	   
{
	int ret,i;
	struct group *grentry = NULL;
	char *ptr;
	vty_out(vty,"The admin user:\n");
	grentry = getgrnam(ADMINGROUP);
	if (grentry)
	{
		for(i=0;ptr=grentry->gr_mem[i];i++)
			vty_out(vty,"%s\n",ptr);
	
	}
	
	vty_out(vty,"The view user:\n");
	grentry = getgrnam(VIEWGROUP);
	if (grentry)
	{
		for(i=0;ptr=grentry->gr_mem[i];i++)
			vty_out(vty,"%s\n",ptr);
	
	}

	return CMD_SUCCESS;
}

DEFUN (passwd_change,
	   passwd_change_cmd,
	   "passwd USERNAME PASSWORD",
	   "User password\n"
	   "User name\n"
	   "The new password;")
{
	if(boot_flag)
	{
	int ret;
	char command[64];
	char passwd[10];
			

		if(strcmp("admin",argv[0]))				
		{
			vty_out(vty,"You can not modify this passwd\n");
			return CMD_WARNING;
		}

	if(!is_user_exsit(argv[0]))		
	{
		vty_out(vty,"the user %s is not exist\n",argv[0]);
		return CMD_WARNING;
	}
#if 0
		if(!strncmp("normal",argv[2],strlen(argv[2])))

		if(!boot_flag)

	{
		sprintf(passwd,"normal");
		ret = dcli_user_passwd_check(argv[0],argv[1]);
		if(ret == 1)
			{
				vty_out(vty,"the user password is a palindrome \n");
				return CMD_WARNING;
			}	
			else if(ret == 2)
			{
				vty_out(vty,"the user password is too simple \n");
				return CMD_WARNING;
			}
 			else if(ret == 3)
			{
				vty_out(vty,"the user password should be not same as username\n");
				return CMD_WARNING;
			}	
			else if(ret == 4)
			{
				vty_out(vty,"the user password too short or too long\n");
				return CMD_WARNING;
			}			
	}
	else
	{
		sprintf(passwd,"security");
		}
#endif		
		sprintf(passwd,"security");
/*
		ret = write_old_password(argv[0], argv[1]);
		if(ret == 1)
		{
			vty_out(vty,"passwd has been already used\n");
				return CMD_WARNING; 	   
		}
*/
	if(argc == 2)
	{
		sprintf(command,"chpass.sh %s \'%s\' %s",argv[0],argv[1],passwd);
	}
	else
	{
		vty_out(vty,"error parameter number\n");
		return CMD_WARNING;

	}	
	ret = execute_dcli_shell(command);
	
	if(ret != 0)
		{
		vty_out(vty,"Change user %s password error\n",argv[0]);
		return CMD_WARNING;
	}
#if 0		
	add_pwd_reply_times(argv[0],argv[1]);
#endif 
	return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"In this state,You should use passwd [USERNAME]\n");
		return CMD_WARNING;
	}
}
DEFUN (passwd_modify,
	   passwd_modify_cmd,
	   "passwd [USERNAME]",
	   "User password\n"
	   "User name\n")
{
	if(!boot_flag)
	{
		int ret;
		char command[64];
		char passwd[10];
		char* loginname=NULL;

		loginname = getenv("USER");
		get_global_variable_status();
		if(argc==1)
		{
			if(!strcmp(loginname,"admin") || !strcmp(loginname,argv[0]))
			{
				if(!is_user_exsit(argv[0])) 	
				{
					vty_out(vty,"the user %s is not exist\n",argv[0]);
					return CMD_WARNING;
				}
				if(!boot_flag)
				{
					sprintf(passwd,"normal");
					if(strongpasswd || !strcmp(argv[0],loginname))
					{
					ret = dcli_user_change_passwd_check(argv[0]);
					if(ret == 1)
					{
						//vty_out(vty,"the user password should be strong\n");
						return CMD_WARNING;
						}	
					}else
					{
						char line1[INPUTSIZE];
						char line2[INPUTSIZE];
						int input;
						int i = 0;
						
						fprintf(stdout, "New system password:");
						while((input = getchar()) != '\n' && i <= 32)
						{
							line1[i++] = (char)input;			
						}
						line1[i] = '\0';

						if(i < 4)
						{
							vty_out(vty,"the user password too short\n");
							return CMD_WARNING;
						}

						i = 0;
						
						fprintf(stdout, "\nRetype new system password:");
						while((input = getchar()) != '\n' && i <= 32)
						{
							line2[i++] = (char)input;			
						}
						line2[i] = '\0';						

						
						if(!strcmp(line1,line2))
						{
							strcpy(input_passwd,line1);
						}
						else
						{
							vty_out(vty,"\nSorry, passwords do not match\n");
							return CMD_WARNING;
						}
						fprintf(stdout, "\n");
					}	
					if(strcmp(argv[0],"admin"))
					{
						ret = dcli_user_passwd_check(argv[0],input_passwd);
						if(ret == 1)
						{
							vty_out(vty,"the user password is a palindrome \n");
							return CMD_WARNING;
						}	
						else if(ret == 2)
						{
							vty_out(vty,"the user password is too simple \n");
							return CMD_WARNING;
						}
	 					else if(ret == 3)
						{
							vty_out(vty,"the user password should be not same as username\n");
							return CMD_WARNING;
						}
						else if(ret == 4)
						{
							vty_out(vty,"the user password too short or too long\n");
							return CMD_WARNING;
						}	
						else if(ret == 5)
						{
							vty_out(vty,"the user password length should be >= %d && <=32\n",passwdlen);
							return CMD_WARNING;
						}
					}				
				}
				else
				{
					sprintf(passwd,"security");
				}
				
				ret = write_old_password(argv[0], input_passwd);
				if(ret ==1)
				{
					vty_out(vty,"Password has been already used. Choose another.\n");
					return CMD_WARNING;
				}
				
				
				sprintf(command,"chpass.sh %s \'%s\' %s",argv[0],input_passwd,passwd);
				
				
				ret = execute_dcli_shell(command);
				
				if(ret != 0)
				{
					vty_out(vty,"Change user %s password error\n",argv[0]);
					return CMD_WARNING;
				}	
#if 0				
				add_pwd_reply_times(argv[0],input_passwd);
#endif

				Htdigest(argv[0],input_passwd,1);
				return CMD_SUCCESS;

			}
			else
			{
				vty_out(vty,"You aren't the user admin,Cann't changed other user's password\n");
				
				return CMD_SUCCESS;

			}

		}
		else{
			if(!is_user_exsit(loginname))		
			{
				vty_out(vty,"the user %s is not exist\n",loginname);
				return CMD_WARNING;
			}
#if 0
				if(!strncmp("normal",argv[2],strlen(argv[2])))
#else
				if(!boot_flag)
#endif
			{
				sprintf(passwd,"normal");				
				
				ret = dcli_user_change_passwd_check(loginname);
				if(ret == 1)
				{
					//vty_out(vty,"the user password should be strong\n");
					return CMD_WARNING;
				}	
			}
			else
			{
				sprintf(passwd,"security");
			}
/*			if(strongpasswd)
			{
			ret = write_old_password(loginname, input_passwd);			
			if(ret ==1)
			{
				vty_out(vty,"password has been already used\n");
				return CMD_WARNING;
				}
			}*/
			

#if 0
			if(argc == 0)
			{
				sprintf(command,"chpass.sh %s \'%s\' %s",loginname,input_passwd,passwd);
			}
			else
			{
				vty_out(vty,"error parameter number\n");
				return CMD_WARNING;

			}	


			ret = execute_dcli_shell(command);
			
			if(ret != 0)
			{
				vty_out(vty,"Change user %s password error\n",loginname);
				return CMD_WARNING;
			}		
		
			add_pwd_reply_times(loginname,input_passwd);
#endif			
			return CMD_SUCCESS;
		}
	}
	else
	{		
		vty_out(vty,"In boot state,You should use passwd USERNAME PASSWORD\n");
		return CMD_WARNING;
	}
}
#if 1

/**zhaocg add for suporting board slot**/
DEFUN (user_add_slot,
	   user_add_slot_cmd,
	   "user add slot <1-15> USERNAME PASSWD (view|enable)",
	   "User command\n"
	   "Add user\n"
	   "Board slot\n"
	   "Slot id\n"
	   "The new user name;user name length should be >=4 & <=32\n"
	   "The user password;user password length should be >=4 & <=32,and in passwd strong state,it should be >=6 & <=32\n"
	   "The user of view\n"
	   "The user of admin\n")
{
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err; 
	
	char cmd[128] = {0};
	char *cmdstr = cmd;
	char *username = argv[1];
	char userrole[32] = {0};
	int ret;
	int opt;
	int slot_id = 0;
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	ret = dcli_user_name_check((char*)argv[1]);
	if(ret == 1)
	{
		vty_out(vty,"the user name should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'\n");
		return CMD_WARNING;
	}
	else if(ret == 2)
	{
		vty_out(vty,"the user name length should be >=4 & <=32\n");
		return CMD_WARNING;
	}
	else if(ret == 3)
	{
		vty_out(vty,"the user name first char  should be 'A'-'Z' or 'a'-'z'\n");
		return CMD_WARNING;
	}
	
	
	if(!boot_flag)
	{
		ret = dcli_user_passwd_check(argv[1],argv[2]);
		if(ret == 1)
		{
			 vty_out(vty,"the user password is a palindrome \n");
			return CMD_WARNING;
		}	
		else if(ret == 2)
		{
			 vty_out(vty,"the user password is too simple \n");
			return CMD_WARNING;
		}
		else if(ret == 3)
		{
			 vty_out(vty,"the user password should be not same as username\n");
			 return CMD_WARNING;
		 }		 
		 else if(ret == 4)
		 {
			 vty_out(vty,"the user password too short or too long\n");
			 return CMD_WARNING;
		 }		 
		 else if(ret == 5)
		 {
			 vty_out(vty,"the user password length should be >= %d && <=32\n",passwdlen);
			 return CMD_WARNING;
		 }
	 }
	
	if(!strncmp("enable",argv[3],strlen(argv[3])))
	{
		sprintf(userrole,"enable");
	}	
	else
	{
		sprintf(userrole,"view");
	}

	sprintf(cmd,"user add %s %s %s",argv[1],argv[2],userrole);
	
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_USER_IS_EXSIT_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,DBUS_TYPE_STRING, &username,DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		dbus_message_unref(reply);
		if (ret)  
	    {  
			vty_out(vty,"the user %s have existed\n",argv[1]);
			return CMD_WARNING;
	    }  
		 	
	
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, 
											SEM_DBUS_OBJPATH,
											SEM_DBUS_INTERFACE,
											SEM_DBUS_USER_ADD_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&boot_flag,
								DBUS_TYPE_STRING,&cmdstr,
								DBUS_TYPE_INVALID);
				
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, -1, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);	
		if (-1 == ret)  
	    {  
	 	   vty_out(vty,"system error!");  
	    }  
		else  
	    {  
	  
	 	   if (WIFEXITED(ret))  
	 	   {  
			   switch (WEXITSTATUS(ret))
			   { 	
					case 0: 		 
						   break;	
					default:			
						vty_out(vty,"Add user %s error\n",argv[1]);			
						break;		
				}	
		   }  
	 	   else  
	 	   {  
	 		   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret));  
		   }  
	   }  
	   
		dbus_message_unref(reply);
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
	}
	
	return CMD_SUCCESS;
}

DEFUN (user_add_slot_all,
	   user_add_slot_all_cmd,
	   "user add slot all USERNAME PASSWD (view|enable)",
	   "User command\n"
	   "Add user\n"
	   "Board slot\n"
	   "ALL board slot\n"
	   "The new user name;user name length should be >=4 & <=32\n"
	   "The user password;user password length should be >=4 & <=32,and in passwd strong state,it should be >=6 & <=32\n"
	   "The user of view\n"
	   "The user of admin\n")
{
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err; 
	
	char cmd[128] = {0};
	char *cmdstr = cmd;
	char *username = argv[0];
	char userrole[32] = {0};
	int ret;
	int slot_id = 0;
	
	ret = dcli_user_name_check((char*)argv[0]);
	if(ret == 1)
	{
		vty_out(vty,"the user name should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'\n");
		return CMD_WARNING;
	}
	else if(ret == 2)
	{
		vty_out(vty,"the user name length should be >=4 & <=32\n");
		return CMD_WARNING;
	}
	else if(ret == 3)
	{
		vty_out(vty,"the user name first char  should be 'A'-'Z' or 'a'-'z'\n");
		return CMD_WARNING;
	}
	
	if(!boot_flag)
	{
		ret = dcli_user_passwd_check(argv[0],argv[1]);
		if(ret == 1)
		{
			 vty_out(vty,"the user password is a palindrome \n");
			return CMD_WARNING;
		}	
		else if(ret == 2)
		{
			 vty_out(vty,"the user password is too simple \n");
			return CMD_WARNING;
		}
		else if(ret == 3)
		{
			 vty_out(vty,"the user password should be not same as username\n");
			 return CMD_WARNING;
		 }		 
		 else if(ret == 4)
		 {
			 vty_out(vty,"the user password too short or too long\n");
			 return CMD_WARNING;
		 }		 
		 else if(ret == 5)
		 {
			 vty_out(vty,"the user password length should be >= %d && <=32\n",passwdlen);
			 return CMD_WARNING;
		 }
	 }
	
	if(!strncmp("enable",argv[2],strlen(argv[2])))
	{
		sprintf(userrole,"enable");
	}	
	else
	{
		sprintf(userrole,"view");
	}

	sprintf(cmd,"user add %s %s %s",argv[0],argv[1],userrole);
	for(slot_id=0;slot_id<16;slot_id++)
	{	
		if(slot_id != HostSlotId)
		{
			if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
			{
				
				query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
							 SEM_DBUS_INTERFACE,SEM_DBUS_USER_IS_EXSIT_SLOT);

				dbus_error_init(&err);

				dbus_message_append_args(query,DBUS_TYPE_STRING, &username,DBUS_TYPE_INVALID);
				reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
				
				dbus_message_unref(query);

				if (NULL == reply)
				{
					vty_out(vty,"Slot %d,<error> failed get reply.\n",slot_id);
					
					if (dbus_error_is_set(&err)) 
					{
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					
					return CMD_WARNING;
				}

				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,&ret);
				dbus_message_unref(reply);
				if (ret)  
			    {  
					vty_out(vty,"Slot %d,the user %s have existed\n",slot_id,argv[0]);
					return CMD_WARNING;
			    }  
				 	
			
				query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, 
													SEM_DBUS_OBJPATH,
													SEM_DBUS_INTERFACE,
													SEM_DBUS_USER_ADD_SLOT);

				dbus_error_init(&err);

				dbus_message_append_args(query,
										DBUS_TYPE_UINT32,&boot_flag,
										DBUS_TYPE_STRING,&cmdstr,
										DBUS_TYPE_INVALID);
						
				reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, -1, &err);
				
				dbus_message_unref(query);

				if (NULL == reply)
				{
					vty_out(vty,"Slot %d,<error> failed get reply.\n",slot_id);
					
					if (dbus_error_is_set(&err)) 
					{
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					
					return CMD_WARNING;
				}

				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,&ret);	
				if (-1 == ret)  
			    {  
			 	   vty_out(vty,"Slot %d,system error!",slot_id);  
			    }  
				else  
			    {  
			  
			 	   if (WIFEXITED(ret))  
			 	   {  
					   switch (WEXITSTATUS(ret))
					   { 	
							case 0: 		 
								   break;	
							default:			
								vty_out(vty,"Slot %d,Add user %s error\n",slot_id,argv[0]);			
								break;		
						}	
				   }  
			 	   else  
			 	   {  
			 		   vty_out(vty,"Slot %d,exit status = [%d]\n",slot_id, WEXITSTATUS(ret));  
				   }  
			   }  
			   
				dbus_message_unref(reply);
			}
		}
	}
	return CMD_SUCCESS;
}

DEFUN (user_del_slot,
	   user_del_slot_cmd,
	   "user delete slot <1-15> USERNAME",
	   "User command\n"
	   "Delete user\n"
	   "board slot\n"
	   "slot id\n"
	   "The deleted user name\n")
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	char cmd[128] = {0};
	char *cmdstr = cmd;
	char* username = NULL;
	int ret;
	int opt;
	int slot_id = 0;
	username = argv[1];
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}

	sprintf(cmd,"user delete %s",argv[1]);
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_USER_IS_EXSIT_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,DBUS_TYPE_STRING, &username,DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&opt);
		dbus_message_unref(reply);
		if (!ret)  
	    {  
			vty_out(vty,"the user %s is not exist\n",argv[1]);
			return CMD_WARNING;
	    }  
		if(opt)
		{
			vty_out(vty,"Can't del user %s which belong admin group\n",argv[1]);
			return CMD_WARNING;
		}
	
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_USER_DEL_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,DBUS_TYPE_STRING, &cmdstr,DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		if (-1 == ret)  
	    {  
	 	   vty_out(vty,"system error!");  
	    }  
		else  
	    {  
			
	  
	 	   if (WIFEXITED(ret))  
	 	   {  
			   switch (WEXITSTATUS(ret))
			   { 	
					case 0: 		 						   
						   break;
					default:		
						vty_out(vty,"Delete user %s error\n",argv[1]);			
						break;		
					
				}	
		   }  
	 	   else  
	 	   {  
	 		   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret));  
		   }  
	   }  
		dbus_message_unref(reply);
		
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
	}
	
	return CMD_SUCCESS;
}
DEFUN (user_del_slot_all,
	   user_del_slot_all_cmd,
	   "user delete slot all USERNAME",
	   "User command\n"
	   "Delete user\n"
	   "board slot\n"
	   "slot id\n"
	   "The deleted user name\n")
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	char cmd[128] = {0};
	char *cmdstr = cmd;
	char *username = NULL;
	int ret;
	int opt;
	int slot_id = 0;
	username = argv[0];
	sprintf(cmd,"user delete %s",argv[0]);
	for(slot_id=0;slot_id<16;slot_id++)
	{	
		if(slot_id != HostSlotId)
		{
			if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
			{
				
				query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
							 SEM_DBUS_INTERFACE,SEM_DBUS_USER_IS_EXSIT_SLOT);

				dbus_error_init(&err);

				dbus_message_append_args(query,DBUS_TYPE_STRING, &username,DBUS_TYPE_INVALID);
				reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
				
				dbus_message_unref(query);

				if (NULL == reply)
				{
					vty_out(vty,"Slot %d,<error> failed get reply.\n",slot_id);
					
					if (dbus_error_is_set(&err)) 
					{
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					
					return CMD_WARNING;
				}

				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,&ret);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&opt);
				dbus_message_unref(reply);
				if (!ret)  
			    {  
					vty_out(vty,"Slot %d,the user %s is not exist\n",slot_id,argv[0]);
					return CMD_WARNING;
			    }  
				if(opt)
				{
					vty_out(vty,"Slot %d,Can't del user %s which belong admin group\n",slot_id,argv[0]);
					return CMD_WARNING;
				}	
			
				query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
							 SEM_DBUS_INTERFACE,SEM_DBUS_USER_DEL_SLOT);

				dbus_error_init(&err);

				dbus_message_append_args(query,DBUS_TYPE_STRING, &cmdstr,DBUS_TYPE_INVALID);
				reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
				
				dbus_message_unref(query);

				if (NULL == reply)
				{
					vty_out(vty,"Slot %d,<error> failed get reply.\n",slot_id);
					
					if (dbus_error_is_set(&err)) 
					{
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					
					return CMD_WARNING;
				}

				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,&ret);
				
				if (-1 == ret)  
			    {  
			 	   vty_out(vty,"Slot %d,system error!",slot_id);  
			    }  
				else  
			    {  	
			  
			 	   if (WIFEXITED(ret))  
			 	   {  
					   switch (WEXITSTATUS(ret))
					   { 	
							case 0: 		 							   
								   break;
							default:			
								vty_out(vty,"Slot %d,Delete user %s error\n",slot_id,argv[0]);			
								break;		
						}	
				   }  
			 	   else  
			 	   {  
			 		   vty_out(vty,"Slot %d,exit status = [%d]\n",slot_id, WEXITSTATUS(ret));  
				   }  
			   }  
				dbus_message_unref(reply);
				
			}
		}
	}
	return CMD_SUCCESS;
}

DEFUN (user_role_change_slot,
	   user_role_change_slot_cmd,
	   "user role slot <1-15> USERNAME (view|enable)",
	   "User command\n"
	   "User role\n"
	   "board slot\n"
	   "slot id\n"
	   "User name\n"
	   "View user\n"
	   "Admin user\n")
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	char cmd[128] = {0};
	char *cmdstr = cmd;
	char *username = NULL;
	int ret;
	char userrole[32]={0};
	int slot_id = 0;
	username = argv[1];
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}

	/*Prevent array subscript beyond the bounds --added by zhaocg*/
	int len = strlen(argv[1]);
	if(len<4||len>32)
	{
		vty_out(vty,"the user name length should be >=4 & <=32\n");
		return CMD_WARNING;
	}
	if(!strcmp(argv[1],"admin"))
	{
		vty_out(vty,"can't change user 'ADMIN' role \n");
		return CMD_WARNING;
	}
	
	if(!strncmp("enable",argv[2],strlen(argv[2])))
	{
		sprintf(userrole,"enable");
		
	}	
	else
	{
		sprintf(userrole,"view");
		
	}
	
	sprintf(cmd,"user role %s %s",argv[1],userrole);
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_USER_IS_EXSIT_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,DBUS_TYPE_STRING, &username,DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		dbus_message_unref(reply);
		if (!ret)  
	    {  
			vty_out(vty,"the user %s is not exist\n",argv[1]);
			return CMD_WARNING;
	    }  
		 	
	
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_USER_ROLE_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,DBUS_TYPE_STRING, &cmdstr,DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		if (-1 == ret)  
	    {  
	 	   vty_out(vty,"system error!");  
	    }  
		else  
	    {  
	 	   if (WIFEXITED(ret))  
	 	   {  
			   switch (WEXITSTATUS(ret))
			   { 	
					case 0: 		 						   
						   break;			
					default:			
						vty_out(vty,"Change user %s role error\n",argv[1]);			
						break;		
				}	
		   }  
	 	   else  
	 	   {  
	 		   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret));  
		   }  
	   }  
		dbus_message_unref(reply);
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
	}
	
	return CMD_SUCCESS;

}

DEFUN (user_role_change_slot_all,
	   user_role_change_slot_all_cmd,
	   "user role slot all USERNAME (view|enable)",
	   "User command\n"
	   "User role\n"
	   "Board slot\n"
	   "All board slot\n"
	   "User name\n"
	   "View user\n"
	   "Admin user\n")
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	char cmd[128] = {0};
	char *cmdstr = cmd;
	char *username = NULL;
	int ret;
	char userrole[32]={0};
	int slot_id = 0;
	username = argv[0];
	/*Prevent array subscript beyond the bounds --added by zhaocg*/
	int len = strlen(argv[0]);
	if(len<4||len>32)
	{
		vty_out(vty,"the user name length should be >=4 & <=32\n");
		return CMD_WARNING;
	}
	if(!strcmp(argv[0],"admin"))
	{
		vty_out(vty,"can't change user 'ADMIN' role \n");
		return CMD_WARNING;
	}
	
	if(!strncmp("enable",argv[1],strlen(argv[1])))
	{
		sprintf(userrole,"enable");
		
	}	
	else
	{
		sprintf(userrole,"view");
		
	}
	
	sprintf(cmd,"user role %s %s",argv[0],userrole);
	for(slot_id=0;slot_id<16;slot_id++)
	{	
		if(slot_id != HostSlotId)
		{
			if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
			{
				
				query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
							 SEM_DBUS_INTERFACE,SEM_DBUS_USER_IS_EXSIT_SLOT);

				dbus_error_init(&err);

				dbus_message_append_args(query,DBUS_TYPE_STRING, &username,DBUS_TYPE_INVALID);
				reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
				
				dbus_message_unref(query);

				if (NULL == reply)
				{
					vty_out(vty,"Slot %d,<error> failed get reply.\n",slot_id);
					
					if (dbus_error_is_set(&err)) 
					{
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					
					return CMD_WARNING;
				}

				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,&ret);
				dbus_message_unref(reply);
				if (!ret)  
			    {  
					vty_out(vty,"Slot %d,the user %s is not exist\n",slot_id,argv[0]);
					return CMD_WARNING;
			    }  
				 	
			
				query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
							 SEM_DBUS_INTERFACE,SEM_DBUS_USER_ROLE_SLOT);

				dbus_error_init(&err);

				dbus_message_append_args(query,DBUS_TYPE_STRING, &cmdstr,DBUS_TYPE_INVALID);
				reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
				
				dbus_message_unref(query);

				if (NULL == reply)
				{
					vty_out(vty,"Slot %d,<error> failed get reply.\n",slot_id);
					
					if (dbus_error_is_set(&err)) 
					{
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					
					return CMD_WARNING;
				}

				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,&ret);
				
				if (-1 == ret)  
			    {  
			 	   vty_out(vty,"Slot %d,system error!");  
			    }  
				else  
			    {  
					//vty_out(vty,"exit status value = [0x%x]\n", ret);	
			  
			 	   if (WIFEXITED(ret))  
			 	   {  
					   switch (WEXITSTATUS(ret))
					   { 	
							case 0: 		 							   
								   break;
							default:			
								vty_out(vty,"Slot %d,Change user %s role error\n",slot_id,argv[0]);			
								break;			
											
						}	
				   }  
			 	   else  
			 	   {  
			 		   vty_out(vty,"Slot %d,exit status = [%d]\n",slot_id, WEXITSTATUS(ret));  
				   }  
			   }  
				dbus_message_unref(reply);
			}
	 	}
	}
	
	return CMD_SUCCESS;

}

DEFUN (show_user_slot,
	   show_user_slot_cmd,
	   "show user slot <1-15>",
	   "Show system infomation\n"
	   "System usr\n"
	   "board slot\n"
	   "slot id\n"
	   )	
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	
	int ret;
	int i;
	int admin_num = 0;
	int view_num = 0;
	int slot_id = 0;
	char  *ptr = NULL;
	slot_id = atoi(argv[0]);

	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_USER_SHOW_SLOT);

		dbus_error_init(&err);


		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&admin_num);

		vty_out(vty,"The admin user:\n");
	
		for(i=0;i<admin_num;i++)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&ptr);
			vty_out(vty,"%s\n",ptr);
		}
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&view_num);
	
		vty_out(vty,"The view user:\n");
		
		for(i=0;i<view_num;i++)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&ptr);
			vty_out(vty,"%s\n",ptr);
		}
		dbus_message_unref(reply);
		
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
	}
	
	return CMD_SUCCESS;
}
DEFUN (show_user_slot_all,
	   show_user_slot_all_cmd,
	   "show user slot all",
	   "Show system infomation\n"
	   "System usr\n"
	   "board slot\n"
	   "slot id\n"
	   )	
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	
	int ret;
	int i;
	int admin_num = 0;
	int view_num = 0;
	int slot_id = 0;
	char  *ptr = NULL;
	
	for(slot_id=0;slot_id<16;slot_id++)
	{
		if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
		{
			query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
						 SEM_DBUS_INTERFACE,SEM_DBUS_USER_SHOW_SLOT);

			dbus_error_init(&err);


			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
			
			dbus_message_unref(query);

			if (NULL == reply)
			{
				vty_out(vty,"Slot %d,<error> failed get reply.\n",slot_id);
				
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				
				return CMD_WARNING;
			}

			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&admin_num);
			vty_out(vty,"The slot %d:\n",slot_id);

			vty_out(vty," The admin user:\n");
		
			for(i=0;i<admin_num;i++)
			{
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&ptr);
				vty_out(vty," %s\n",ptr);
			}
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&view_num);
		
			vty_out(vty," The view user:\n");
			
			for(i=0;i<view_num;i++)
			{
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&ptr);
				vty_out(vty," %s\n",ptr);
			}
			dbus_message_unref(reply);
			
		}

	}
	return CMD_SUCCESS;
}

DEFUN (passwd_modify_slot,
	   passwd_modify_slot_cmd,
	   "passwd slot <1-15> USERNAME PASSWORD",
	   "User password\n"
	   "board slot\n"
	   "slot id\n"
	   "User name\n"
	   "The new password\n")
{
	if(!boot_flag)
	{
		DBusMessage *query=NULL; 
		DBusMessage *reply=NULL;
		DBusMessageIter  iter;
		DBusError err;
		char cmd[128] = {0};
		char *cmdstr = cmd;
	
		char * username = NULL;
		char* loginname=NULL;
		int ret;
		int slot_id = 0;
		char line1[INPUTSIZE] = {0};
		char line2[INPUTSIZE] = {0};
		int input;
		int i = 0;
		
		username = argv[1];
		
		char passwd[] = "normal";
		loginname = getenv("USER");
		get_global_variable_status();
		
		slot_id = atoi(argv[0]);
		if(slot_id == HostSlotId)
		{
			vty_out(vty,"Can't config the active master board\n");
			return CMD_WARNING;
		}

	
		if(!strcmp(loginname,"admin") || !strcmp(loginname,argv[1]))
		{
			if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
			{
				query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
							 SEM_DBUS_INTERFACE,SEM_DBUS_USER_IS_EXSIT_SLOT);

				dbus_error_init(&err);

				dbus_message_append_args(query,DBUS_TYPE_STRING, &username,DBUS_TYPE_INVALID);
				reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
				
				dbus_message_unref(query);

				if (NULL == reply)
				{
					vty_out(vty,"<error> failed get reply.\n");
					
					if (dbus_error_is_set(&err)) 
					{
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					
					return CMD_WARNING;
				}

				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,&ret);
				dbus_message_unref(reply);
				if (!ret)  
			    {  
					vty_out(vty,"the user %s is not exist\n",argv[1]);
					return CMD_WARNING;
			    }  
				 	
			}	
#if 0		
			fprintf(stdout, "New system password:");
			while((input = getchar()) != '\n' && i <= 32)
			{
				line1[i++] = (char)input;			
			}
			line1[i] = '\0';

			if(i < 4)
			{
				vty_out(vty,"the user password too short\n");
				return CMD_WARNING;
			}

			i = 0;
			
			fprintf(stdout, "\nRetype new system password:");
			while((input = getchar()) != '\n' && i <= 32)
			{
				line2[i++] = (char)input;			
			}
			line2[i] = '\0';						

			
			if(!strcmp(line1,line2))
			{
				strcpy(input_passwd,line1);
			}
			else
			{
				vty_out(vty,"\nSorry, passwords do not match\n");
				return CMD_WARNING;
			}
			fprintf(stdout, "\n");
#endif			
			
		
			if(get_user_role((char *)loginname) == 0)
			{
				vty_out(vty,"user %s is view role ,can't change other user password\n",argv[0]);
				return CMD_WARNING;
			}
			ret = dcli_user_passwd_check(argv[1],argv[2]);
			if(ret == 1)
			{
				vty_out(vty,"the user password is a palindrome \n");
				return CMD_WARNING;
			}	
			else if(ret == 2)
			{
				vty_out(vty,"the user password is too simple \n");
				return CMD_WARNING;
			}
				else if(ret == 3)
			{
				vty_out(vty,"the user password should be not same as username\n");
				return CMD_WARNING;
			}
			else if(ret == 4)
			{
				vty_out(vty,"the user password too short or too long\n");
				return CMD_WARNING;
			}	
			else if(ret == 5)
			{
				vty_out(vty,"the user password length should be >= %d && <=32\n",passwdlen);
				return CMD_WARNING;
			}
						
		
				
			sprintf(cmd,"chpass.sh %s \'%s\' %s",argv[1],argv[2],passwd);
			if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
			{
				query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
							 SEM_DBUS_INTERFACE,SEM_DBUS_USER_PASSWD_SLOT);

				dbus_error_init(&err);

				dbus_message_append_args(query,DBUS_TYPE_STRING, &cmdstr,DBUS_TYPE_INVALID);
				reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
				
				dbus_message_unref(query);

				if (NULL == reply)
				{
					vty_out(vty,"<error> failed get reply.\n");
					
					if (dbus_error_is_set(&err)) 
					{
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					
					return CMD_WARNING;
				}

				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,&ret);
				
				if (-1 == ret)  
			    {  
			 	   vty_out(vty,"system error!");  
			    }  
				else  
			    {  
			  
			 	   if (WIFEXITED(ret))  
			 	   {  
					   switch (WEXITSTATUS(ret))
					   { 	
							case 0: 		
								   break;
							default:			
								vty_out(vty,"Change user %s password error\n",argv[1]);				
								break;		
						}	
				   }  
			 	   else  
			 	   {  
			 		   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret));  
				   }  
			   }  
				dbus_message_unref(reply);
				
			}
			else
			{
				vty_out(vty,"The slot doesn't exist");
			}
			
			return CMD_SUCCESS;
		
		}
		else
		{
			vty_out(vty,"You aren't the user admin,Cann't changed other user's password\n");
				
			return CMD_SUCCESS;

		}
	}
	else
	{		
		vty_out(vty,"In boot state,You should use passwd USERNAME PASSWORD\n");
		return CMD_WARNING;
	}
}

void user_show_running(void)
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	
	int ret;
	int i;
	int admin_num = 0;
	int view_num = 0;
	int slot_id = 0;
	char  *ptr = NULL;

	for(slot_id=1;slot_id<16;slot_id++)
	{	
		if(slot_id != HostSlotId)
		{
			if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
			{
				query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
							 SEM_DBUS_INTERFACE,SEM_DBUS_USER_SHOW_RUNNING);

				dbus_error_init(&err);

				dbus_message_append_args(query,DBUS_TYPE_UINT32, &slot_id,DBUS_TYPE_INVALID);

				reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
				
				dbus_message_unref(query);

				if (NULL == reply)
				{
					fprintf(stderr,"<error> failed get reply.\n");
					
					if (dbus_error_is_set(&err)) 
					{
						fprintf(stderr,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					
					return CMD_WARNING;
				}

				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,&admin_num);
				
				for(i=0;i<admin_num;i++)
				{
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&ptr);
					vtysh_add_entry(ptr);
				}
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&view_num);
				
				for(i=0;i<view_num;i++)
				{
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&ptr);
					vtysh_add_entry(ptr);
				}
				
				dbus_message_unref(reply);	
			}
		}
	}
	
	return CMD_SUCCESS;
}

/**zhaocg end for suporting board slot**/

#endif

DEFUN (who_func,
	   who_func_cmd,
	   "who",
	   "Show current user in system\n")
{
	return WEXITSTATUS(system("who.sh"));
}

DEFUN (whoami_func,
	   whoami_func_cmd,
	   "who am i",
	   "Show current user in system\n"
	   "Am\n"
	   "I\n")
{
	return WEXITSTATUS(system("whoami.sh"));
}

DEFUN (set_system_consolepwd_func1,
       set_system_consolepwd_func_cmd1,
       "console username USERNAME password PWD",
       "Set system console infomation\n"
       "Username\n"
       "Please enter username\n"
       "Password\n"
       "Please enter password \n")
{
	int ret;
	char passwd[10];
	char cmd[128];
	char *p_pwd=NULL,*username=NULL;

	if(argc==2)
	{
		username=argv[0];
		p_pwd=argv[1];
	}
	else if(argc == 1)
	{
		p_pwd=argv[0];
	}
	
	if(username)
	{
		ret = dcli_user_name_check(username);
		if(ret == 1)
		{
			vty_out(vty,"the user name should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'\n");
			return CMD_WARNING;
		}
		else if(ret == 2)
		{
			vty_out(vty,"the user name length should be >=4 & <=32\n");
			return CMD_WARNING;
		}
		else if(ret == 3)
		{
			vty_out(vty,"the user name first char  should be 'A'-'Z' or 'a'-'z'\n");
			return CMD_WARNING;
		}
	}
	if(username)
		sprintf(cmd,"sudo set_console_pwd.sh %s",username);
	else
		sprintf(cmd,"sudo set_console_pwd.sh ");

	if(!boot_flag)
	{
		sprintf(passwd,"normal");
		ret = dcli_user_passwd_check(username,p_pwd);
		if(ret == 1)
		{
			vty_out(vty,"the user password is a palindrome \n");
			return CMD_WARNING;
		}	
		else if(ret == 2)
		{
			vty_out(vty,"the user password is too simple \n");
			return CMD_WARNING;
		}
 		else if(ret == 3)
		{
			vty_out(vty,"the user password should be not same as username\n");
			return CMD_WARNING;
		}
		else if(ret == 4)
		{
			vty_out(vty,"the user password too short or too long\n");
			return CMD_WARNING;
		}
		else if(ret == 5)
		{
			vty_out(vty,"the user password length should be >= %d && <=32\n",passwdlen);
			return CMD_WARNING;
		}
	}
	else
	{
		sprintf(passwd,"security");
	}
	
	sprintf(cmd,"%s \'%s\' %s",cmd,p_pwd,passwd);
	system(cmd);
	
	return CMD_SUCCESS;
	

}
#if 1
ALIAS (set_system_consolepwd_func1,
       set_system_consolepwd_func_cmd,
       "console password PWD",
       "Set system console infomation\n"
       "Password\n"
       "Please enter password \n")
#else
DEFUN (set_system_consolepwd_func,
       set_system_consolepwd_func_cmd,
       "console password PWD",
       "Set system console infomation\n"
       "Password\n"
       "Please enter password \n")
{
	int ret;
	char passwd[10];
	char cmd[128];

	if(!boot_flag)
	{
		sprintf(passwd,"normal");
		ret = dcli_user_passwd_check(argv[0]);
		if(ret == 1)
		{
			vty_out(vty,"The password should be 'A'-'Z'	'a'-'z' '1'-'9'or '_'\n");
			return CMD_WARNING;
		} 
		else if(ret == 2)
		{
			vty_out(vty,"The password length should be >=6 & <=16\n");
			return CMD_WARNING;
		}
	}
	else
	{
		sprintf(passwd,"security");
	}

	sprintf(cmd,"sudo set_console_pwd.sh \'%s\' %s",argv[0],passwd);
	system(cmd);
	return CMD_SUCCESS;
}

#endif

DEFUN (no_system_consolepwd_func,
       no_system_consolepwd_func_cmd,
       "no console password",
       NO_STR
       "System console infomation\n"
       "Password\n")
{
	char cmd[128] = {0};
	sprintf(cmd,"sudo rm %s >> /dev/null 2> /dev/null",CONSOLEPWDFILE);
	system(cmd);
	sprintf(cmd,"sudo rm %s >> /dev/null 2> /dev/null",CONSOLEUSRFILE);
	system(cmd);
	return CMD_SUCCESS;
}


#if 0

/* Execute command in child process. */
static int
execute_dcli_command (const char *command, int argc,char* argv1,char* argv2,char* argv3,char* argv4)
{
	int ret,paranum=0;
	pid_t pid;
	int status;
	char *tmp=NULL;
	char *cmdstr[8];

	pid = fork ();
	if (pid < 0)
    {
      /* Failure of fork(). */
      fprintf (stderr, "Can't fork: %s\n", safe_strerror (errno));
      exit (1);
    }
  	else if (pid == 0)
    {
	    switch (argc)
		{
		case 0:
			ret = execlp (command, command,(const char *)NULL);
			break;
		case 1:
			ret = execlp (command, command, argv1, (const char *)NULL);
			break;
		case 2:
			ret = execlp (command, command, argv1, argv2, (const char *)NULL);
			break;
		case 3:
			ret = execlp (command, command, argv1, argv2, argv3,(const char *)NULL);
			break;
		case 4:
			ret = execlp (command, command, argv1, argv2,argv3,argv4, (const char *)NULL);
			break;
		}
	      /* When execlp suceed, this part is not executed. */
      fprintf (stderr, "Can't execute %s: %s\n", command, safe_strerror (errno));
      exit (1);
    }
  	else
    {
      /* This is parent. */
      ret = wait4 (pid, &status, 0, NULL);
    }
  
  return 0;
}



/****************************************
*added user to system
*input 
*       name: user name
*	  passwd: user password
*output 
* 	  none
*return
*       0     OK
*       -1   the user is exist
*       -2   system error
*
*****************************************/

int dcli_user_add(const char* name,const char* password)
{
	int ret;
	struct passwd *pwd = NULL;
	char num[16];

	pwd = getpwuid(getuid());
	if(pwd)
		printf("user name is %s\n",pwd->pw_name);

	pwd = getpwnam(name);
	if(pwd)
	{
		return -1;
	}
	sprintf(num,"%d",g_ViewGroupID);
	ret = execute_dcli_command("useradd",3, "-g",num,name,NULL);
	if(ret)
	{
		return -2;
	}
	if(dcli_user_pwd_change(name))
	{
		dcli_user_del(name);
		return -3;
	}
	return ret;
}
/****************************************
*delete user from system
*input 
*       name: user name
*output 
* 	  none
*return
*       0     OK
*       -1   the user is not exist
*       -2   system error
*	  -3    delete myself
*****************************************/

int dcli_user_del(char* name)
{
	int ret;
	int uid;
	struct passwd *passwd;

	
	uid = geteuid();
	passwd = getpwnam(name);
	if(!passwd)
		return -1;
	if(uid == passwd->pw_uid)
		return -3;
	if(passwd->pw_gid == g_AdminGroupID)
		return -4;
	
	ret = execute_dcli_command("userdel",1, name, NULL, NULL, NULL);
	if(ret)
	{
		return -2;
	}
	return 0;

}
int dcli_user_pwd_change(const char* name)
{
	int ret;
	struct passwd *pwd = NULL;


	if(!name)
	{
		ret = execute_dcli_command("passwd",0,NULL,NULL,NULL,NULL);
	}
	else
	{
		pwd = getpwuid(getuid());
		if(!pwd)							/*it is not executed,if system no error*/
			return -1;
		if(pwd->pw_gid != g_AdminGroupID)/*it is not executed,if system no error*/
			return -2;
		ret = execute_dcli_command("passwd",1,name,NULL,NULL,NULL);
	}
	if(ret != 0)
	{
		return -3;
	}
	return 0;

}
int dcli_user_role_change(char* name,int admin)
{
	int ret;
	int uid;
	struct passwd *passwd;
	char groupid[16];

	uid = geteuid();
	passwd = getpwnam(name);
	if(!passwd)
		return -1;
	if(uid == passwd->pw_uid)
		return -3;
	if(passwd->pw_gid == g_AdminGroupID)
		return -2;
	if(admin)
		sprintf(groupid,"%d",g_AdminGroupID);
	else
		sprintf(groupid,"%d",g_ViewGroupID);
	ret = execute_dcli_command("usermod",3,name,"-g",groupid,NULL);
	if(ret)
		return -4;
}

DEFUN (user_add,
	   user_add_cmd,
	   "user add USERNAME",
	   "User command\n"
	   "Add user\n"
	   "The new user name\n")
{
	int ret;
	ret = dcli_user_add(argv[0],argv[1]);
	if(-2 == ret)
	{
		vty_out(vty,"Can't add user %s\n",argv[0]);
	}
	else if(-1 == ret)
	{
		vty_out(vty,"the user %s is exist\n",argv[0]);
	}
	else
	{
		vty_out(vty,"Add user %s success\n",argv[0]);

	}
	return CMD_SUCCESS;
}

DEFUN (user_del,
	   user_del_cmd,
	   "user delete USERNAME",
	   "User command\n"
	   "Delete user\n"
	   "The deleted user name\n")
{
	int ret;

    ret = dcli_user_del(argv[0]);
	if(-2 == ret)
	{
		vty_out(vty,"Can't del user %s\n",argv[0]);
	}	
	else if(-3 == ret)
	{
		vty_out(vty,"Can't del user self\n");
	}
	else if(-4 == ret)
	{
		vty_out(vty,"Can't del admin user\n");
	}
	else if(-1 == ret)
	{
		vty_out(vty,"the user %s is not exist\n",argv[0]);
	}
	return CMD_SUCCESS;
}

DEFUN (user_role_change,
	   user_role_change_cmd,
	   "user role USERNAME (view|admin)",
	   "User command\n"
	   "User role\n"
	   "User name\n"
	   "View user\n"
	   "Admin user\n")
{
	int ret;
	int admin = 0;
	if(!strcmp(argv[1],"admin"))
	{
		vty_out(vty,"can't change user 'ADMIN' role \n");
		return CMD_SUCCESS;

	}
	else if(!strcmp(argv[1],"view"))
		admin =0;
	else if(!strcmp(argv[1],"admin"))
		admin = 1;
	else
	{
		vty_out(vty,"Error parament\n");
		return CMD_SUCCESS;
	}		
	ret = dcli_user_role_change(argv[0],admin);
	if(-1 == ret)
	{
		vty_out(vty,"Can't find username %s\n",argv[0]);
		return CMD_SUCCESS;
	}
	else if(-2 == ret)
	{
		vty_out(vty,"you aren't admin user,cann't change other user role\n");
		return CMD_SUCCESS;

	}
	else if(-3 == ret)
	{
		vty_out(vty,"you can't change self role\n");
		return CMD_SUCCESS;

	}
	else		
		vty_out(vty,"change user role error\n");
	return CMD_SUCCESS;

}

DEFUN (passwd_change,
	   passwd_change_cmd,
	   "passwd [USERNAME]",
	   "User password\n")	   
{
	int ret;
	if(argc = 0)
	{
		dcli_user_pwd_change(NULL);
	}
	else
	{

		ret=dcli_user_pwd_change(argv[0]);
		if(-2== ret)
			vty_out(vty,"you aren't admin user,cann't change other user password\n");

	}		
	return CMD_SUCCESS;
}
DEFUN (show_user_all,
	   show_user_all_cmd,
	   "show user",
	   "Show system infomation\n"
	   "System usr\n")	   
{
	int ret;
	struct group *grentry = NULL;
	vty_out(vty,"The admin user:\n");
	grentry = getgrnam(ADMINGROUP);
	if (grentry)
		vty_out(vty,"%s\n",*grentry->gr_mem);
	
	vty_out(vty,"The admin user:\n");
	grentry = getgrnam(VIEWGROUP);
	if (grentry)
		vty_out(vty,"%s\n",*grentry->gr_mem);

	return CMD_SUCCESS;
}
#endif
extern void vtysh_add_entry(char *cmd);
void get_admin_user_pw(void)
{
	struct spwd *spwd=NULL;
	
	spwd = getspnam("admin");
	if(spwd)
	{
		char tmp[81];
		sprintf(tmp,"passwd admin %s\n",spwd->sp_pwdp);
		vtysh_add_entry(tmp);
	}

}

int get_group_user(char* groupname)
{
	struct group *grentry = NULL;
	struct passwd *passwd = NULL;
	struct spwd *spwd=NULL;
	char *ptr=NULL;
	int i;
    char cmdgroup[16];

	memset(cmdgroup,0,16);
	if(!groupname)
		return -1;
	if(strcmp(groupname,ADMINGROUP) &&  strcmp(groupname,VIEWGROUP))
		return -1;
	if(0 == strcmp(groupname,ADMINGROUP))
		sprintf(cmdgroup,"enable");
	else
		sprintf(cmdgroup,"view");
	grentry = getgrnam(groupname);
	if (grentry)
	{
		for(i=0;ptr=grentry->gr_mem[i];i++)
		{
			passwd=getpwnam(ptr);
			if(passwd)
			{
				int j;
				spwd = getspnam(ptr);
				if(spwd)
				{
					if(strcmp(ptr,"admin"))
					{
						char tmp[81];
						sprintf(tmp,"user add %s %s %s\n",ptr,spwd->sp_pwdp,cmdgroup);
						vtysh_add_entry(tmp);
					}
					else
						continue;				
					
				}
			}
		}
			
	}

  return 0;
}
void get_console_pwd()
{
	FILE* fp = NULL;
	char pwd[128]={0};
	char usr[128]={0};
	char cmd[256]={0};
	fp = fopen(CONSOLEPWDFILE, "r");

	if(NULL == fp) {
		return;
	}
	else 
	{
		fgets(pwd,128,fp);
		fclose(fp);

		
		fp = fopen(CONSOLEUSRFILE, "r");
		if(NULL==fp)
		{
			pwd[strlen(pwd)-1]=0;
			sprintf(cmd,"console password %s\n",pwd);
			vtysh_add_entry(cmd);
			return;
		}
		else
		{
			fgets(usr,128,fp);
			fclose(fp);
			usr[strlen(usr)-1]=0;
			pwd[strlen(pwd)-1]=0;
			sprintf(cmd,"console username %s password %s\n",usr,pwd);
			vtysh_add_entry(cmd);
			return;

		}
	}
	
}

// add by zengxx@autelan.com for htdigest 

void Md5(const char *str , char *md5){

	md5_state_t state;
	md5_byte_t digest[16];
	char hex_output[16*2 + 1];

	aute_md5_init(&state);
	aute_md5_append(&state,(const md5_byte_t *)str,strlen(str));
	aute_md5_finish(&state , digest);

	int di;

	for(di = 0 ; di < 16 ; di++){
		sprintf(hex_output + di*2 , "%02x",digest[di]);
	}

	strcpy(md5,hex_output);
}

int get_htdigest_user(){

	char buf[1024] = {0};
	char user_name[1024] = {0};
	char user_passwd[1024] = {0};
	char command[1024] = {0};
	
	FILE *fp;
	
	if(NULL == (fp = fopen(HTDIGEST,"r+"))){
		return -1;
	}
	
	while(NULL != fgets(buf,1024,fp)){
		
		sscanf(buf,"%[^:]:%*[^:]:%[^\n]",user_name,user_passwd);	
		
		sprintf(command,"htdigest %s %s\n",user_name,user_passwd);
		vtysh_add_entry(command);
		
		memset(buf,0,1024);	
		memset(user_name,0,32);
		memset(user_passwd,0,32);
	} 
	if(fp)
		fclose(fp);
	return 0;

}


DEFUN (htdigest_add,
	   htdigest_add_cmd,
	   "htdigest NAME PASSWD",
	   "can't use this command\n"
)
{
	if(boot_flag)
	{
		Htdigest(argv[0],argv[1],0);
	}
	else
	{
		vty_out(vty,"You should not use the command in this state\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


#if 0
/* Moved wsm showrun to dcli_wsm.c by guoxb@autelan.com */
/*
 * Write ipfwd config to cli.conf
 * luoxun add
 */
void dcli_user_ipfwd_write ()
{
    FILE* fp = NULL;
	fp = fopen("/sys/module/cavium_ip_offload/parameters/cw_ipfwd_debug", "r");

	if(NULL == fp) {
		/*Do nothing because ipfwd default to be uninsmoded.*/
	}
	else {
		char _tmpstr[64];
		fclose(fp);
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"USER IPFWD");
		vtysh_add_show_string(_tmpstr);		
		vtysh_add_show_string("set flow-based-forwarding enable\n");
	}
	
}
#endif
void set_file_attr(char* filename)
{
	char cmd[64];
	sprintf(cmd,"sudo chmod 777 %s  >/dev/NULL",filename);
 
}
#if 0
int get_pwd_setting(int *passwdlen,int* passwdstrong)
{
	FILE* fp = NULL;
	char pwd[128]={0};

	set_file_attr(CONPWDSETTINGFILE);

	fp = fopen(CONPWDSETTINGFILE, "r");
	if(NULL == fp) {
		return 1;
	}
	else 
	{
		fgets(pwd,128,fp);
		fclose(fp);
		if(2==sscanf(pwd,"%d,%d",passwdlen,passwdstrong))
			return 0;
		else
			return 1;
	}
	
}
int set_pwd_setting(int passwdlen,int passwdstrong)
{
	FILE* fp = NULL;
	char pwd[128]={0};

	set_file_attr(CONPWDSETTINGFILE);

	fp = fopen(CONPWDSETTINGFILE, "w");

	if(NULL == fp) {
		return 1;
	}
	else 
	{
		fprintf(fp,"%d,%d\n",passwdlen,passwdstrong);
		fflush(fp);
		fclose(fp);
		return 0;
	}
	
}
#endif

int get_login_setting(int* maxdays)
{
	FILE* fp = NULL;
	char buf[512]={0};
	int getsetting = 0;
	
	set_file_attr(CONLOGINSETTINGFILE);

	fp = fopen(CONLOGINSETTINGFILE, "r");
	if(NULL == fp) {
		return 1;
	}
	else 
	{
		while(fgets(buf,512,fp))
		{
			if(buf[0] == '#')
				continue;
			else if(1==sscanf(buf,"PASS_MAX_DAYS   %d",maxdays))
			{
				getsetting=1;
				break;
			}
		}
		fclose(fp);
	}
	if(getsetting)
		return 0;
	else
		return 1;
	
}

int set_login_setting(int maxdays)
{
	FILE* fp = NULL,*fpbk=NULL; 
	char buf[512]={0};
	int getsetting = 0;
	int get_maxdays;
	set_file_attr(CONLOGINSETTINGFILE);
	set_file_attr(CONLOGINSETTINGFILEBK);

	fp = fopen(CONLOGINSETTINGFILE, "r");
	fpbk = fopen(CONLOGINSETTINGFILEBK, "w");
	
	if((NULL == fp))
	{
		if(fpbk)
			fclose(fpbk);
		return 1;
	}
	else if(NULL == fpbk) 
	{
		if(fp)
			fclose(fp);

		return 1;

	}
	else 
		
	{
		while(fgets(buf,512,fp))
		{
			if(1==sscanf(buf,"PASS_MAX_DAYS   %d\n",&get_maxdays))
			{
				if(get_maxdays == maxdays)
				{
					
					fclose(fp);
					fclose(fpbk);
					unlink (CONLOGINSETTINGFILEBK);
					return 0;
				}
				fprintf(fpbk,"PASS_MAX_DAYS   %d\n",maxdays);
				getsetting =1;
			}
			else
			{
				fprintf(fpbk,buf);
			}
		}
		if(getsetting==0)
			fprintf(fpbk,"PASS_MAX_DAYS   %d\n",maxdays);
		fclose(fp);
		fclose(fpbk);
		
		unlink (CONLOGINSETTINGFILE);
		rename ( CONLOGINSETTINGFILEBK,CONLOGINSETTINGFILE);
	}
 	return 0;
}

int get_pwd_err_setting(int* times)
{
	FILE* fp = NULL;
	char buf[512]={0};
	int getsetting = 0;

	set_file_attr(CONPWDERRSETTINGFILE);

	fp = fopen(CONPWDERRSETTINGFILE, "r");
	if(NULL == fp) {
		
		return 1;
	}
	else 
	{
		while(fgets(buf,512,fp))
		{
			if(buf[0] == '#')
				continue;
			else if(1==sscanf(buf,"auth    required        pam_tally.so per_user deny=%d",times))
			{
				getsetting=1;
				break;
			}
		}
		fclose(fp);
	}
	if(getsetting)
		return 0;
	else
		return 1;
	
}

int set_pwd_err_setting(int times)
{
	FILE* fp = NULL,*fpbk=NULL; 
	char buf[512]={0};
	int getsetting = 0;
	int getpwderr=0;

	
	set_file_attr(CONPWDERRSETTINGFILE);
	set_file_attr(CONPWDERRSETTINGFILEBK);

	fp = fopen(CONPWDERRSETTINGFILE, "r");
	fpbk = fopen(CONPWDERRSETTINGFILEBK, "w");
	
	if((NULL == fp))
	{
		if(fpbk)
			fclose(fpbk);

		return 1;
	}
	else if(NULL == fpbk) 
	{
		if(fp)
			fclose(fp);

		return 1;

	}
	else 
	{
		while(fgets(buf,512,fp))
		{
			if(buf[0] == '#')
				fprintf(fpbk,buf);
			else if(1==sscanf(buf,"auth    required        pam_tally.so per_user deny=%d",&getpwderr))
			{
				if(0 == times)
				{
					continue;
				}
				if(getpwderr == times)
				{
					
					fclose(fp);
					fclose(fpbk);
					unlink (CONPWDERRSETTINGFILEBK);
					return 0;
				}
				fprintf(fpbk,"auth    required        pam_tally.so per_user deny=%d\n",times);
				getsetting =1;
			}
			else
			{
				fprintf(fpbk,buf);
			}
		}
		if(getsetting == 0 && times != 0)
			fprintf(fpbk,"auth	  required		  pam_tally.so per_user deny=%d\n",times);
		fclose(fp);
		fclose(fpbk);
		
		unlink (CONPWDERRSETTINGFILE);
		rename ( CONPWDERRSETTINGFILEBK,CONPWDERRSETTINGFILE);
	}
 	return 0;
}

#if 1
int get_pwd_unrepeat_setting()
{
	FILE* fp = NULL;
	char buf[512]={0};
	char t_buf[512];
	char *t_str=NULL;
	int minlen=0,difok=0,dcredit=0,ucredit=0,lcrecit=0;
	

	set_file_attr(CONPWDSYSSETTING);
	fp = fopen(CONPWDSYSSETTING, "r");
	strongpasswd =0;
	
	if(NULL == fp) {
		
		return 0;
	}
	else 
	{
	/* password   required   pam_unix.so nullok md5 remember=2  obscure min=6*/
		while(fgets(buf,512,fp))
		{
		
			if(buf[0] == '#')
				continue;
			else if(2==sscanf(buf,"password required pam_unix.so nullok md5 remember=%d min=%d debug",&passwdunreplytimes,&passwdlen))
			{
				continue;
			}
			else if(2==sscanf(buf,"password required pam_unix.so use_authtok nullok md5 remember=%d min=%d debug",&passwdunreplytimes,&passwdlen))
			{
				continue;
			}
			else if(5==sscanf(buf,"password required pam_cracklib.so minlen=%d difok=%d dcredit=%d ucredit=%d lcredit=%d debug",&minlen,&difok,&dcredit,&ucredit,&lcrecit))
			{
				strongpasswd =1;				
				passwdlen = 6;
			}
			
		}
		
		
	}
	fclose(fp);
	return 1;
	
}

int set_pwd_unrepeat_setting()
{
	FILE* fp = NULL,*fpbk=NULL; 
	char buf[512]={0},t_buf[512]={0};
	int getsetting = 0;
	int getpwderr=0;
	int num1=0,num2=0;
	int minlen=0,difok=0,dcredit=0,ucredit=0,lcrecit=0;

	
	set_file_attr(CONPWDSYSSETTING);
	set_file_attr(CONPWDSYSSETTINGBK);

	fp = fopen(CONPWDSYSSETTING, "r");
	fpbk = fopen(CONPWDSYSSETTINGBK, "w");
	
	if((NULL == fp))
	{
	
		if(fpbk)
			fclose(fpbk);

		return 1;
	}
	else if(NULL == fpbk) 
	{
	
		if(fp)
			fclose(fp);
		return 1;

	}
	else 
	{
		while(fgets(buf,512,fp))
		{
			
			if(buf[0] == '#'){

				fprintf(fpbk,buf);
				continue;
			}
			else if(2==sscanf(buf,"password required pam_unix.so nullok md5 remember=%d min=%d debug",&num1,&num2))
			{	
				continue;	
			}
			else if(5==sscanf(buf,"password required pam_cracklib.so minlen=%d difok=%d dcredit=%d ucredit=%d lcredit=%d debug",&minlen,&difok,&dcredit,&ucredit,&lcrecit))
			{
				continue;	
			}
			else if(2==sscanf(buf,"password required pam_unix.so use_authtok nullok md5 remember=%d min=%d debug",&num1,&num2))
			{
				continue;	
			}
			else
			{
				fprintf(fpbk,buf);
				continue;

			}
		}
		

		if(strongpasswd) 
		{
			sprintf(t_buf,"password required pam_cracklib.so minlen=%d difok=1 dcredit=-1 ucredit=-1 lcredit=-1 debug\n",passwdlen);
			fprintf(fpbk,t_buf);
			memset(t_buf,0,512);
			sprintf(t_buf,"password required pam_unix.so use_authtok nullok md5 remember=%d min=%d debug\n",passwdunreplytimes,passwdlen);
			fprintf(fpbk,t_buf);
		}
		else
		{
			sprintf(t_buf,"password required pam_unix.so nullok md5 remember=%d min=%d debug\n",passwdunreplytimes,passwdlen);
			fprintf(fpbk,t_buf);
		}
		

			
		fclose(fp);
		fclose(fpbk);
		
		unlink (CONPWDSYSSETTING);
		rename ( CONPWDSYSSETTINGBK,CONPWDSYSSETTING);
	}
 	return 0;
}
#if 0
int get_pwd_reply_times(char* username)
{
	FILE* fp = NULL;
	char buf[512]={0};
	int getsetting = 0 ;

	set_file_attr(CONPWDUNREPLYFILE);

	fp = fopen(CONPWDUNREPLYFILE, "r");
	if(NULL == fp) {
		
		return getsetting;
	}
	else 
	{
		while(fgets(buf,512,fp))
			if(!strncmp(buf,username,sizeof(username)))
				getsetting++;
		fclose(fp);
		return getsetting;
	}
	
}
#endif

#if 1 //added by houxx for getting global variable state	
int get_global_variable_status(void)
{
	
	get_pwd_unrepeat_setting();
	get_pwd_err_setting(&passwmaxerrtimes);
	get_login_setting(&passwdalivetime);	

	return 0;
	
}
#endif

#if 0
int add_pwd_reply_times(char* username,char* passwd)
{
	FILE* fp = NULL,*fpbk=NULL; 
	char buf[512]={0},formatstr[128]={0};
	int getreplytimes = get_pwd_reply_times(username);
	int getfirst=0;

	get_pwd_unrepeat_setting();

	sprintf(formatstr,"%s:%s\n",username,passwd);
	if(passwdunreplytimes <= getreplytimes)
	{
		set_file_attr(CONPWDUNREPLYFILE);
		set_file_attr(CONPWDUNREPLYFILEBK);
		
		fp = fopen(CONPWDUNREPLYFILE, "r");
		fpbk = fopen(CONPWDUNREPLYFILEBK, "w");
		
		if((NULL == fp))
		{
		
			return 1;
		}
		else if(NULL == fpbk) 
		{
		
			return 1;
		
		}
		else 
		{
			while(fgets(buf,512,fp))
			{
				if(strcmp(buf,formatstr)==0 && getfirst==0)
				{
					getfirst = 1;
					continue;
				}
				else
				{
					fprintf(fpbk,buf);
				}
		
			}
			
			fprintf(fpbk,formatstr);
			fclose(fp);
			fclose(fpbk);
			
			unlink (CONPWDUNREPLYFILE);
			rename ( CONPWDUNREPLYFILEBK,CONPWDUNREPLYFILE);
		}

	}
	else
	{
	
		set_file_attr(CONPWDUNREPLYFILE);
		fp = fopen(CONPWDUNREPLYFILE, "a+");
		
		if((NULL == fp))
		{
		
			return 1;
		}
		else 
		{
			fprintf(fp,formatstr);
			fclose(fp);
			return 0;
		
		}
		

	}
}
#endif

int pwd_reply_check(char* username,char* passwd)
{
	FILE* fp = NULL;
	char buf[512]={0},formatsrt[128];
	int getsetting = 0 ;
	
	if(!strcmp(username,passwd))
		return 1;
	set_file_attr(CONPWDUNREPLYFILE);

	fp = fopen(CONPWDUNREPLYFILE, "r");
	if(NULL == fp) {
		
		return getsetting;
	}
	else 
	{
		sprintf(formatsrt,"%s:%s\n",username,passwd);
		while(fgets(buf,512,fp))
		{
			if(!strncmp(buf,formatsrt,sizeof(formatsrt)))
				getsetting++;
		}
		fclose(fp);
		return getsetting;
	}
	
}
 
#else
int get_pwd_reply_times(char* username)
{
	FILE* fp = NULL;
	char buf[512]={0};
	int getsetting = 0 ;

	set_file_attr(CONPWDUNREPLYFILE);

	fp = fopen(CONPWDUNREPLYFILE, "r");
	if(NULL == fp) {
		
		return getsetting;
	}
	else 
	{
		while(fgets(buf,512,fp))
			if(!strncmp(buf,username,sizeof(username)))
				getsetting++;
		fclose(fp);
		return getsetting;
	}
	
}


int add_pwd_reply_times(char* username,char* passwd)
{
	FILE* fp = NULL,*fpbk=NULL; 
	char buf[512]={0},formatstr[128]={0};
	int getsetting = get_pwd_unreply_setting() ;
	int getreplytimes = get_pwd_reply_times(username);
	int getfirst=0;

	sprintf(formatstr,"%s:%s\n",username,passwd);
	if(getsetting <= getreplytimes)
	{
		set_file_attr(CONPWDUNREPLYFILE);
		set_file_attr(CONPWDUNREPLYFILEBK);
		
		fp = fopen(CONPWDUNREPLYFILE, "r");
		fpbk = fopen(CONPWDUNREPLYFILEBK, "w");
		
		if((NULL == fp))
		{
		
			return 1;
		}
		else if(NULL == fpbk) 
		{
		
			return 1;
		
		}
		else 
		{
			while(fgets(buf,512,fp))
			{
				if(strcmp(buf,formatstr)==0 && getfirst==0)
				{
					getfirst = 1;
					continue;
				}
				else
				{
					fprintf(fpbk,buf);
				}
		
			}
			
			fprintf(fpbk,formatstr);
			fclose(fp);
			fclose(fpbk);
			
			unlink (CONPWDUNREPLYFILE);
			rename ( CONPWDUNREPLYFILEBK,CONPWDUNREPLYFILE);
		}

	}
	else
	{
	
		set_file_attr(CONPWDUNREPLYFILE);
		fp = fopen(CONPWDUNREPLYFILE, "a+");
		
		if((NULL == fp))
		{
		
			return 1;
		}
		else 
		{
			fprintf(fp,formatstr);
			fclose(fp);
			return 0;
		
		}
		

	}
}

int pwd_reply_check(char* username,char* passwd)
{
	FILE* fp = NULL;
	char buf[512]={0},formatsrt[128];
	int getsetting = 0 ;
	
	if(!strcmp(username,passwd))
		return 1;
	set_file_attr(CONPWDUNREPLYFILE);

	fp = fopen(CONPWDUNREPLYFILE, "r");
	if(NULL == fp) {
		
		return getsetting;
	}
	else 
	{
		sprintf(formatsrt,"%s:%s\n",username,passwd);
		while(fgets(buf,512,fp))
		{
			if(!strncmp(buf,formatsrt,sizeof(formatsrt)))
				getsetting++;
		}
		fclose(fp);
		return getsetting;
	}
	
}
#endif
#if 1

int get_login_status(void)
{	
	FILE* fp = NULL;
	char buf[512]={0};
	int getstatus = 0;
	int length=0;
	
	set_file_attr(CONLOGINSETTING);

	fp = fopen(CONLOGINSETTING, "r");
	if(NULL == fp) {
		return -1;
	}
	else 
	{
		length = strlen("@include common-auth");
		while(fgets(buf,512,fp))
		{
			if(strncmp(buf,"@include common-auth",length) == 0)
			{
				getstatus = 1;
				break;
			}			
		}
		fclose(fp);
	}
	return getstatus;
}

int set_login_remote_status(char* buf_remote)
{
	FILE* fp = NULL;
	char buf[512]={0};
	char secret[32]={0};
	int address[4]={0};
	int port=0;
	int timeout=0;

	
	set_file_attr(CONPAMRADIUSAUTH);

	fp = fopen(CONPAMRADIUSAUTH, "r");
	if(NULL == fp) {
		return -1;
	}
	else 
	{
		while(fgets(buf,512,fp))
		{			
			if (buf[0] == '#')
				continue;
			else if(7 == sscanf(buf,"%d.%d.%d.%d:%d %s %d",&address[0],&address[1],&address[2],&address[3],&port,secret,&timeout))
			{
				sprintf(buf_remote,"user login remote radius_server %d.%d.%d.%d port %d key %s timeout %d",address[0],address[1],address[2],address[3],port,secret,timeout);
				vtysh_add_show_string(buf_remote);
			}			
		}
		fclose(fp);
	}
	return 0;
}

int set_login_conf_setting(const char *login)
{
	char cmd_str[256];

	unlink (CONLOGINSETTING);
	sprintf(cmd_str,"cp /etc/pam.d/%s-login %s",login,CONLOGINSETTING);
	system(cmd_str);
	unlink (CONSSHSETTING);
	sprintf(cmd_str,"cp /etc/pam.d/%s-ssh %s",login,CONSSHSETTING);
	system(cmd_str);
 	return 0;

}

int set_pam_radius_auth_setting(const char* address,const char* port,const char* key,const char* timeout)
{
	FILE* fp = NULL,*fpbk=NULL; 
	char buf[512]={0},t_buf[512]={0};
	char cmd_str[128]={0};
	int length=0;
	int t_address[4]={0};

	if((4 != sscanf(address,"%d.%d.%d.%d",&t_address[0],&t_address[1],&t_address[2],&t_address[3]))||strlen(key)>32)
	{
		return 1;
	}
	set_file_attr(CONPAMRADIUSAUTH);
	set_file_attr(CONPAMRADIUSAUTHBK);

	sprintf(cmd_str,"sed -i '/%s:/d' %s",address,CONPAMRADIUSAUTH);
	system(cmd_str);

	fp = fopen(CONPAMRADIUSAUTH, "r");
	fpbk = fopen(CONPAMRADIUSAUTHBK, "w");
	
	if((NULL == fp))
	{
	
		if(fpbk)
			fclose(fpbk);

		return 1;
	}
	else if(NULL == fpbk) 
	{
	
		if(fp)
			fclose(fp);
		return 1;

	}
	else 
	{
		while(fgets(buf,512,fp))
		{
			length = strlen("# server[:port]	shared_secret      timeout (s)");
			if(strncmp(buf,"# server[:port]	shared_secret      timeout (s)",length) == 0)
			{				
				fprintf(fpbk,buf);
				sprintf(t_buf,"%s:%s %s %s\n",address,port,key,timeout);
				fprintf(fpbk,t_buf);
				continue;	
			}
			else
			{
				fprintf(fpbk,buf);
				continue;

			}
		}
		
			
		fclose(fp);
		fclose(fpbk);
		
		unlink (CONPAMRADIUSAUTH);
		rename (CONPAMRADIUSAUTHBK,CONPAMRADIUSAUTH);
	}
 	return 0;
}


int unset_pam_radius_auth_setting(const char* address)
{
	char cmd_str[128]={0};
	int t_address[4]={0};

	if(4 != sscanf(address,"%d.%d.%d.%d",&t_address[0],&t_address[1],&t_address[2],&t_address[3]))
	{
		return 1;
	}
	set_file_attr(CONPAMRADIUSAUTH);

	sprintf(cmd_str,"sed -i '/%s:/d' %s",address,CONPAMRADIUSAUTH);
	system(cmd_str);
 	return 0;
}


DEFUN (user_login,
	   user_login_cmd,
	   "user login (local|remote)",
	   "User command\n"
	   "User login\n"
	   "User login should be local or remote\n")
{			
	if(set_login_conf_setting(argv[0]))
	{
		vty_out(vty,"Set config file error\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN (user_login_remote,
	   user_login_remote_cmd,
	   "user login remote radius_server A.B.C.D port <1-65535> key KEY timeout <1-60>",
	   "User command\n"
	   "User login\n"
	   "User login to the remote\n"
	   "Radius server\n"
	   "Radius server address\n"
	   "Port of radius server\n"
	   "Port number should be 1-2000\n"
	   "Secret of authentication\n"
	   "Length of Secret key should be less than 32\n"
	   "Authentication timeout"
	   "Timeout should be <1-8>\n")

{
	if(set_pam_radius_auth_setting(argv[0],argv[1],argv[2],argv[3]))
	{
		vty_out(vty,"Set config file error\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN (del_user_login_remote,
	   del_user_login_remote_cmd,
	   "del user login remote radius_server A.B.C.D",
	   "delete user login config\n"
	   "User command\n"
	   "User login\n"
	   "User login to the remote\n"
	   "Radius server\n"
	   "Radius server address\n")

{
	if(unset_pam_radius_auth_setting(argv[0]))
	{
		vty_out(vty,"Unset config file error\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

#endif

DEFUN (passwd_minlen,
	   passwd_minlen_cmd,
	   "passwd minlength <4-8>",
	   "User password\n"
	   "Password min length\n"
	   "Min password lenth should be 4-8\n")
{
	get_pwd_unrepeat_setting();

	passwdlen = atoi(argv[0]);
	if(set_pwd_unrepeat_setting())
	{
		vty_out(vty,"Set config file error\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
DEFUN (passwd_strong,
	   passwd_strong_cmd,
	   "passwd strong",
	   "User password\n"
	   "Password should be stong\n")
{
	get_pwd_unrepeat_setting();

	strongpasswd = 1;
	passwdlen = 6;
	if(set_pwd_unrepeat_setting())
	{
		vty_out(vty,"Set config file error\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}
DEFUN (passwd_alive_time,
	   passwd_alive_time_cmd,
	   "passwd alive time <1-180>",
	   "User password\n"
	   "Keep alive\n"
	   "Alive time\n"
	   "Keep alive time 1-180 days\n")
{
	int alivetime = 0;

	get_login_setting(&alivetime);

	if(alivetime == atoi(argv[0]))
		return CMD_SUCCESS;
	if(0==set_login_setting(atoi(argv[0])))
	{
		passwdalivetime = atoi(argv[0]);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}
DEFUN (passwd_max_error_times,
	   passwd_max_error_times_cmd,
	   "passwd max error <3-10>",
	   "User password\n"
	   "Max\n"
	   "Password error\n"
	   "Max error times 3-10,after error the user shold be locked\n")
{
	int maxerrtimes = 0;

	get_pwd_err_setting(&maxerrtimes);
	
	if(maxerrtimes == atoi(argv[0]))
		return CMD_SUCCESS;
	if(0==set_pwd_err_setting(atoi(argv[0])))
	{
		passwmaxerrtimes = atoi(argv[0]);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}
DEFUN (no_passwd_max_error_times,
	   no_passwd_max_error_times_cmd,
	   "no passwd max error",
	   NO_STR
	   "User password\n"
	   "Max\n"
	   "Password error\n")
{
	
	if(0==set_pwd_err_setting(0))
	{
		passwmaxerrtimes = 0;
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (passwd_unreply_times,
	   passwd_unreply_times_cmd,
	   "passwd unrepeat  <3-10>",
	   "User password\n"
	   "Unreply times\n"
	   "Max unreply times 3-10\n")
{
	int maxerrtimes = 0;

	get_pwd_unrepeat_setting();
	
	passwdunreplytimes = atoi(argv[0]);
	if(set_pwd_unrepeat_setting())
	{
		vty_out(vty,"Set config file error\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN (no_passwd_strong,
	   no_passwd_strong_cmd,
	   "no passwd strong",
	   NO_STR
	   "User password\n"
	   "Password should be stong\n")
{
	get_pwd_unrepeat_setting();

	strongpasswd = 0;
	passwdlen = 4;
	if(set_pwd_unrepeat_setting())
	{
		vty_out(vty,"Set config file error\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}


DEFUN (user_lock,
	   user_lock_cmd,
	   "user lock USERNAME",	   
	   "User command\n"
	   "User lock\n"
	   "User name\n"
	   )
{
	int ret;
	char command[64];

	
	if(!is_user_exsit(argv[0])) 	
	{
		vty_out(vty,"the user %s is not exist\n",argv[0]);
		return CMD_WARNING;
	}
	ret = is_user_self(argv[0]);
	if(ret == 1)
	{
		vty_out(vty,"Can't lock user %s self\n",argv[0]);
		return CMD_WARNING;
	}
	else if(ret == -1)
	{
		vty_out(vty,"the user %s is not exist\n",argv[0]);
		return CMD_WARNING;
	}
	
	sprintf(command,"sudo passwd -l %s > /dev/NULL",argv[0]);
	ret = execute_dcli_shell(command);
	if(ret != 0){
/*		vty_out(vty,"Lock user %s error\n",argv[0]);*/
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN (user_unlock,
	   user_unlock_cmd,
	   "user unlock USERNAME",	   
	   "User command\n"
	   "User unlock\n"
	   "User name\n"
	   )
{
	int ret;
	char command[64];

	
	if(!is_user_exsit(argv[0])) 	
	{
		vty_out(vty,"the user %s is not exist\n",argv[0]);
		return CMD_WARNING;
	}
	ret = is_user_self(argv[0]);
	if(ret == 1)
	{
		vty_out(vty,"Can't del user %s self\n",argv[0]);
		return CMD_WARNING;
	}
	else if(ret == -1)
	{
		vty_out(vty,"the user %s is not exist\n",argv[0]);
		return CMD_WARNING;
	}
	
	sprintf(command,"sudo passwd -u %s > /dev/NULL",argv[0]);
	ret = execute_dcli_shell(command);
	if(access("/var/log/faillog", 0) == 0)
	{
		sprintf(command,"sudo faillog -r -u %s >/dev/NULL",argv[0]);
		ret = execute_dcli_shell(command);
	}
	return CMD_SUCCESS;
}


int dcli_user_manage_write (struct vty *vty)
{
	char _tmpstr[64],buf[64];
	memset(_tmpstr,0,64);
	int alivetime;

	sprintf(_tmpstr,BUILDING_MOUDLE,"USER MANAGEMENT");

	vtysh_add_show_string(_tmpstr);
#if 1
//get login status by hxx
	{
#define __BUF_LEN__ 512		
		char buf[__BUF_LEN__];
		int status;
		status = get_login_status();
		
		memset(buf,0,__BUF_LEN__);
		
		if(status == 0)
		{
			sprintf(buf,"user login remote");
			vtysh_add_show_string(buf);
		}

		memset(buf,0,__BUF_LEN__);


		set_login_remote_status(buf);
#undef __BUF_LEN__ 				
	}	
#endif
	if(get_login_setting(&passwdalivetime)==0 && passwdalivetime != 90)
	{
		sprintf(buf,"passwd alive time %d",passwdalivetime);
		vtysh_add_show_string(buf);
		memset(buf,0,64);
	}
	if(get_pwd_err_setting(&passwmaxerrtimes)==0) /*  && passwmaxerrtimes != 3 */
	{
		sprintf(buf,"passwd max error %d",passwmaxerrtimes);
		vtysh_add_show_string(buf);
		memset(buf,0,64);
	}
	get_admin_user_pw();
	get_group_user(VIEWGROUP);
	get_group_user(ADMINGROUP);
	user_show_running();
	get_console_pwd();

	
	get_pwd_unrepeat_setting();
	if(passwdunreplytimes!=3)
	{
		sprintf(buf,"passwd unrepeat %d",passwdunreplytimes);
		vtysh_add_show_string(buf);
		memset(buf,0,64);
	}
	if(passwdlen != 4)
	{
		sprintf(buf,"passwd minlength %d",passwdlen);
		vtysh_add_show_string(buf);
		memset(buf,0,64);
	}
	if(strongpasswd)
	{
		sprintf(buf,"passwd strong");
		vtysh_add_show_string(buf);
		memset(buf,0,64);		
	}

	/* show ipfwd is insmoded or not. Add by luoxun*/
//	dcli_user_ipfwd_write();
	dcli_sys_global_show_running_cfg();
	dcli_user_authentication_file_sync_show_running();
	dcli_intf_advanced_routing_show_running();
	dcli_intf_bonding_show_running();
	dcli_interrupt_rxmax_show_running_config();
  return 0;
}


void dcli_user_init()
{
#if 0
	struct group *grentry = NULL;
	grentry = getgrnam(ADMINGROUP);
	if (grentry)
		g_AdminGroupID =  grentry->gr_gid;

	grentry = getgrnam(VIEWGROUP);
	if (grentry)
		g_ViewGroupID =  grentry->gr_gid;

#endif
	get_login_setting(&passwdalivetime);
/*
	set_pwd_unrepeat_setting();
*/
	install_element(ENABLE_NODE,&user_add_slot_cmd);
	install_element(ENABLE_NODE,&user_add_slot_all_cmd);
	install_element(ENABLE_NODE,&user_del_slot_cmd);
	install_element(ENABLE_NODE,&user_del_slot_all_cmd);
	install_element(ENABLE_NODE,&user_role_change_slot_cmd);
	install_element(ENABLE_NODE,&user_role_change_slot_all_cmd);
	install_element(ENABLE_NODE,&show_user_slot_cmd);
	install_element(ENABLE_NODE,&show_user_slot_all_cmd);
	install_element(ENABLE_NODE,&passwd_modify_slot_cmd);
	
	install_element(CONFIG_NODE,&user_add_slot_cmd);
	install_element(CONFIG_NODE,&user_add_slot_all_cmd);
	install_element(CONFIG_NODE,&user_del_slot_cmd);
	install_element(CONFIG_NODE,&user_del_slot_all_cmd);
	install_element(CONFIG_NODE,&user_role_change_slot_cmd);
	install_element(CONFIG_NODE,&user_role_change_slot_all_cmd);
	install_element(CONFIG_NODE,&show_user_slot_cmd);
	install_element(CONFIG_NODE,&show_user_slot_all_cmd);
	install_element(CONFIG_NODE,&passwd_modify_slot_cmd);

	install_element(CONFIG_NODE,&user_add_cmd);
	install_element(CONFIG_NODE,&user_del_cmd);
	install_element(CONFIG_NODE,&user_role_change_cmd);	
	install_element(CONFIG_NODE,&passwd_change_cmd);
	install_element(CONFIG_NODE,&passwd_modify_cmd);

	install_element(ENABLE_NODE,&user_add_cmd);
	install_element(ENABLE_NODE,&user_del_cmd);
	install_element(ENABLE_NODE,&user_role_change_cmd);	
	install_element(ENABLE_NODE,&passwd_change_cmd);
	install_element(ENABLE_NODE,&passwd_modify_cmd);
	install_element(ENABLE_NODE,&show_user_all_cmd);
	install_element(ENABLE_NODE,&whoami_func_cmd);
	install_element(ENABLE_NODE,&who_func_cmd);	

	
	install_element(CONFIG_NODE,&set_system_consolepwd_func_cmd);
	install_element(ENABLE_NODE,&set_system_consolepwd_func_cmd);
	install_element(CONFIG_NODE,&set_system_consolepwd_func_cmd1);
	install_element(ENABLE_NODE,&set_system_consolepwd_func_cmd1);
	install_element(CONFIG_NODE,&no_system_consolepwd_func_cmd);
	install_element(ENABLE_NODE,&no_system_consolepwd_func_cmd);

	install_element(CONFIG_NODE,&user_login_cmd);
	install_element(ENABLE_NODE,&user_login_cmd);
	install_element(CONFIG_NODE,&user_login_remote_cmd);
	install_element(ENABLE_NODE,&user_login_remote_cmd);	
	install_element(CONFIG_NODE,&del_user_login_remote_cmd);
	install_element(ENABLE_NODE,&del_user_login_remote_cmd);	
	install_element(CONFIG_NODE,&passwd_minlen_cmd);
	install_element(ENABLE_NODE,&passwd_minlen_cmd);
	install_element(CONFIG_NODE,&passwd_strong_cmd);
	install_element(ENABLE_NODE,&passwd_strong_cmd);
	install_element(CONFIG_NODE,&no_passwd_strong_cmd);
	install_element(ENABLE_NODE,&no_passwd_strong_cmd);
	install_element(CONFIG_NODE,&user_lock_cmd);
	install_element(ENABLE_NODE,&user_lock_cmd);
	install_element(CONFIG_NODE,&user_unlock_cmd);
	install_element(ENABLE_NODE,&user_unlock_cmd);
	
	/*gujd : 2013-03-06, pm  2:46. Add code for user authentication file sync to  other boards.*/
	install_element(CONFIG_NODE,&user_authentication_file_sync_cmd);
	install_element(CONFIG_NODE,&no_user_authentication_file_sync_cmd);

	install_element(CONFIG_NODE,&passwd_alive_time_cmd);
	install_element(ENABLE_NODE,&passwd_alive_time_cmd);

	install_element(CONFIG_NODE,&passwd_max_error_times_cmd);
	install_element(ENABLE_NODE,&passwd_max_error_times_cmd);

	install_element(CONFIG_NODE,&no_passwd_max_error_times_cmd);
	install_element(ENABLE_NODE,&no_passwd_max_error_times_cmd);
	install_element(CONFIG_NODE,&passwd_unreply_times_cmd);
	install_element(ENABLE_NODE,&passwd_unreply_times_cmd);

	install_element(CONFIG_NODE,&htdigest_add_cmd);
	install_element(ENABLE_NODE,&htdigest_add_cmd);
	return ;
}

#ifdef __cplusplus
}
#endif

