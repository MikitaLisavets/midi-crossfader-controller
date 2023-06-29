#include <main.h>
#include <display.h>
#include <midi.h>
#include <utils.h>

// === Initialize variables ===

uint8_t TRACK_PINS[NUMBER_OF_TRACKS] = {5, 7, 9, A1};
uint8_t PAGE_PINS[NUMBER_OF_PAGES] = {6, 8, A2, A0};

bool isMenuMode = false;
bool isSubMenuActive = false;

uint8_t pageIndex = 0;
int16_t potValue;
int16_t faderValue = 0;
int8_t menuSelectedRow = 0;
int8_t trackOffset = 0;

Settings settings;

StateEvent stateEvent;

uint8_t midiValues[NUMBER_OF_PAGES][ALL_TRACKS] = {
  {0}
};

uint8_t previousMidiValues[NUMBER_OF_PAGES][ALL_TRACKS] = {
  {0}
};

// ============================

// === Handlers ===

void reset_settings_to_default() {
  settings = {
    .midiChannel = 0,
    .faderThreshold = 10,
    .scrollFastSpeed = 5,
    .stageIndexes = {
      {
        {0},
      },
      {
        {0},
      }
    },
    .midiValues = {
      {
        {{0}}
      },
      {
        {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
        {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
        {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
        {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
      }
    },
    .ccValues = {
      {1, 2, 3, 4, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
      {5, 6, 7, 8, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40},
      {9, 10, 11, 12, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52},
      {13, 14, 15, 16, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64}
    },
    .autoLoadSettings = false,
  };
}

void reset_state_to_default() {
  stateEvent = {
    .pageChanged = false,
    .stageChanged = false,
    .midiValuesChanged = false,
    .midiValuesSwap = false,
    .trackIndex = -1,
    .side = SIDE_LEFT
  };
}

void handle_page_press(uint8_t newPageIndex) {
  /*
    only PAGE button pressed:
      - change page index
  */

  pageIndex = newPageIndex;

  stateEvent.pageChanged = true;
}

void handle_stage_change(int8_t newStageIndex, int8_t trackIndex, side_t side) {
  int8_t indexWithOffset = (trackOffset + trackIndex) % ALL_TRACKS;

  if (is_button_pressed(trackIndex)) {
    /*
      PAGE button and LEFT/RIGHT button and TRACK button are pressed:
        - change left/right stage for track on current page
    */
    settings.stageIndexes[side][pageIndex][indexWithOffset] = newStageIndex;
  } else {
    /*
      PAGE button and LEFT/RIGHT button pressed:
        - change left/right stage for all tracks on current page
    */
    for (int8_t tIndex = 0; tIndex < ALL_TRACKS; tIndex++) {
      settings.stageIndexes[side][pageIndex][tIndex] = newStageIndex;
    }
  }

  stateEvent.stageChanged = true;
  stateEvent.trackIndex = indexWithOffset;
  stateEvent.side = side;
}

void handle_track_press(byte trackIndex) {
  /*
    TRACK button pressed:
      - send midi event to learn with current value
  */

  int8_t indexWithOffset = (trackOffset + trackIndex) % ALL_TRACKS;


  control_change(settings.midiChannel, settings.ccValues[pageIndex][indexWithOffset], midiValues[pageIndex][indexWithOffset]);
  send_midi();

  stateEvent.trackIndex = indexWithOffset;
}

void handle_midi_value_change(int8_t trackIndex, side_t side) {
  /*
    TRACK button and LEFT/RIGHT button pressed:
      listen to POT and change left/right midi value
  */
  int8_t speed = is_encoder_turned_fast() ? settings.scrollFastSpeed : 1;
  int8_t indexWithOffset = (trackOffset + trackIndex) % ALL_TRACKS;

  if (is_encoder_turned_right()) {
    potValue = safe_midi_value(settings.midiValues[side][pageIndex][indexWithOffset][settings.stageIndexes[side][pageIndex][indexWithOffset]] + speed);
    settings.midiValues[side][pageIndex][indexWithOffset][settings.stageIndexes[side][pageIndex][indexWithOffset]] = potValue;
  }
  if (is_encoder_turned_left()) {
    potValue = safe_midi_value(settings.midiValues[side][pageIndex][indexWithOffset][settings.stageIndexes[side][pageIndex][indexWithOffset]] - speed);
    settings.midiValues[side][pageIndex][indexWithOffset][settings.stageIndexes[side][pageIndex][indexWithOffset]] = potValue;
  }

  stateEvent.midiValuesChanged = true;
  stateEvent.trackIndex = indexWithOffset;
  stateEvent.side = side;
}

void handle_midi_values_swap(byte trackIndex) {
  /*
    TRACK button and LEFT or RIGHT button pressed and POT button are pressed:
      - swap left and right midi values
  */
  int8_t indexWithOffset = (trackOffset + trackIndex) % ALL_TRACKS;
  int8_t temp = settings.midiValues[SIDE_LEFT][pageIndex][indexWithOffset][settings.stageIndexes[SIDE_LEFT][pageIndex][indexWithOffset]];
  settings.midiValues[SIDE_LEFT][pageIndex][indexWithOffset][settings.stageIndexes[SIDE_LEFT][pageIndex][indexWithOffset]] = settings.midiValues[SIDE_RIGHT][pageIndex][indexWithOffset][settings.stageIndexes[SIDE_RIGHT][pageIndex][indexWithOffset]];
  settings.midiValues[SIDE_RIGHT][pageIndex][indexWithOffset][settings.stageIndexes[SIDE_RIGHT][pageIndex][indexWithOffset]] = temp;

  stateEvent.midiValuesSwap = true;
  stateEvent.trackIndex = indexWithOffset;
}

void handle_menu() {
  int8_t pIndex = get_pressed_page_button();
  int8_t tIndex = get_pressed_track_button();

  if (is_encoder_turned_right()) {
    if (isSubMenuActive) {
      if (menuSelectedRow == MENU_MIDI_CHANNEL) {
        settings.midiChannel = safe_midi_value(settings.midiChannel + 1);
      }
      if (menuSelectedRow == MENU_FADER_THRESHOLD) {
        settings.faderThreshold = safe_midi_value(settings.faderThreshold + 1);
      }
      if (menuSelectedRow == MENU_AUTO_LOAD_SETTINGS) {
        settings.autoLoadSettings = settings.autoLoadSettings ? false : true;
      }
      if (menuSelectedRow == MENU_SCROLL_FAST_SPEED) {
        settings.scrollFastSpeed = safe_midi_value(settings.scrollFastSpeed + 1);
      }
      if (menuSelectedRow == MENU_A1_CC) { increase_cc_value(0, 0); }
      if (menuSelectedRow == MENU_B1_CC) { increase_cc_value(0, 1); }
      if (menuSelectedRow == MENU_C1_CC) { increase_cc_value(0, 2); }
      if (menuSelectedRow == MENU_D1_CC) { increase_cc_value(0, 3); }
      if (menuSelectedRow == MENU_E1_CC) { increase_cc_value(0, 4); }
      if (menuSelectedRow == MENU_F1_CC) { increase_cc_value(0, 5); }
      if (menuSelectedRow == MENU_G1_CC) { increase_cc_value(0, 6); }
      if (menuSelectedRow == MENU_H1_CC) { increase_cc_value(0, 7); }
      if (menuSelectedRow == MENU_I1_CC) { increase_cc_value(0, 8); }
      if (menuSelectedRow == MENU_J1_CC) { increase_cc_value(0, 9); }
      if (menuSelectedRow == MENU_K1_CC) { increase_cc_value(0, 10); }
      if (menuSelectedRow == MENU_L1_CC) { increase_cc_value(0, 11); }
      if (menuSelectedRow == MENU_M1_CC) { increase_cc_value(0, 12); }
      if (menuSelectedRow == MENU_N1_CC) { increase_cc_value(0, 13); }
      if (menuSelectedRow == MENU_O1_CC) { increase_cc_value(0, 14); }
      if (menuSelectedRow == MENU_P1_CC) { increase_cc_value(0, 15); }

      if (menuSelectedRow == MENU_A2_CC) { increase_cc_value(1, 0); }
      if (menuSelectedRow == MENU_B2_CC) { increase_cc_value(1, 1); }
      if (menuSelectedRow == MENU_C2_CC) { increase_cc_value(1, 2); }
      if (menuSelectedRow == MENU_D2_CC) { increase_cc_value(1, 3); }
      if (menuSelectedRow == MENU_E2_CC) { increase_cc_value(1, 4); }
      if (menuSelectedRow == MENU_F2_CC) { increase_cc_value(1, 5); }
      if (menuSelectedRow == MENU_G2_CC) { increase_cc_value(1, 6); }
      if (menuSelectedRow == MENU_H2_CC) { increase_cc_value(1, 7); }
      if (menuSelectedRow == MENU_I2_CC) { increase_cc_value(1, 8); }
      if (menuSelectedRow == MENU_J2_CC) { increase_cc_value(1, 9); }
      if (menuSelectedRow == MENU_K2_CC) { increase_cc_value(1, 10); }
      if (menuSelectedRow == MENU_L2_CC) { increase_cc_value(1, 11); }
      if (menuSelectedRow == MENU_M2_CC) { increase_cc_value(1, 12); }
      if (menuSelectedRow == MENU_N2_CC) { increase_cc_value(1, 13); }
      if (menuSelectedRow == MENU_O2_CC) { increase_cc_value(1, 14); }
      if (menuSelectedRow == MENU_P2_CC) { increase_cc_value(1, 15); }

      if (menuSelectedRow == MENU_A3_CC) { increase_cc_value(2, 0); }
      if (menuSelectedRow == MENU_B3_CC) { increase_cc_value(2, 1); }
      if (menuSelectedRow == MENU_C3_CC) { increase_cc_value(2, 2); }
      if (menuSelectedRow == MENU_D3_CC) { increase_cc_value(2, 3); }
      if (menuSelectedRow == MENU_E3_CC) { increase_cc_value(2, 4); }
      if (menuSelectedRow == MENU_F3_CC) { increase_cc_value(2, 5); }
      if (menuSelectedRow == MENU_G3_CC) { increase_cc_value(2, 6); }
      if (menuSelectedRow == MENU_H3_CC) { increase_cc_value(2, 7); }
      if (menuSelectedRow == MENU_I3_CC) { increase_cc_value(2, 8); }
      if (menuSelectedRow == MENU_J3_CC) { increase_cc_value(2, 9); }
      if (menuSelectedRow == MENU_K3_CC) { increase_cc_value(2, 10); }
      if (menuSelectedRow == MENU_L3_CC) { increase_cc_value(2, 11); }
      if (menuSelectedRow == MENU_M3_CC) { increase_cc_value(2, 12); }
      if (menuSelectedRow == MENU_N3_CC) { increase_cc_value(2, 13); }
      if (menuSelectedRow == MENU_O3_CC) { increase_cc_value(2, 14); }
      if (menuSelectedRow == MENU_P3_CC) { increase_cc_value(2, 15); }

      if (menuSelectedRow == MENU_A4_CC) { increase_cc_value(3, 0); }
      if (menuSelectedRow == MENU_B4_CC) { increase_cc_value(3, 1); }
      if (menuSelectedRow == MENU_C4_CC) { increase_cc_value(3, 2); }
      if (menuSelectedRow == MENU_D4_CC) { increase_cc_value(3, 3); }
      if (menuSelectedRow == MENU_E4_CC) { increase_cc_value(3, 4); }
      if (menuSelectedRow == MENU_F4_CC) { increase_cc_value(3, 5); }
      if (menuSelectedRow == MENU_G4_CC) { increase_cc_value(3, 6); }
      if (menuSelectedRow == MENU_H4_CC) { increase_cc_value(3, 7); }
      if (menuSelectedRow == MENU_I4_CC) { increase_cc_value(3, 8); }
      if (menuSelectedRow == MENU_J4_CC) { increase_cc_value(3, 9); }
      if (menuSelectedRow == MENU_K4_CC) { increase_cc_value(3, 10); }
      if (menuSelectedRow == MENU_L4_CC) { increase_cc_value(3, 11); }
      if (menuSelectedRow == MENU_M4_CC) { increase_cc_value(3, 12); }
      if (menuSelectedRow == MENU_N4_CC) { increase_cc_value(3, 13); }
      if (menuSelectedRow == MENU_O4_CC) { increase_cc_value(3, 14); }
      if (menuSelectedRow == MENU_P4_CC) { increase_cc_value(3, 15); }
    } else {
      menuSelectedRow++;
      if (menuSelectedRow >= MAX_MENU_ROWS) {
        menuSelectedRow = 0;
      }
    }
  }

  if (is_encoder_turned_left()) {
    if (isSubMenuActive) {
      if (menuSelectedRow == MENU_MIDI_CHANNEL) {
        settings.midiChannel = safe_midi_value(settings.midiChannel - 1);
      }
      if (menuSelectedRow == MENU_FADER_THRESHOLD) {
        settings.faderThreshold = safe_midi_value(settings.faderThreshold - 1);
      }
      if (menuSelectedRow == MENU_AUTO_LOAD_SETTINGS) {
        settings.autoLoadSettings = settings.autoLoadSettings ? false : true;
      }
      if (menuSelectedRow == MENU_SCROLL_FAST_SPEED) {
        settings.scrollFastSpeed = safe_midi_value(settings.scrollFastSpeed - 1);
      }
      if (menuSelectedRow == MENU_A1_CC) { decrease_cc_value(0, 0); }
      if (menuSelectedRow == MENU_B1_CC) { decrease_cc_value(0, 1); }
      if (menuSelectedRow == MENU_C1_CC) { decrease_cc_value(0, 2); }
      if (menuSelectedRow == MENU_D1_CC) { decrease_cc_value(0, 3); }
      if (menuSelectedRow == MENU_E1_CC) { decrease_cc_value(0, 4); }
      if (menuSelectedRow == MENU_F1_CC) { decrease_cc_value(0, 5); }
      if (menuSelectedRow == MENU_G1_CC) { decrease_cc_value(0, 6); }
      if (menuSelectedRow == MENU_H1_CC) { decrease_cc_value(0, 7); }
      if (menuSelectedRow == MENU_I1_CC) { decrease_cc_value(0, 8); }
      if (menuSelectedRow == MENU_J1_CC) { decrease_cc_value(0, 9); }
      if (menuSelectedRow == MENU_K1_CC) { decrease_cc_value(0, 10); }
      if (menuSelectedRow == MENU_L1_CC) { decrease_cc_value(0, 11); }
      if (menuSelectedRow == MENU_M1_CC) { decrease_cc_value(0, 12); }
      if (menuSelectedRow == MENU_N1_CC) { decrease_cc_value(0, 13); }
      if (menuSelectedRow == MENU_O1_CC) { decrease_cc_value(0, 14); }
      if (menuSelectedRow == MENU_P1_CC) { decrease_cc_value(0, 15); }

      if (menuSelectedRow == MENU_A2_CC) { decrease_cc_value(1, 0); }
      if (menuSelectedRow == MENU_B2_CC) { decrease_cc_value(1, 1); }
      if (menuSelectedRow == MENU_C2_CC) { decrease_cc_value(1, 2); }
      if (menuSelectedRow == MENU_D2_CC) { decrease_cc_value(1, 3); }
      if (menuSelectedRow == MENU_E2_CC) { decrease_cc_value(1, 4); }
      if (menuSelectedRow == MENU_F2_CC) { decrease_cc_value(1, 5); }
      if (menuSelectedRow == MENU_G2_CC) { decrease_cc_value(1, 6); }
      if (menuSelectedRow == MENU_H2_CC) { decrease_cc_value(1, 7); }
      if (menuSelectedRow == MENU_I2_CC) { decrease_cc_value(1, 8); }
      if (menuSelectedRow == MENU_J2_CC) { decrease_cc_value(1, 9); }
      if (menuSelectedRow == MENU_K2_CC) { decrease_cc_value(1, 10); }
      if (menuSelectedRow == MENU_L2_CC) { decrease_cc_value(1, 11); }
      if (menuSelectedRow == MENU_M2_CC) { decrease_cc_value(1, 12); }
      if (menuSelectedRow == MENU_N2_CC) { decrease_cc_value(1, 13); }
      if (menuSelectedRow == MENU_O2_CC) { decrease_cc_value(1, 14); }
      if (menuSelectedRow == MENU_P2_CC) { decrease_cc_value(1, 15); }

      if (menuSelectedRow == MENU_A3_CC) { decrease_cc_value(2, 0); }
      if (menuSelectedRow == MENU_B3_CC) { decrease_cc_value(2, 1); }
      if (menuSelectedRow == MENU_C3_CC) { decrease_cc_value(2, 2); }
      if (menuSelectedRow == MENU_D3_CC) { decrease_cc_value(2, 3); }
      if (menuSelectedRow == MENU_E3_CC) { decrease_cc_value(2, 4); }
      if (menuSelectedRow == MENU_F3_CC) { decrease_cc_value(2, 5); }
      if (menuSelectedRow == MENU_G3_CC) { decrease_cc_value(2, 6); }
      if (menuSelectedRow == MENU_H3_CC) { decrease_cc_value(2, 7); }
      if (menuSelectedRow == MENU_I3_CC) { decrease_cc_value(2, 8); }
      if (menuSelectedRow == MENU_J3_CC) { decrease_cc_value(2, 9); }
      if (menuSelectedRow == MENU_K3_CC) { decrease_cc_value(2, 10); }
      if (menuSelectedRow == MENU_L3_CC) { decrease_cc_value(2, 11); }
      if (menuSelectedRow == MENU_M3_CC) { decrease_cc_value(2, 12); }
      if (menuSelectedRow == MENU_N3_CC) { decrease_cc_value(2, 13); }
      if (menuSelectedRow == MENU_O3_CC) { decrease_cc_value(2, 14); }
      if (menuSelectedRow == MENU_P3_CC) { decrease_cc_value(2, 15); }

      if (menuSelectedRow == MENU_A4_CC) { decrease_cc_value(3, 0); }
      if (menuSelectedRow == MENU_B4_CC) { decrease_cc_value(3, 1); }
      if (menuSelectedRow == MENU_C4_CC) { decrease_cc_value(3, 2); }
      if (menuSelectedRow == MENU_D4_CC) { decrease_cc_value(3, 3); }
      if (menuSelectedRow == MENU_E4_CC) { decrease_cc_value(3, 4); }
      if (menuSelectedRow == MENU_F4_CC) { decrease_cc_value(3, 5); }
      if (menuSelectedRow == MENU_G4_CC) { decrease_cc_value(3, 6); }
      if (menuSelectedRow == MENU_H4_CC) { decrease_cc_value(3, 7); }
      if (menuSelectedRow == MENU_I4_CC) { decrease_cc_value(3, 8); }
      if (menuSelectedRow == MENU_J4_CC) { decrease_cc_value(3, 9); }
      if (menuSelectedRow == MENU_K4_CC) { decrease_cc_value(3, 10); }
      if (menuSelectedRow == MENU_L4_CC) { decrease_cc_value(3, 11); }
      if (menuSelectedRow == MENU_M4_CC) { decrease_cc_value(3, 12); }
      if (menuSelectedRow == MENU_N4_CC) { decrease_cc_value(3, 13); }
      if (menuSelectedRow == MENU_O4_CC) { decrease_cc_value(3, 14); }
      if (menuSelectedRow == MENU_P4_CC) { decrease_cc_value(3, 15); }
    } else {
      menuSelectedRow--;
      if (menuSelectedRow < 0) {
        menuSelectedRow = MAX_MENU_ROWS - 1;
      }
    }
  }

  if (is_encoder_clicked() || is_right_button_pressed()) {
    if (menuSelectedRow == MENU_LOAD) {
      render_loading();
      load_settings(settings);
    } else if (menuSelectedRow == MENU_SAVE) {
      render_saving();
      save_settings(settings);
    } else if (menuSelectedRow == MENU_RESET) {
      render_resetting();
      reset_settings_to_default();
    } else if (isSubMenuActive) {
      isSubMenuActive = false;
      render_menu();
    } else {
      isSubMenuActive = true;
      render_menu();
    }
    delay(CLICK_TIMEOUT);
  }

  if (is_left_button_pressed()) {
    if (isSubMenuActive) {
      isSubMenuActive = false;
      render_menu();
      delay(CLICK_TIMEOUT);
    } else {
      isMenuMode = false;
      menuSelectedRow = 0;
    }
    return;
  }


  if (is_button_pressed(pIndex) || is_button_pressed(tIndex)) {
    isSubMenuActive = false;
    isMenuMode = false;
    menuSelectedRow = 0;
    return;
  }

  render_menu();
}

// ================

void setup() {
  // Turn off board leds
  pinMode(LED_BUILTIN_TX, INPUT);
  pinMode(LED_BUILTIN_RX, INPUT);

  // Initialize board pins
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);

  for (int8_t i = 0; i < NUMBER_OF_TRACKS; i++) {
    pinMode(TRACK_PINS[i], INPUT_PULLUP);
  }

  for (int8_t i = 0; i < NUMBER_OF_PAGES; i++) {
    pinMode(PAGE_PINS[i], INPUT_PULLUP);
  }

  init_display();
  init_encoder();

  load_settings(settings);
  if (!settings.autoLoadSettings) {
    reset_settings_to_default();
  }
}

void loop() {
  reset_state_to_default();

  encoder_tick();

  if (isMenuMode) {
    handle_menu();
    return;
  } else if (is_encoder_clicked()) {
    isMenuMode = true;
  }

  int8_t pIndex = get_pressed_page_button();
  int8_t tIndex = get_pressed_track_button();

  if (is_left_button_pressed() && is_right_button_pressed()) {
    if (is_button_pressed(tIndex)) {
      handle_midi_values_swap(tIndex);
    }
  } else if (is_left_button_pressed()) {
    if (is_button_pressed(tIndex) && is_button_pressed(pIndex)) {
      handle_stage_change(pIndex, tIndex, SIDE_LEFT);
    } else if (is_button_pressed(pIndex)) {
      handle_stage_change(pIndex, -1, SIDE_LEFT);
    } else if (is_button_pressed(tIndex)) {
      handle_midi_value_change(tIndex, SIDE_LEFT);
    }
  } else if (is_right_button_pressed()) {
    if (is_button_pressed(tIndex) && is_button_pressed(pIndex)) {
      handle_stage_change(pIndex, tIndex, SIDE_RIGHT);
    } else if (is_button_pressed(pIndex)) {
      handle_stage_change(pIndex, -1, SIDE_RIGHT);
    } else if (is_button_pressed(tIndex)) {
      handle_midi_value_change(tIndex, SIDE_RIGHT);
    }
  } else if (is_button_pressed(tIndex)) {
    handle_track_press(tIndex);
  } else if (is_button_pressed(pIndex)) {
    handle_page_press(pIndex);
  } else {
    if (is_encoder_turned_left()) {
      if (trackOffset - 4 < 0) {
        trackOffset = ALL_TRACKS - 4;
      } else {
        trackOffset = trackOffset - 4;
      }
    }
    if (is_encoder_turned_right()) {
      if (trackOffset + 4 >= ALL_TRACKS) {
        trackOffset = 0;
      } else {
        trackOffset = trackOffset + 4;
      }
    }
  }

  faderValue = analogRead(FADER_PIN);

  if (faderValue > 1023 - settings.faderThreshold) {
    faderValue = 1023 - settings.faderThreshold;
  } else if (faderValue < 0 + settings.faderThreshold) {
    faderValue = 0 + settings.faderThreshold;
  }

  for (int8_t trackIndex = 0; trackIndex < ALL_TRACKS; trackIndex++) {
    if (settings.midiValues[SIDE_RIGHT][pageIndex][trackIndex][settings.stageIndexes[SIDE_RIGHT][pageIndex][trackIndex]] < settings.midiValues[SIDE_LEFT][pageIndex][trackIndex][settings.stageIndexes[SIDE_LEFT][pageIndex][trackIndex]]) {
      midiValues[pageIndex][trackIndex] = map(faderValue, 1023 - settings.faderThreshold, 0 + settings.faderThreshold, settings.midiValues[SIDE_RIGHT][pageIndex][trackIndex][settings.stageIndexes[SIDE_RIGHT][pageIndex][trackIndex]], settings.midiValues[SIDE_LEFT][pageIndex][trackIndex][settings.stageIndexes[SIDE_LEFT][pageIndex][trackIndex]]);
    } else {
      midiValues[pageIndex][trackIndex] = map(faderValue, 0 + settings.faderThreshold, 1023 - settings.faderThreshold, settings.midiValues[SIDE_LEFT][pageIndex][trackIndex][settings.stageIndexes[SIDE_LEFT][pageIndex][trackIndex]], settings.midiValues[SIDE_RIGHT][pageIndex][trackIndex][settings.stageIndexes[SIDE_RIGHT][pageIndex][trackIndex]]);
    }

    if (previousMidiValues[pageIndex][trackIndex] != midiValues[pageIndex][trackIndex]) {
      control_change(settings.midiChannel, settings.ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
      previousMidiValues[pageIndex][trackIndex] = midiValues[pageIndex][trackIndex];
      send_midi();
    }
  }

  render_main(stateEvent);
}

