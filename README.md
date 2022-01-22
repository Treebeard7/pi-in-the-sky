# Pi in the Sky

## Project title

**Raspberry Pi in the Sky** 

A self-stabalizing RC plane ✈️

## Team members

Matthew Tan and Rohan Sanda

## Project description

Pi in the Sky is a fully functional RC plane built from scratch. It consists of two flight modes: manual and stabalized.
Manual mode allows for low-latency, long-range (up to 20 km) flight, while stabalized mode enables PID-based stabalization
controls. We categorize the various components of our project into two distinct systems: the ground unit and air unit.

_Ground Unit_ 🕹️

Our transmitter contains code to read joystick and button values and transmit control information to the air unit. We use an
MCP3008 ADC to read analog joystick values and transmit them to the ground Pi over a SPI bus. We developed a synchronizing UART
based protocol to reliably transmit information to the air unit. The transmitter also emits heartbeat pulses to ensure connectivity with
the air unit.

_Air Unit_ 🎈

The air unit receives control commands from the ground unit using UART interrupts (implemented using GPIO interrupts on the RX pin).
Servo commands are relayed to the PCA9685 PWM servo control board over I2C. The manual and PID control systems are defined and
tuned. Additionally, a fail-safe system is written to shut off controls if heartbeat pulses are not received from transmitter. We
use SPI and I2C protocols to read fused euler angles from a BNO055 IMU by first routing the data through an Arduino Nano. These
values are used to as inputs to the PID loop.

## References

We used the following external libraries in our project:

- SPI.H written by Omar Rizwan
- MCP3008.H written by Chris Gregg
- I2C.H
- ADAFRUIT_SENSOR.H
- ADAFRUIT_BNO055.H
- UTILITY/IMUMATHS.H
- PCA9685.H written by github user lcthums (student modified)

## Self-evaluation

Overall, we delivered on primary goals for this project: to build and test fly a manually controlled RC plane. In addition, we
accomplished many of our bonus objectives like establishing a PID control loop by reading IMU data. This progress would not have
been possible without strong collaboration and hours of time spent debugging mission-critical bugs. Here are a few listed below:

- Finding a low-latency, long-range communication system: Before settling on the RFD900 radios, we tried using WiFi with ESP32s
  and even a developed a LPF to read PWM signals sent directly from a normal RC transmitter. While we got both methods to work,
  we went with the RFD900s due to their low-latency and higher range resolution.
- Powering the air system: Powering the numerous air-side modules: the raspberry pi, servo driver and servos, and RFD900, arduino nano, and BNO055 required a large amount of power that our existing ESC's (Electronic speed controller) BEC (battery elimination circuit - a linear step down converter to 5v) was unable to power the aircraft without browning out. We had to use a second BEC and a second battery to ensure the system had adequte power margin to sustain flight if every single servo moved at once.
- Getting sensor-fused IMU data: We were unable to connect the BNO055 directly to the Pi via I2C since the Pi's I2C hardware does not
  support clock stretching. We had to include an Arduino Nano intermediary, and write a SPI protocol for communicating between the Pi and Nano.

## Photos

Check out our photos in the "images" folder. They are from our test flight! We took a video of the flight, but took some screenshots of the video to put up on Github. Contact us if you want more footage! 
