/*
 * Functions for using the pca9685 servo driver
 *
 * CS107e Fall Quarter Final Project
 * Pi in the Sky - Rohan Sanda and Matthew Tan
 * Author: https://github.com/lcthums/PCA9685
 * Modified by: 
 *      Matthew Tan <matthtan@stanford.edu> 
 *      Rohan Sanda <rsanda@stanford.edu>
 * 
 * Sun Nov 21 2021
 * 
 * Statement of student work:
 * Modified methods labled "//STUDENT MODIFIED" as well as all i2c write commands for use with cs107e library
 */

#include "i2c.h"
#include "pca9685.h"

//STUDENT MODIFIED	
#define PWM_MIN 900
#define PWM_MAX 2100

//STUDENT ADDED
void i2c_reg_write(unsigned addr, int reg, int data) {
	char buffer[] = {reg, data};
	i2c_write(addr, buffer, 2);
}


void PCA9685_init() {
	i2c_reg_write(PCA9685_ADDRESS, PCA9685_MODE1, PCA9685_ALLCALL | PCA9685_AI);
    i2c_reg_write(PCA9685_ADDRESS, PCA9685_MODE2, PCA9685_OUTDRV);
}

//STUDENT MODIFIED
void PCA9685_setPWM(char channel, int value) {
	value = value < PWM_MIN ? PWM_MIN:
			value > PWM_MAX ? PWM_MAX:
			value;

	value = (int)((value * .229f)-6.21f);//rough linear scaling to get 900-2100us

	char buf[5];
	buf[0] = PCA9685_LED0_ON_L + (PCA9685_REGISTERS_PER_CHANNEL * channel);
	buf[1] = buf[2] = 0x00;
	buf[3] = value & 0xFF; buf[4] = (value >> 8) & 0xF;

	i2c_write(PCA9685_ADDRESS, buf, 5);
}

//STUDENT MODIFIED
void PCA9685_setFreq(unsigned short freq) {
	freq = freq < 24? 0xFF:
			freq > 1526? 0x03:
			(int)(25000000.0f / (4096 * freq) - 0.5f);

	i2c_reg_write(PCA9685_ADDRESS, PCA9685_PRE_SCALE, freq);
}

void PCA9685_stop() {
	i2c_reg_write(PCA9685_ADDRESS, PCA9685_MODE1, PCA9685_SLEEP);
}
