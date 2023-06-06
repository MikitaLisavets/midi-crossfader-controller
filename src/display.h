#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <globals.h>

void init_display();
void render_main(StateEvent stateEvent);

void render_menu();
void render_loading();
void render_saving();
void render_resetting();

#endif