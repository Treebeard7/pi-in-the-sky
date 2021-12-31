/*
 * Authors: Rohan Sanda <rsanda@stanford.edu>
 *          Matthew Tan <matthtan@stanford.edu>
 *
 * Pi in the Sky - Autumn 2021 
 *
 * This file contains the main script that is run on the remote control (ground unit).
 * 
 * Credit: We make use of the spi.h, uart.h, assert.h, mcp3008.h libraries provided by the course.
 *
 */

#include "printf.h"
#include "uart.h"
#include "uart_extras.h"
#include "timer.h"
#include "protocol.h"
#include "mcp3008.h"
#include "gpio.h"
#include "assert.h"
#include "spi.h"

/* 
 * Constants:
 * NUM_CHANNELS --> number of channels to read from ADC
 * SENSITIVITY --> difference threshold for transmitting new readings to plane
 * HEARTBEAT_PULSE --> Send heartbeat every x loop iterations
 */
#define NUM_CHANNELS 8
#define SENSITIVITY 20
#define HEARTBEAT_PULSE 100

//Order: throttle, aileron, elevator, rudder, xx, flight_mode (we haven't assigned functions to the rest)
static int channels[] = { 2, 6, 7, 1, 3, 4, 5, 0 };
static int history[] = { 0, 0, 0, 0, 0, 0, 0, 0};

//iteratative variable for the heartbeat pulse
static int loop_iter = 0;

//1 = in flight mode, 0 = out
static int flight_mode = 0;

//Absolute value helper function
int abs(int a) {
    if (a < 0) return -1*a;
    return a;
}

//Test function that prints out joystick readings from ADC
void test_joystick(void) {
    while (1) {
        unsigned int a = mcp3008_read(7);   //elevator  
        unsigned int b = mcp3008_read(6);   //aileron   
        unsigned int c = mcp3008_read(5);   //button
        unsigned int d = mcp3008_read(4);   //
        unsigned int e = mcp3008_read(3);   //left button   
        unsigned int f = mcp3008_read(2);   //throttle
        unsigned int g = mcp3008_read(1);   //rudder
        unsigned int h = mcp3008_read(0);   //button

        printf("a: %d\n b: %d\n c: %d\n d: %d\n e: %d\n f: %d\n g: %d\n h: %d\n\n", a, b, c, d, e, f, g, h);
        timer_delay_ms(500);
    }
}

//Test function for protocol
void new_protocol_tx(void) {
    unsigned int history= 0;
    unsigned int history1 = 0;
    while (1) {
          unsigned int curr_val = mcp3008_read(7);
          unsigned int curr_val1 = mcp3008_read(6);

          if (curr_val < 1) {
             curr_val = 1;
          }
          if (curr_val1 < 1) {
             curr_val1 = 1;
          }
        
        if (abs(curr_val - history) > SENSITIVITY || abs(curr_val1 - history1) > SENSITIVITY) {

            unsigned int byte1 = curr_val & 0xFF;
            unsigned int byte2 = (curr_val >> 8) & 0xFF;
        
            unsigned int byte3 = curr_val1 & 0xFF;
            unsigned int byte4 = (curr_val1 >> 8) & 0xFF;

          if (abs(curr_val - history) > SENSITIVITY) {
                history = curr_val;
            }  
          if (abs(curr_val1 - history1) > SENSITIVITY) {
              history1 = curr_val1;
            }

        uart_send(0);
        uart_send(1);
       //timer_delay_ms(50);
       uart_send(byte1);
       //timer_delayi_ms(50);
       uart_send(byte2);
       //timer_delay_ms(50);
       uart_send(byte3);
       //timer_delay_ms(50);
       uart_send(byte4);
       //timer_delay_ms(50);
        
       uart_flush();
       timer_delay_ms(50);       
        }
    }
}

//Filter SPI for noise
int filter(int a) {
    //discard angles more than 360 degrees
    if (a > 360) return 0;
    //We found that 65 and 67 were noise numbers, so we just omit them all the time
    if (a  == 65 || a == 67) return 0;
    return 1;
}

void test_spi(void) {
    spi_init(SPI_CE0, 1024);

    unsigned int euler_x = 0; 
    unsigned int euler_y = 0;
    unsigned int euler_z = 0;

    while (1) {
        unsigned char tx[6] = { 0x41, 0x42, 0x43, 0x44, 0x45, 0x46 };
        unsigned char rx[6];
        spi_transfer(tx, rx, 6);


        unsigned int x = (rx[2] << 8) | rx[1];
        unsigned int y = (rx[4] << 8) | rx[3];
        unsigned int z = (rx[0] << 8) | rx[5];
    
        euler_x = (filter(x, euler_x)) ? x : euler_x;
        euler_y = (filter(y, euler_y)) ? y : euler_y;
        euler_z = (filter(z, euler_z)) ? z : euler_z; 

        printf("x: %d  y: %d  z: %d\n", euler_x, euler_y, euler_z);

        timer_delay_ms(50);
    }
}

/*
 * Called every iteration of the while loop in main.
 * Transmits control commands to plane, and handles
 * special cases like reversing the order of the 
 * throttle, aileron, and rudder, and sending flight control
 * mode.
 */
void handle_transmit(void) {
    for (int i = 1; i < NUM_CHANNELS; i++) {
        int curr_value = mcp3008_read(channels[i-1]);
        
        //Case: flight control button pressed: turn flight control on or off
        if (i == FLIGHT_MODE) {
            //Turn flight mode off if on, vice versa
            if (curr_value > 500 && flight_mode) {
                flight_mode = flight_mode == 1 ? 0 : 1;
                transmit(FLIGHT_MODE, 0);
                while (curr_value > 500) {
                    curr_value = mcp3008_read(i);
                }
            } 
        }
        
        //Case: joystick positions 
        if (abs(history[i-1] - curr_value) > SENSITIVITY) {
            //Reverse order of throttle, rudder, and aileron so pushing up sends larger value
            if (i == THROTTLE || i == RUDDER || i == AILERON) {
                curr_value = 1024 - curr_value;
            }
            transmit(i, curr_value);
            history[i-1] = curr_value;
            uart_flush();
            timer_delay_ms(50);
        }
    }   
}


//Main loop for the transmitter. Just loop over and over,
//checking peripheral values and sending information up to plane.
void main (void) {
    gpio_init();
    uart_init();
    mcp3008_init();

    timer_delay(3); 

    while (1) {
        handle_transmit();
        if (loop_iter == HEARTBEAT_PULSE) {
            //90 is the heartbeat code
            uart_send(90);
            loop_iter = 0;
        }
        loop_iter++;
    }
}
