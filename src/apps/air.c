#include "printf.h"
#include "uart.h"
#include "interrupts.h"
#include "pca9685.h"
#include "timer.h"
#include "protocol.h"
#include "uart_extras.h"
#include "assert.h"
#include "gpio.h"
#include "gpio_interrupts.h"
#include "gpio_extra.h"
#include "spi.h"

#define CONTROL_MAX_CHAN 4 //number of servos (protocol dictates that all servo channels are first in line from type = 0)
#define HEARTBEAT_THRESHOLD 2000 //number of control loop ierations (roughly takes DT) before failsafe

//If the last collected sensor value (for I and D terms) is too long ago (like you just switched into an autopilot mode)
#define MAX_DT 500000 //this is in us.
#define DT 1000//this is in us. (1/DT * 10^6) = pid frequency (eg: 1/1000 * 10^6 = 1000hz pid loop) VERY ROUGHLY (naive delay)

//PID contstants
#define PITCH_K -.001
#define PITCH_P 10000
#define PITCH_I 0
#define PITCH_D 0

#define ROLL_K -.001
#define ROLL_P 10000
#define ROLL_I 0
#define ROLL_D 0

//Rudder aileron naive mixer for stabilized mode
#define RUDDER_MIX_K -.5

//Hardware RC channel defs
#define THROTTLE_CHAN 0
#define AILERON_CHAN 1
#define ELEVATOR_CHAN 2
#define RUDDER_CHAN 3

//Hardware RC trim defs
#define THROTTLE_TRIM 0
#define AILERON_TRIM 0
#define ELEVATOR_TRIM 110
#define RUDDER_TRIM 5

static int flight_paired = 0;//initial contact state

//SPI protocol for communicatin with arduino. tx contains codes to getting the right byte from Arduino.
static unsigned char tx[6] = { 0x41, 0x42, 0x43, 0x44, 0x45, 0x46 };
static unsigned char rx[6];

//pitch roll and yaw values from imu
static int euler_x = 0;//magnetic heading
static int euler_y = 0;//roll
static int euler_z = 0;//pitch

//method promises for later use
void setControls(int control, int value);
void get_angles(void);

//initialize static flight management variables
static int flightMode = MANUAL;
static int armed = 0;
static int pitchSetpoint = 0;
static int rollSetpoint = 0;

void setControls(int control, int value){
    if(control < 0 || control > CONTROL_MAX_CHAN){
        return;
    }
    switch(control){
        case THROTTLE:
            PCA9685_setPWM(THROTTLE_CHAN, value + THROTTLE_TRIM);
            break;
        case AILERON:
            PCA9685_setPWM(AILERON_CHAN, value + AILERON_TRIM);
            break;
        case ELEVATOR:
            PCA9685_setPWM(ELEVATOR_CHAN, value + ELEVATOR_TRIM);
            break;
        case RUDDER:
            PCA9685_setPWM(RUDDER_CHAN, value + RUDDER_TRIM);
            break;
    }
}

static unsigned int heartbeat_counter = 0;
static unsigned int value;
static unsigned int state = 0; // 0 indicates listening for type. 1 indicates set servo, make enum later
static unsigned int type = 0;  // see protocol

void commandHandler(unsigned int pc, void *aux_data) {
    
    char rx = uart_recv();
    gpio_clear_event(GPIO_PIN15);
    
    // ground side is written to never send 0x90 hearbeat pulse during command transmission, so state should be 0
    if (rx == 90 && state == 0) {
        heartbeat_counter = 0;
        return;
    }

    if (rx == 0 && state == 0) {//initial contact synchronization message
        if (!flight_paired) {
            flight_paired = 1;
        }
        state = 1;
        return;
    }

    if (state == 1) {//second contact get control type (control surface, flight mode)
        if (rx >= 1 && rx <=CONTROL_MAX_CHAN) {
            type = rx;
            state = 2;
            return;
        }
        else if (rx == FLIGHT_MODE) {
            flightMode = flightMode == 1 ? 0 : 1;
        }
        else {
            state = 0;
            return;
        }
    }

    if (state == 2) {//third contact get least significant bit of channel value
        value = rx;
        state = 3;
        return;
    }

    if (state == 3) {//fourth contact get most significant bit of channel value
        value |= (rx << 8);
        state = 0;
        heartbeat_counter = 0;//reset failsafe countdown, as we've recieved a good command
        setControls(type, value + 1000);
        return;
    }

}

