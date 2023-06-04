#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <globals.h>

void init_display();
void render_page_press();
void render_main();
void render_stage_change(int8_t trackIndex, side_t side);
void render_track_press(int8_t trackIndex);
void render_midi_value_change(int8_t trackIndex, side_t side);
void render_midi_values_swap(int8_t trackIndex);

void render_menu();
void render_loading();
void render_saving();

#endif