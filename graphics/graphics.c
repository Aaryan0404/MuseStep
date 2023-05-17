#include "assert.h"
#include "console.h"
#include "fb.h"
#include "gl.h"
#include "printf.h"
#include "strings.h"
#include "timer.h"
#include "uart.h"
#include "malloc.h"
#include "naj.h"
#include "interrupts.h"


#define NUM_MOTORS 8
#define MOTOR_OFF 0xff
#define FRAME_DURATION 10000

const int DEPTH = 4;
const int display_scaler = 10; 
const int width = 92; 
const int height = 100; 

const int key_height = 4 * display_scaler; 
const int key_width  = 1 * display_scaler; 

const int box_height = 1 * display_scaler;
const int box_width  = key_width;

const color_t colors[] = {GL_RED, GL_ORANGE, GL_YELLOW, GL_GREEN, GL_CYAN, GL_MAGENTA, GL_PURPLE, GL_SILVER};

int motor_notes[100][NUM_MOTORS];

void color_boxs(int *arr, int row) {
    for (int i = 0; i < 8; i++) {
        int index = arr[i]; 
        // color keys based on index
        if (index >= 0 && index <= 87 && index != MOTOR_OFF) {
            int box_x = index * display_scaler;
            int box_y = key_height + ((row - 1) * display_scaler);
            gl_draw_rect(box_x, box_y, box_width, box_height, colors[i]);
        }
    }
}

void color_piano() {
    int ten_counter = 0; 
    for (int r = 0; r < 3; r++) {
        gl_draw_rect(ten_counter, 0, key_width, key_height, (r % 2 == 0) ? GL_WHITE : GL_BLACK);
        ten_counter += key_width;  
    }
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 5; j++) {
            gl_draw_rect(ten_counter, 0, key_width, key_height, (j % 2 == 0) ? GL_WHITE : GL_BLACK); 
            ten_counter += key_width;  
        }
        for (int k = 0; k < 7; k++) {
            gl_draw_rect(ten_counter, 0, key_width, key_height, (k % 2 == 0) ? GL_WHITE : GL_BLACK); 
            ten_counter += key_width;  
        }
    }
    gl_draw_rect(ten_counter, 0, key_width, key_height, GL_WHITE);
    ten_counter += key_width; 
}

void color_keys(int *arr) {
    for (int i = 0; i < 8; i++) {
        int index = arr[i]; 
        // color keys based on index
        if (index >= 0 && index <= 87 && index != MOTOR_OFF) {
            int key_x = index * display_scaler; // x-cor of pressed key 
            int key_y = 0 * display_scaler;     // y-cor of pressed key

            // color/uncolor pressed key red depending on indicator
            gl_draw_rect(key_x, key_y, key_width, key_height, colors[i]);
        }
    }
}

static void handle_naj_byte(void){
    // Helper function to read and process a byte being received over naj
    static unsigned int bytes_received = 0;
    static unsigned char note_num;
    static unsigned char motor_num;

    unsigned char data = naj_read_byte();

    // Byte 0 - note number
    if (bytes_received == 0) {
        if (data != 0xee) {
            return; 
        }
        bytes_received++;
        return; 
    }

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
        if (motor_num < NUM_MOTORS) {
            motor_notes[0][motor_num] = note_num;
        }

        bytes_received = 0;
    }
}

void draw_frame(int time_step) { 
    // 2d array to keep track of pressed keys - max of 8 keys being played at once
    if (time_step == 0) {
        color_keys(&motor_notes[0][0]);
        return; 
    }
    else {
        color_keys(&motor_notes[0][0]);
        for (int i = 1; i <= (time_step + 1); i++) {
            color_boxs(&motor_notes[i][0], i);
        } 
    }
}

void main(void)
{
    interrupts_init();
    naj_init_read(); 
    interrupts_global_enable(); 
    uart_init();
    timer_init();
    printf("Executing main() in graphics.c\n");

    // Set all motors to not playing (OxFF)
    for (int r = 0; r < 100; r++) {
        for (int c = 0; c < NUM_MOTORS; c++) {
            motor_notes[r][c] = MOTOR_OFF;
        }
    }

    gl_init(width * display_scaler, height * display_scaler, FB_DOUBLEBUFFER);
    gl_clear(GL_BLUE);
    gl_swap_buffer();

    printf("Waiting for host...\n"); 
    while (1) {
        if (naj_has_data() && naj_read_byte() == 0x19) {
            break;
        }
    }
    printf("Host connected\n"); 

    int time_step = 0; 
    while (1) {
        unsigned int start = timer_get_ticks();

        gl_clear(GL_BLACK);
        color_piano();
        draw_frame(time_step);
        gl_swap_buffer();
        time_step++; 

        for (int r = 99; r >= 1; r--) {
            for (int c = 0; c < NUM_MOTORS; c++)
            {
                motor_notes[r][c] = motor_notes[r - 1][c];
            }
        }

        while (naj_has_data()) {
            handle_naj_byte();
        }        

        while (timer_get_ticks() - start < FRAME_DURATION) {}
    }

    printf("Completed main() in graphics.c\n");
    uart_putchar(EOT);
}
