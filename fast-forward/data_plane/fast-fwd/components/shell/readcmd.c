
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "malloc.h"
#include "shell.h"
#include "uart.h"

extern struct cmd cmd_list[MAX_CMDS];
extern int uart_index;

#define MAX_HISTORY     	64
#define MAX_CMD			64
#define CMD_BUF_SIZE 	256

#define KEY_BACKSPACE  	0x08
#define KEY_ENTER       	0x0D
#define KEY_ESC        		0x1B
#define KEY_DEL         	0x7f
#define KEY_LEFT        	0x44
#define KEY_RIGHT       	0x43
#define KEY_DOWN        	0x42
#define KEY_UP          		0x41
#define KEY_TAB         	0x09
#define KEY_UNKNOWN     0xFFF

static int history_len = 0;
static int history_idx = 0;
static char *history[MAX_HISTORY];
static int cmd_tab[MAX_CMDS];

/**
* add cmd-executed to history
* param @line cmd-executed
*/
void add_to_history(char *line)
{
	int i;

	if (!line || !*line) return;
	if (history_len > 0 && strcmp(line, history[history_len - 1]) == 0) return;
	
	/*if history overload, unload the first*/
	if (history_len == MAX_HISTORY)
	{
		free(history[0]);
		for (i = 0; i < history_len - 1; i++) history[i] = history[i + 1];
		history_len--;
	}

	history[history_len] = malloc(strlen(line) + 1);
	if (history[history_len]) 
	{
		strcpy(history[history_len], line);
		history_len++;
	}
}

/**
* get key typed by user
*/
static int get_key()
{
	int ch;

	ch = uart_read_byte(uart_index);
	if (ch <= 0) 
	{
		return -1;
	}
	switch (ch)
	{
	case 0x08: 
		return KEY_BACKSPACE;
	case 0x09: 
		return KEY_TAB;
	case 0x0D:
		return KEY_ENTER;
	case 0x7f:
		return KEY_DEL;
	/*
	**	Extended for other key
	*/
	case 0x1B:
	{
		ch = uart_read_byte(0);
		switch (ch)
		{
		case 0x1B: 
			return KEY_ESC;
		case 0x5B:
			ch = uart_read_byte(0);
			switch (ch)
			{
			case 0x41: 
				return KEY_UP;
			case 0x42: 
				return KEY_DOWN;
			case 0x43: 
				return KEY_RIGHT;
			case 0x44: 
				return KEY_LEFT;
			default: 
				return KEY_UNKNOWN;
			}
			break;

		default:
			return KEY_UNKNOWN;
		}

		break;
	}
	default: 
		return ch;
	}
}

