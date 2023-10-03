#include <main.h>
#include <display.h>
#include <midi.h>
#include <utils.h>

// === Initialize variables ===

uint8_t TRACK_PINS[NUMBER_OF_TRACKS] = {5, 7, 9, A1};
uint8_t PAGE_PINS[NUMBER_OF_PAGES] = {6, 8, A2, A0};

bool isMenuMode = false;
bool isSubMenuActive = false;

bool shouldScreenUpdate = false;

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
    .variantIndexes = {{{0}}, {{0}}},
    .midiValues = {{{{0}}}, {{{0}}}},
    .ccValues = {{0}},
    .autoLoadSettings = false,
  };

  for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++) {
      for (uint8_t j = 0; j < ALL_TRACKS; j++) {
          for (uint8_t k = 0; k < NUMBER_OF_VARIANTS; k++) {
              settings.midiValues[1][i][j][k] = 127;
          }
      }
  }

  uint8_t count = 0;
  for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++) {
    for (uint8_t j = 0; j < ALL_TRACKS; j++) {
      settings.ccValues[i][j] = count;
      count++;
    }
  }
}

void reset_state_to_default() {
  stateEvent = {
    .pageChanged = false,
    .variantChanged = false,
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

void handle_variant_change(int8_t newvariantIndex, int8_t trackIndex, side_t side) {
  int8_t indexWithOffset = (trackOffset + trackIndex) % ALL_TRACKS;

  if (is_button_pressed(trackIndex)) {
    /*
      PAGE button and LEFT/RIGHT button and TRACK button are pressed:
        - change left/right variant for track on current page
    */
    settings.variantIndexes[side][pageIndex][indexWithOffset] = newvariantIndex;
    stateEvent.trackIndex = indexWithOffset;
  } else {
    /*
      PAGE button and LEFT/RIGHT button pressed:
        - change left/right variant for all tracks on current page
    */
    for (int8_t tIndex = 0; tIndex < ALL_TRACKS; tIndex++) {
      settings.variantIndexes[side][pageIndex][tIndex] = newvariantIndex;
    }
    stateEvent.trackIndex = -1;
  }

  stateEvent.variantChanged = true;
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
    potValue = safe_midi_value(settings.midiValues[side][pageIndex][indexWithOffset][settings.variantIndexes[side][pageIndex][indexWithOffset]] + speed);
    settings.midiValues[side][pageIndex][indexWithOffset][settings.variantIndexes[side][pageIndex][indexWithOffset]] = potValue;
  }
  if (is_encoder_turned_left()) {
    potValue = safe_midi_value(settings.midiValues[side][pageIndex][indexWithOffset][settings.variantIndexes[side][pageIndex][indexWithOffset]] - speed);
    settings.midiValues[side][pageIndex][indexWithOffset][settings.variantIndexes[side][pageIndex][indexWithOffset]] = potValue;
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
  int8_t temp = settings.midiValues[SIDE_LEFT][pageIndex][indexWithOffset][settings.variantIndexes[SIDE_LEFT][pageIndex][indexWithOffset]];
  settings.midiValues[SIDE_LEFT][pageIndex][indexWithOffset][settings.variantIndexes[SIDE_LEFT][pageIndex][indexWithOffset]] = settings.midiValues[SIDE_RIGHT][pageIndex][indexWithOffset][settings.variantIndexes[SIDE_RIGHT][pageIndex][indexWithOffset]];
  settings.midiValues[SIDE_RIGHT][pageIndex][indexWithOffset][settings.variantIndexes[SIDE_RIGHT][pageIndex][indexWithOffset]] = temp;

  stateEvent.midiValuesSwap = true;
  stateEvent.trackIndex = indexWithOffset;
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
  reset_state_to_default();

  render_main(stateEvent);
}

void loop_menu() {
  bool wasAction = false;

  int8_t pIndex = get_pressed_page_button();
  int8_t tIndex = get_pressed_track_button();

  if (is_encoder_turned_right()) {
    wasAction = true;
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


      uint8_t count = MENU_CC;
      for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++) {
        for (uint8_t j = 0; j < ALL_TRACKS; j++) {
          if (menuSelectedRow == count) {
            increase_cc_value(i, j);
          }
          count++;
        }
      }
    } else {
      menuSelectedRow++;
      if (menuSelectedRow >= MAX_MENU_ROWS) {
        menuSelectedRow = 0;
      }
    }
  }

  if (is_encoder_turned_left()) {
    wasAction = true;
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

      uint8_t count = MENU_CC;
      for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++) {
        for (uint8_t j = 0; j < ALL_TRACKS; j++) {
          if (menuSelectedRow == count) {
            decrease_cc_value(i, j);
          }
          count++;
        }
      }
    } else {
      menuSelectedRow--;
      if (menuSelectedRow < 0) {
        menuSelectedRow = MAX_MENU_ROWS - 1;
      }
    }
  }

  if (is_encoder_clicked() || is_right_button_pressed()) {
    wasAction = true;
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
      shouldScreenUpdate = true;
    }
    return;
  }


  if (is_button_pressed(pIndex) || is_button_pressed(tIndex)) {
    shouldScreenUpdate = true;
    isSubMenuActive = false;
    isMenuMode = false;
    menuSelectedRow = 0;
    return;
  }

  if (shouldScreenUpdate) {
    render_menu();
  }

  shouldScreenUpdate = wasAction;
}

