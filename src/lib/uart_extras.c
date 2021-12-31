/*
 * Rohan Sanda <rsanda@stanford.edu>
 * Matthew Tan <matthtan@stanford.edu>
 * 
 * CS107E
 * Pi in the Sky - Autumn 2021
 *
 * This file is identical to the uart.c file but contains an additional uart_recv_timer()
 * function that does not block indefinitely, but rather only for a specified time. We did
 * not end up using this function in our final implementation.
 */

#include "gpio.h"
#include <stdint.h>
#include "uart.h"
#include "uart_extras.h"
#include "timer.h"

struct UART {
    uint32_t data; // I/O Data
    uint32_t ier;  // Interrupt enable
    uint32_t iir;  // Interrupt identify and fifo enables/clears
    uint32_t lcr;  // line control register
    uint32_t mcr;  // modem control register
    uint32_t lsr;  // line status register
    uint32_t msr;  // modem status register
    uint32_t scratch;
    uint32_t cntl; // control register
    uint32_t stat; // status register
    uint32_t baud; // baud rate register
} ;

// AUX bits
#define AUX_ENABLES 0x20215004
#define AUX_ENABLE  0x00000001

// Mini UART
#define MINI_UART_BASE 0x20215040

#define MINI_UART_IIR_RX_FIFO_CLEAR  0x00000002
#define MINI_UART_IIR_TX_FIFO_CLEAR  0x00000004
#define MINI_UART_IIR_RX_FIFO_ENABLE 0x00000080
#define MINI_UART_IIR_TX_FIFO_ENABLE 0x00000040

#define MINI_UART_LCR_8BIT       0x00000003

#define MINI_UART_LSR_RX_READY   0x00000001
#define MINI_UART_LSR_TX_READY   0x00000010
#define MINI_UART_LSR_TX_EMPTY   0x00000020

#define MINI_UART_CNTL_TX_ENABLE 0x00000002
#define MINI_UART_CNTL_RX_ENABLE 0x00000001

static volatile struct UART *uart = (struct UART*) MINI_UART_BASE;
static bool initialized = false;

//Max blocking time in us
#define TIMER_THRESH 1000

//Uart receive function but blocks only for TIMER_TRESH
int uart_recv_timer(char *buf) {
    int start = timer_get_ticks();

    while(!uart_haschar()) {
        if (timer_get_ticks() - start > TIMER_THRESH) {
            return 0;
        }
    }
    
    char data = uart->data & 0xFF;
    
    if (data == EOT) data = EOF;
    *buf = data;
    return 1;
}



