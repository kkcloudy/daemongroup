
/**
 * @file uart.c
 *
 * Sample uart code showing input and output using the uart.
 *
 * File version info: 
 *
 */

#include <stdio.h>
#include <string.h>
#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-uart.h"
#include "cvmx-interrupt.h"
#include "cvmx-sysinfo.h"
#include "uart.h"

/**
 * Buffer used for pending TX data to the uart
 */
static uart_buffer_t tx_buffer;

/**
 * Buffer used to store data from the uart
 */
static uart_buffer_t rx_buffer;

/**
 * Initialize a buffer to empty
 *
 * @param buffer Buffer to initialize
 */
static inline void uart_buffer_initalize(uart_buffer_t *buffer)
{
    buffer->head = 0;
    buffer->tail = 0;
}


/**
 * Check if a buffer is empty
 *
 * @param buffer Buffer to check
 * @return True if the buffer is empty
 */
static inline int uart_buffer_empty(const uart_buffer_t *buffer)
{
    return (buffer->head == buffer->tail);
}


/**
 * Check if a buffer is full
 *
 * @param buffer Buffer to check
 * @return True of the buffer is full
 */
static inline int uart_buffer_full(const uart_buffer_t *buffer)
{
    if (buffer->head < buffer->tail)
        return (buffer->head + 1 == buffer->tail);
    else
        return ((buffer->tail == 0) && (buffer->head == BUFFER_SIZE-1));
}


/**
 * Read a byte from the buffer. It returns zero if the buffer
 * if empty. You should check uart_buffer_empty() before calling
 * this function.
 *
 * @param buffer Buffer to read from
 * @return Data or zero on error (buffer empty)
 */
static inline uint8_t uart_buffer_read(uart_buffer_t *buffer)
{
    if (uart_buffer_empty(buffer))
        return 0;

    uint8_t result = buffer->data[buffer->tail];
    buffer->tail = (buffer->tail + 1) & (BUFFER_SIZE-1);
    return result;
}


/**
 * Write a byte to the buffer. The byte will be lost if the
 * buffer is full. Check uart_buffer_full() before calling
 * this function.
 *
 * @param buffer Buffer to write to
 * @param data
 */
static inline void uart_buffer_write(uart_buffer_t *buffer, uint8_t data)
{
    if (uart_buffer_full(buffer))
        return;

    buffer->data[buffer->head] = data;
    buffer->head = (buffer->head + 1) & (BUFFER_SIZE-1);
}

/**
 * Setup a uart for use
 *
 * @param uart_index Uart to setup (0 or 1)
 * @return Zero on success
 */
int uart_setup(int uart_index)
{
    //cvmx_uart_fcr_t fcrval;
    cvmx_uart_ier_t ierval;
    //cvmx_uart_mcr_t mcrval;
   // cvmx_uart_lcr_t lcrval;

    /* Empty the RX and TX buffers */
    uart_buffer_initalize(&rx_buffer);
    uart_buffer_initalize(&tx_buffer);

    /* no need to initialize uart since bootloader has done it already */
    /* only need to initialize the additional uart interrupt enables */

    ierval.u64 = 0;
    ierval.s.ptime = 0; /* Enable the THRE programmable interrupts */
    ierval.s.etbei = 0; /* Interrupt when the TX buffer is empty */
    ierval.s.erbfi = 1; /* Interrupt when there is RX data */
    cvmx_write_csr(CVMX_MIO_UARTX_IER(uart_index), ierval.u64);

    return 0;
}

/**
 * Uart interrupt handler. Currently this function only
 * handles the RX and TX data well. Uart errors print a
 * message, but do nothing else. RX data is stored into
 * the buffer "rx_buffer". TX data is read from the buffer
 * "tx_buffer". Remember that TX interrupts only occur when
 * we transition to empty. The first byte sent when the Uart
 * is idle must be sent directly to the uart and not buffer
 * in tx_buffer.
 *
 * @param irq_number The interrupt number that occurred. This will be 2 for the
 *                   uarts.
 * @param registers  CPU registers at time of interrupt
 * @param user_arg   Optional argument supplied when we registered the
 *                   interrupt handler. Here it contains the uart port number.
 */
 void  uart_interrupt(int irq_number, uint64_t registers[32], void *user_arg)
{
    const unsigned long uart_index = (unsigned long)user_arg;
    int count;
    cvmx_uart_iir_t iir;
    cvmx_uart_lsr_t lsr;
    iir.u64 = cvmx_read_csr(CVMX_MIO_UARTX_IIR(uart_index));
    lsr.u64 = cvmx_read_csr(CVMX_MIO_UARTX_LSR(uart_index));

    switch (iir.s.iid)
    {
        case CVMX_UART_IID_NONE:
            /* Nothing to do */
            /*printf("UART NONE\n");*/
            break;
        case CVMX_UART_IID_RX_ERROR:
            printf("UART RX ERROR lsr=%llx\n", (unsigned long long)lsr.u64);
            break;
        case CVMX_UART_IID_RX_TIMEOUT:
            /*printf("UART RX TIMEOUT lsr=%llx\n", (unsigned long long)lsr.u64);*/
            break;
        case CVMX_UART_IID_MODEM:
            printf("UART MODEM STATUS lsr=%llx\n", (unsigned long long)lsr.u64);
            break;
        case CVMX_UART_IID_RX_DATA:
            /* Handled below */
            break;
        case CVMX_UART_IID_TX_EMPTY:
            /* We know the uart is empty, so write out the max amount of data,
                64 bytes */
            count = 64;
            while ((!uart_buffer_empty(&tx_buffer)) && (count--))
            {
                cvmx_write_csr(CVMX_MIO_UARTX_THR(uart_index), uart_buffer_read(&tx_buffer));
            }
            break;
        case CVMX_UART_IID_BUSY:
            /* This is needed on CN30XX and CN31XX Pass1 chips. This is a
                workaround for Errata UART-300 */
            cvmx_read_csr(CVMX_MIO_UARTX_USR(uart_index));
    }

    /* Regardless of why we got here read in any pending data */
    while (lsr.s.dr)
    {
        if (uart_buffer_full(&rx_buffer))
            printf("UART RX BUFFER OVERFLOW\n");
        else
            uart_buffer_write(&rx_buffer, cvmx_read_csr(CVMX_MIO_UARTX_RBR(uart_index)));
        lsr.u64 = cvmx_read_csr(CVMX_MIO_UARTX_LSR(uart_index));
    }
}

