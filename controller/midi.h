#ifndef _MIDI_H

#include "ringbuffer.h"

/* Definitions for motor tracking */
#define MIDI_MOTOR_OFF 0xFF
#define NAJ_START_PACKET 0xEE

/* Definitions for MIDI messages */
#define MIDI_STATUS_ON 0xFE // Status indicator sent by modern MIDI devices at a regular to show line is still valid
#define MIDI_NOTE_ON 0x9 // 0b1001cccc
#define MIDI_NOTE_OFF 0x8 // 0b1000cccc
#define MIDI_ACTION_OTHER 0
#define MIDI_PIANO_OFFSET 21 // Offset maps Midi key indices to piano key indices

/* Enum for recognized midi actions */
// enum midi_actions_t { MIDI_ACTION_ON, MIDI_ACTION_OFF, MIDI_ACTION_OTHER };

/* Struct defining form of a "midi event" */
struct midi_event_t {
    unsigned int action;
    unsigned int channel;
    unsigned int key;
    unsigned int velocity;
};

/* Initializes midi module (for consistency) */
void midi_init(unsigned char* motor_array, unsigned int size, unsigned int mode);

/**
 * Compiles several midi sequences into an event type.
 * Blocking: Waits until falling edge detected before returning valid event (will try again if 0xFE or invalid event)
**/
struct midi_event_t midi_read_event(void);

/* Takes a midi event input and changes the state of the motors appropriately */
void midi_update_motors(struct midi_event_t event, unsigned char* motor_array, unsigned int size);

#endif
