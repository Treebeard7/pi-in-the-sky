#ifndef PROTOCOL_H
#define PROTOCOL_H

/*
 * Implementation of protocol.md
 *
 * CS107e Fall Quarter Final Project
 * Pi in the Sky - Rohan Sanda and Matthew Tan
 * Authors: Rohan Sanda <rsanda@stanford.edu> 
 * and Matthew Tan <matthtan@stanford.edu> for CS107e Aut 21
 * Thu Nov 25 2021
 */

//MESSAGE TYPES
#define THROTTLE 1
#define AILERON 2
#define ELEVATOR 3
#define RUDDER 4
#define BOMB_DROP 5
#define FLIGHT_MODE 6
#define FLIGHT_TERMINATION 7
#define HEARTBEAT 90

//FLIGHT MODES
#define MANUAL 0
#define STABILIZED 1

/*
 * Initializes the protocol library. Needs to only
 * be called by both the air and ground Pi's.
 */
void protocol_init();

/*
 * Transmit one message to other Pi
 * Return 1 if successful, 0 otherwise
 * 1 message = 1 byte (type) + 1 byte (message)
 */
int transmit(int type, int message);

#endif