void uart_write_byte_nointr(int uart_index, uint8_t ch)
{
    cvmx_uart_lsr_t lsrval;

    /* Spin until there is room */
    do
    {
        lsrval.u64 = cvmx_read_csr(CVMX_MIO_UARTX_LSR(uart_index));
    }
    while (lsrval.s.thre == 0);

    /* Write the byte */
    cvmx_write_csr(CVMX_MIO_UARTX_THR(uart_index), ch);
}

/**
 * Put a single byte to uart port.
 *
 * @param uart_index Uart to write to (0 or 1)
 * @param ch         Byte to write
 */
void uart_write_byte(int uart_index, uint8_t ch)
{
    printf("ERROR, We don't use interrupt to send the char\r\n");
    return;
	
    /* if the TX buffer is empty we may need to send it directly to the
        UART. */
    if (uart_buffer_empty(&tx_buffer))
    {
        cvmx_uart_lsr_t lsr;
        lsr.u64 = cvmx_read_csr(CVMX_MIO_UARTX_LSR(uart_index));

        if (!lsr.s.thre)
        {
            /* Write the byte now. The uart has room */
            cvmx_write_csr(CVMX_MIO_UARTX_THR(uart_index), ch);
            return;
        }
        /* UART didn't have room so we need to buffer it */
    }

    while (uart_buffer_full(&tx_buffer))
    {
        /* Spin until there is room */
    }
    uart_buffer_write(&tx_buffer, ch);
}


/**
 * Write a string to the uart
 *
 * @param uart_index Uart to use (0 or 1)
 * @param str        String to write
 */
void uart_write_string(int uart_index, const char *str)
{
    /* Just loop writing one byte at a time */
    while (*str)
    {
        //uart_write_byte(uart_index, *str);
        uart_write_byte_nointr(uart_index, *str);
        str++;
    }
}


/**
 * Get a single byte from serial port.
 *
 * @param uart_index Uart to read from (0 or 1)
 * @return The byte read
 */
uint8_t uart_read_byte(int uart_index)
{
#if 0
    while (uart_buffer_empty(&rx_buffer))
    {
        /* Spin until data is available */
    }
#endif

    if (uart_buffer_empty(&rx_buffer))
        return 0;
	
    return uart_buffer_read(&rx_buffer);
}

/**
 * Get a single byte from serial port.
 *
 * @param uart_index Uart to read from (0 or 1)
 * @return The byte read
 */
uint8_t uart_read_byte_wait(int uart_index)
{
    while (uart_buffer_empty(&rx_buffer))
    {
        /* Spin until data is available */
    }

    return uart_buffer_read(&rx_buffer);
}

/**
 * Read a line from the uart and return it as a string.
 * Carriage returns and newlines are removed.
 *
 * @param uart_index Uart to read from (0 or 1)
 * @param buffer     Buffer to put the result in
 * @param bufferSize Size of the buffer
 * @return Length of the string
 */
int uart_read_string(int uart_index, char *buffer, int bufferSize)
{
    int count = 0;

    /* Read until we get a newline or run out of room */
    while (count < bufferSize - 1)
    {
        buffer[count] = uart_read_byte(uart_index);
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
	{
           if (buffer[count] == '\r')
               continue;
           if (buffer[count] == '\n')
               break;
	}
	else
	{
           //uart_write_byte(uart_index, buffer[count]);  /* echo */
           uart_write_byte_nointr(uart_index, buffer[count]);
           if (buffer[count] == '\r') {
               //uart_write_byte(uart_index, '\n');  /* linefeed */
               uart_write_byte_nointr(uart_index, '\n');  /* linefeed */   
               break;
           }
	}
        count++;
    }

    /* Null terminate the string to make life easy */
    buffer[count] = 0;

    return count;
}