/**
* read cmd
* param @buf place cmd
* param @bufsize size of buf
*/
int read_cmd(char *buf, int bufsize)
{
	static int i   = 0;
	static int key = 0;		  /*key typed*/ 		
	static int idx = 0;               /*the pos in current cmd */
	static int len = 0;		  /*len of current cmd*/
	int done = 0;			  /*cmd end, enter is typed*/

	if (bufsize <= 0) 	
		return -1;
	
	key = get_key();
	if (key < 0) 
		return -1;

	switch (key)
	{
	case KEY_LEFT: /*left arrow*/
		if (idx > 0)
		{
			uart_write_byte_nointr(uart_index, '\b');
			idx--;
		}
		break;

	case KEY_RIGHT:/*right arrow*/
		if (idx < len)
		{
			uart_write_byte_nointr(uart_index, buf[idx]);
			idx++;
		}
		break;
	case KEY_DEL: /*delete*/
		if (idx < len)
		{
			len--;
			memmove(buf + idx, buf + idx + 1, len - idx);
			buf[len] = 0;
			for (i = idx; i < len; i++) uart_write_byte_nointr(uart_index, buf[i]);
			uart_write_byte_nointr(uart_index, ' ');
			uart_write_byte_nointr(uart_index, '\b');
			for (i = idx; i < len; i++) uart_write_byte_nointr(uart_index, '\b');
		}
		break;
	case KEY_BACKSPACE:/*backspace*/
		if (idx > 0)
		{
			uart_write_byte_nointr(uart_index, '\b');
			idx--;
			len--;
			memmove(buf + idx, buf + idx + 1, len - idx);
			buf[len]=0;
			for (i = idx; i < len; i++) uart_write_byte_nointr(uart_index, buf[i]);
			uart_write_byte_nointr(uart_index, ' ');
			uart_write_byte_nointr(uart_index, '\b');
			for (i = idx; i < len; i++) uart_write_byte_nointr(uart_index, '\b');
		}
		break;

	case KEY_ENTER :/*enter*/
		uart_write_byte_nointr(uart_index, '\r');
		uart_write_byte_nointr(uart_index, '\n');
		idx = 0;
		key = 0;
		i   = 0;
		done = 1;
		break;
	case KEY_TAB:/*tab*/
		{
			int j, k;
			k = 0;

			if (len > bufsize - 1) 
				len = bufsize - 1;
			else if (!len)
				break;

			for(j = 0; j <= MAX_CMDS - 1; j++)
			{
				for(i=0; (i < len) && (cmd_list[j].name[i] == buf[i]); i++)
				;
				if (i == len)
					cmd_tab[k++] = j;
			}

			if (k == 1)
			{
				char *cmdptr = NULL;
				cmdptr = cmd_list[cmd_tab[0]].name;
				for (i = 0; i < len; i++) uart_write_byte_nointr(uart_index, '\b');
				for (i = 0; i < len; i++) uart_write_byte_nointr(uart_index, ' ');
				for (i = 0; i < len; i++) uart_write_byte_nointr(uart_index, '\b');
				uart_write_string(uart_index, cmdptr);
				strncpy(buf, cmdptr, strlen(cmdptr));
				len = idx = strlen(cmdptr);
			}
			else if (k > 1)
			{
				uart_write_string(uart_index, "\r\n");
				for(j=0; j<k; j++)
				{
					uart_write_string(uart_index, cmd_list[cmd_tab[j]].name);
					uart_write_byte_nointr(uart_index, '\t');
				}
				uart_write_string(uart_index, "\r\n");
				print_prompt();
				uart_write_string(uart_index, buf);
			}
			break;
		}
	case KEY_UP:/*up arrow, list the cmd by  executed order, from back to front*/
		if (history_idx > 0)
		{
			history_idx--;
			for (i = 0; i < idx; i++) uart_write_byte_nointr(uart_index, '\b');
			for (i = 0; i < len; i++) uart_write_byte_nointr(uart_index, ' ');
			for (i = 0; i < len; i++) uart_write_byte_nointr(uart_index, '\b');
			len = strlen(history[history_idx]);
			if (len > bufsize - 1) 
				len = bufsize - 1;
			idx = len;
			memcpy(buf, history[history_idx], len);
			for (i = 0; i < len; i++) uart_write_byte_nointr(uart_index, buf[i]);
		}
		break;

	case KEY_DOWN:/*up down, list cmd, from front to back*/
		if (history_idx < history_len - 1)
		{
			history_idx++;
			for (i = 0; i < idx; i++) uart_write_byte_nointr(uart_index, '\b');
			for (i = 0; i < len; i++) uart_write_byte_nointr(uart_index, ' ');
			for (i = 0; i < len; i++) uart_write_byte_nointr(uart_index, '\b');
			len = strlen(history[history_idx]);
			if (len > bufsize - 1) 
				len = bufsize - 1;
			idx = len;
			memcpy(buf, history[history_idx], len);
			for (i = 0; i < len; i++)uart_write_byte_nointr(uart_index, buf[i]);
		}
		break;
	case KEY_UNKNOWN:
		break;

	default: /*other char*/
		if (key >= 0x20 && key <= 0xFF)
		{						
				if (len < bufsize - 1)
				{
					if (idx < len) memmove(buf + idx + 1, buf + idx, len - idx);
					buf[idx] = key;
					len++;
					for (i = idx; i < len; i++) uart_write_byte_nointr(uart_index, buf[i]);
					idx++;
					for (i = idx; i < len; i++) uart_write_byte_nointr(uart_index, '\b');
				}
		}
		break;
	}
	/*enter is typed, cmd completed*/
	if(done)
	{
		int 	tlen;
		if(buf[0] == '!')
		{
			if(buf[1] == '!')
			{
				if(history_len)
				{
					len = strlen(history[history_len-1]);
					memcpy(buf,history[history_len-1],len);
					printf("%s\n",buf);
				}
				else
				{
					len = 0;
				}
			}
			else
			{
				int hnum = atoi(&buf[1]);
				if(history[hnum])
				{
					len = strlen(history[hnum]);
					memcpy(buf,history[hnum],len);
					printf("%s\n",buf);
				}
				else
				{
					len = 0;
				}
			}				
		}
		
		buf[len] = 0;
		tlen = len +1;
		len = 0;
		if(tlen > 1)	
			add_to_history(buf);
		history_idx = history_len;
		return tlen;
	}
	else
	{
		return 0;
	}
}

/**
hist handler
*/	
int32_t print_history(int argc, char *argv[])
{
	int 	i;
	for(i=0 ; i <MAX_HISTORY; i++)
	{
		if(history[i])
			printf("%d %s\n",i, history[i]);
	}

	return CMD_EXEC_SUCCESS;
}

