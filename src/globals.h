#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <Arduino.h>

#define CLK_PIN 1
#define DT_PIN 0
#define SW_PIN A3

#define FADER_PIN A0
#define LEFT_PIN A1
#define RIGHT_PIN A2

#define NUMBER_OF_TRACKS 4
#define NUMBER_OF_PAGES 4
#define NUMBER_OF_STAGES 4

#define NUMBER_OF_SIDES 2

#define MAX_MENU_ROWS 23

extern uint8_t TRACK_PINS[NUMBER_OF_TRACKS];
extern uint8_t PAGE_PINS[NUMBER_OF_PAGES];

extern bool isMenuMode;
extern bool isSubMenuActive;
extern uint8_t pageIndex;
extern int16_t potValue;
extern int16_t faderValue;
extern int8_t menuSelectedRow;

extern long count;

enum menu_t : uint8_t {
  MENU_LOAD = 0,
  MENU_SAVE = 1,
  MENU_RESET = 2,
  MENU_AUTO_LOAD_SETTINGS = 3,
  MENU_FADER_THRESHOLD = 4,
  MENU_UI = 5,
  MENU_MIDI_CHANNEL = 6,
  MENU_A1_CC = 7,
  MENU_B1_CC = 8,
  MENU_C1_CC = 9,
  MENU_D1_CC = 10,
  MENU_A2_CC = 11,
  MENU_B2_CC = 12,
  MENU_C2_CC = 13,
  MENU_D2_CC = 14,
  MENU_A3_CC = 15,
  MENU_B3_CC = 16,
  MENU_C3_CC = 17,
  MENU_D3_CC = 18,
  MENU_A4_CC = 19,
  MENU_B4_CC = 20,
  MENU_C4_CC = 21,
  MENU_D4_CC = 22,
};

enum side_t : int8_t {
  SIDE_LEFT = 0,
  SIDE_RIGHT = 1,
};

struct Settings {
  uint8_t midiChannel;
  uint8_t faderThreshold;
  uint8_t stageIndexes[NUMBER_OF_SIDES][NUMBER_OF_PAGES][NUMBER_OF_TRACKS];
  uint8_t midiValues[NUMBER_OF_SIDES][NUMBER_OF_PAGES][NUMBER_OF_TRACKS][NUMBER_OF_STAGES];
  uint8_t ccValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS];
  bool autoLoadSettings;
  bool minimalUI;
};

extern Settings settings;

extern uint8_t midiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS];

extern uint8_t previousMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS];

#endif