void loop_main() {
  if (is_encoder_clicked()) {
    isMenuMode = true;
    shouldScreenUpdate = true;
    return;
  }

  bool wasAction = false;

  int8_t pIndex = get_pressed_page_button();
  int8_t tIndex = get_pressed_track_button();

  if (is_left_button_pressed() && is_right_button_pressed()) {
    if (is_button_pressed(tIndex)) {
      handle_midi_values_swap(tIndex);
    }
  } else if (is_left_button_pressed()) {
    if (is_button_pressed(tIndex) && is_button_pressed(pIndex)) {
      wasAction = true;
      handle_variant_change(pIndex, tIndex, SIDE_LEFT);
    } else if (is_button_pressed(pIndex)) {
      wasAction = true;
      handle_variant_change(pIndex, -1, SIDE_LEFT);
    } else if (is_button_pressed(tIndex)) {
      wasAction = true;
      handle_midi_value_change(tIndex, SIDE_LEFT);
    }
  } else if (is_right_button_pressed()) {
    if (is_button_pressed(tIndex) && is_button_pressed(pIndex)) {
      wasAction = true;
      handle_variant_change(pIndex, tIndex, SIDE_RIGHT);
    } else if (is_button_pressed(pIndex)) {
      wasAction = true;
      handle_variant_change(pIndex, -1, SIDE_RIGHT);
    } else if (is_button_pressed(tIndex)) {
      wasAction = true;
      handle_midi_value_change(tIndex, SIDE_RIGHT);
    }
  } else if (is_button_pressed(tIndex)) {
    wasAction = true;
    handle_track_press(tIndex);
  } else if (is_button_pressed(pIndex)) {
    wasAction = true;
    handle_page_press(pIndex);
  } else {
    if (is_encoder_turned_left()) {
      wasAction = true;
      if (trackOffset - 4 < 0) {
        trackOffset = ALL_TRACKS - 4;
      } else {
        trackOffset = trackOffset - 4;
      }
    }
    if (is_encoder_turned_right()) {
      wasAction = true;
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
    if (settings.midiValues[SIDE_RIGHT][pageIndex][trackIndex][settings.variantIndexes[SIDE_RIGHT][pageIndex][trackIndex]] < settings.midiValues[SIDE_LEFT][pageIndex][trackIndex][settings.variantIndexes[SIDE_LEFT][pageIndex][trackIndex]]) {
      midiValues[pageIndex][trackIndex] = map(faderValue, 1023 - settings.faderThreshold, 0 + settings.faderThreshold, settings.midiValues[SIDE_RIGHT][pageIndex][trackIndex][settings.variantIndexes[SIDE_RIGHT][pageIndex][trackIndex]], settings.midiValues[SIDE_LEFT][pageIndex][trackIndex][settings.variantIndexes[SIDE_LEFT][pageIndex][trackIndex]]);
    } else {
      midiValues[pageIndex][trackIndex] = map(faderValue, 0 + settings.faderThreshold, 1023 - settings.faderThreshold, settings.midiValues[SIDE_LEFT][pageIndex][trackIndex][settings.variantIndexes[SIDE_LEFT][pageIndex][trackIndex]], settings.midiValues[SIDE_RIGHT][pageIndex][trackIndex][settings.variantIndexes[SIDE_RIGHT][pageIndex][trackIndex]]);
    }

    if (previousMidiValues[pageIndex][trackIndex] != midiValues[pageIndex][trackIndex]) {
      control_change(settings.midiChannel, settings.ccValues[pageIndex][trackIndex], midiValues[pageIndex][trackIndex]);
      previousMidiValues[pageIndex][trackIndex] = midiValues[pageIndex][trackIndex];
      send_midi();
    }
  }

  if (shouldScreenUpdate) {
    render_main(stateEvent);
  }

  shouldScreenUpdate = wasAction;
}

void loop() {
  reset_state_to_default();
  encoder_tick();

  if (isMenuMode) {
    loop_menu();
  } else {
    loop_main();
  }
}

