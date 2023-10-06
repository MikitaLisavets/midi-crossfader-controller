#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <Arduino.h>

#define VERSION 1.17

#define CLK_PIN 1
#define DT_PIN 0
#define SW_PIN 16

#define FADER_PIN 10
#define LEFT_PIN 4
#define RIGHT_PIN 15

#define NUMBER_OF_ALL_TRACKS 16
#define NUMBER_OF_TRACKS_ON_SCREEN 4
#define NUMBER_OF_PAGES 4
#define NUMBER_OF_VARIANTS 4
#define NUMBER_OF_SIDES 2

#define MAX_MENU_ROWS 7 + NUMBER_OF_ALL_TRACKS * NUMBER_OF_PAGES

#define CLICK_TIMEOUT 200

// #define PERFORMANCE_CHECK

extern uint8_t TRACK_PINS[NUMBER_OF_TRACKS_ON_SCREEN];
extern uint8_t PAGE_PINS[NUMBER_OF_PAGES];

extern bool isMenuActive;
extern bool isSubMenuActive;

extern uint8_t pageIndex;
extern int16_t potValue;
extern int16_t faderValue;
extern int8_t menuSelectedRow;
extern int8_t trackOffset;

enum menu_t : uint8_t {
  MENU_AUTO_LOAD_SETTINGS = 0,
  MENU_LOAD,
  MENU_SAVE,
  MENU_RESET,
  MENU_FADER_THRESHOLD,
  MENU_MIDI_CHANNEL,
  MENU_SCROLL_FAST_SPEED,
  MENU_CC,
};

enum side_t : int8_t {
  SIDE_LEFT = 0,
  SIDE_RIGHT,
};

struct Settings {
  uint8_t midiChannel;
  uint8_t faderThreshold;
  uint8_t scrollFastSpeed;
  uint8_t variantIndexes[NUMBER_OF_SIDES][NUMBER_OF_PAGES][NUMBER_OF_ALL_TRACKS];
  uint8_t midiValues[NUMBER_OF_SIDES][NUMBER_OF_PAGES][NUMBER_OF_ALL_TRACKS][NUMBER_OF_VARIANTS];
  uint8_t ccValues[NUMBER_OF_PAGES][NUMBER_OF_ALL_TRACKS];
  bool autoLoadSettings;
};

extern Settings settings;

struct StateEvent {
  bool pageChanged;
  bool variantChanged;
  bool midiValuesChanged;
  bool midiValuesSwap;
  int8_t trackIndex;
  side_t side;
};

extern uint8_t midiValues[NUMBER_OF_PAGES][NUMBER_OF_ALL_TRACKS];

extern uint8_t previousMidiValues[NUMBER_OF_PAGES][NUMBER_OF_ALL_TRACKS];

#endif
