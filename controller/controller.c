#include "uart.h"
#include "printf.h"
#include "midi.h"
#include "naj.h"
#include "timer.h"
#include "interrupts.h"
#include "ringbuffer.h"

#define MOTOR_NUM 8
#define MIDI_MODE 0 // 0 = live, 1 = file

static unsigned char motor_array[MOTOR_NUM];

static void print_motor_state() {
    printf("Motors: ");
    for(int i = 0; i < MOTOR_NUM; i++) {
        printf("%d ", motor_array[i]);
    }
    printf("\n");
}

void main(void)
{
    interrupts_init();
    uart_init();
    midi_init(motor_array, MOTOR_NUM, MIDI_MODE);
    naj_init_write();

    naj_write_byte(0x19);

    interrupts_global_enable();

    while(1) {
        struct midi_event_t event = midi_read_event();
        midi_update_motors(event, motor_array, MOTOR_NUM);
        print_motor_state();
    }

    uart_putchar(EOT);
}
