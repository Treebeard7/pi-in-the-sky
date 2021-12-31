## Project title

Raspberry Pi in the Sky

## Team members

Rohan Sanda
Matthew Tan

## Project description

Short description of the basic idea and end goal you are aiming for.
Include stretch goals and/or fallback position to indicate how you
might fine-tune the project scope if needed.

2 raspberry pis are used to remote control a model airplane.

One pi serves as a ground unit, with 2, 2-axis analog joystics, an ADC on SPI, and wifi module.
The other pi serves as an air unit, with the PCA9685 I2C driven servo controller and wifi module.

Phase 1: RC control system - Create a 3-4 channel communication protocol via wifi with one pi acting as a ground control station and the other acting as an onboard flight controller.

Phase 2: Implement P(I)(D) control loop for basic stabilization functionality - Create a basic stability program that maintains a commanded flight attitude.

Phase 3: Implement basic autonomous flight control scripting - Create a basic autonomous flight profile (time based) that maintains altitude and flies commanded attitudes. Eg: Fly level for 10 seconds, turn right 90 degrees, x4

Extensions:

- Keychain camera (press button, take picture, store on SD card)
- Servo bomb (press button, drop payload)

## Hardware, budget

Itemize what hardware, if any, you need and your plan for acquiring it.
Rough budget for the project. Remember we will reimburse you up to $20 per person.

- Two joysticks (order) https://www.adafruit.com/product/512 x2 (11.90)
- Wifi module (order) x2 (22.00)
- Two raspberry pis
- Model airplane borrowed from SUAVE
  Total (33.90)

## Major tasks, member responsibilities

Major task breakdown and who is responsible for each task.

Matthew: Air unit - implement i2c library for the PCA9685, IMU communication, and autopilot functionality.

Rohan: Ground unit - implement wifi module library, ADC and analog joystick input, and autopilot scripting.

## Schedule, midpoint milestones

What are the one-week goals you plan to meet for next week?

- Phase 1 over Thanksgiving Break
- Phase 2 (toughest) Week 1 and 2
- Phase 3 remainder of time

## Resources needed, issues

Are there books/code/tools/materials that you need access to?
What are the possible risks to overcome? What are you concerned about?
How can we help you succeed?

If we get to autopilot functionality (the coolest part of this whole thing), and the chip does NOT have hardware sensor fusion... well, can we get an IMU that has onboard sensor fusion?

If the wifi range is too limited or too high latency to control the plane (under manual mode), implementing a uart based telemetry radio system.