int abs(int a) {
    if (a < 0) return -1*a;
    return a;
}

int filter(int a, int b) {//filters out magic numbers from spi protocol
    if (a > 360) return 0;
    if (a == 65 || a == 67) return 0;
    return 1;
}

void get_angles(void) {//gets the angles from the imu
    spi_transfer(tx, rx, 6);
    
    unsigned int x = (rx[2] << 8) | rx[1];
    unsigned int y = (rx[4] << 8) | rx[3];
    unsigned int z = (rx[0] << 8) | rx[5];

    euler_x = ((filter(x, euler_x)) ? x : euler_x);
    euler_y = (int)((filter(y, euler_y)) ? y : euler_y) - 180;
    euler_z = (int)((filter(z, euler_z)) ? z : euler_z);
    euler_z = euler_z > 180 ? euler_z - 360 : euler_z;//perform transformation for pilot reference frame
}


void main(void)
{
    //setup hardware
    uart_init();
    i2c_init();
    spi_init(SPI_CE0, 1024);//init spi for arduino
    PCA9685_stop();//reset pca
    PCA9685_setFreq(55);//standard servo frequency
    PCA9685_init();
    timer_init();
    gpio_init();
    
    printf("Hello SKY! I'm the air unit.\n");
    setControls(THROTTLE, 1000);//initialize the throttle esc

    //setup interrupts
    interrupts_init();
    gpio_interrupts_init();
    gpio_interrupts_enable();
    gpio_enable_event_detection(GPIO_PIN15, GPIO_DETECT_FALLING_EDGE);
    gpio_interrupts_register_handler(GPIO_PIN15, commandHandler, NULL);  
    interrupts_global_enable();

    //setup pid
    unsigned int time = timer_get_ticks();//this is in us
    unsigned int dt = 0;
    int pitchError = 0;
    int rollError = 0;
    int integratedPitchError = 0;
    int integratedRollError = 0;
    int lastPitchError = 0;
    int lastRollError = 0;

    //setup control inputs (from GCS)
    int elevatorCommand = 0;
    int aileronCommand = 0;
    flightMode = MANUAL;
    flight_paired = 0;

    while(1){
        if (heartbeat_counter > HEARTBEAT_THRESHOLD) {//if we haven't received a heartbeat in a while (rx failsafe)
            //flight termination sequence
            setControls(THROTTLE,1000);
            PCA9685_stop();
            interrupts_global_disable();
            uart_putchar(EOT);
            break;
        }
        if (flight_paired){//wait for initial contact
            heartbeat_counter++;//and start the heartbeat counter for rx faile
        }

        if(flightMode == STABILIZED){
            get_angles();

            dt = timer_get_ticks() - time;//(current time - last time) returns in us
            time += dt;//update current time

            if(dt > MAX_DT){//if the last time is too long ago (can lead to actuator saturation), grab a fresh dt time step
                timer_delay_us(DT);
                dt = timer_get_ticks() - time;//(current time - last time) returns in us
                time += dt;//update current time
            }

            //read from sensor
            pitchError = euler_z - pitchSetpoint;
            rollError = euler_y - rollSetpoint;

            //calculate integral
            integratedPitchError += pitchError;
            integratedRollError += rollError;

            //calculate actuator output with PID (Kp*e + Ki*sumError*dt + Kd*deltaError/dt)
            elevatorCommand = (PITCH_K*PITCH_P*pitchError) + (PITCH_K*PITCH_I*integratedPitchError*dt) + (PITCH_K*PITCH_D*(pitchError-lastPitchError)/dt);
            aileronCommand = (ROLL_K*ROLL_P*rollError) + (ROLL_K*ROLL_I*integratedRollError*dt) + (ROLL_K*ROLL_D*(rollError-lastRollError)/dt);

            //set the actuators according to calculated value
            setControls(ELEVATOR, elevatorCommand+1500);
            setControls(AILERON, aileronCommand+1500);
            setControls(RUDDER, RUDDER_MIX_K*aileronCommand+1500);

            //update last errors with current errors for next calculation
            lastPitchError = pitchError;
            lastRollError = rollError;
        }
        //wait for dt
        timer_delay_us(DT);
    }
}
