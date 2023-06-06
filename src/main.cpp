#include <main.h>
#include <display.h>
#include <midi.h>
#include <utils.h>

// === Initialize variables ===

uint8_t TRACK_PINS[NUMBER_OF_TRACKS] = {4, 5, 6, 7};
uint8_t PAGE_PINS[NUMBER_OF_PAGES] = {8, 9, 10, 16};

bool isMenuMode = false;
bool isSubMenuActive = false;

uint8_t pageIndex = 0;
int16_t potValue;
int16_t faderValue = 0;
int8_t menuSelectedRow = 0;

Settings settings = {
  .midiChannel = 0,
  .faderThreshold = 10,
  .stageIndexes = {
    {
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    }
  },
  .midiValues = {
    {
      {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
    },
    {
      {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
      {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
      {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}},
      {{127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}, {127, 127, 127, 127}}
    }
  },
  .ccValues = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12},
    {13, 14, 15, 16}
  },
  .autoLoadSettings = false,
  .minimalUI = true
};

Settings defaultSettings = settings;

uint8_t midiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

uint8_t previousMidiValues[NUMBER_OF_PAGES][NUMBER_OF_TRACKS] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

// ============================

// === Handlers ===

void handle_page_press(uint8_t newPageIndex) {
  /*
    only PAGE button pressed:
      - change page index
  */

  pageIndex = newPageIndex;
  render_page_press();
}

void handle_stage_change(int8_t newStageIndex, int8_t trackIndex, side_t side) {
  if (is_button_pressed(trackIndex)) {
    /*
      PAGE button and LEFT/RIGHT button and TRACK button are pressed:
        - change left/right stage for track on current page
    */
    settings.stageIndexes[side][pageIndex][trackIndex] = newStageIndex;
  } else {
    /*
      PAGE button and LEFT/RIGHT button pressed:
        - change left/right stage for all tracks on current page
    */
    for (int tIndex = 0; tIndex < NUMBER_OF_TRACKS; tIndex++) {
      settings.stageIndexes[side][pageIndex][tIndex] = newStageIndex;
    }
  }

  render_stage_change(trackIndex, side);
}

