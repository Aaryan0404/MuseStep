#include "uart.h"
#include "printf.h"
#include "timer.h"
#include "gpio.h"
#include "notes.h"

#include "interrupts.h"

#include "naj.h"

#define NUM_MOTORS 8

// Constants defining the pitch to play each note, given in microseconds between each step
// Mirrors the namec constants (NOTE_C0 etc) defined in notes.h
// Indices in this array correspond to indices on a physical piano (A0 is index 0 and is the leftmost key)
const unsigned int note_delays[] = {
    36364, // A0
    34317, // A#0/Bb0
    32394, // B0
    30581, // C1
    28860, // C#1/Db1
    27241, // D1
    25714, // D#1/Eb1
    24272, // E1
    22910, // F1
    21622, // F#1/Gb1
    20408, // G1
    19264, // G#1/Ab1
    18182, // A1
    17161, // A#1/Bb1
    16197, // B1
    15288, // C2
    14430, // C#2/Db2
    13620, // D2
    12857, // D#2/Eb2
    12134, // E2
    11453, // F2
    10811, // F#2/Gb2
    10204, // G2
    9631, // G#2/Ab2
    9091, // A2
    8581, // A#2/Bb2
    8099, // B2
    7645, // C3
    3608, // 7216, // C#3/Db3
    3405, // 6811, // D3
    3214, // 6428, // D#3/Eb3
    6068, // E3
    5727, // F3
    5405, // F#3/Gb3
    2551, // 5102, // G3
    4816, // G#3/Ab3
    4545, // A3
    4290, // A#3/Bb3
    4050, // B3
    3822, // C4
    3608, // C#4/Db4
    3405, // D4
    3214, // D#4/Eb4
    3034, // E4
    2863, // F4
    2703, // F#4/Gb4
    2551, // G4
    2408, // G#4/Ab4
    2273, // A4
    2145, // A#4/Bb4
    2025, // B4
    1911, // C5
    1804, // C#5/Db5
    1703, // D5
    1607, // D#5/Eb5
    1517, // E5
    1432, // F5
    1351, // F#5/Gb5
    1276, // G5
    1204, // G#5/Ab5
    1136, // A5
    1073, // A#5/Bb5
    1012, // B5
    956, // C6
    902, // C#6/Db6
    851, // D6
    804, // D#6/Eb6
    758, // E6
    716, // F6
    676, // F#6/Gb6
    638, // G6
    602, // G#6/Ab6
    568, // A6
    536, // A#6/Bb6
    506, // B6
    478, // C7
    451, // C#7/Db7
    426, // D7
    402, // D#7/Eb7
    379, // E7
    358, // F7
    338, // F#7/Gb7
    319, // G7
    301, // G#7/Ab7
    284, // A7
    268, // A#7/Bb7
    253, // B7
    239, // C8
};

// Store step pins for each motor
const int step_pins[NUM_MOTORS] = {
    GPIO_PIN2,
    GPIO_PIN3,
    GPIO_PIN4,
    GPIO_PIN17,
    GPIO_PIN27,
    GPIO_PIN22,
    GPIO_PIN10,
    GPIO_PIN9
};


// Store the delay between pulses for each motor, and whether each motor is playing
unsigned int motor_delays[NUM_MOTORS] = {
    NOTE_C2,
    NOTE_E2,
    NOTE_G2,
    NOTE_C3,
    NOTE_E3,
    NOTE_G3,
    NOTE_C4,
    NOTE_E4
};    // Microseconds between each step pulse

unsigned int motor_active[NUM_MOTORS];    // 1 = playing, 0 = not playing

// Store the next step time for each motor (the value of `timer_get_ticks` when each motor next needs to step)
unsigned int next_step_times[NUM_MOTORS];

static void step_motor(unsigned int motor_num) {
    // Simple helper function to make the desired motor make a single step and update next step time
    // Pulses that motor's step pin high then low
    gpio_write(step_pins[motor_num], 1);
    gpio_write(step_pins[motor_num], 0);
}

static void handle_naj_byte(void){
    // Helper function to read and process a byte being received over naj
    static unsigned int bytes_received = 0;
    static unsigned char note_num;
    static unsigned char motor_num;

    unsigned char data = naj_read_byte();

    if (bytes_received == 0) {
        if (data != 0xee) return;

        bytes_received++;
        return;
    }

    // Byte 1 - note number
    if (bytes_received == 1) {
        printf("Note: %02x", data);
        note_num = data;
        bytes_received++;
        return;
    }

    // Byte 1 - data
    if (bytes_received == 2) {
        motor_num = data;
        printf("     motor: %02x\n", data);

        // Packet is now complete - update motor accordingly
        if (note_num == 0xff) {
            // Sentinel value 0xff = turn off specified motor
            motor_active[motor_num] = 0;
        } else {
            // Other values - activte motor and set to desired note delay
            //if (note_num <= 14) note_num += 12;
            //note_num += 12;

            motor_delays[motor_num] = note_delays[note_num];
            next_step_times[motor_num] = timer_get_ticks() + motor_delays[motor_num];
            motor_active[motor_num] = 1;
        }

        bytes_received = 0;
    }
}

void main(void)
{

    // Initialize interupts and naj module
    interrupts_init();
    gpio_init();

    naj_init_read();

    interrupts_global_enable();

    // Set all motor step pins to outputs
    // Also initialize all motors to not playing with the maximum possible delay
    for (int i = 0; i < NUM_MOTORS; i++) {
        gpio_set_output(step_pins[i]);

        motor_active[i] = 0;

    }

    // Wait for start signal from host
    printf("Waiting for host...");
    while (1) {
        if (naj_has_data()) {
            if (naj_read_byte() == 0x19) break;
        }
    }
    printf("Connected!\n");

    while (1) {
        // Read and process naj data that has been received
        if (naj_has_data()) {
            handle_naj_byte();
        }

        // Loop through all motors
        for (int i = 0; i < NUM_MOTORS; i++) {
            if (!motor_active[i]) continue; // Skip if this motor is not playing

            // If motor needs to step, do step and calculate next step time based on frequency
            unsigned int time = timer_get_ticks();
            if (time > next_step_times[i]) {
                next_step_times[i] = time + motor_delays[i];
                step_motor(i);
            }
        }
    }
}
