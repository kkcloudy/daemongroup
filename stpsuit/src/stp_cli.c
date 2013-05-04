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
* stp_cli.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for internal CLI in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1.1.1 $
*******************************************************************************/
#ifdef __cplusplus
    extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "stp_cli.h"

static CMD_DSCR_T* TheList = 0;

static int
stp_cli_parse_parms (const char* line, int skip_words, char** argv)
{
  static char pool[MAX_CLI_BUFF];
  register char* ptr;
  register int argc = 0, iii;

  ptr = strchr (line, '\n');
  if (ptr) *ptr = '\0';
  memcpy (pool, line, MAX_CLI_BUFF);
  pool[MAX_CLI_BUFF - 1] = '\0';

  for (iii = 0; iii < MAXPARAMNUM; iii++) {
    ptr = strtok (iii ? NULL : pool, "\t\n ");
    if (! ptr) break;
    if (skip_words-- <= 0) {
      /* printf ("\targv[%d]='%s' skip_words=%d\n", argc, ptr, skip_words); */
      argv[argc++] = ptr;
    } else {
      /* printf ("\tskip '%s' skip_words now %d\n", ptr, skip_words); */
    }
  }

  return argc;
}

int stp_cli_count_words (char* line)
{
  static char pool[MAX_CLI_BUFF];
  register char* ptr = NULL;
  register int cnt;

  ptr = strchr (line, '\n');
  if (ptr) *ptr = '\0';
  strncpy (pool, line, MAX_CLI_BUFF);
  pool[MAX_CLI_BUFF - 1] = '\0';

  for (cnt = 0;; cnt++) {
    ptr =  strtok (cnt ? NULL : pool, "\t\n ");
    if (! ptr) {
      break;
    }
  }

  return cnt - 1;
}

void stp_cli_register_language (const CMD_DSCR_T* cmd_list)
{
  TheList = ( CMD_DSCR_T*) cmd_list;
}

static int stp_cli_help_on_param (int iii, CMD_PAR_DSCR_T* par, int cry_on_empty)
{
  register int kkk;

  if (! par->param_help) {
    if (cry_on_empty)
      printf ("absent parameter #%d\n", iii);
    return 1;
  }

  printf ("    arg%2d: %s", iii + 1, par->param_help);
  switch (par->param_type) {
    case CMD_PAR_NUMBER:
      printf ("; integer in [%ld, %ld]",par->number_limits.min,par->number_limits.max);
      break;
    case CMD_PAR_STRING:
      break;
    case CMD_PAR_BOOL_YN:
    case CMD_PAR_ENUM:
      printf ("\n");
      for (kkk = 0; par->string_selector[kkk].string_value; kkk++) {
         printf ("           %-20s\t%s\n", 
                 par->string_selector[kkk].string_value,
                 par->string_selector[kkk].string_help);
      }
      break;
  }

  if (par->default_value) {
      printf ("           default '%s'", par->default_value);
  }
  printf (" \n");
  return 0;
}

void stp_cli_help_on_command (CMD_DSCR_T* reg, char brief)
{
  register CMD_PAR_DSCR_T* par;
  register int iii;

  if (brief) {
    printf ("%-20s %s\n", reg->cmd_name, reg->cmd_help);
    return;
  }
  printf ("%s: %s\n", reg->cmd_name, reg->cmd_help);
  for (iii = 0; iii < 2 + strlen (reg->cmd_name) + strlen (reg->cmd_help); iii++)
    printf ("-");

  printf ("\n");
  if (reg->param->param_help)
    printf ("  arguments:\n  ----------\n");
  for (iii = 0, par = reg->param; ; iii++, par++) {
    if (! par->param_help) break;
    stp_cli_help_on_param (iii + 1, par, 0);
  }
  if (reg->param->param_help)
    printf ("\n");
}

static int stp_cli_dummy (int argc, char** argv)
{
  return 0;
}

static CMD_DSCR_T stdcmd[] = {
  THE_COMMAND("exit", "'shutdown'")
  THE_FUNC(stp_cli_help_on_param)

  THE_COMMAND("?", "help")
  THE_FUNC(stp_cli_help_on_param)

  END_OF_LANG
};

static char* stp_cli_get_commnad_name (int list_index)
{
  register CMD_DSCR_T* reg;

  if (list_index < 2 && list_index >= 0) {
    return stdcmd[list_index].cmd_name;
  }

  list_index -= 2;
  reg = TheList + list_index;
  if (reg->cmd_name && *reg->cmd_name)
    return reg->cmd_name;
  return NULL;
}

