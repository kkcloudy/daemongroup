
#ifndef _UART_

#define _UART_ 

#include "cvmx.h"

#define BUFFER_SIZE 4096    /* Size of the uart buffers. Must be a power of 2 */

/**
 * Structure to use for TX and RX buffers
 */
typedef struct
{
    uint8_t data[BUFFER_SIZE];  /* Data buffered for the uart */
    volatile int head;          /* New characters are put here */
    volatile int tail;          /* Characters are removed from here */
} uart_buffer_t;


extern   void  uart_write_byte(int uart_index, uint8_t ch);
extern void uart_write_byte_nointr(int uart_index, uint8_t ch);
extern  void uart_write_string(int uart_index, const char *str);
extern  uint8_t uart_read_byte(int uart_index);
extern  uint8_t uart_read_byte_wait(int uart_index);
extern int uart_read_string(int uart_index, char *buf, int bufsize);
extern uint8_t uart_read_byte_nowait(int uart_index);
extern void  uart_interrupt(int irq_number, uint64_t registers[32], void *user_arg);
extern int uart_setup(int uart_index);
#endif
