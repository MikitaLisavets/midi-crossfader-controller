#ifndef __MIDI_H
#define __MIDI_H

#include <globals.h>

void control_change(uint8_t channel, uint8_t control, uint8_t value);
void send_midi();

#endif