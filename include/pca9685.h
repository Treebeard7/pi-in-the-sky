#ifndef PCA9685_H
#define PCA9685_H

/*
 * Functions for using the pca9685 servo driver
 *
 * CS107e Fall Quarter Final Project
 * Pi in the Sky - Rohan Sanda and Matthew Tan
 * Author: https://github.com/lcthums/PCA9685
 * Modified by: 
 *      Matthew Tan <matthtan@stanford.edu>
 *      Rohan Sanda <rsanda@stanford.edu>
 * Sun Nov 21 2021
 */
#include "i2c.h"

#define PCA9685_MODE1 0x00
#define PCA9685_MODE2 0x01
#define PCA9685_LED0_ON_L 0x06
#define PCA9685_ADDRESS 0x40
#define PCA9685_ALLCALL 0x01
#define PCA9685_SLEEP 0x10
#define PCA9685_AI 0x20
#define PCA9685_OUTDRV 0x04
#define PCA9685_PRE_SCALE 0xFE
#define PCA9685_MAX_DUTY_CICLE 4095
#define PCA9685_REGISTERS_PER_CHANNEL 4

void PCA9685_init();
void PCA9685_setPWM(char channel, int value);//set a value between 0 and 1000us with 500 being roughly the middle STUDENT MODIFIED
void PCA9685_setFreq(unsigned short freq);
void PCA9685_stop();
#endif