void handle_track_press(byte trackIndex) {
  /*
    TRACK button pressed:
      - send midi event to learn with current value
  */

  control_change(settings.midiChannel, settings.ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
  send_midi();
  render_track_press(trackIndex);

  delay(100);
}

void handle_midi_value_change(int8_t trackIndex, side_t side) {
  /*
    TRACK button and LEFT/RIGHT button pressed:
      listen to POT and change left/right midi value
  */
  int8_t speed = is_encoder_turned_fast() ? 5 : 1;

  if (is_encoder_turned_right()) {
    potValue = safe_midi_value(settings.midiValues[side][pageIndex][trackIndex][settings.stageIndexes[side][pageIndex][trackIndex]] + speed);
    settings.midiValues[side][pageIndex][trackIndex][settings.stageIndexes[side][pageIndex][trackIndex]] = potValue;
  }
  if (is_encoder_turned_left()) {
    potValue = safe_midi_value(settings.midiValues[side][pageIndex][trackIndex][settings.stageIndexes[side][pageIndex][trackIndex]] - speed);
    settings.midiValues[side][pageIndex][trackIndex][settings.stageIndexes[side][pageIndex][trackIndex]] = potValue;
  }

  render_midi_value_change(trackIndex, side);
}

void handle_midi_values_swap(byte trackIndex) {
  /*
    TRACK button and LEFT or RIGHT button pressed and POT button are pressed:
      - swap left and right midi values
  */
  byte temp = settings.midiValues[SIDE_LEFT][pageIndex][trackIndex][settings.stageIndexes[SIDE_LEFT][pageIndex][trackIndex]];
  settings.midiValues[SIDE_LEFT][pageIndex][trackIndex][settings.stageIndexes[SIDE_LEFT][pageIndex][trackIndex]] = settings.midiValues[SIDE_RIGHT][pageIndex][trackIndex][settings.stageIndexes[SIDE_RIGHT][pageIndex][trackIndex]];
  settings.midiValues[SIDE_RIGHT][pageIndex][trackIndex][settings.stageIndexes[SIDE_RIGHT][pageIndex][trackIndex]] = temp;
  render_midi_values_swap(trackIndex);

  delay(300);
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
      if (menuSelectedRow == MENU_UI) {
        settings.minimalUI = settings.minimalUI ? false : true;
      }
      if (menuSelectedRow == MENU_A1_CC) {
        settings.ccValues[0][0] = safe_midi_value(settings.ccValues[0][0] + 1);
      }
      if (menuSelectedRow == MENU_B1_CC) {
        settings.ccValues[0][1] = safe_midi_value(settings.ccValues[0][1] + 1);
      }
      if (menuSelectedRow == MENU_C1_CC) {
        settings.ccValues[0][2] = safe_midi_value(settings.ccValues[0][2] + 1);
      }
      if (menuSelectedRow == MENU_D1_CC) {
        settings.ccValues[0][3] = safe_midi_value(settings.ccValues[0][3] + 1);
      }
      if (menuSelectedRow == MENU_A2_CC) {
        settings.ccValues[1][0] = safe_midi_value(settings.ccValues[1][0] + 1);
      }
      if (menuSelectedRow == MENU_B2_CC) {
        settings.ccValues[1][1] = safe_midi_value(settings.ccValues[1][1] + 1);
      }
      if (menuSelectedRow == MENU_C2_CC) {
        settings.ccValues[1][2] = safe_midi_value(settings.ccValues[1][2] + 1);
      }
      if (menuSelectedRow == MENU_D2_CC) {
        settings.ccValues[1][3] = safe_midi_value(settings.ccValues[1][3] + 1);
      }
      if (menuSelectedRow == MENU_A3_CC) {
        settings.ccValues[2][0] = safe_midi_value(settings.ccValues[2][0] + 1);
      }
      if (menuSelectedRow == MENU_B3_CC) {
        settings.ccValues[2][1] = safe_midi_value(settings.ccValues[2][1] + 1);
      }
      if (menuSelectedRow == MENU_C3_CC) {
        settings.ccValues[2][2] = safe_midi_value(settings.ccValues[2][2] + 1);
      }
      if (menuSelectedRow == MENU_D3_CC) {
        settings.ccValues[2][3] = safe_midi_value(settings.ccValues[2][3] + 1);
      }
      if (menuSelectedRow == MENU_A4_CC) {
        settings.ccValues[3][0] = safe_midi_value(settings.ccValues[3][0] + 1);
      }
      if (menuSelectedRow == MENU_B4_CC) {
        settings.ccValues[3][1] = safe_midi_value(settings.ccValues[3][1] + 1);
      }
      if (menuSelectedRow == MENU_C4_CC) {
        settings.ccValues[3][2] = safe_midi_value(settings.ccValues[3][2] + 1);
      }
      if (menuSelectedRow == MENU_D4_CC) {
        settings.ccValues[3][3] = safe_midi_value(settings.ccValues[3][3] + 1);
      }
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
      if (menuSelectedRow == MENU_UI) {
        settings.minimalUI = settings.minimalUI ? false : true;
      }
      if (menuSelectedRow == MENU_A1_CC) {
        settings.ccValues[0][0] = safe_midi_value(settings.ccValues[0][0] - 1);
      }
      if (menuSelectedRow == MENU_B1_CC) {
        settings.ccValues[0][1] = safe_midi_value(settings.ccValues[0][1] - 1);
      }
      if (menuSelectedRow == MENU_C1_CC) {
        settings.ccValues[0][2] = safe_midi_value(settings.ccValues[0][2] - 1);
      }
      if (menuSelectedRow == MENU_D1_CC) {
        settings.ccValues[0][3] = safe_midi_value(settings.ccValues[0][3] - 1);
      }
      if (menuSelectedRow == MENU_A2_CC) {
        settings.ccValues[1][0] = safe_midi_value(settings.ccValues[1][0] - 1);
      }
      if (menuSelectedRow == MENU_B2_CC) {
        settings.ccValues[1][1] = safe_midi_value(settings.ccValues[1][1] - 1);
      }
      if (menuSelectedRow == MENU_C2_CC) {
        settings.ccValues[1][2] = safe_midi_value(settings.ccValues[1][2] - 1);
      }
      if (menuSelectedRow == MENU_D2_CC) {
        settings.ccValues[1][3] = safe_midi_value(settings.ccValues[1][3] - 1);
      }
      if (menuSelectedRow == MENU_A3_CC) {
        settings.ccValues[2][0] = safe_midi_value(settings.ccValues[2][0] - 1);
      }
      if (menuSelectedRow == MENU_B3_CC) {
        settings.ccValues[2][1] = safe_midi_value(settings.ccValues[2][1] - 1);
      }
      if (menuSelectedRow == MENU_C3_CC) {
        settings.ccValues[2][2] = safe_midi_value(settings.ccValues[2][2] - 1);
      }
      if (menuSelectedRow == MENU_D3_CC) {
        settings.ccValues[2][3] = safe_midi_value(settings.ccValues[2][3] - 1);
      }
      if (menuSelectedRow == MENU_A4_CC) {
        settings.ccValues[3][0] = safe_midi_value(settings.ccValues[3][0] - 1);
      }
      if (menuSelectedRow == MENU_B4_CC) {
        settings.ccValues[3][1] = safe_midi_value(settings.ccValues[3][1] - 1);
      }
      if (menuSelectedRow == MENU_C4_CC) {
        settings.ccValues[3][2] = safe_midi_value(settings.ccValues[3][2] - 1);
      }
      if (menuSelectedRow == MENU_D4_CC) {
        settings.ccValues[3][3] = safe_midi_value(settings.ccValues[3][3] - 1);
      }

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
  }  else if (menuSelectedRow == MENU_RESET) {
      render_resetting();
      settings = defaultSettings;
    } else if (isSubMenuActive) {
      isSubMenuActive = false;
      render_menu();
    } else {
      isSubMenuActive = true;
      render_menu();
    }
    delay(300);
  }

  if (is_left_button_pressed()) {
    if (isSubMenuActive) {
      isSubMenuActive = false;
      render_menu();
      delay(300);
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

  for (int i = 0; i < NUMBER_OF_TRACKS; i++) {
    pinMode(TRACK_PINS[i], INPUT_PULLUP);
  }

  for (int i = 0; i < NUMBER_OF_PAGES; i++) {
    pinMode(PAGE_PINS[i], INPUT_PULLUP);
  }

  Settings initSettings;
  load_settings(initSettings);
  if (initSettings.autoLoadSettings) {
    settings = initSettings;
  }

  init_display();
  init_encoder();
}

void loop() {
  encoder_tick();

  if (!isMenuMode && is_encoder_clicked()) {
    isMenuMode = true;
  }

  int8_t pIndex = get_pressed_page_button();
  int8_t tIndex = get_pressed_track_button();

  if (isMenuMode) {
    handle_menu();
    return;
  }

  if (is_left_button_pressed() && is_right_button_pressed()) {
    if (is_button_pressed(tIndex)) {
      handle_midi_values_swap(tIndex);
      return;
    }
  } else if (is_left_button_pressed()) {
    if (is_button_pressed(tIndex) && is_button_pressed(pIndex)) {
      handle_stage_change(pIndex, tIndex, SIDE_LEFT);
      return;
    } else if (is_button_pressed(pIndex)) {
      handle_stage_change(pIndex, -1, SIDE_LEFT);
      return;
    } else if (is_button_pressed(tIndex)) {
      handle_midi_value_change(tIndex, SIDE_LEFT);
      return;
    }
  } else if (is_right_button_pressed()) {
    if (is_button_pressed(tIndex) && is_button_pressed(pIndex)) {
      handle_stage_change(pIndex, tIndex, SIDE_RIGHT);
      return;
    } else if (is_button_pressed(pIndex)) {
      handle_stage_change(pIndex, -1, SIDE_RIGHT);
      return;
    } else if (is_button_pressed(tIndex)) {
      handle_midi_value_change(tIndex, SIDE_RIGHT);
      return;
    }
  } else if (is_button_pressed(tIndex)) {
    handle_track_press(tIndex);
    return;
  } else if (is_button_pressed(pIndex)) {
    handle_page_press(pIndex);
    return;
  }

  faderValue = analogRead(FADER_PIN);

  if (faderValue > 1023 - settings.faderThreshold) {
    faderValue = 1023 - settings.faderThreshold;
  } else if (faderValue < 0 + settings.faderThreshold) {
    faderValue = 0 + settings.faderThreshold;
  }

  for (int trackIndex = 0; trackIndex < NUMBER_OF_TRACKS; trackIndex++) {
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

  render_main();
}

