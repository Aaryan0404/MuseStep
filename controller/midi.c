#include "midi.h"
#include "gpio.h"
#include "printf.h"
#include "timer.h"
#include "naj.h"
#include "gpio_interrupts.h"
#include "gpio_extra.h"
#include "uart.h"

#define MIDI_PIN GPIO_PIN4
#define BAUD 31250
#define DELAY (1000000 / BAUD)

static void midi_read_byte_handler(unsigned int pc, void *aux_data);

static rb_t *midi_seq_queue;
static unsigned int midi_mode;

void midi_init(unsigned char* motor_array, unsigned int size, unsigned int mode) {
    gpio_set_input(MIDI_PIN);
    gpio_set_pullup(MIDI_PIN);

    // Turn all motors "off"
    for(unsigned int i = 0; i < size; i++) {
        motor_array[i] = MIDI_MOTOR_OFF;
    }

    midi_seq_queue = rb_new();

    gpio_interrupts_init();
    gpio_enable_event_detection(MIDI_PIN, GPIO_DETECT_FALLING_EDGE);
    gpio_interrupts_register_handler(MIDI_PIN, midi_read_byte_handler, midi_seq_queue);
    gpio_interrupts_enable();

    midi_mode = mode;

    printf("Midi initialized!\n");
}

void midi_read_byte_handler(unsigned int pc, void *aux_data) {
    // ! This probably prevents a race condition... but it works!
    timer_delay_us(1);

    unsigned int seq = 0;
    for(int i = 0; i < 10; i++) {
        int val = gpio_read(MIDI_PIN);
        if(i != 0 && i != 9) {
            seq |= (val << (i - 1));
        }
        if(i < 9) timer_delay_us(DELAY); // Skip delay for last bit to not mess up timing
    }

    rb_t *rb = (rb_t*) aux_data;
    if(seq != MIDI_STATUS_ON) rb_enqueue(rb, seq);

    gpio_clear_event(MIDI_PIN);
}

struct midi_event_t midi_read_event(void) {

    struct midi_event_t event;
    int bytes_recieved = 0;

    while(1) {
        // Wait until there is data
        if(rb_empty(midi_seq_queue)) continue;

        // Recieve one byte at a time
        int data;
        rb_dequeue(midi_seq_queue, &data);
        printf("%x\n", data);
        bytes_recieved++;

        // Assign data to correct field in struct
        if(bytes_recieved == 1) {
            unsigned int action = ((unsigned int) data) >> 4;
            unsigned int channel = ((unsigned int) data) & 0xF;

            if(action == MIDI_NOTE_ON || action == MIDI_NOTE_OFF) {
                event.action = action;
            } else {
                // Skip event if trying to read file
                if(midi_mode == 1) {
                        bytes_recieved = 0;
                        continue;
                }

                event.action = MIDI_ACTION_OTHER; // ! This is questionable
            }

            event.channel = channel;
        } else if(bytes_recieved == 2) {
            if(event.action == MIDI_NOTE_ON || event.action == MIDI_NOTE_OFF) {
                event.key = data & 0x7F;
            } else {
                event.key = MIDI_MOTOR_OFF; // ! Also questionable
            }
        } else if(bytes_recieved == 3) {
            if(event.action == MIDI_NOTE_ON || event.action == MIDI_NOTE_OFF) {
                event.velocity = data & 0x7F;
            } else {
                event.velocity = MIDI_MOTOR_OFF; // ! Watch out
            }
            return event;
        }
    }
}

void midi_update_motors(struct midi_event_t event, unsigned char* motor_array, unsigned int size) {

    if(midi_mode == 0) {
        // Note on: Search array for empty motor
        if(event.action == MIDI_NOTE_ON && event.velocity > 0) {
            for(unsigned char i = 0; i < size; i++) {
                if(motor_array[i] == MIDI_MOTOR_OFF) {
                    // Replace note
                    motor_array[i] = event.key;

                    // Send updated state to motors: Key (piano indexed), Motor
                    naj_write_byte(NAJ_START_PACKET);
                    naj_write_byte(event.key - MIDI_PIANO_OFFSET);
                    naj_write_byte(i);
                    break;
                }
            }
        } else if(event.action == MIDI_NOTE_OFF || (event.action == MIDI_NOTE_ON && event.velocity == 0)) {
            // Note off: Search array for given note
            for(unsigned char i = 0; i < size; i++) {
                if(motor_array[i] == event.key) {
                    // Replace note, send udpated state through NAJ protocol, abort
                    motor_array[i] = MIDI_MOTOR_OFF;

                    // Send updated state to motors: OFF (0xFF), Motor
                    naj_write_byte(NAJ_START_PACKET);
                    naj_write_byte(MIDI_MOTOR_OFF);
                    naj_write_byte(i);
                    break;
                }
            }
        }
    } else {
        // Map channel to motor (default)
        // int motors_to_channel[] = {0, 1, 2, 3, 4, 5, 6, 7};

        // Megalovaia (Simple)
        int motors_to_channel[] = {0, 0, 0, 0, 1, 1, 1, 1};

        // Megalovaia (Complicated)
        // int motors_to_channel[] = {0, 0, 2, 2, 6, 6, 1, 3};

        // Never Gonna Give You Up
        // int motors_to_channel[] = {0, 1, 3, 3, 3, 4, 4, 4};

        // Wii Channel Theme
        // int motors_to_channel[] = {0, 0, 0, 0, 1, 1, 1, 1};

        if(event.action == MIDI_NOTE_ON && event.channel < 8) {
            for(int i = 0; i < 8; i++) {
                if(motors_to_channel[i] == event.channel) {
                    motor_array[i] = event.key;
                    naj_write_byte(NAJ_START_PACKET);
                    naj_write_byte(event.key - MIDI_PIANO_OFFSET);
                    naj_write_byte(i);
                }
            }
            
        } else if(event.action == MIDI_NOTE_OFF && event.channel < 8) {
            for(int i = 0; i < 8; i++) {
                if(motors_to_channel[i] == event.channel) {
                    motor_array[i] = event.key;
                    naj_write_byte(NAJ_START_PACKET);
                    naj_write_byte(MIDI_MOTOR_OFF);
                    naj_write_byte(i);
                }
            }
        }
    }

    // TODO Can probably unify Naj protocol code
}
