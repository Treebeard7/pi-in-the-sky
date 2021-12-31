/*
 * Authors: Rohan Sanda --> rsanda@stanford.edu
 *         Matthew Tan --> matthtan@stanford.edu
 *
 * CS107E Autumn 2021
 * 
 * Credit to http://www.penguintutor.com/electronics/rpi-arduino-spi
 * for describing the SPI protocol.
 * 
 * This script uses the Adafruit libraries for reading the BNO055
 * data over I2C. I have implemented the SPI slave communication to
 * interface with the Raspberry Pi (master).
 * 
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55);
volatile byte Slavereceived;

//debug flag (prints sensor output to compare against received input on Pi side)
static boolean test = true;

//euler angles
static int orient_x = 0;
static int orient_y = 0;
static int orient_z = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin(0x8);
  bno.begin();
  bno.setExtCrystalUse(true);

  delay(1000);
  
  pinMode(MISO, OUTPUT);

  //Set Arduino to slave
  SPCR |= _BV(SPE);

  //Turn on SPI interrupts
  SPCR |= (1<<7);
}

//SPI Handler (triggered on falling edge)
ISR (SPI_STC_vect)
  {    
    while ((SPSR & (1 << SPIF)) != 0) { }
    
    Slavereceived = SPDR;
    unsigned int buf = 0;

    //start code is 0x41
    if (Slavereceived == 0x41) {
      buf = orient_x & 0xFF;
    } else if (Slavereceived == 0x42) {
      buf = (orient_x >> 8) & 0xFF;
    } else if (Slavereceived == 0x43) {
      buf = (orient_y) & 0xFF;
    } else if (Slavereceived == 0x44) {
      buf = (orient_y >> 8) & 0xFF;
    } else if (Slavereceived == 0x45) {
      buf = (orient_z) & 0xFF;
    } else if (Slavereceived == 0x46) {
      buf = (orient_z >> 8) & 0xFF;
    }

    SPDR = (byte) buf;    
  }

void loop() {
  sensors_event_t event; 
  bno.getEvent(&event);

  //add 180 and convert to int so we can easily send over SPI
  orient_x = (int) event.orientation.x;
  orient_y = ((int) event.orientation.y) + 180;
  orient_z = ((int) event.orientation.z) + 180;

  if (test) {
    Serial.print("X: ");
    Serial.print(orient_x);
    Serial.print("\tY: ");
    Serial.print(orient_y);
    Serial.print("\tZ: ");
    Serial.print(orient_z);
    Serial.println("");
    delay(100);
  }  
}