static int stp_cli_find_command (const char* line, char forHelp, CMD_DSCR_T **reg_ptr)
{
  register CMD_DSCR_T* reg;
  register int iii = 0;

  for (reg = stdcmd; reg->cmd_name; reg++) {
    if (! strncasecmp (reg->cmd_name, line, strlen (line))) {
      *reg_ptr = reg; iii++;
      if (forHelp)
        stp_cli_help_on_param (reg, 1);
    }
  }

  for (reg = TheList; reg->cmd_name; reg++) {
    if (! strncasecmp (reg->cmd_name, line, strlen (line))) {
      *reg_ptr = reg; iii++;
      if (forHelp)
        stp_cli_help_on_param (reg, 1);
    }
  }

  return iii;
}

void stp_cli_usage (void)
{
  CMD_DSCR_T* reg;

  if (! TheList) {
    printf ("Sorry, command list hasn't been registered\n");
    return;
  }

  printf ("List of possible commands:\n");
  for (reg = TheList; reg->cmd_name; reg++) {
    stp_cli_help_on_param (reg, 1);
  }

  printf ("'standard' commands:\n");
  for (reg = stdcmd; reg->cmd_name; reg++) {
    stp_cli_help_on_param (reg, 1);
  }
}

void stp_cli_debug_dump_args (char* title, int argc, char** argv)
{
  int iii;
  printf ("in %s argc=%d\n", title, argc);
  for (iii = 0; iii < argc; iii++)
    printf ("\targv[%d]='%s'\n", iii, argv[iii]);
  printf ("\n");
}


static int stp_cli_count_cmd_params (CMD_DSCR_T* reg)
{
  register int iii;
  register CMD_PAR_DSCR_T* par;

  for (iii = 0, par = reg->param; ; iii++, par++) {
    if (! par->param_help) break;
  }
  return iii;
}

static void
stp_cli_help_brosed_line (int argc, char** argv, const char* line)
{
  char pool[MAX_CLI_BUFF];
  CMD_DSCR_T* reg;
  int iii, nf;

  printf ("\n");
#if 0
  stp_cli_debug_dump_args ("stp_cli_help_brosed_line", argc, argv);
#endif

  memset (pool, 0, MAX_CLI_BUFF);
  for (iii = 0; iii < argc; iii++) {
    if (iii) strcat (pool, " ");
    strcat (pool, argv[iii]);
    nf = stp_cli_find_command (pool, 0, &reg);
    if (1 == nf) {
      nf = stp_cli_count_cmd_params (reg);
      iii++;
#if 0
      printf ("iii=%d argc=%d nf=%d\n", iii, argc, nf);
#endif
      nf = strlen (line);
      if (nf && ' ' == line[nf - 1])
        argc++;
      if (iii < argc) {
        iii = argc - iii - 1;
        if (! stp_cli_help_on_param (iii + 1, reg->param + iii, 1)) {
          return;
        }
      }
      stp_cli_help_on_param (reg, 0);
      return;
    } else if (! nf) {
      printf ("\nunknown <%s>\n", pool);
      usage ();
      return;
    }
  }
  stp_cli_find_command (pool, 1, &reg);
}

void stp_cli_help (int argc, char** argv, const char* line)
{
#if 0
  stp_cli_debug_dump_args ("stp_cli_help", argc, argv);
#endif
  if (argc > 1)
    stp_cli_help_brosed_line (argc - 1, argv + 1, line);
  else
    stp_cli_usage ();
}

static void stp_cli_set_defaults (CMD_DSCR_T* reg, char** argv)
{
  register int iii;
  register CMD_PAR_DSCR_T* par;

  for (iii = 0, par = reg->param; ; iii++, par++) {
    if (! par->param_help) break;
    argv[iii + 1] = par->default_value;
  }
}

static int stp_cli_call_callback (CMD_DSCR_T* reg, const char* line, int* argc, char** argv)
{
  int cnt;

  if (reg->clbk) {
    cnt = stp_cli_count_words (reg->cmd_name);

    stp_cli_set_defaults (reg, argv);
    *argc = stp_cli_parse_parms (line, cnt, argv);
    return (*reg->clbk) (*argc, argv);
  }
  printf ("<Empty command !>\n");
  return 0;
}

