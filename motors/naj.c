// This file implements the functions for the NAJ interface as defined in `naj.h`
#include "naj.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupts.h"
#include "interrupts.h"
#include "ringbuffer.h"
#include "timer.h"

#include "printf.h"

static const unsigned int data_pins[] = {
    NAJ_BIT0,
    NAJ_BIT1,
    NAJ_BIT2,
    NAJ_BIT3,
    NAJ_BIT4,
    NAJ_BIT5,
    NAJ_BIT6,
    NAJ_BIT7,
};

// Internal ringbuffer to store bytes as they arrive
static rb_t *data_ringbuffer;

static void handle_clock_pulse(unsigned int pc, void *aux_data) {
    // Interrupt handler to run on the rising edge of the clock pulse
    unsigned int data = 0;

    for (int i = 0; i < 8; i++) {
        data |= (gpio_read(data_pins[i]) << i);
    }

    rb_enqueue(data_ringbuffer, data);
    gpio_clear_event(NAJ_CLOCK);
}


// Initialize this device to read data
void naj_init_read(void) {
    // Set all data pins and clock to inputs
    gpio_set_input(NAJ_CLOCK);
    gpio_set_pulldown(NAJ_CLOCK);

    for (int i = 0; i < 8; i++) {
        gpio_set_input(data_pins[i]);
    }

    data_ringbuffer = rb_new();

    // Initialize interrupts on the clock pin
    // Globlal interrupts must have already been enabled my the mian PROGRAM
    gpio_enable_event_detection(NAJ_CLOCK, GPIO_DETECT_RISING_EDGE);

    gpio_interrupts_init();
    gpio_interrupts_register_handler(NAJ_CLOCK, handle_clock_pulse, NULL);
    gpio_interrupts_enable();
}

// Initialize this device to write data
void naj_init_write(void) {
    // Set all data pins and clock to inputs
    gpio_set_output(NAJ_CLOCK);
    gpio_write(NAJ_CLOCK, 0);

    for (int i = 0; i < 8; i++) {
        gpio_set_output(data_pins[i]);
        gpio_write(data_pins[i], 0);
    }
}

// Write one byte of data over the NAJ bus
void naj_write_byte(unsigned char data) {
    // Write bits to all pins
    for (int i = 0; i < 8; i++) {
        gpio_write(data_pins[i], data & (1 << i));
    }

    // Pulse clock
    gpio_write(NAJ_CLOCK, 1);
    timer_delay_us(NAJ_DELAY);
    gpio_write(NAJ_CLOCK, 0);
}

// To be used only in reading mode
// Return 1 if there is data in the internal ring buffer, 0 otherwise
unsigned char naj_has_data(void) {
    return !rb_empty(data_ringbuffer);
}

// To be used only in reading mode
// Return the most recent byte in the ringbuffer
unsigned char naj_read_byte(void) {
    int data;
    rb_dequeue(data_ringbuffer, &data);

    return data;
}
