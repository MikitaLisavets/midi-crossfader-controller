#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <Arduino.h>

#define VERSION 1.20

#define CLK_PIN 1
#define DT_PIN 0
#define SW_PIN 16

#define FADER_PIN 10
#define LEFT_PIN 4
#define RIGHT_PIN 15

#define ALL_TRACKS 16
#define NUMBER_OF_TRACKS 4
#define NUMBER_OF_PAGES 4
#define NUMBER_OF_STAGES 4

#define NUMBER_OF_SIDES 2

#define MAX_MENU_ROWS 36

#define CLICK_TIMEOUT 300

extern uint8_t TRACK_PINS[NUMBER_OF_TRACKS];
extern uint8_t PAGE_PINS[NUMBER_OF_PAGES];

extern bool isMenuMode;
extern bool isSubMenuActive;
extern uint8_t pageIndex;
extern int16_t potValue;
extern int16_t faderValue;
extern int8_t menuSelectedRow;
extern int8_t trackOffset;

enum menu_t : uint8_t {
  MENU_LOAD = 0,
  MENU_SAVE,
  MENU_RESET,
  MENU_AUTO_LOAD_SETTINGS,
  MENU_FADER_THRESHOLD,
  MENU_MIDI_CHANNEL,
  MENU_A1_CC,
  MENU_B1_CC,
  MENU_C1_CC,
  MENU_D1_CC,
  MENU_E1_CC,
  MENU_F1_CC,
  MENU_G1_CC,
  MENU_H1_CC,
  MENU_A2_CC,
  MENU_B2_CC,
  MENU_C2_CC,
  MENU_D2_CC,
  MENU_E2_CC,
  MENU_F2_CC,
  MENU_G2_CC,
  MENU_H2_CC,
  MENU_A3_CC,
  MENU_B3_CC,
  MENU_C3_CC,
  MENU_D3_CC,
  MENU_E3_CC,
  MENU_F3_CC,
  MENU_G3_CC,
  MENU_H3_CC,
  MENU_A4_CC,
  MENU_B4_CC,
  MENU_C4_CC,
  MENU_D4_CC,
  MENU_E4_CC,
  MENU_F4_CC,
  MENU_G4_CC,
  MENU_H4_CC
};

enum side_t : int8_t {
  SIDE_LEFT = 0,
  SIDE_RIGHT,
};

struct Settings {
  uint8_t midiChannel;
  uint8_t faderThreshold;
  uint8_t stageIndexes[NUMBER_OF_SIDES][NUMBER_OF_PAGES][ALL_TRACKS];
  uint8_t midiValues[NUMBER_OF_SIDES][NUMBER_OF_PAGES][ALL_TRACKS][NUMBER_OF_STAGES];
  uint8_t ccValues[NUMBER_OF_PAGES][ALL_TRACKS];
  bool autoLoadSettings;
};

extern Settings settings;

struct StateEvent {
  bool pageChanged;
  bool stageChanged;
  bool midiValuesChanged;
  bool midiValuesSwap;
  int8_t trackIndex;
  side_t side;
};

extern uint8_t midiValues[NUMBER_OF_PAGES][ALL_TRACKS];

extern uint8_t previousMidiValues[NUMBER_OF_PAGES][ALL_TRACKS];

#endif