int stp_cli_execute_command (const char* line)
{
  CMD_DSCR_T* reg;
  int argc, nf;
  char *argv[MAXPARAMNUM];

  if ('\n' == *line || ! *line) return 0;

  /* check "common commands" */
  if ('q' == *line || ! strncasecmp ("ex", line, 2)) {
    return 4;
  }

  if ('?' == *line || 'h' == *line) {
    argc = stp_cli_parse_parms (line, 0, argv);
    stp_cli_help (argc, argv, line);
    return 0;
  }

  if (! TheList) {
    printf ("Sorry, command list hasn't been registered\n");
    return -11;
  }

  for (reg = TheList; reg->cmd_name; reg++) {
    if (! strncasecmp (reg->cmd_name, line, strlen (reg->cmd_name))) {
      return stp_cli_call_callback (reg, line, &argc, argv);
    }
  }

  nf = stp_cli_find_command (line, 0, &reg);
  if (1 == nf)
    return stp_cli_call_callback (reg, line, &argc, argv);
  
  printf ("unknown command: <%s>", line);
  stp_cli_usage();
  return 0;
}

extern char shutdown_flag;

char stp_cli_read_command (void)
{
  if (!rl_line_buffer) {
    return 1;
  }

  if (*rl_line_buffer != 0) {
    add_history (rl_line_buffer);
    /** printf ("\n try to call <%s>\n", rl_line_buffer); **/

    if (0 != stp_cli_execute_command (rl_line_buffer)) {
      printf("Goodbye, I'm a gonner\n");
      rl_callback_handler_remove ();
      return 2;
    }
  }

  return 0;
}

#ifdef OLD_READLINE
void stp_cli_rl_read_cli (void)
#else
void stp_cli_rl_read_cli (char *str)
#endif
{
  shutdown_flag |= stp_cli_read_command ();
  rl_callback_handler_install (stp_mngr_get_prompt (), stp_cli_rl_read_cli);
}

char* stp_cli_sprint_time_stamp (void)
{
  time_t      clock;
  struct      tm *local_tm;
  static char time_str[20];

  time(&clock);
  local_tm = localtime (&clock);
  strftime(time_str, 20 - 1, "%H:%M:%S", local_tm);
  return time_str;
}

int complete_status;

/* To disable readline's filename completion */
#ifdef OLD_READLINE
int stp_cli_completion_entry_fucntion (int ignore, int invoking_key)
{ return 0; }
#else
char* stp_cli_completion_entry_fucntion (const char *str, int ignore)
{ return NULL; }
#endif

#ifdef OLD_READLINE
char* stp_cli_command_generator (char* text, int state)
#else
char* stp_cli_command_generator (const char* text, int state)
#endif
{
  static int list_index, len;
  char *name, dlen;

/****
  printf (" state=%d list_index=%d rl_line_buffer'%s' text'%s'\n",
          state, list_index, rl_line_buffer, text);
****/

  dlen = strlen (rl_line_buffer) - strlen (text);
  if (! state) {
    list_index = 0;
    len = strlen (rl_line_buffer);
/****
    printf ("\tlen=%d text<%s>\n", len, text);
****/
  }

  for (;;) { 
    name = stp_cli_get_commnad_name (list_index);
    if (! name) break;
    list_index++;
    if (! strncmp (rl_line_buffer, name, len)) {
/****
      printf (" find <%s> => return '%s'\n", name, name + dlen);
****/
      return strdup (name + dlen);
    }
  }

  return ((char *)NULL);
}

#ifdef OLD_READLINE
int stp_cli_inline_help (void)
#else
int stp_cli_inline_help (int a, int b)
#endif
{
  int argc;
  char *argv[MAXPARAMNUM];

  if (! *rl_line_buffer) {
    stp_cli_usage ();
  } else {
    argc = stp_cli_parse_parms (rl_line_buffer, 0, argv);
    stp_cli_help_brosed_line (argc, argv, (const char*) rl_line_buffer);
  }
  
  rl_on_new_line();
  return 0;

}

char **
stp_cli_private_completion (char *text, int start, int end)
{
  char **matches = NULL;

#ifdef OLD_READLINE
  matches = completion_matches (text, stp_cli_command_generator);
#else
  matches = rl_completion_matches (text, stp_cli_command_generator);
#endif

  return matches;
}

void stp_cli_rl_init ()
{
  /* disable completion */
#if 0
  rl_bind_key ('\t', rl_insert);  
#else
  rl_callback_handler_install (stp_mngr_get_prompt (), stp_cli_rl_read_cli);
  rl_bind_key ('?', stp_cli_inline_help);
  rl_completion_entry_function = stp_cli_completion_entry_fucntion;
  rl_attempted_completion_function = (CPPFunction *)stp_cli_private_completion;
  rl_completion_append_character = '\0';
#endif
}

void stp_cli_rl_shutdown ()
{
  rl_initialize ();
}
#ifdef __cplusplus
}
#endif

