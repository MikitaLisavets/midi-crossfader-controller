#ifndef __UTILS_H
#define __UTILS_H

#include <globals.h>

void load_settings(Settings &settings);
void save_settings(Settings &settings);

void init_encoder();
void encoder_tick();
bool is_encoder_turned_right();
bool is_encoder_turned_left();
bool is_encoder_turned_fast();
bool is_encoder_clicked();
bool is_encoder_hold();

bool is_left_button_pressed();
bool is_right_button_pressed();
bool is_track_button_pressed();
bool is_button_pressed(int8_t buttonIndex);
int8_t get_pressed_track_button();
int8_t get_pressed_page_button();

#endif