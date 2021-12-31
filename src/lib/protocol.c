/*
 * Authors: Rohan Sanda <rsanda@stanford.edu>
 *          Matthew Tan <matthtan@stanford.edu>
 *
 * CS107E - Autumn 2021
 * Pi in the Sky
 *
 * This file contains the implementation for the protocol library.
 * It mainly consists of the transmit function, which sends
 * information in a specified order that the plane's handler
 * function can understand.
 */
#include "protocol.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "uart.h"
#include "uart_extras.h"
#include "assert.h"
#include "printf.h"

static bool protocol_initialize = false;

void protocol_init() {
    uart_init();
    gpio_init();

    protocol_initialize = true;
}

/*
 * Function that sends control command to plane.
 *
 * @param type --> a number 1-8 corresponding to control (defined in protocol.h)
 * @param curr_value --> value of type (used to set servo). Can be a maximum of 2 bytes
 * since ADC outputs a range from 0-1024.
 *
 * @return --> 1 for successful transmission; 0 for invalid input
 */
int transmit(int type, int curr_value) {
   
    //Exit if invalid type 
    if (type > 8) return 0;

    //0 is reserved as the "synchronization" bit, so it should not be sent for any other purpose
    if (curr_value < 1) {
        curr_value = 1;
    }

    //byte1 = least significant part of integer
    //byte2 = most significant part of integer
    int byte1 = curr_value & 0xFF;
    int byte2 = (curr_value >> 8) & 0xFF;

    //Synchronize
    uart_send(0);

    //Send type, and least sig, followed by most sig
    uart_send(type);
    uart_send(byte1);
    uart_send(byte2);

    return 1;
}



