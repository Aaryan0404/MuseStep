// This file defines the methods for the 'NAJ' procol (NiklasJoshAaryan)
// This is a really simple interface for communicating between two pis
// using 8 data bits in parallel and one clock line

// The host sets all 8 data bits and then pulses the clock pin

#ifndef _NAJ_H
#define _NAJ_H

#include "gpio.h"

// Defines how long to hold the pulse clock pin for when sending a byte
#define NAJ_DELAY 10

// Constants to define which pin
#define NAJ_CLOCK GPIO_PIN23

// Least significant bit first
#define NAJ_BIT0 GPIO_PIN24
#define NAJ_BIT1 GPIO_PIN25
#define NAJ_BIT2 GPIO_PIN8
#define NAJ_BIT3 GPIO_PIN7
#define NAJ_BIT4 GPIO_PIN12
#define NAJ_BIT5 GPIO_PIN16
#define NAJ_BIT6 GPIO_PIN20
#define NAJ_BIT7 GPIO_PIN21

// Initialize this device to read data
void naj_init_read(void);

// Initialize this device to write data
void naj_init_write(void);

// Write one byte of data over the NAJ bus
void naj_write_byte(unsigned char data);

// Read one byte of data over the NAJ bus
unsigned char naj_read_byte(void);

// To be used in reading mode
// Returns 1 if there is data in the internal ring buffer, 0 otherwise
unsigned char naj_has_data(void);


#endif
