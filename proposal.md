## MuseStep (making music with stepper motors)

## Team members
* Aaryan Singhal
* Josh Delgadillo
* Niklas Vainio

## Project description
We want to make a music instrument that plays music using motors (in our case stepper motors). The device should be able to play live music from a MIDI keyboard, or receive pre-recorded MIDI files from a computer. We also want to build a music visualizer (similar to current tools such as *synthesia*) to show a graphical representation of the notes playing in real time.

**An example of stepper motor music** https://www.youtube.com/watch?v=ROpLjd9iQQU&t=65s&ab_channel=NSTB
**An example of music visualization with synthesia** https://www.youtube.com/watch?v=4y33h81phKU&ab_channel=PatrikPietschmann

As a stretch goal, we would like to incorporate a game aspect into the project. We could play a pre-recorded MIDI file, and the user would have to hit the keys according to the notes on the screen, with their inputs being played on the motors. The user could get a score based on how accurately they play the song, and we could also implement some way to track high scores. For clarify, we're choosing to name our base project goal "reactive mode" and our stretch project goal "instructive mode". We hope implementing the former will make the latter an easier task to accomplish and expect the primary challenges in doing so will be optimzing the pipeline through which MIDI-input is read and processed by our graphics library. 

The design of the project can be broken down into 3 main areas:

### 1) Stepper motor driver
The Raspberry pi must be able to drive the stepper motors (of which we plan to have 8) at specific speeds in order to create the desired sounds. The stepper motors will be controlled by A4988 driver chips, which have a "STEP" input that causes the motor to make a step every time it is pulsed. The Raspberry Pi will therefore have to pulse each chip's STEP pin at a specific frequency, which we will accomplish with a scheduling algorithm using the timer module.

Since driving the motors requires precise timing, we will have one Raspberry Pi dedicated to this purpose. A second Raspberry Pi will handle MIDI input and graphics (described below) and will talk to the first Pi using PS/2 (since we already coded fast read/write functions using interrupts!)

### 2) MIDI Input
The RaspberryPi will be configured with the ability to accept MIDI input from a piano keyboard. The key pressed will be read by a midi-parsing library through GPIO and then translated into a frequency on the step motor. The MIDI library will be capable of receiving input through both a physical midi device (piano keyboard) and a midi file on laptop through a USB-MIDI cable.

### 3) Music vislualizer
The secondary RaspberryPi will drive a graphical display, presenting notes as they are played via the MIDI keyboard. Key presses on the MIDI will be transmitted from the primary RaspberryPi to the secondary one through (current choice, althought may change) the ps2 protocol. Upon receiving info about the key press, the secondary RaspberryPi will generate colored rectangles (visually: constant width, length dependent on duration of key press) that will fall down from the top of the screen to the bottom. The display will be configured in SingleBuffer mode and pixels (at relevant x, y coordinates) will be erased/drawn to display colored rectangles falling. 

## Hardware, budget
#### Electronics
* Stepper motors (x8)
* Stepper motor driver boards (A4988) (x8)
* USB to MIDI cable
* Lab bench power supply (have already in lab)
* MIDI keyboard (have already in lab, Josh also has one)
* Monitor/Screen with HDMI input (have already in lab)

#### Mechanical supplies
* Some fixture (eg a piece of wood) to attach the motors to (should be able to make/obtain from lab64)
* M3 bolts to attach the motors (should be able to obtain from lab64)

The stepper motors, driver boards and USB to MIDI cable have already been ordered on Amazon (to arrive over the weekend/early next week). The total price of the order (including taxes and shipping) was $95.66 - we agreed that after being reimbursed $60, we would all contribute $12 to cover the remaining costs.

## Tasks, member responsibilities
Each team member should be responsible for a clearly identifiable part of the project.
1. Stepper Motor Controller - Niklas Vainio
2. MIDI Input/Library - Josh Delgadillo
3. Music visualizer - Aaryan Singhal
4. Integration - everyone

## Schedule, midpoint milestones
**Milestones for lab 3/15**
* Get the the motors spinning at specific frequencies (Niklas)
* Be able to receive data over MIDI (Josh)
* Get rectangles scrolling on screen - screen should have 88 keys (Aaryan)

## Resources needed, issues
We will use the MIDI code provided in the examples directory, as well as online resources for the motors and chips we are using. There are also examples of people online creating similar projects, which will be useful reference points for high-level design and could help us if we run into issues.

Issues we anticipate include being able to drive the motors at the correct frequencies to create pleasing sound, being able to anchor them sufficiently well to create good sound, and being able to correctly synchronize all the parts of the system (as everything requires precise timing). Failing to solve any of these issues would just lead to worse quality music, but we hope to still have a partially working end product even if we run into problems.
